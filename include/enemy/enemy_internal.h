#ifndef ENEMY_INTERNAL_H
#define ENEMY_INTERNAL_H

#include "enemy.h"

typedef struct EnemyCombatContext
{
    Rectangle enemyHurtbox;
    Rectangle playerHurtbox;

    float enemyCenterX;
    float playerCenterX;

    float distanceX;
    float absoluteDistanceX;

    Rectangle enemyFeet;
    Rectangle playerFeet;

    float enemyGroundY;
    float playerGroundY;

    float depthDifference;
    float absoluteDepthDifference;

    float aggroDistance;
    bool playerDetected;

} EnemyCombatContext;


// ============================================================
// 0068 - OPTIONAL LOW-HP ENRAGE PHASE
// ============================================================

static inline bool EnemyIsEnraged(const Enemy *enemy)
{
    return
        enemy->isAlive &&
        enemy->hp > 0 &&
        enemy->enrageHpThreshold > 0 &&
        enemy->hp <= enemy->enrageHpThreshold;
}

static inline float EnemyGetMovementSpeedMultiplier(const Enemy *enemy)
{
    if (
        EnemyIsEnraged(enemy) &&
        enemy->enrageMovementSpeedMultiplier > 0.0f
    )
    {
        return enemy->enrageMovementSpeedMultiplier;
    }

    return 1.0f;
}

static inline float EnemyGetAnimationSpeedMultiplier(const Enemy *enemy)
{
    if (
        EnemyIsEnraged(enemy) &&
        enemy->enrageAnimationSpeedMultiplier > 0.0f
    )
    {
        return enemy->enrageAnimationSpeedMultiplier;
    }

    return 1.0f;
}


// ============================================================
// COLLISION / DEPTH / SCALE
// ============================================================

float EnemyGetPerspectiveScale(const Enemy *enemy);
Rectangle EnemyGetScaledHurtbox(const Enemy *enemy);

EnemyCombatContext EnemyBuildCombatContext(
    const Enemy *enemy,
    const Player *player
);

bool EnemyIsPlayerInVerticalRange(
    const Enemy *enemy,
    const Player *player
);

bool EnemyIsSameGroundDepth(
    const Enemy *enemy,
    const Player *player
);


// ============================================================
// MOVEMENT / AI
// ============================================================

bool EnemyUpdateEntrance(
    Enemy *enemy,
    float deltaTime
);

void EnemyClampToStage(
    Enemy *enemy,
    float screenWidth,
    float walkAreaTop,
    float walkAreaBottom
);

void EnemyUpdateChase(
    Enemy *enemy,
    const Player *player,
    float deltaTime,
    float screenWidth,
    float walkAreaTop,
    float walkAreaBottom,
    EnemyCombatContext *context
);


// ============================================================
// ANIMATION
// ============================================================

void EnemyUpdateAnimation(
    Enemy *enemy,
    float deltaTime,
    bool isWalking
);


// ============================================================
// ATTACK
// ============================================================

void EnemyUpdateAttack(
    Enemy *enemy,
    Player *player,
    float deltaTime,
    const EnemyCombatContext *context
);


// ============================================================
// HIT / DAMAGE
// ============================================================

void EnemyUpdateHitReaction(
    Enemy *enemy,
    float deltaTime,
    float screenWidth,
    float walkAreaTop,
    float walkAreaBottom
);

// 0050 - Update death freeze, 2-frame death sprite sequence, and body hold.
void EnemyUpdateDeath(
    Enemy *enemy,
    float deltaTime,
    float screenWidth,
    float walkAreaTop,
    float walkAreaBottom
);

void EnemyCheckPlayerAttack(
    Enemy *enemy,
    Player *player
);

#endif