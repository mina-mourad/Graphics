#pragma once
#include "Game.h"

bool UpdateWaves(float dt);
bool UpdateSodas(float dt);
void UpdateShots(float dt);
void ResetWaves();

void DrawSoda(SodaType type, float x, float footY, float size, float attack);
void DrawSodas();
void DrawShots();
