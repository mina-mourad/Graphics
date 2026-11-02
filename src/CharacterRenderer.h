
#pragma once

#include "GameConfig.h"

#include <glm/glm.hpp>

// Draws the enemy and player characters using simple shapes (no image files).
// Every enemy has three animations: Run, Attack and Idle; players have Walk,
// Attack and Idle, plus jump and duck.
namespace CharacterRenderer
{
    // Which animation an enemy is playing.
    enum class EnemyAnim
    {
        Run,
        Attack,
        Idle
    };

    // Timing of one attack animation, in seconds. EnemySystem uses these so
    // the slipper / pebble leaves the hand (or the knife lands) on the same
    // frame the drawing shows it.
    constexpr float ATTACK_DURATION = 0.9f;
    constexpr float ATTACK_HIT_TIME = 0.5f;

    // Draws one enemy. (x, y) = enemy x position and the center height of
    // its lane. animTime = seconds since the current animation started.
    void Draw(float x, float y, EnemyType type, EnemyAnim anim, float animTime);

    // World position of the enemy's weapon hand at ATTACK_HIT_TIME.
    // Projectiles start from here.
    glm::vec2 GetAttackPoint(float x, float y, EnemyType type);

    // Projectiles. spin = rotation in radians.
    void DrawSlipper(float x, float y, float spin);
    void DrawPebble(float x, float y);

    // ---- Players (the shabab defending the koshk) ----------------------------
    enum class PlayerAnim
    {
        Walk,
        Attack,
        Idle
    };

    // One player attack: the stick lands / the tomato leaves the hand at
    // PLAYER_HIT_TIME.
    constexpr float PLAYER_ATTACK_DURATION = 0.7f;
    constexpr float PLAYER_HIT_TIME = 0.36f;

    // Draws one player standing in a lane (x = position, laneY = lane center).
    // jumpHeight lifts him off the ground, duck (0..1) crouches him down.
    // walkTime >= 0 while attacking = he keeps walking (legs walk, arms attack).
    void DrawPlayer(float x, float laneY, PlayerType type, PlayerAnim anim, float animTime,
                    float jumpHeight = 0.0f, float duck = 0.0f, float walkTime = -1.0f);

    // Same, with the feet at an exact point and any size (player cards, cutscene).
    void DrawPlayerAt(glm::vec2 feet, float scale, PlayerType type, PlayerAnim anim, float animTime,
                      float jump = 0.0f, float duck = 0.0f, float walkTime = -1.0f);

    // Where a bullet starts (the gunman's pistol muzzle at PLAYER_HIT_TIME).
    glm::vec2 GetGunMuzzle(float x, float laneY);

    void DrawBullet(float x, float y);
    void DrawBulletHit(float x, float y, float t);   // t = 0..1 through the impact
}
