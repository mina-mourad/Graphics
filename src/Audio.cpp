#include "Audio.h"

// miniaudio: only the low-level playback device is needed.
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

// How the sounds are made
// -----------------------
// Every sound is an array of samples (44100 per second) filled once at
// start-up with sine waves, noise and envelopes (a volume curve that rises
// and decays). The instruments are the ones you hear in a Cairo street:
//   darbuka   - "doum" (deep) and "tak" (sharp) drum hits
//   riq       - a tambourine (short jingly noise)
//   oud       - a plucked string (a few harmonics that decay)
//   nay       - a breathy flute with vibrato
//   mizmar    - a loud nasal reed (lots of harmonics) for the battle tune
// Melodies use the Hijaz scale (the typical "Egyptian" sound).
namespace
{
    constexpr int   SR = 44100;
    constexpr float PI = 3.14159265358979f;
    using Buf = std::vector<float>;

    // ------------------------------------------------------------ helpers
    unsigned noiseState = 22222u;
    float Noise()
    {
        noiseState = noiseState * 1664525u + 1013904223u;
        return static_cast<float>((noiseState >> 8) & 0xFFFF) / 32768.0f - 1.0f;
    }
    float Freq(float semitonesFromA4) { return 440.0f * std::pow(2.0f, semitonesFromA4 / 12.0f); }
    Buf Make(float seconds) { return Buf(static_cast<size_t>(seconds * SR), 0.0f); }
    size_t At(float seconds) { return static_cast<size_t>(seconds * SR); }

    // Hijaz scale on D, in semitones from A4: D Eb F# G A Bb C D Eb F#
    const float HIJAZ[10] = { -7, -6, -3, -2, 0, 1, 3, 5, 6, 9 };

    void Normalize(Buf& b, float peak)
    {
        float m = 0.0f;
        for (float v : b) m = std::max(m, std::fabs(v));
        if (m > 0.0f) for (float& v : b) v *= peak / m;
    }

    // ------------------------------------------------------------ instruments
    void Doum(Buf& b, float start, float vol)
    {
        float ph = 0.0f;
        for (size_t i = 0; i < At(0.45f) && At(start) + i < b.size(); ++i)
        {
            float t = (float)i / SR;
            float hz = 62.0f + 70.0f * std::exp(-25.0f * t);
            ph += 2.0f * PI * hz / SR;
            b[At(start) + i] += (std::sin(ph) * std::exp(-6.5f * t) + 0.15f * Noise() * std::exp(-60.0f * t)) * vol;
        }
    }

    void Tak(Buf& b, float start, float vol, float bright = 1.0f)
    {
        float lp = 0.0f;
        for (size_t i = 0; i < At(0.12f) && At(start) + i < b.size(); ++i)
        {
            float t = (float)i / SR;
            lp += 0.6f * bright * (Noise() - lp);
            float ring = std::sin(2.0f * PI * 760.0f * t) * 0.5f + std::sin(2.0f * PI * 1180.0f * t) * 0.3f;
            b[At(start) + i] += (lp * 0.9f + ring) * std::exp(-38.0f * t) * vol;
        }
    }

    void Riq(Buf& b, float start, float vol)
    {
        float prev = 0.0f;
        for (size_t i = 0; i < At(0.09f) && At(start) + i < b.size(); ++i)
        {
            float t = (float)i / SR;
            float n = Noise();
            float hp = n - prev;   // simple high-pass: difference of two samples
            prev = n;
            float jingle = std::sin(2 * PI * 5200.0f * t) + std::sin(2 * PI * 7100.0f * t);
            b[At(start) + i] += (hp * 0.6f + jingle * 0.15f) * std::exp(-45.0f * t) * vol;
        }
    }

    void Oud(Buf& b, float start, float hz, float len, float vol)
    {
        for (size_t i = 0; i < At(len) && At(start) + i < b.size(); ++i)
        {
            float t = (float)i / SR;
            float env = std::exp(-4.0f * t) * std::min(1.0f, t * 500.0f);
            float v = 0.0f;
            for (int k = 1; k <= 5; ++k)   // two slightly detuned strings, like an oud course
                v += (std::sin(2 * PI * hz * k * t) + std::sin(2 * PI * hz * 1.003f * k * t)) * std::exp(-0.6f * k * t) / (k * 1.2f);
            b[At(start) + i] += v * env * vol * 0.4f;
        }
    }

