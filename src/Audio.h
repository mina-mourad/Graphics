#pragma once

// Sound bonus. Every sound effect and both music tracks are synthesised in
// code when the game starts (no audio files to ship): a darbuka rhythm, an
// oud and a nay / mizmar melody in the Hijaz scale, a tuk-tuk horn, a
// zaghrouta for the win, slipper smacks, tomato splats, coin clinks...
//
// Playback uses the single-header miniaudio library (external/miniaudio).
// If no audio device exists, every call is a harmless no-op.
namespace Audio
{
    enum class Sfx
    {
        Click,          // menu buttons
        LaneSelect,     // darbuka "tak" when a lane is toggled
        Place,          // a defender joins the lane
        Locked,         // locked card / not allowed
        Swing,          // nabboot stick swishes through the air
        StickHit,       // ...and lands with a wooden thwack
        TomatoThrow,
        TomatoSplat,
        SlipperThrow,
        SlipperHit,     // the famous slipper smack
        Slingshot,
        PebbleHit,
        Knife,
        PlayerHurt,
        PlayerDown,
        EnemyDown,
        Coin,           // pound coins clinking
        Jump,
        Duck,
        WaveHorn,       // tuk-tuk horn: a new wave arrives
        Win,            // zaghrouta + darbuka roll
        Lose,           // sad nay
        TukTuk,         // tuk-tuk engine putter + horn (cutscene)
        Count
    };

    enum class Music { None, Menu, Battle };

    bool Init();                         // false when no device could be opened (game stays silent)
    void Shutdown();
    void Play(Sfx s, float volume = 1.0f);
    void SetMusic(Music m);              // fades over; calling with the current track does nothing
    void SetMusicDucked(bool ducked);    // quieter music, e.g. on the pause menu

    // ---- Sound settings (main menu) ----------------------------------------
    // Volumes go from 0 to 10.
    int  GetMasterVolume();
    int  GetMusicVolume();
    int  GetEffectsVolume();
    void ChangeMasterVolume(int delta);
    void ChangeMusicVolume(int delta);
    void ChangeEffectsVolume(int delta);
    void SetMuted(bool muted);
    bool IsMuted();

    // Test hook: renders `frames` stereo float frames exactly as the audio device would.
    void RenderForTest(float* out, int frames);
    int  SampleRate();
}
