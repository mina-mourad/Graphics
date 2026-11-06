#pragma once
void UpdateHud();
void DrawHud();
void DrawHealth(float x, float y, int hp, int maxHp, bool isCandy);
void DrawCubes();
void SugarCube(float x, float y, float size);
bool InBox(float px, float py, float x, float y, float w, float h);
