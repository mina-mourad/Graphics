
#include "EnemySystem.h"
#include "GameConfig.h"
#include "CharacterRenderer.h"

#include <cstdlib>
#include <ctime>
#include <vector>

namespace EnemySystem
{
    // Information stored for every enemy.
    struct Enemy
    {
        float x;
        int lane;
        float speed;
        EnemyType type;
        int health;
        int damage;
    };

    // Stores all active enemies.
    std::vector<Enemy> enemies;

    // Returns the number of active enemies.
    int GetEnemyCount()
    {
        return static_cast<int>(enemies.size());
    }

    // Sets enemy properties according to its type.
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

        // Initialize random number generation.
        std::srand(
            static_cast<unsigned int>(std::time(nullptr))
        );
    }

    // Creates a new enemy.
    void SpawnEnemy(EnemyType type)
    {
        Enemy enemy{};

        // Spawn enemy on the right side.
        enemy.x = ENEMY_SPAWN_X;

        // Select one of the three lanes randomly.
        enemy.lane = std::rand() % NUM_LANES;

        // Set health, damage, speed, and type.
        ConfigureEnemy(enemy, type);

        // Add the enemy to the active enemies list.
        enemies.push_back(enemy);
    }

    // Updates enemy movement.
    void Update(float deltaTime)
    {
        // Move all enemies toward the left.
        for (auto& enemy : enemies)
        {
            enemy.x -= enemy.speed * deltaTime;
        }

        // Remove enemies that reach the base.
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

    // Draws all active enemies.
    void Render()
    {
        for (const auto& enemy : enemies)
        {
            CharacterRenderer::Draw(
                enemy.x,
                LANE_Y[enemy.lane],
                enemy.type
            );
        }
    }

    // Clears enemy data.
    void Shutdown()
    {
        enemies.clear();
    }
}
