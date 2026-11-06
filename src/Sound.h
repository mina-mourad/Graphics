#pragma once
void InitSound();
void CloseSound();
void PlaySound(const char* name);
void ChangeVolume(float change);
void ToggleMute();
float GetVolume();
bool IsMuted();