    void Nay(Buf& b, float start, float hz, float len, float vol, float glide = 0.0f)
    {
        float ph = 0.0f, lp = 0.0f;
        for (size_t i = 0; i < At(len) && At(start) + i < b.size(); ++i)
        {
            float t = (float)i / SR;
            float env = std::min(1.0f, t * 6.0f) * std::min(1.0f, (len - t) * 5.0f);
            float f = hz * (1.0f + glide * t / len) * (1.0f + 0.008f * std::sin(2 * PI * 5.0f * t) * std::min(1.0f, t * 2.0f));
            ph += 2 * PI * f / SR;
            lp += 0.08f * (Noise() - lp);   // breath
            b[At(start) + i] += (std::sin(ph) + 0.2f * std::sin(2 * ph) + 0.6f * lp) * env * vol;
        }
    }

    void Mizmar(Buf& b, float start, float hz, float len, float vol)
    {
        float ph = 0.0f;
        for (size_t i = 0; i < At(len) && At(start) + i < b.size(); ++i)
        {
            float t = (float)i / SR;
            float env = std::min(1.0f, t * 30.0f) * std::min(1.0f, (len - t) * 20.0f);
            ph += 2 * PI * hz * (1.0f + 0.01f * std::sin(2 * PI * 6.5f * t)) / SR;
            float v = 0.0f;
            for (int k = 1; k <= 7; ++k) v += std::sin(ph * k) / (k % 2 ? k : k * 1.6f);   // nasal: strong odd harmonics
            b[At(start) + i] += v * env * vol * 0.35f;
        }
    }

    void Bass(Buf& b, float start, float hz, float len, float vol)
    {
        for (size_t i = 0; i < At(len) && At(start) + i < b.size(); ++i)
        {
            float t = (float)i / SR;
            float env = std::min(1.0f, t * 80.0f) * std::exp(-2.5f * t);
            b[At(start) + i] += (std::sin(2 * PI * hz * t) + 0.3f * std::sin(4 * PI * hz * t)) * env * vol;
        }
    }

    // A pitch sweep with a decaying envelope (wave: 0 sine, 1 saw, 2 square).
    Buf Sweep(float len, float f0, float f1, float vol, float decay, int wave = 0, float noiseMix = 0.0f, float vibHz = 0.0f, float vibAmt = 0.08f)
    {
        Buf b = Make(len);
        float ph = 0.0f;
        for (size_t i = 0; i < b.size(); ++i)
        {
            float t = (float)i / SR, k = t / len;
            float hz = f0 + (f1 - f0) * k;
            if (vibHz > 0.0f) hz *= 1.0f + vibAmt * std::sin(2 * PI * vibHz * t);
            ph += hz / SR;
            float frac = ph - std::floor(ph);
            float tone = wave == 0 ? std::sin(2 * PI * ph) : wave == 1 ? (2.0f * frac - 1.0f) * 0.6f : (frac < 0.5f ? 0.5f : -0.5f);
            float env = std::exp(-decay * t) * std::min(1.0f, t * 300.0f);
            b[i] = (tone * (1.0f - noiseMix) + Noise() * noiseMix) * env * vol;
        }
        return b;
    }

    // Filtered noise; the filter opens or closes over time (whooshes, splats).
    Buf NoiseBurst(float len, float vol, float decay, float lpStart, float lpEnd, float wobbleHz = 0.0f)
    {
        Buf b = Make(len);
        float lp = 0.0f;
        for (size_t i = 0; i < b.size(); ++i)
        {
            float t = (float)i / SR, k = t / len;
            lp += (lpStart + (lpEnd - lpStart) * k) * (Noise() - lp);
            float wob = wobbleHz > 0.0f ? 0.55f + 0.45f * std::sin(2 * PI * wobbleHz * t) : 1.0f;
            b[i] = lp * std::exp(-decay * t) * std::min(1.0f, t * 200.0f) * vol * wob;
        }
        return b;
    }

