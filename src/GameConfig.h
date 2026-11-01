
#pragma once

// Shared world and gameplay constants.

// World dimensions.
constexpr float WORLD_WIDTH  = 16.0f;
constexpr float WORLD_HEIGHT = 9.0f;

// Lane settings.
constexpr int NUM_LANES = 3;

constexpr float LANE_WIDTH = WORLD_WIDTH * 0.9f;
constexpr float LANE_HEIGHT = WORLD_HEIGHT * 0.18f;

constexpr float LANE_Y[NUM_LANES] = {
    1.5f,
    4.5f,
    7.5f
};

constexpr float LANE_START_X = 0.0f;
constexpr float LANE_END_X = WORLD_WIDTH;

// Tower placement.
constexpr float TILE_SIZE = 1.0f;

// Enemy settings.
constexpr float ENEMY_SPAWN_X = WORLD_WIDTH - 0.5f;
constexpr float ENEMY_SIZE = 1.0f;

// Gameplay types.
enum class PlayerType
{
    Melee,
    Ranged,
    Defense
};

enum class EnemyType
{
    EasyRanged,
    MediumRanged,
    HardMelee
};
