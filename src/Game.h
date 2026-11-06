#pragma once

#include <glm/glm.hpp>
#include <vector>

const int   WindowW = 1024;
const int   WindowH = 768;
const float Aspect   = 4.0f / 3.0f;

const int   Lanes = 3;
const float LaneY[Lanes] = { -0.45f, 0.0f, 0.45f };
const float LaneH = 0.45f;
const float LaneLeft  = -1.08f;
const float LaneRight =  1.15f;
const float StartX = -0.98f;
const float SpawnX =  1.10f;
const float BaseX  = -1.08f;
const float Touch   =  0.16f;

inline float FloorY(int lane) { return LaneY[lane] - 0.17f; }

enum CandyType { Lollipop, Bonbon, Chocolate };
enum SodaType  { Bottle, Can, Cup };

const int   CandyHp[3]   = { 5, 5, 7 };
const int   MeleeCost    = 10;
const int   CandyCost[3] = { MeleeCost, 2 * MeleeCost, 3 * MeleeCost };
const float CandySpeed[3] = { 0.15f, 0.13f, 0.0f };
const int   SodaHp[3]    = { 5, 5, 7 };
const int   SodaDamage[3] = { 1, 1, 2 };
const float SodaSpeed[3] = { 0.06f, 0.06f, 0.13f };
const int   SodaSugar[3] = { 15, 20, 30 };
const int   StartSugar = 3 * MeleeCost;

struct Candy
{
    CandyType type;
    int   lane;
    float x;
    int   hp;
    float cooldown;
    float attack;
    float jump;
    float duck;
    float hurt;
};

struct Soda
{
    SodaType type;
    int   lane;
    float x;
    int   hp;
    float cooldown;
    float attack;
    float hurt;
};

struct Shot
{
    int   lane;
    float x, y;
    float speed;
    bool  high;
    int   damage;
    float pop;
};

struct Cube
{
    int   lane;
    float x;
    int   value;
};

extern std::vector<Candy> candies;
extern std::vector<Soda>  sodas;
extern std::vector<Shot>  shots;
extern std::vector<Cube>  cubes;
extern int  sugar;
extern int  sugarCollected;
bool CardUnlocked(CandyType type);
extern bool laneSelected[Lanes];
extern int  hoverLane;
extern bool placingChocolate;
extern int  wave;
extern float gameTime;

extern float mouseX, mouseY;
extern bool  mouseClicked;
extern bool  keyPressed[512];

void ResetGame();
void GameUpdate(float dt);
void GameDraw();
bool GameWantsExit();
