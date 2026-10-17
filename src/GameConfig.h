#pragma once

// Shared world/coordinate-system constants and gameplay enums.
//
// Every system (towers, enemies, UI) should read positions and sizes from
// here rather than hardcoding its own numbers, so everyone's code agrees on
// the same world. None of these need to be final values -- they just need to
// exist and be the ONE place the whole team edits when they change.

// World units, not pixels -- Renderer2D maps this onto whatever the actual
// window/screen size is, so gameplay code never has to think in pixels.
constexpr float WORLD_WIDTH  = 16.0f;
constexpr float WORLD_HEIGHT = 9.0f;

constexpr int   NUM_LANES = 3;
constexpr float LANE_Y[NUM_LANES] = { 1.5f, 4.5f, 7.5f };   // placeholder lane Y positions, tune later
constexpr float LANE_START_X = 0.0f;                         // where enemies spawn (right edge if the lane runs right-to-left)
constexpr float LANE_END_X   = WORLD_WIDTH;                  // where an enemy "reaches the base" / triggers a loss of health

constexpr float TILE_SIZE = 1.0f;   // size of one tower-placement slot, in world units

enum class PlayerType { Melee, Ranged, Defense };
enum class EnemyType  { EasyRanged, MediumRanged, HardMelee };
