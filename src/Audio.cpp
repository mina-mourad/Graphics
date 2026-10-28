#include "Audio.h"

#define MA_NO_DECODING
#define MA_NO_ENCODING
#define MA_NO_GENERATION
#define MA_NO_ENGINE
#define MA_NO_RESOURCE_MANAGER
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#include <algorithm>
#include <cmath>
#include <mutex>
#include <vector>

namespace
{
    constexpr int   SR = 44100;
    constexpr float PI = 3.14159265358979f;
    using Buf = std::vector<float>;

    // ---------------------------------------------------------------- helpers
    unsigned noiseState = 22222u;
    float Noise() { noiseState = noiseState * 1664525u + 1013904223u; return static_cast<float>((noiseState >> 8) & 0xFFFF) / 32768.0f - 1.0f; }
    float Freq(float semitonesFromA4) { return 440.0f * std::pow(2.0f, semitonesFromA4 / 12.0f); }
    Buf Make(float seconds) { return Buf(static_cast<size_t>(seconds * SR), 0.0f); }
    float Saw(float ph) { return 2.0f * (ph - std::floor(ph + 0.5f)); }

    // Adds a decaying plucked tone (a few harmonics, like an oud) into `b`.
    void Pluck(Buf& b, float start, float hz, float len, float vol, float decay = 5.0f)
    {
        size_t s0 = static_cast<size_t>(start * SR), n = static_cast<size_t>(len * SR);
        for (size_t i = 0; i < n && s0 + i < b.size(); ++i) {
            float t = static_cast<float>(i) / SR;
            float env = std::exp(-decay * t) * std::min(1.0f, t * 400.0f);
            float ph = 2.0f * PI * hz * t;
            float v = std::sin(ph) + 0.5f * std::sin(2 * ph) + 0.25f * std::sin(3 * ph) + 0.12f * std::sin(4 * ph);
            b[s0 + i] += v * env * vol * 0.55f;
        }
    }
    // Soft flute-like lead with vibrato.
    void Flute(Buf& b, float start, float hz, float len, float vol)
    {
        size_t s0 = static_cast<size_t>(start * SR), n = static_cast<size_t>(len * SR);
        for (size_t i = 0; i < n && s0 + i < b.size(); ++i) {
            float t = static_cast<float>(i) / SR;
            float env = std::min(1.0f, t * 12.0f) * std::min(1.0f, (len - t) * 8.0f);
            float vib = 1.0f + 0.006f * std::sin(2 * PI * 5.5f * t);
            float ph = 2.0f * PI * hz * vib * t;
            float v = std::sin(ph) + 0.18f * std::sin(2 * ph) + 0.04f * Noise();
            b[s0 + i] += v * env * vol;
        }
    }
    void Drum(Buf& b, float start, float vol, float pitch = 1.0f)
    {
        size_t s0 = static_cast<size_t>(start * SR), n = static_cast<size_t>(0.35f * SR);
        float ph = 0.0f;
        for (size_t i = 0; i < n && s0 + i < b.size(); ++i) {
            float t = static_cast<float>(i) / SR;
            float hz = (55.0f + 90.0f * std::exp(-30.0f * t)) * pitch;
            ph += 2.0f * PI * hz / SR;
            float v = std::sin(ph) * std::exp(-9.0f * t) + 0.25f * Noise() * std::exp(-45.0f * t);
            b[s0 + i] += v * vol;
        }
    }
    void Drone(Buf& b, float hz, float vol)
    {
        for (size_t i = 0; i < b.size(); ++i) {
            float t = static_cast<float>(i) / SR;
            float total = static_cast<float>(b.size()) / SR;
            float swell = 0.75f + 0.25f * std::sin(2 * PI * t / total * 2.0f);   // loops cleanly
            b[i] += (std::sin(2 * PI * hz * t) + 0.4f * std::sin(2 * PI * hz * 1.5f * t) + 0.2f * std::sin(2 * PI * hz * 2.0f * t)) * vol * swell;
        }
    }
    void Normalize(Buf& b, float peak)
    {
        float m = 0.0f; for (float v : b) m = std::max(m, std::fabs(v));
        if (m > 0.0f) for (float& v : b) v *= peak / m;
    }
    void FadeEnds(Buf& b, float sec)
    {
        size_t n = std::min(static_cast<size_t>(sec * SR), b.size() / 2);
        for (size_t i = 0; i < n; ++i) { float k = static_cast<float>(i) / n; b[i] *= k; b[b.size() - 1 - i] *= k; }
    }

    // Phrygian dominant scale on D (semitones from A4): D Eb F# G A Bb C D
    const float SCALE[8] = { -7, -6, -3, -2, 0, 1, 3, 5 };

    // ------------------------------------------------------------------ SFX
    struct Sound { Buf data; float gain; };
    Sound sfx[19];

