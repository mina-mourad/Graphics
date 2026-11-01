
#pragma once

#include "GameConfig.h"

#include <glm/glm.hpp>

// Draws the three enemy characters using simple shapes (no image files).
// Every enemy has three animations: Run, Attack and Idle.
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
}
