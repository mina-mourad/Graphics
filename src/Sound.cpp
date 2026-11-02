// [NOT LAB] The labs have no sound, so this uses the miniaudio library
// (one header file: external/miniaudio/miniaudio.h). Its "engine" can load
// and play a sound file in one line.
// Sound files (from the internet, free to use - see sounds/CREDITS.txt).
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include "Sound.h"
#include <string>

static ma_engine engine;
static ma_sound music;
static bool ready = false;
static float volume = 0.7f;
static bool muted = false;

void InitSound()
{
    if (ma_engine_init(NULL, &engine) != MA_SUCCESS) return;   // no sound card: game stays silent
    ready = true;
    std::string file = std::string(SOUND_DIR) + "music.mp3";
    if (ma_sound_init_from_file(&engine, file.c_str(), 0, NULL, NULL, &music) == MA_SUCCESS) {
        ma_sound_set_looping(&music, MA_TRUE);
        ma_sound_set_volume(&music, 0.5f);
        ma_sound_start(&music);
    }
    ma_engine_set_volume(&engine, volume);
}

void CloseSound()
{
    if (ready) ma_engine_uninit(&engine);
}

void PlaySound(const char* name)
{
    if (!ready) return;
    std::string file = std::string(SOUND_DIR) + name + ".wav";
    ma_engine_play_sound(&engine, file.c_str(), NULL);
}

// The engine volume changes every sound, also the ones already playing.
void ChangeVolume(float change)
{
    volume += change;
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;
    if (ready) ma_engine_set_volume(&engine, muted ? 0.0f : volume);
}

void ToggleMute()
{
    muted = !muted;
    if (ready) ma_engine_set_volume(&engine, muted ? 0.0f : volume);
}

float GetVolume() { return volume; }
bool IsMuted() { return muted; }
