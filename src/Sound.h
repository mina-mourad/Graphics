#pragma once
// Person 2 (candies, intro and sound).
// Background music and sound effects (bonus). Sound files are in /sounds.
// no lab code in this file (only declarations; the code is in the .cpp file)
void InitSound();
void CloseSound();
void PlaySound(const char* name);   // e.g. PlaySound("click") plays sounds/click.wav
void ChangeVolume(float change);    // +0.1 louder, -0.1 quieter
void ToggleMute();
float GetVolume();
bool IsMuted();
