#ifndef ENEMY_WAVE_H
#define ENEMY_WAVE_H

#include "raylib.h"
#include "player.h"
#include "enemy.h"

typedef struct EnemyWaveSystem
{
    int currentWave;
    bool waveActive;
    bool waveFinished;
    bool reinforcementSpawned;
    float fightBoundaryX;
    int activeEnemyCount;
    int activeEnemyStartIndex;
} EnemyWaveSystem;

EnemyWaveSystem InitEnemyWaveSystem(void);

void UpdateEnemyWaveSystem(
    EnemyWaveSystem *waves,
    Player *player,
    Enemy *enemies,
    int enemyCapacity,
    float stageWorldWidth,
    const Camera2D *camera,
    int screenWidth,
    float walkAreaTop,
    float walkAreaBottom
);

int GetEnemyWaveActiveCount(const EnemyWaveSystem *waves);
int GetEnemyWaveStartIndex(const EnemyWaveSystem *waves);
bool IsEnemyWaveActive(const EnemyWaveSystem *waves);

void LoadEnemyWaveSharedTextures(void);
void UnloadEnemyWaveSharedTextures(void);

#endif