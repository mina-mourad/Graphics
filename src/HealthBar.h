#pragma once

#include <glm/glm.hpp>

// Health display drawn under every player and enemy: a dark frame with one
// block per life, so it is clear at a glance how many hits are left.
namespace HealthBar
{
    // `center` = middle of the bar. Players get green blocks, enemies red.
    void Draw(glm::vec2 center, int health, int maxHealth, bool isPlayer);
}
