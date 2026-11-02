#include "WaveManager.h"
#include "EnemySystem.h"
#include "GameConfig.h"
#include "Audio.h"

#include <cstdlib>

namespace WaveManager
{
    int currentWave = 1;

    int spawnedEnemies = 0;

    float spawnTimer = 0.0f;

    // A short break before each wave, so the WAVE banner can be read.
    const float WAVE_BREAK = 3.0f;
    float breakTimer = 0.0f;

    // Time between two enemies of the same wave: random inside a range that
    // gets shorter every wave (waves get harder).
    const float SPAWN_MIN[NUM_WAVES] = { 4.5f, 4.0f, 3.2f };
    const float SPAWN_MAX[NUM_WAVES] = { 6.5f, 5.5f, 4.6f };

    float RandomRange(float lo, float hi)
    {
        return lo + (hi - lo) * static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
    }

    void Init()
    {
        currentWave = 1;
        spawnedEnemies = 0;
        spawnTimer = 0.0f;
        breakTimer = WAVE_BREAK;
    }

    // Selects an enemy type based on the current wave.
    EnemyType ChooseEnemyType()
    {
        // Wave 1: Easy enemies only.
        if (currentWave == 1)
            return EnemyType::EasyRanged;

        // Wave 2: Easy and Medium enemies.
        if (currentWave == 2)
            return (spawnedEnemies % 2 == 0) ? EnemyType::EasyRanged : EnemyType::MediumRanged;

        // Wave 3: Easy, Medium, and Hard enemies.
        if (spawnedEnemies % 3 == 0)
            return EnemyType::EasyRanged;
        if (spawnedEnemies % 3 == 1)
            return EnemyType::MediumRanged;
        return EnemyType::HardMelee;
    }

    void Update(float deltaTime)
    {
        if (currentWave > NUM_WAVES)
            return;

        // Break before the wave: the wave begins with its first enemy.
        if (breakTimer > 0.0f)
        {
            breakTimer -= deltaTime;
            if (breakTimer <= 0.0f)
            {
                spawnTimer = 0.0f;
                Audio::Play(Audio::Sfx::WaveHorn);   // a tuk-tuk horn announces the gang
            }
            return;
        }

        const int quota = WAVE_ENEMY_COUNT[currentWave - 1];

        // Spawn the next enemy when its time has come. The wave ends as soon
        // as its last enemy has appeared.
        spawnTimer -= deltaTime;
        if (spawnedEnemies < quota && spawnTimer <= 0.0f)
        {
            EnemySystem::SpawnEnemy(ChooseEnemyType());
            spawnedEnemies++;
            spawnTimer = RandomRange(SPAWN_MIN[currentWave - 1], SPAWN_MAX[currentWave - 1]);
        }

        // Next wave once every enemy of this wave is defeated.
        if (spawnedEnemies == quota && EnemySystem::GetEnemyCount() == 0)
        {
            currentWave++;
            spawnedEnemies = 0;
            if (currentWave <= NUM_WAVES)
                breakTimer = WAVE_BREAK;
        }
    }

    int GetCurrentWave() { return currentWave; }

    int GetEnemiesLeftToSpawn()
    {
        if (currentWave > NUM_WAVES) return 0;
        return WAVE_ENEMY_COUNT[currentWave - 1] - spawnedEnemies;
    }

    float GetBreakTime() { return breakTimer > 0.0f ? breakTimer : 0.0f; }

    bool AllWavesCleared() { return currentWave > NUM_WAVES && EnemySystem::GetEnemyCount() == 0; }

    void Shutdown()
    {
        currentWave = 1;
        spawnedEnemies = 0;
        spawnTimer = 0.0f;
        breakTimer = 0.0f;
    }
}
