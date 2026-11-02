#pragma once

#include <glm/glm.hpp>

// Coins (Egyptian pound coins). The player starts with START_COINS, pays
// for every defender placed, and earns coins dropped by defeated enemies.
// Dropped coins lie in the lane until a player walks into them.
namespace Economy
{
    void Init();

    int  GetCoins();
    bool CanAfford(int cost);
    void Spend(int cost);

    // A defeated enemy drops `value` coins at (lane, x).
    void DropCoins(int lane, float x, int value);

    // Picks up every dropped coin a player at (lane, x) is touching.
    void CollectAt(int lane, float x);

    void Update(float deltaTime);
    void RenderLane(int lane);

    // Draws one coin (also used by the HUD). `spin` 1 = facing us, 0 = edge on.
    void DrawCoin(glm::vec2 center, float radius, float spin = 1.0f);
}
