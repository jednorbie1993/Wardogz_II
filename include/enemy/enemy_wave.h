#ifndef ENEMY_WAVE_H
#define ENEMY_WAVE_H

#include "raylib.h"
#include "player.h"
#include "enemy.h"

// ============================================================
// 0074 - STAGE 1 ENEMY WAVE MANAGER
// ============================================================
// Normal Stage 1 enemy encounters before the Vargas sequence.
//
// v1 behavior:
// - Wave 1 triggers at 9% of the stage.
// - One Hooligan appears from off-screen ahead of the player.
// - Forward progress is locked while the encounter is active.
// - When the enemy is defeated, the stage unlocks.
// - The structure is ready for later 18%, 27%, ... waves.

typedef struct EnemyWaveSystem
{
    int currentWave;

    bool waveActive;
    bool waveFinished;

    float fightBoundaryX;

    int activeEnemyCount;
    int activeEnemyStartIndex;

} EnemyWaveSystem;


// Start with no active normal-enemy encounter.
EnemyWaveSystem InitEnemyWaveSystem(void);


// Update Stage 1 wave triggers, stage lock, and encounter completion.
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


// Number of normal enemies currently owned by the wave system.
int GetEnemyWaveActiveCount(const EnemyWaveSystem *waves);


// Index of the first active normal enemy in the shared enemy array.
int GetEnemyWaveStartIndex(const EnemyWaveSystem *waves);


// True while Jamber is locked inside a normal-enemy encounter.
bool IsEnemyWaveActive(const EnemyWaveSystem *waves);

#endif