    void Mix(Buf& dst, const Buf& src, float at = 0.0f, float g = 1.0f)
    {
        size_t o = At(at);
        if (dst.size() < o + src.size()) dst.resize(o + src.size(), 0.0f);
        for (size_t i = 0; i < src.size(); ++i) dst[o + i] += src[i] * g;
    }

    // A bright metal "ping" made of inharmonic partials (coins, knife).
    Buf Ping(float len, float hz, float vol, float decay)
    {
        Buf b = Make(len);
        for (size_t i = 0; i < b.size(); ++i)
        {
            float t = (float)i / SR;
            float v = std::sin(2 * PI * hz * t) + 0.6f * std::sin(2 * PI * hz * 2.76f * t) + 0.3f * std::sin(2 * PI * hz * 5.4f * t);
            b[i] = v * std::exp(-decay * t) * std::min(1.0f, t * 800.0f) * vol;
        }
        return b;
    }

    // ------------------------------------------------------------ sound effects
    struct Sound { Buf data; float gain; };
    Sound sfx[static_cast<int>(Audio::Sfx::Count)];

    Sound& S(Audio::Sfx id) { return sfx[static_cast<int>(id)]; }

    void BuildSfx()
    {
        using Audio::Sfx;

        { Buf b = Make(0.08f); Tak(b, 0.0f, 0.5f, 1.4f); S(Sfx::Click) = { b, 0.45f }; }
        { Buf b = Make(0.15f); Tak(b, 0.0f, 1.0f); S(Sfx::LaneSelect) = { b, 0.5f }; }
        { Buf b = Make(0.6f); Doum(b, 0.0f, 1.0f); Tak(b, 0.12f, 0.6f); Oud(b, 0.05f, Freq(HIJAZ[4]), 0.4f, 0.5f); Oud(b, 0.15f, Freq(HIJAZ[7]), 0.4f, 0.5f); S(Sfx::Place) = { b, 0.6f }; }
        S(Sfx::Locked) = { Sweep(0.18f, 150, 120, 0.5f, 10, 2), 0.45f };
        S(Sfx::Swing)  = { NoiseBurst(0.22f, 1.2f, 9, 0.05f, 0.35f), 0.45f };
        { Buf b = Sweep(0.12f, 190, 120, 0.9f, 30); Mix(b, Ping(0.1f, 420, 0.3f, 50)); Mix(b, NoiseBurst(0.06f, 0.9f, 60, 0.6f, 0.3f)); S(Sfx::StickHit) = { b, 0.6f }; }
        S(Sfx::TomatoThrow) = { NoiseBurst(0.18f, 0.9f, 12, 0.1f, 0.3f), 0.35f };
        { Buf b = NoiseBurst(0.28f, 1.3f, 12, 0.35f, 0.04f, 38.0f); Mix(b, Sweep(0.2f, 320, 70, 0.6f, 14)); S(Sfx::TomatoSplat) = { b, 0.6f }; }
        S(Sfx::SlipperThrow) = { NoiseBurst(0.3f, 1.0f, 7, 0.08f, 0.25f, 24.0f), 0.4f };
        { Buf b = NoiseBurst(0.05f, 1.6f, 70, 0.95f, 0.8f); Mix(b, Sweep(0.1f, 140, 70, 0.7f, 30)); S(Sfx::SlipperHit) = { b, 0.65f }; }
        { Buf b = Sweep(0.25f, 260, 200, 0.6f, 12, 0, 0.0f, 28.0f, 0.15f); Mix(b, NoiseBurst(0.03f, 0.8f, 80, 0.9f, 0.6f)); S(Sfx::Slingshot) = { b, 0.45f }; }
        { Buf b = Ping(0.06f, 2400, 0.5f, 70); Mix(b, Ping(0.06f, 1900, 0.4f, 70), 0.07f); S(Sfx::PebbleHit) = { b, 0.45f }; }
        { Buf b = Ping(0.45f, 1900, 0.5f, 9); Mix(b, NoiseBurst(0.25f, 0.8f, 10, 0.5f, 0.9f)); S(Sfx::Knife) = { b, 0.45f }; }
        { Buf b = Sweep(0.24f, 210, 130, 0.6f, 11, 1); Mix(b, Sweep(0.24f, 420, 260, 0.3f, 11)); S(Sfx::PlayerHurt) = { b, 0.55f }; }
        { Buf b = Sweep(0.6f, 330, 90, 0.5f, 4, 1, 0.1f); Doum(b, 0.35f, 0.9f); S(Sfx::PlayerDown) = { b, 0.6f }; }
        { Buf b = Sweep(0.55f, 900, 180, 0.4f, 4, 0, 0.0f, 9.0f, 0.04f); Doum(b, 0.42f, 1.0f); S(Sfx::EnemyDown) = { b, 0.6f }; }
        { Buf b = Ping(0.35f, 2600, 0.5f, 14); Mix(b, Ping(0.35f, 3500, 0.45f, 12), 0.07f); S(Sfx::Coin) = { b, 0.5f }; }
        S(Sfx::Jump) = { Sweep(0.22f, 240, 700, 0.5f, 8, 0, 0.0f, 18.0f, 0.05f), 0.4f };
        S(Sfx::Duck) = { NoiseBurst(0.16f, 1.0f, 14, 0.4f, 0.05f), 0.4f };

        // tuk-tuk horn: two buzzy tones together, "beep-beeep"
        {
            Buf b = Make(1.0f);
            for (int k = 0; k < 2; ++k)
            {
                float st = k == 0 ? 0.0f : 0.28f, len = k == 0 ? 0.2f : 0.5f;
                Mix(b, Sweep(len, 415, 415, 0.5f, 0.5f, 2), st);
                Mix(b, Sweep(len, 523, 523, 0.4f, 0.5f, 2), st);
            }
            S(Sfx::WaveHorn) = { b, 0.55f };
        }

        // the win: a zaghrouta (the joyful trilled ululation at Egyptian
        // celebrations) over a darbuka roll
        {
            Buf b = Make(2.4f);
            float ph = 0.0f;
            for (size_t i = 0; i < At(1.9f); ++i)
            {
                float t = (float)i / SR;
                float trill = std::sin(2 * PI * 11.0f * t);                    // the tongue trill
                float hz = 1050.0f + 140.0f * trill + 60.0f * std::sin(2 * PI * 0.7f * t);
                ph += 2 * PI * hz / SR;
                float env = std::min(1.0f, t * 8.0f) * std::min(1.0f, (1.9f - t) * 3.0f) * (0.75f + 0.25f * trill);
                b[i] += (std::sin(ph) + 0.25f * std::sin(2 * ph)) * env * 0.5f;
            }
            for (int k = 0; k < 12; ++k) Tak(b, 0.05f + k * 0.12f, 0.5f);
            Doum(b, 1.6f, 1.2f);
            Tak(b, 1.85f, 0.8f);
            S(Sfx::Win) = { b, 0.6f };
        }
        // the loss: a sad nay falling down the scale
        {
            Buf b = Make(3.0f);
            const int notes[5] = { 7, 6, 5, 4, 1 };
            for (int k = 0; k < 5; ++k)
                Nay(b, k * 0.45f, Freq(HIJAZ[notes[k]]), k == 4 ? 1.2f : 0.5f, 0.45f, k == 4 ? -0.03f : 0.0f);
            Doum(b, 0.0f, 0.8f);
            Doum(b, 1.8f, 0.9f);
            S(Sfx::Lose) = { b, 0.6f };
        }
        // tuk-tuk passing: putter of the little engine, then the horn
        {
            Buf b = Make(2.4f);
            for (size_t i = 0; i < b.size(); ++i)
            {
                float t = (float)i / SR;
                float putter = std::pow(0.5f + 0.5f * std::sin(2 * PI * 23.0f * t), 6.0f);
                float env = std::min(1.0f, t * 2.0f) * std::min(1.0f, (2.4f - t) * 1.5f);
                b[i] = (Noise() * 0.5f + std::sin(2 * PI * 70.0f * t)) * putter * env * 0.6f;
            }
            Mix(b, S(Sfx::WaveHorn).data, 0.9f, 0.8f);
            S(Sfx::TukTuk) = { b, 0.55f };
        }

        for (Sound& s : sfx) Normalize(s.data, 0.9f);
    }