    Buf Sweep(float len, float f0, float f1, float vol, float decay, int wave = 0, float noiseMix = 0.0f, float vibHz = 0.0f)
    {
        Buf b = Make(len);
        float ph = 0.0f;
        for (size_t i = 0; i < b.size(); ++i) {
            float t = static_cast<float>(i) / SR, k = t / len;
            float hz = f0 + (f1 - f0) * k;
            if (vibHz > 0.0f) hz *= 1.0f + 0.08f * std::sin(2 * PI * vibHz * t);
            ph += hz / SR;
            float tone = wave == 0 ? std::sin(2 * PI * ph) : wave == 1 ? Saw(ph) * 0.6f : (std::sin(2 * PI * ph) > 0 ? 0.5f : -0.5f);
            float env = std::exp(-decay * t) * std::min(1.0f, t * 300.0f);
            b[i] = (tone * (1.0f - noiseMix) + Noise() * noiseMix) * env * vol;
        }
        return b;
    }
    Buf NoiseBurst(float len, float vol, float decay, float lpStart, float lpEnd)
    {
        Buf b = Make(len); float lp = 0.0f;
        for (size_t i = 0; i < b.size(); ++i) {
            float t = static_cast<float>(i) / SR, k = t / len;
            float a = lpStart + (lpEnd - lpStart) * k;          // one-pole low-pass coefficient (swish)
            lp += a * (Noise() - lp);
            b[i] = lp * std::exp(-decay * t) * std::min(1.0f, t * 200.0f) * vol;
        }
        return b;
    }
    void Mix(Buf& dst, const Buf& src, float at = 0.0f, float g = 1.0f)
    {
        size_t o = static_cast<size_t>(at * SR);
        if (dst.size() < o + src.size()) dst.resize(o + src.size(), 0.0f);
        for (size_t i = 0; i < src.size(); ++i) dst[o + i] += src[i] * g;
    }
    Buf Tones(const std::vector<float>& hz, float step, float len, float vol, float decay)
    {
        Buf b = Make(step * hz.size() + len);
        for (size_t i = 0; i < hz.size(); ++i) Pluck(b, step * i, hz[i], len, vol, decay);
        return b;
    }

    void BuildSfx()
    {
        auto S = [](Audio::Sfx id) -> Sound& { return sfx[static_cast<int>(id)]; };
        S(Audio::Sfx::Click).data  = Sweep(0.07f, 900, 700, 0.5f, 30);
        { Buf b = Tones({ Freq(-2), Freq(5) }, 0.07f, 0.25f, 0.6f, 9); Mix(b, Sweep(0.18f, 120, 60, 0.7f, 14)); S(Audio::Sfx::Place).data = b; }
        S(Audio::Sfx::Locked).data = Sweep(0.16f, 170, 130, 0.5f, 10, 2);
        S(Audio::Sfx::Sword).data  = NoiseBurst(0.20f, 1.2f, 12, 0.08f, 0.45f);
        { Buf b = NoiseBurst(0.14f, 0.7f, 18, 0.5f, 0.15f); Mix(b, Sweep(0.18f, 620, 380, 0.35f, 22, 1)); S(Audio::Sfx::Arrow).data = b; }
        { Buf b = Sweep(0.14f, 150, 60, 0.9f, 22); Mix(b, NoiseBurst(0.08f, 0.8f, 40, 0.5f, 0.3f)); S(Audio::Sfx::Hit).data = b; }
        { Buf b = Sweep(0.22f, 330, 140, 0.55f, 11, 1); Mix(b, NoiseBurst(0.10f, 0.4f, 35, 0.4f, 0.2f)); S(Audio::Sfx::Hurt).data = b; }
        { Buf b = Sweep(0.45f, 420, 70, 0.5f, 5, 1, 0.25f); Mix(b, Sweep(0.2f, 110, 45, 0.8f, 12)); S(Audio::Sfx::EnemyDie).data = b; }
        { Buf b = Sweep(0.5f, 300, 80, 0.5f, 4, 2, 0.15f); Mix(b, Sweep(0.25f, 90, 40, 0.8f, 10)); S(Audio::Sfx::PlayerDie).data = b; }
        { Buf b = Make(0.4f); Pluck(b, 0.0f, 1318.5f, 0.2f, 0.5f, 18); Pluck(b, 0.07f, 1760.0f, 0.3f, 0.5f, 12); S(Audio::Sfx::Coin).data = b; }
        S(Audio::Sfx::Jump).data   = Sweep(0.18f, 250, 650, 0.45f, 9);
        S(Audio::Sfx::Duck).data   = Sweep(0.15f, 420, 200, 0.45f, 11);
        S(Audio::Sfx::SpiritBolt).data = Sweep(0.45f, 800, 300, 0.45f, 4, 0, 0.0f, 14.0f);
        { Buf b = NoiseBurst(0.07f, 0.6f, 40, 0.7f, 0.5f); Mix(b, Sweep(0.22f, 1500, 900, 0.3f, 14, 2)); S(Audio::Sfx::Stinger).data = b; }
        { Buf b = Make(1.1f);
          for (int k = 0; k < 2; ++k) {
              float hz = k == 0 ? Freq(-19) : Freq(-12);
              float st = k * 0.45f;
              for (size_t i = 0; i < static_cast<size_t>(0.6f * SR); ++i) {
                  float t = static_cast<float>(i) / SR, ph = hz * t;
                  float env = std::min(1.0f, t * 15.0f) * std::exp(-2.5f * t);
                  size_t idx = static_cast<size_t>(st * SR) + i; if (idx < b.size()) b[idx] += (Saw(ph) * 0.5f + std::sin(2 * PI * ph) * 0.5f) * env * 0.6f;
              }
          }
          S(Audio::Sfx::Horn).data = b; }
        S(Audio::Sfx::Win).data  = Tones({ Freq(-7), Freq(-3), Freq(0), Freq(5), Freq(0), Freq(5), Freq(8) }, 0.16f, 0.9f, 0.55f, 3.5f);
        S(Audio::Sfx::Lose).data = Tones({ Freq(0), Freq(-2), Freq(-3), Freq(-7), Freq(-12) }, 0.28f, 1.2f, 0.55f, 2.5f);
        { Buf b = Sweep(1.1f, 95, 55, 0.7f, 2.2, 1, 0.12f, 6.0f); S(Audio::Sfx::Roar).data = b; }
        { Buf b = Make(0.9f); Pluck(b, 0.0f, Freq(-7), 0.5f, 0.6f, 4); Pluck(b, 0.12f, Freq(0), 0.5f, 0.6f, 4); Pluck(b, 0.24f, Freq(3), 0.6f, 0.6f, 3.5f); Drum(b, 0.0f, 0.8f); S(Audio::Sfx::Wave).data = b; }

        const float gain[19] = { 0.5f, 0.6f, 0.5f, 0.5f, 0.45f, 0.6f, 0.55f, 0.6f, 0.6f, 0.55f, 0.4f, 0.4f, 0.45f, 0.45f, 0.65f, 0.7f, 0.7f, 0.6f, 0.6f };
        for (int i = 0; i < 19; ++i) { Normalize(sfx[i].data, 0.9f); sfx[i].gain = gain[i]; }
    }

