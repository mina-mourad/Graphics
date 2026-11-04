#pragma once
// Person 2: the interface - candy cards, sugar counter, wave number,
// health bars, and the sugar cubes lying on the shelves.
// no lab code in this file (only declarations; the code is in the .cpp file)
void UpdateHud();                                    // card clicks and keys 1 / 2 / 3
void DrawHud();
void DrawHealth(float x, float y, int hp, int maxHp, bool isCandy);
void DrawCubes();
void SugarCube(float x, float y, float size);
bool InBox(float px, float py, float x, float y, float w, float h);   // Lab 3 point-in-triangle on 2 halves
