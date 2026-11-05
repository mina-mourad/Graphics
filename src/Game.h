#pragma once
// Person 4 (store, game flow and intro).
// Shared numbers, types and lists used by every part of the game.
// (Plain C++: structs, enums and std::vector. Not graphics, so not in the labs.)
// no lab code in this file

#include <glm/glm.hpp>
#include <vector>

// ---- Screen -------------------------------------------------------------
// Like the labs we draw in normalized coordinates. y goes from -1 to 1 and,
// because the window is 4:3, x goes from -Aspect to Aspect (see Shapes.cpp).
const int   WindowW = 1024;
const int   WindowH = 768;
const float Aspect   = 4.0f / 3.0f;

// ---- Shelves (lanes) ----------------------------------------------------
const int   Lanes = 3;
const float LaneY[Lanes] = { -0.45f, 0.0f, 0.45f };   // middle of each shelf, bottom to top
const float LaneH = 0.45f;                             // height of one shelf
const float LaneLeft  = -1.08f;                        // the candy jar ends here
const float LaneRight =  1.15f;                        // the fridge starts here
const float StartX = -0.98f;                           // candies appear here (left end)
const float SpawnX =  1.10f;                           // sodas appear at the right end of the shelf (by the fridge)
const float BaseX  = -1.08f;                           // a soda here reached the candy jar
const float Touch   =  0.16f;                           // closer than this = they collide

// Feet of a character standing on shelf `lane`.
inline float FloorY(int lane) { return LaneY[lane] - 0.17f; }
// no lab code

// ---- Characters ---------------------------------------------------------
enum CandyType { Lollipop, Bonbon, Chocolate };   // melee, ranged, defense (players)
enum SodaType  { Bottle, Can, Cup };              // easy, medium, hard (enemies)

const int   CandyHp[3]   = { 5, 5, 7 };
// Buying candies (spec 3.2 and the table on page 9):
//  - the Melee player (lollipop) is the basic starter player, unlocked from the start
//  - the game starts with enough sugar for three Melee players
//  - Long Range (bonbon) is unlocked through 2x the sugar the Melee player needs,
//    Defense (chocolate) through 3x
//  - "as the player collects enough coins, the other player types are progressively
//    unlocked": only sugar COLLECTED from defeated sodas unlocks them, so at the start
//    every card except the lollipop is locked (see CardUnlocked in Hud.cpp)
const int   MeleeCost    = 10;
const int   CandyCost[3] = { MeleeCost, 2 * MeleeCost, 3 * MeleeCost };   // 10, 20, 30
const float CandySpeed[3] = { 0.15f, 0.13f, 0.0f };
const int   SodaHp[3]    = { 5, 5, 7 };
const int   SodaDamage[3] = { 1, 1, 2 };
const float SodaSpeed[3] = { 0.06f, 0.06f, 0.13f };   // the cup runs
const int   SodaSugar[3] = { 15, 20, 30 };             // sugar dropped when defeated
const int   StartSugar = 3 * MeleeCost;                // enough for three lollipops

struct Candy
{
    CandyType type;
    int   lane;
    float x;
    int   hp;
    float cooldown;   // time until the next attack
    float attack;     // > 0 while the attack animation plays
    float jump;       // > 0 while jumping
    float duck;       // > 0 while ducking
    float hurt;       // > 0 just after being hit (flash)
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

struct Shot          // a candy pellet, or soda fizz / syrup
{
    int   lane;
    float x, y;
    float speed;      // + goes right (candy pellets), - goes left (soda shots)
    bool  high;       // soda shots: true = fizz up high (duck), false = syrup low (jump)
    int   damage;
    float pop;        // > 0 while the pellet pops after hitting
};

struct Cube          // sugar cube dropped by a defeated soda
{
    int   lane;
    float x;
    int   value;
};

// ---- The game world (defined in Game.cpp) --------------------------------
extern std::vector<Candy> candies;
extern std::vector<Soda>  sodas;
extern std::vector<Shot>  shots;
extern std::vector<Cube>  cubes;
extern int  sugar;              // sugar you have now (to buy candies)
extern int  sugarCollected;     // all sugar picked up from defeated sodas (unlocks cards)
bool CardUnlocked(CandyType type);   // in Hud.cpp
extern bool laneSelected[Lanes];
extern int  hoverLane;
extern bool placingChocolate;   // true after the chocolate card: the next shelf click places it
extern int  wave;               // 1..3
extern float gameTime;          // seconds, for animations

// ---- Input (filled by the Lab 3 style callbacks in main.cpp) -------------
extern float mouseX, mouseY;    // in our coordinates
extern bool  mouseClicked;      // true for one frame
extern bool  keyPressed[512];   // true for one frame

void ResetGame();
void GameUpdate(float dt);
void GameDraw();
bool GameWantsExit();