    // ------------------------------------------------------------ music
    Buf musicBuf[3];

    // Maqsoum rhythm, 8 eighth notes: D T . T D . T .
    void Maqsoum(Buf& b, float t0, float eighth, float vol)
    {
        Doum(b, t0, vol);
        Tak(b, t0 + eighth * 1, vol * 0.55f);
        Tak(b, t0 + eighth * 3, vol * 0.55f);
        Doum(b, t0 + eighth * 4, vol * 0.9f);
        Tak(b, t0 + eighth * 6, vol * 0.55f);
        Tak(b, t0 + eighth * 7, vol * 0.3f, 1.4f);
    }

    // Menu / cutscene: calm oud melody, nay answers, soft maqsoum.
    Buf BuildMenuMusic()
    {
        const float bpm = 92.0f, beat = 60.0f / bpm, eighth = beat / 2.0f;
        const int bars = 8;
        Buf b = Make(beat * 4 * bars);
        const int melody[32] = { 4, 5, 4, 3,  2, 3, 1, 0,  4, 6, 5, 4,  3, 2, 1, 0,
                                 7, 6, 5, 4,  5, 4, 3, 2,  3, 4, 5, 4,  3, 1, 0, 0 };
        for (int bar = 0; bar < bars; ++bar)
        {
            float t0 = bar * beat * 4;
            Maqsoum(b, t0, eighth, 0.45f);
            for (int k = 0; k < 4; ++k)
                Oud(b, t0 + k * beat, Freq(HIJAZ[melody[bar * 4 + k]]), beat * 1.5f, 0.35f);
            Bass(b, t0, Freq(HIJAZ[0] - 24), beat * 2.0f, 0.35f);
            if (bar % 2 == 1)
                Nay(b, t0 + beat * 0.5f, Freq(HIJAZ[(bar * 3) % 8] + 12), beat * 3.2f, 0.12f);
        }
        Normalize(b, 0.8f);
        return b;
    }

