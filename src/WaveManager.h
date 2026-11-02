#pragma once

namespace WaveManager
{
    void Init();

    void Update(float deltaTime);

    // 1, 2 or 3 while playing (4 once every wave is done).
    int GetCurrentWave();

    // Enemies of the current wave that have not appeared yet.
    int GetEnemiesLeftToSpawn();

    // Seconds left in the short break before the next wave (0 when none).
    float GetBreakTime();

    // True when all three waves have spawned and every enemy is defeated.
    bool AllWavesCleared();

    void Shutdown();
}
