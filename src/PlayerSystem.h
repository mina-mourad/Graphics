#pragma once

#include "GameConfig.h"

// Person 2: Player Systems.
//
// Lane selection (hover + click toggle, several lanes at once), placing
// players from the cards, and everything the three player types do:
// walking, attacking, jumping / ducking, taking damage and being removed.
namespace PlayerSystem
{
    void Init();
    void Shutdown();

    // ---- Lanes -------------------------------------------------------------
    // Reads the mouse: hovering highlights a lane, a left click toggles it.
    // Returns true if the click was used by a lane.
    bool UpdateLanes();
    int  GetHoveredLane();             // -1 when the mouse is not over a lane
    bool IsLaneSelected(int lane);
    int  GetSelectedLaneCount();
    float GetLaneHighlight(int lane);  // value for Renderer2D::SetHighlight

    // ---- Placement ---------------------------------------------------------
    // Places one `type` player at the left end of every selected lane, paying
    // for each one. Returns how many were placed.
    int PlaceOnSelectedLanes(PlayerType type);

    // ---- Per frame -----------------------------------------------------------
    // W / Up: jump, S / Down: duck (players in the selected lanes).
    void HandleKeys();

    // Tells the enemy system where every player is. Call before
    // EnemySystem::Update().
    void ShareWithEnemies();

    // Applies the enemy hits, moves and animates every player, fires
    // attacks, collects coins. Call after EnemySystem::Update().
    void Update(float deltaTime);

    void RenderLane(int lane);
    void RenderProjectiles();

    int GetPlayerCount();
    int GetPlayerCountInLane(int lane, bool includeDefense);
}
