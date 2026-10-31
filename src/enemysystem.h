#pragma once

#include "GameConfig.h"

namespace EnemySystem
{
    void Init();
    void SpawnEnemy(EnemyType type);
    void Update(float deltaTime);
    void Render();
    int GetEnemyCount();
    void Shutdown();
}