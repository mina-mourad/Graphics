#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#undef PlaySound
#include "Sound.h"
#include <string>

#ifndef SoundDir
#define SoundDir "sounds/"
#endif

static ma_engine engine;
static ma_sound music;
static bool ready = false;
static float volume = 0.7f;
static bool muted = false;

void InitSound()
{
    if (ma_engine_init(NULL, &engine) != MA_SUCCESS) 
    {
        return;
    }
    ready = true;
    std::string file = std::string(SoundDir) + "music.mp3";
    if (ma_sound_init_from_file(&engine, file.c_str(), 0, NULL, NULL, &music) == MA_SUCCESS) {
        ma_sound_set_looping(&music, MA_TRUE);
        ma_sound_set_volume(&music, 0.5f);
        ma_sound_start(&music);
    }
    ma_engine_set_volume(&engine, volume);
}

void CloseSound()
{
    if (ready) 
    {
        ma_engine_uninit(&engine);
    }
}

void PlaySound(const char* name)
{
    if (!ready) 
    {
        return;
    }
    std::string file = std::string(SoundDir) + name + ".wav";
    ma_engine_play_sound(&engine, file.c_str(), NULL);
}

void ChangeVolume(float change)
{
    volume += change;
    if (volume < 0.0f) 
    {
        volume = 0.0f;
    }
    if (volume > 1.0f) 
    {
        volume = 1.0f;
    }
    if (ready) {
        if (muted) {
            ma_engine_set_volume(&engine, 0.0f);
        }
        else {
            ma_engine_set_volume(&engine, volume);
        }
    }
}

void ToggleMute()
{
    muted = !muted;
    if (ready) {
        if (muted) {
            ma_engine_set_volume(&engine, 0.0f);
        }
        else {
            ma_engine_set_volume(&engine, volume);
        }
    }
}

float GetVolume() { return volume; }
bool IsMuted() { return muted; }
