
#include "EnemySystem.h"
#include "GameConfig.h"
#include "Renderer2D.h"

#include <glm/glm.hpp>
#include <cstdlib>
#include <ctime>
#include <vector>

namespace EnemySystem {

    struct Enemy {
        float x;
        int lane;
        float speed;
        EnemyType type;
    int health;
    };

    std::vector<Enemy> enemies;
    float spawnTimer = 0.0f;

    void Init() {
        enemies.clear();
        spawnTimer = 0.0f;
        std::srand(static_cast<unsigned int>(std::time(nullptr)));
    }

    void Update(float deltaTime) {
        spawnTimer += deltaTime;

        // Spawn an enemy every 2 seconds.
        if (spawnTimer >= 2.0f) {
            spawnTimer = 0.0f;

            Enemy enemy;
            enemy.x = WORLD_WIDTH - 0.5f;
            enemy.lane = std::rand() % NUM_LANES;
            enemy.speed = 2.0f;

            enemies.push_back(enemy);
        }

        // Move every enemy toward the home base on the left.
        for (auto& enemy : enemies) {
            enemy.x -= enemy.speed * deltaTime;
        }

        // Remove enemies that reach the home base.
        for (auto it = enemies.begin(); it != enemies.end();) {
            if (it->x <= 0.5f) {
                it = enemies.erase(it);
            } else {
                ++it;
            }
        }
    }

    void Render() {
        for (const auto& enemy : enemies) {
            float y = LANE_Y[enemy.lane];

            Renderer2D::DrawQuad(
                glm::vec2(enemy.x, y),
                glm::vec2(0.7f, 0.7f),
                glm::vec3(0.9f, 0.2f, 0.2f)
            );
        }
    }

    void Shutdown() {
        enemies.clear();
    }

}