    // Battle: a faster street-wedding / mahraganat groove - steady kick,
    // maqsoum on darbuka, riq, mizmar riff and bass.
    Buf BuildBattleMusic()
    {
        const float bpm = 124.0f, beat = 60.0f / bpm, eighth = beat / 2.0f;
        const int bars = 8;
        Buf b = Make(beat * 4 * bars);
        const int riff[16] = { 0, 1, 2, 1, 4, 3, 2, 1,  4, 5, 4, 3, 2, 1, 2, 0 };
        const int bassNotes[8] = { 0, 0, 3, 4, 0, 0, 5, 4 };
        for (int bar = 0; bar < bars; ++bar)
        {
            float t0 = bar * beat * 4;
            for (int k = 0; k < 4; ++k) Doum(b, t0 + k * beat, 0.55f);        // four-on-the-floor kick
            Maqsoum(b, t0, eighth, 0.35f);
            for (int k = 0; k < 16; ++k) Riq(b, t0 + k * eighth * 0.5f, k % 4 == 2 ? 0.35f : 0.18f);
            for (int k = 0; k < 2; ++k)
                Bass(b, t0 + k * beat * 2, Freq(HIJAZ[bassNotes[bar]] - 24), beat * 1.8f, 0.45f);
            int shift = (bar % 4 >= 2) ? 1 : 0;
            for (int k = 0; k < 8; ++k)
            {
                int n = riff[(bar % 2) * 8 + k] + shift;
                Mizmar(b, t0 + k * eighth, Freq(HIJAZ[n] + 12), eighth * 0.9f, 0.16f);
            }
        }
        Normalize(b, 0.8f);
        return b;
    }

    // ------------------------------------------------------------ mixer
    struct Voice { const Buf* buf = nullptr; size_t pos = 0; float vol = 0.0f; };
    constexpr int MAX_VOICES = 32;
    Voice voices[MAX_VOICES];
    std::mutex lock;
    int curTrack = 0, nextTrack = 0;
    size_t musicPos = 0;
    float musicFade = 1.0f;
    float musicDuck = 1.0f, musicDuckTarget = 1.0f;

