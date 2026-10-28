#pragma once

// Shared world/coordinate-system constants and gameplay enums.
//
// World units, not pixels. Renderer2D maps WORLD_WIDTH x WORLD_HEIGHT onto the
// window. (0,0) is the bottom-left corner of the world; +Y is up.
//
// Layout of the 16 x 9 world (see the spec, sections 3.1 and 4.2):
//
//   y = 9.0  +--------------------------------------------------+
//            |  HUD band: player cards (left), wave, coins (right)|
//   y = 6.8  +----+---------------------------------------------+
//            |HOME|  lane 3  (top)                              |
//            |BASE|  lane 2                                     |  <- players
//            |    |  lane 1                                     |     walk right,
//            |    |  lane 0  (bottom)                           |     enemies left
//   y = 0.8  +----+---------------------------------------------+
//            |  foreground strip (rocks, cacti, torches)         |
//   y = 0.0  +--------------------------------------------------+
//            x=0  x=1.3                                       x=16

constexpr float WORLD_WIDTH  = 16.0f;
constexpr float WORLD_HEIGHT = 9.0f;

// ---- Lanes --------------------------------------------------------------
constexpr int   NUM_LANES        = 4;                 // spec: at least 3
constexpr float LANE_HEIGHT      = 1.5f;
constexpr float PLAYFIELD_BOTTOM = 0.8f;
constexpr float PLAYFIELD_TOP    = PLAYFIELD_BOTTOM + NUM_LANES * LANE_HEIGHT;   // 6.8
constexpr float LANE_Y[NUM_LANES] = { 1.55f, 3.05f, 4.55f, 6.05f };               // lane centres

// ---- Horizontal layout --------------------------------------------------
constexpr float HOME_BASE_RIGHT  = 1.3f;     // right edge of the home base (left end of every lane)
constexpr float PLAYER_SPAWN_X   = 1.75f;    // players appear at the leftmost spot of the lane
constexpr float ENEMY_SPAWN_X    = 16.5f;    // enemies walk in from just past the right edge
constexpr float LANE_RIGHT_END   = 16.2f;    // a player that walks past this has reached the end
constexpr float BASE_REACHED_X   = 1.45f;    // an enemy at or left of this x has reached the home base

// ---- Characters ---------------------------------------------------------
constexpr float FOOT_OFFSET      = 0.55f;    // feet are this far below the lane centre
constexpr float CONTACT_DIST     = 0.85f;    // player and enemy "collide" when this close

enum class PlayerType { Melee, Ranged, Defense };
enum class EnemyType  { EasyRanged, MediumRanged, HardMelee };

// ---- Player stats (spec section 3.2, table on page 9) -------------------
constexpr int   PLAYER_MAX_HP[3]   = { 5, 5, 7 };          // Melee, Ranged, Defense
constexpr float PLAYER_SPEED[3]    = { 0.95f, 0.85f, 0.0f };
constexpr float PLAYER_COOLDOWN[3] = { 1.1f, 1.6f, 0.0f }; // seconds between attacks (Defense never attacks)
constexpr int   PLAYER_DAMAGE      = 1;                    // every attacking player deals 1 per attack
constexpr float MELEE_REACH        = 0.95f;

// Coins: the game starts with enough for three Melee players. Every card is
// "unlocked" while the coin total is at or above its cost and falls back to
// locked (grayscale) when the total drops below it.
//   Melee 10 | Ranged 2 x 20 = 40 | Defense 3 x 20 = 60   (start: 30 coins)
constexpr int   START_COINS        = 30;
constexpr int   UNLOCK_BASE        = 20;
constexpr int   PLAYER_COST[3]     = { 10, 2 * UNLOCK_BASE, 3 * UNLOCK_BASE };

// ---- Enemy stats (spec section 3.3) -------------------------------------
constexpr int   ENEMY_MAX_HP[3]    = { 5, 5, 7 };          // Easy, Medium, Hard
constexpr float ENEMY_SPEED[3]     = { 0.75f, 0.75f, 1.55f };  // Hard melee enemy runs
constexpr int   ENEMY_DAMAGE[3]    = { 1, 1, 2 };
constexpr float ENEMY_COOLDOWN[3]  = { 2.1f, 2.3f, 1.1f };
constexpr int   ENEMY_COIN[3]      = { 10, 15, 25 };

// ---- Waves (spec section 3.3) -------------------------------------------
constexpr int   NUM_WAVES          = 3;
constexpr int   WAVE_ENEMY_COUNT[NUM_WAVES] = { 4, 6, 12 };   // minimums are 3, 5, 10

// ---- Timing -------------------------------------------------------------
constexpr float JUMP_TIME          = 0.75f;
constexpr float DUCK_TIME          = 0.75f;
constexpr float ATTACK_WINDUP      = 0.40f;   // length of the attack animation