    // ---------------------------------------------------------------- music
    Buf musicBuf[3];

    Buf BuildTheme()      // slow, mysterious: drone + sparse oud + flute
    {
        const float bpm = 72.0f, beat = 60.0f / bpm; const int bars = 8;
        Buf b = Make(beat * 4 * bars);
        Drone(b, 73.4f, 0.10f);
        const int mel[16] = { 4, 5, 4, 2, 3, 2, 1, 0, 4, 6, 5, 4, 3, 2, 1, 0 };
        for (int i = 0; i < 16; ++i) Flute(b, beat * 2 * i, Freq(SCALE[mel[i]] + 12), beat * 1.7f, 0.16f);
        for (int bar = 0; bar < bars; ++bar) {
            Pluck(b, bar * beat * 4, Freq(SCALE[0]), 1.2f, 0.28f, 2.5f);
            Pluck(b, bar * beat * 4 + beat * 2, Freq(SCALE[4] - 12), 1.2f, 0.22f, 2.5f);
            Drum(b, bar * beat * 4, 0.35f, 0.8f);
            Drum(b, bar * beat * 4 + beat * 3, 0.2f, 1.1f);
        }
        FadeEnds(b, 0.01f); Normalize(b, 0.8f); return b;
    }
    Buf BuildBattle()     // driving: faster, steady frame drum, arpeggio + bass
    {
        const float bpm = 116.0f, beat = 60.0f / bpm; const int bars = 8;
        Buf b = Make(beat * 4 * bars);
        Drone(b, 73.4f, 0.07f);
        const int arp[8] = { 0, 2, 4, 2, 5, 4, 2, 1 };               // scale degrees, eighth notes
        const int bass[8] = { 0, 0, 5, 4, 0, 0, 3, 4 };
        for (int bar = 0; bar < bars; ++bar) {
            float t0 = bar * beat * 4;
            for (int i = 0; i < 8; ++i) Pluck(b, t0 + i * beat * 0.5f, Freq(SCALE[(arp[i] + (bar % 4 >= 2 ? 1 : 0)) % 8] + 12), 0.5f, 0.2f, 7.0f);
            Pluck(b, t0, Freq(SCALE[bass[bar]] - 12), 1.0f, 0.35f, 3.0f);
            Pluck(b, t0 + beat * 2, Freq(SCALE[bass[bar]] - 12), 1.0f, 0.3f, 3.0f);
            for (int i = 0; i < 4; ++i) Drum(b, t0 + i * beat, i % 2 == 0 ? 0.55f : 0.35f, i % 2 == 0 ? 0.9f : 1.2f);
            Drum(b, t0 + beat * 3.5f, 0.25f, 1.4f);
            if (bar % 2 == 1) Flute(b, t0, Freq(SCALE[(bar * 3) % 8] + 12), beat * 3.5f, 0.12f);
        }
        FadeEnds(b, 0.01f); Normalize(b, 0.8f); return b;
    }

