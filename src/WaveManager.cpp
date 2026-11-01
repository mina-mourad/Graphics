
#include "WaveManager.h"
#include "EnemySystem.h"
#include "GameConfig.h"

namespace WaveManager
{
    int currentWave = 1;

    int spawnedEnemies = 0;

    int enemiesPerWave[] = { 3, 5, 10 };

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
        if (currentWave == 1)
        {
            return EnemyType::EasyRanged;
        }

        if (currentWave == 2)
        {
            if (spawnedEnemies % 2 == 0)
                return EnemyType::EasyRanged;
            else
                return EnemyType::MediumRanged;
        }

        // Wave 3: all three enemy types.
        if (spawnedEnemies % 3 == 0)
            return EnemyType::EasyRanged;

        if (spawnedEnemies % 3 == 1)
            return EnemyType::MediumRanged;

        return EnemyType::HardMelee;
    }

    void Update(float deltaTime)
    {
        // Stop when all three waves are complete.
        if (currentWave > 3)
            return;

        spawnTimer += deltaTime;

        // Spawn enemies until this wave reaches its quota.
        if (spawnedEnemies < enemiesPerWave[currentWave - 1]
            && spawnTimer >= SPAWN_INTERVAL)
        {
            spawnTimer = 0.0f;

            EnemyType type = ChooseEnemyType();

            EnemySystem::SpawnEnemy(type);

            spawnedEnemies++;
        }

        // Start the next wave when all enemies from this wave are gone.
        if (spawnedEnemies == enemiesPerWave[currentWave - 1]
            && EnemySystem::GetEnemyCount() == 0)
        {
            currentWave++;

            spawnedEnemies = 0;
            spawnTimer = 0.0f;
        }
    }

    void Shutdown()
    {
        currentWave = 1;
        spawnedEnemies = 0;
        spawnTimer = 0.0f;
    }
}
