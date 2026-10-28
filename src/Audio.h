#pragma once

// Procedural sound: every effect and both music loops are synthesised in code at
// start-up, so there are no audio files to ship. Playback uses miniaudio
// (external/miniaudio/miniaudio.h). If no audio device exists, every call is a
// harmless no-op and the game still runs silently.

namespace Audio
{
    enum class Sfx
    {
        Click, Place, Locked, Sword, Arrow, Hit, Hurt, EnemyDie, PlayerDie, Coin,
        Jump, Duck, SpiritBolt, Stinger, Horn, Win, Lose, Roar, Wave
    };
    enum class Music { None, Theme, Battle };

    bool Init();                         // returns false when no device could be opened
    void Shutdown();
    void Play(Sfx s, float volume = 1.0f);
    void SetMusic(Music m);              // cross-fades; calling with the current track does nothing
    void SetMusicDucked(bool ducked);    // quieter music, e.g. on the pause menu
    void ToggleMute();
    bool IsMuted();

    // Test hook: renders `frames` stereo float frames exactly as the audio callback would.
    void RenderForTest(float* out, int frames);
    int  SampleRate();
}
