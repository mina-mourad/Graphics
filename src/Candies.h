#pragma once
// Person 2: the candies (players) - placing them, what they do, drawing them.
#include "Game.h"

void PlaceCandies(CandyType type);   // one candy at the left end of every selected shelf
void PlaceChocolate(int lane, float x); // the chocolate bar goes wherever you click on a shelf
float ChocolateSpot(float mouseX);   // where it would stand for this mouse x
void UpdateCandies(float dt);

// Draws one candy with its feet at (x, footY). size 1 = normal, smaller for the cards.
// attack 0..1 = attack animation, squash < 1 = ducking.
void DrawCandy(CandyType type, float x, float footY, float size, float attack, float squash);
void DrawCandies();
