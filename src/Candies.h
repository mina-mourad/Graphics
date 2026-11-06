#pragma once
#include "Game.h"

void PlaceCandies(CandyType type);
void PlaceChocolate(int lane, float x);
void UpdateCandies(float dt);

void DrawCandy(CandyType type, float x, float footY, float size, float attack, float squash);
void DrawCandies();
