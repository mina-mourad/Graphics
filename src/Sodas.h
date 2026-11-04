#pragma once
// Person 3: the sodas (enemies), the three waves, and all flying shots.
// no lab code in this file (only declarations; the code is in the .cpp file)
#include "Game.h"

bool UpdateWaves(float dt);       // returns true when all 3 waves are beaten
bool UpdateSodas(float dt);       // returns true if a soda reached the candy jar
void UpdateShots(float dt);
void ResetWaves();

// Draws one soda with its bottom at (x, footY). attack 0..1 = attack animation.
void DrawSoda(SodaType type, float x, float footY, float size, float attack);
void DrawSodas();
void DrawShots();