    int masterVolume = 8, musicVolume = 6, effectsVolume = 8;
    bool muted = false;
    bool ready = false, deviceOpen = false;
    ma_device device;

    void Build()
    {
        if (ready) return;
        BuildSfx();
        musicBuf[1] = BuildMenuMusic();
        musicBuf[2] = BuildBattleMusic();
        ready = true;
    }

    // Fills the sound card's buffer. The volume settings are applied here,
    // so they change sounds that are already playing too.
    void MixFrames(float* out, int frames)
    {
        std::lock_guard<std::mutex> g(lock);
        const float master = muted ? 0.0f : masterVolume / 10.0f;
        const float musicGain = master * musicVolume / 10.0f;
        const float sfxGain = master * effectsVolume / 10.0f;

        for (int f = 0; f < frames; ++f)
        {
            float s = 0.0f;

            // music: fade out the old track, switch, fade in the new one
            if (curTrack != nextTrack)
            {
                musicFade -= 1.0f / (0.6f * SR);
                if (musicFade <= 0.0f) { musicFade = 0.0f; curTrack = nextTrack; musicPos = 0; }
            }
            else if (musicFade < 1.0f)
                musicFade = std::min(1.0f, musicFade + 1.0f / (0.8f * SR));
            musicDuck += (musicDuckTarget - musicDuck) * 0.0008f;

            if (curTrack != 0)
            {
                const Buf& m = musicBuf[curTrack];
                s += m[musicPos] * musicFade * musicDuck * 0.45f * musicGain;
                if (++musicPos >= m.size()) musicPos = 0;
            }
            for (Voice& v : voices)
            {
                if (!v.buf) continue;
                s += (*v.buf)[v.pos] * v.vol * sfxGain;
                if (++v.pos >= v.buf->size()) v.buf = nullptr;
            }
            s = std::tanh(s);   // soft limiter so many sounds at once never crackle
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
        Build();
        if (deviceOpen) return true;
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

        // the same sound started twice in the same instant would just be louder
        for (Voice& v : voices)
            if (v.buf == &snd.data && v.pos < At(0.03f)) return;

        Voice* slot = nullptr;
        for (Voice& v : voices) if (!v.buf) { slot = &v; break; }
        if (!slot)
        {
            slot = &voices[0];   // all busy: replace the one closest to its end
            for (Voice& v : voices)
                if (v.buf->size() - v.pos < slot->buf->size() - slot->pos) slot = &v;
        }
        slot->buf = &snd.data;
        slot->pos = 0;
        slot->vol = snd.gain * volume;
    }

    void SetMusic(Music m)
    {
        std::lock_guard<std::mutex> g(lock);
        int t = static_cast<int>(m);
        if (t == nextTrack) return;
        nextTrack = t;
        if (curTrack == 0) { curTrack = t; musicPos = 0; musicFade = 0.0f; }
    }

    void SetMusicDucked(bool d) { std::lock_guard<std::mutex> g(lock); musicDuckTarget = d ? 0.35f : 1.0f; }

    int GetMasterVolume() { return masterVolume; }
    int GetMusicVolume() { return musicVolume; }
    int GetEffectsVolume() { return effectsVolume; }

    void ChangeMasterVolume(int d)  { std::lock_guard<std::mutex> g(lock); masterVolume = std::clamp(masterVolume + d, 0, 10); }
    void ChangeMusicVolume(int d)   { std::lock_guard<std::mutex> g(lock); musicVolume = std::clamp(musicVolume + d, 0, 10); }
    void ChangeEffectsVolume(int d) { std::lock_guard<std::mutex> g(lock); effectsVolume = std::clamp(effectsVolume + d, 0, 10); }
    void SetMuted(bool m) { std::lock_guard<std::mutex> g(lock); muted = m; }
    bool IsMuted() { return muted; }
    int  SampleRate() { return SR; }

    void RenderForTest(float* out, int frames)
    {
        Build();
        MixFrames(out, frames);
    }
}
