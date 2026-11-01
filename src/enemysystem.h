
#pragma once

#include "GameConfig.h"

#include <vector>

namespace EnemySystem
{
    // How an enemy attack travels, so the player system knows how to dodge it.
    enum class AttackHeight
    {
        High,   // slipper at head height -> the player must DUCK
        Low,    // pebble at foot height  -> the player must JUMP
        Melee   // knife slash up close
    };

    // One attack that reached a player during the last Update().
    struct EnemyHit
    {
        int lane;
        float playerX;       // x position of the player that was hit
        int damage;
        AttackHeight height;
    };

    void Init();

    void SpawnEnemy(EnemyType type);

    void Update(float deltaTime);

    void Render();

    int GetEnemyCount();

    void Shutdown();

    // --- Connection to the player system --------------------------------
    // Every frame, BEFORE EnemySystem::Update(), the player system should
    // call ClearPlayers() and then AddPlayer() once for each player on the
    // field. Enemies stop and attack when a player is in their lane.
    void ClearPlayers();
    void AddPlayer(int lane, float x);

    // Attacks that reached a player during the last Update(). The player
    // system decides if each hit lands (for example, a High attack misses
    // a ducking player) and applies the damage.
    const std::vector<EnemyHit>& GetHits();
}
