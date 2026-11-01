
#include "EnemySystem.h"
#include "GameConfig.h"
#include "Renderer2D.h"

#include <glm/glm.hpp>
#include <cstdlib>
#include <ctime>
#include <vector>

namespace EnemySystem {

    // Information stored for every enemy.
    struct Enemy {
        float x;
        int lane;
        float speed;
        EnemyType type;
        int health;
        int damage;
    };

    // Stores all enemies currently in the game.
    std::vector<Enemy> enemies;

    // Returns the number of enemies currently in the game.
    int GetEnemyCount()
    {
        return static_cast<int>(enemies.size());
    }

    // Configures an enemy based on its type.
    void ConfigureEnemy(Enemy& enemy, EnemyType type)
    {
        enemy.type = type;

        switch (type)
        {
            case EnemyType::EasyRanged:
                enemy.health = 5;
                enemy.damage = 1;
                enemy.speed = 2.0f;
                break;

            case EnemyType::MediumRanged:
                enemy.health = 5;
                enemy.damage = 1;
                enemy.speed = 2.0f;
                break;

            case EnemyType::HardMelee:
                enemy.health = 7;
                enemy.damage = 2;
                enemy.speed = 3.0f;
                break;
        }
    }

    // Initializes the enemy system.
    void Init()
    {
        enemies.clear();

        std::srand(static_cast<unsigned int>(std::time(nullptr)));
    }

    // Creates an enemy when requested by the Wave Manager.
    void SpawnEnemy(EnemyType type)
    {
        Enemy enemy;

        // Spawn on the right side.
        enemy.x = WORLD_WIDTH - 0.5f;

        // Choose a random lane.
        enemy.lane = std::rand() % NUM_LANES;

        // Set health, damage, speed, and type.
        ConfigureEnemy(enemy, type);

        // Add the enemy to the list.
        enemies.push_back(enemy);
    }

    // Updates enemy movement.
    void Update(float deltaTime)
    {
        // Move every enemy toward the left.
        for (auto& enemy : enemies)
        {
            enemy.x -= enemy.speed * deltaTime;
        }

        // Remove enemies that reach the home base.
        for (auto it = enemies.begin(); it != enemies.end();)
        {
            if (it->x <= 0.5f)
            {
                it = enemies.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    // Draws all enemies.
    void Render()
    {
        for (const auto& enemy : enemies)
        {
            float y = LANE_Y[enemy.lane];

            glm::vec3 color;

            switch (enemy.type)
            {
                case EnemyType::EasyRanged:
                    color = glm::vec3(0.2f, 0.8f, 0.2f);
                    break;

                case EnemyType::MediumRanged:
                    color = glm::vec3(0.2f, 0.4f, 0.9f);
                    break;

                case EnemyType::HardMelee:
                    color = glm::vec3(0.8f, 0.1f, 0.1f);
                    break;
            }

            Renderer2D::DrawQuad(
                glm::vec2(enemy.x, y),
                glm::vec2(0.7f, 0.7f),
                color
            );
        }
    }

    // Cleans up the enemy system.
    void Shutdown()
    {
        enemies.clear();
    }

}