    // ---------------------------------------------------------------- mixer
    struct Voice { const Buf* buf = nullptr; size_t pos = 0; float vol = 0.0f; };
    constexpr int MAX_VOICES = 24;
    Voice voices[MAX_VOICES];
    std::mutex lock;
    int   curTrack = 0, nextTrack = 0;
    size_t musicPos = 0;
    float musicFade = 1.0f;           // 0..1 cross-fade level of the current track
    float musicDuck = 1.0f, musicDuckTarget = 1.0f;
    bool  muted = false;
    bool  ready = false, deviceOpen = false;
    ma_device device;

    void MixFrames(float* out, int frames)
    {
        std::lock_guard<std::mutex> g(lock);
        for (int f = 0; f < frames; ++f) {
            float s = 0.0f;
            if (!muted) {
                // music with a quick fade out -> switch -> fade in
                if (curTrack != nextTrack) {
                    musicFade -= 1.0f / (0.6f * SR);
                    if (musicFade <= 0.0f) { musicFade = 0.0f; curTrack = nextTrack; musicPos = 0; }
                } else if (musicFade < 1.0f) {
                    musicFade = std::min(1.0f, musicFade + 1.0f / (0.8f * SR));
                }
                musicDuck += (musicDuckTarget - musicDuck) * 0.0008f;
                if (curTrack != 0) {
                    const Buf& m = musicBuf[curTrack];
                    s += m[musicPos] * musicFade * musicDuck * 0.32f;
                    if (++musicPos >= m.size()) musicPos = 0;
                }
                for (Voice& v : voices) {
                    if (!v.buf) continue;
                    s += (*v.buf)[v.pos] * v.vol;
                    if (++v.pos >= v.buf->size()) v.buf = nullptr;
                }
            }
            s = std::tanh(s);                    // soft limiter
            out[f * 2] = out[f * 2 + 1] = s;
        }
    }

    void Callback(ma_device*, void* output, const void*, ma_uint32 frames)
    {
        MixFrames(static_cast<float*>(output), static_cast<int>(frames));
    }
}

namespace Audio
{
    bool Init()
    {
        if (!ready) {
            BuildSfx();
            musicBuf[1] = BuildTheme();
            musicBuf[2] = BuildBattle();
            ready = true;
        }
        ma_device_config cfg = ma_device_config_init(ma_device_type_playback);
        cfg.playback.format = ma_format_f32;
        cfg.playback.channels = 2;
        cfg.sampleRate = SR;
        cfg.dataCallback = Callback;
        if (ma_device_init(nullptr, &cfg, &device) != MA_SUCCESS) return false;
        if (ma_device_start(&device) != MA_SUCCESS) { ma_device_uninit(&device); return false; }
        deviceOpen = true;
        return true;
    }

    void Shutdown()
    {
        if (deviceOpen) { ma_device_uninit(&device); deviceOpen = false; }
    }

    void Play(Sfx s, float volume)
    {
        if (!ready) return;
        std::lock_guard<std::mutex> g(lock);
        Sound& snd = sfx[static_cast<int>(s)];
        Voice* slot = nullptr;
        for (Voice& v : voices) if (!v.buf) { slot = &v; break; }
        if (!slot) slot = &voices[0];                 // steal the oldest slot when everything is busy
        slot->buf = &snd.data; slot->pos = 0; slot->vol = snd.gain * volume;
    }

    void SetMusic(Music m)
    {
        std::lock_guard<std::mutex> g(lock);
        int t = static_cast<int>(m);
        if (t == nextTrack) return;
        nextTrack = t;
        if (curTrack == 0) { curTrack = t; musicPos = 0; musicFade = 0.0f; }   // nothing playing yet: just fade in
    }

    void SetMusicDucked(bool d) { std::lock_guard<std::mutex> g(lock); musicDuckTarget = d ? 0.35f : 1.0f; }
    void ToggleMute() { std::lock_guard<std::mutex> g(lock); muted = !muted; }
    bool IsMuted() { return muted; }
    int  SampleRate() { return SR; }

    void RenderForTest(float* out, int frames)
    {
        if (!ready) { BuildSfx(); musicBuf[1] = BuildTheme(); musicBuf[2] = BuildBattle(); ready = true; }
        MixFrames(out, frames);
    }
}
