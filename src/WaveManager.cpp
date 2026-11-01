
#include "WaveManager.h"
#include "EnemySystem.h"
#include "GameConfig.h"

namespace WaveManager
{
    int currentWave = 1;

    int spawnedEnemies = 0;

    const int enemiesPerWave[] = { 3, 5, 10 };

    float spawnTimer = 0.0f;

    const float SPAWN_INTERVAL = 2.0f;

    // Starts the wave system.
    void Init()
    {
        currentWave = 1;
        spawnedEnemies = 0;
        spawnTimer = 0.0f;
    }

    // Selects an enemy type based on the current wave.
    EnemyType ChooseEnemyType()
    {
        // Wave 1: Easy enemies only.
        if (currentWave == 1)
        {
            return EnemyType::EasyRanged;
        }

        // Wave 2: Easy and Medium enemies.
        if (currentWave == 2)
        {
            if (spawnedEnemies % 2 == 0)
                return EnemyType::EasyRanged;

            return EnemyType::MediumRanged;
        }

        // Wave 3: Easy, Medium, and Hard enemies.
        if (spawnedEnemies % 3 == 0)
            return EnemyType::EasyRanged;

        if (spawnedEnemies % 3 == 1)
            return EnemyType::MediumRanged;

        return EnemyType::HardMelee;
    }

    // Updates wave spawning and progression.
    void Update(float deltaTime)
    {
        // All three waves are complete.
        if (currentWave > 3)
            return;

        // Count the time between enemy spawns.
        spawnTimer += deltaTime;

        // Spawn the next enemy when enough time has passed.
        if (spawnedEnemies < enemiesPerWave[currentWave - 1]
            && spawnTimer >= SPAWN_INTERVAL)
        {
            spawnTimer = 0.0f;

            EnemyType type = ChooseEnemyType();

            EnemySystem::SpawnEnemy(type);

            spawnedEnemies++;
        }

        // Wait until all spawned enemies are gone.
        if (spawnedEnemies == enemiesPerWave[currentWave - 1]
            && EnemySystem::GetEnemyCount() == 0)
        {
            currentWave++;

            spawnedEnemies = 0;
            spawnTimer = 0.0f;
        }
    }

    // Resets the wave system.
    void Shutdown()
    {
        currentWave = 1;
        spawnedEnemies = 0;
        spawnTimer = 0.0f;
    }
}
