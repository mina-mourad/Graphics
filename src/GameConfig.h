#pragma once

// Shared world and gameplay constants. Every system reads its numbers from
// here so the whole team works in the same world.
//
//   y = 9.0  +---------------------------------------------------+
//            | HUD: player cards (left), wave, coins (right)      |
//            | rooftops of the hara (background)                  |
//   y = 5.85 +------+--------------------------------------------+
//            |KOSHK |  lane 2 (top)                               |
//            |(home |  lane 1                                     | players walk right,
//            | base)|  lane 0 (bottom)                            | enemies walk left
//   y = 0.6  +------+--------------------------------------------+
//            | near sidewalk (rubble, tires, crates)              |
//   y = 0.0  +---------------------------------------------------+
//           x=0   x=1.55                                       x=16

// World dimensions (world units, not pixels).
constexpr float WORLD_WIDTH  = 16.0f;
constexpr float WORLD_HEIGHT = 9.0f;

// ---- Lanes ---------------------------------------------------------------
constexpr int   NUM_LANES        = 3;                    // spec: at least 3
constexpr float LANE_HEIGHT      = 1.75f;
constexpr float PLAYFIELD_BOTTOM = 0.6f;
constexpr float PLAYFIELD_TOP    = PLAYFIELD_BOTTOM + NUM_LANES * LANE_HEIGHT;   // 5.85
constexpr float LANE_Y[NUM_LANES] = { 1.475f, 3.225f, 4.975f };                  // lane centers

constexpr float HOME_BASE_RIGHT  = 1.55f;                // right edge of the koshk
constexpr float LANE_LEFT        = HOME_BASE_RIGHT;      // lanes start at the koshk
constexpr float LANE_RIGHT       = WORLD_WIDTH;
constexpr float LANE_WIDTH       = LANE_RIGHT - LANE_LEFT;

constexpr float LANE_START_X     = 1.95f;                // players appear here (left end of a lane)
constexpr float LANE_END_X       = WORLD_WIDTH + 0.4f;   // players that walk past this leave the scene

// Characters stand with their feet this far below the lane center.
constexpr float FOOT_OFFSET      = 0.55f;

// ---- Enemies -------------------------------------------------------------
constexpr float ENEMY_SPAWN_X    = WORLD_WIDTH + 0.4f;   // they walk in from just past the right edge
constexpr float ENEMY_SIZE       = 1.0f;
constexpr float BASE_REACHED_X   = HOME_BASE_RIGHT + 0.1f;  // an enemy here has reached the koshk

// Player and enemy "collide" (stop and fight) when they are this close.
constexpr float CONTACT_DIST     = 0.8f;

// ---- Gameplay types ------------------------------------------------------
enum class PlayerType
{
    Melee,      // El Bawab: the doorman with his wooden nabboot stick
    Ranged,     // Bayya' el Tamatem: the tomato seller
    Defense     // El Me'allem: the mechanic behind a metal shutter shield
};

enum class EnemyType
{
    EasyRanged,     // slipper thrower (attack from above -> duck)
    MediumRanged,   // slingshot kid (attack from below -> jump)
    HardMelee       // knife thug (runs, close range, 2 damage)
};

// ---- Player stats (spec 3.2) ---------------------------------------------
constexpr int   PLAYER_MAX_HP[3]   = { 5, 5, 7 };          // Melee, Ranged, Defense
constexpr float PLAYER_SPEED[3]    = { 0.9f, 0.8f, 0.0f };
constexpr float PLAYER_COOLDOWN[3] = { 0.5f, 1.0f, 0.0f }; // seconds between attacks (after the 0.7 s animation)
constexpr int   PLAYER_DAMAGE      = 1;                    // every attack deals 1 damage

// Coins. The game starts with enough for three Melee players. A card is
// unlocked while the coins are at or above its cost and locks again below it.
//   Melee 10 | Ranged 2 x 20 = 40 | Defense 3 x 20 = 60
constexpr int   START_COINS        = 30;
constexpr int   UNLOCK_BASE        = 20;
constexpr int   PLAYER_COST[3]     = { 10, 2 * UNLOCK_BASE, 3 * UNLOCK_BASE };

// ---- Enemy stats (spec 3.3) ----------------------------------------------
constexpr int   ENEMY_MAX_HP[3]    = { 5, 5, 7 };          // Easy, Medium, Hard
constexpr int   ENEMY_DAMAGE[3]    = { 1, 1, 2 };
constexpr float ENEMY_SPEED[3]     = { 0.7f, 0.7f, 1.35f };   // the knife thug runs
constexpr int   ENEMY_COIN[3]      = { 10, 15, 25 };          // pounds dropped when defeated

// ---- Waves (spec 3.3) ----------------------------------------------------
constexpr int   NUM_WAVES          = 3;
constexpr int   WAVE_ENEMY_COUNT[NUM_WAVES] = { 3, 5, 10 };

// ---- Timing --------------------------------------------------------------
constexpr float JUMP_TIME          = 0.75f;
constexpr float DUCK_TIME          = 0.75f;
