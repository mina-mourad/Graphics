#pragma once

#include "GameConfig.h"

#include <glm/glm.hpp>
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
        int playerId;
        int damage;
        AttackHeight height;
    };

    // One enemy defeated during the last Update() (it drops coins here).
    struct Defeated
    {
        int lane;
        float x;
        EnemyType type;
    };

    void Init();

    void SpawnEnemy(EnemyType type);

    void Update(float deltaTime);

    // Draws the enemies of one lane (the game draws lanes back to front),
    // and all flying projectiles.
    void RenderLane(int lane);
    void RenderProjectiles();

    int GetEnemyCount();
    int GetEnemyCountInLane(int lane);

    // True once an enemy has walked into the koshk (the game is lost).
    bool ReachedBase();

    void Shutdown();

    // --- Connection to the player system --------------------------------
    // Every frame, BEFORE EnemySystem::Update(), the player system calls
    // ClearPlayers() and then AddPlayer() once for each player on the field.
    // Enemies stop and fight when they touch a player in their lane, and
    // projectiles fly past a player who is ducking (High) or jumping (Low).
    void ClearPlayers();
    void AddPlayer(int id, int lane, float x, bool ducking, bool jumping, bool isDefense);

    // Attacks that reached a player during the last Update(). The dodge check
    // is already done; the player system only applies the damage.
    const std::vector<EnemyHit>& GetHits();

    // Enemies defeated during the last Update().
    const std::vector<Defeated>& GetDefeated();

    // Player attacks: damages the closest enemy in `lane` whose x is between
    // fromX and toX. Returns true (and where it was hit) if one was found.
    bool HitEnemyInRange(int lane, float fromX, float toX, int damage, glm::vec2* hitPoint = nullptr);

    // True if an enemy in `lane` is touching a player standing at playerX.
    bool EnemyInContact(int lane, float playerX);
}
