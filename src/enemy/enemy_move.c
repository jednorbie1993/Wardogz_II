#include "enemy_internal.h"


static float GetNextEnemyAttackStopDistance(
    const Enemy *enemy
)
{
    if (enemy->nextAttackMove == ENEMY_ATTACK_ELBOW)
    {
        return enemy->elbowStopDistance;
    }

    return enemy->punchStopDistance;
}


void StartEnemyEntrance(
    Enemy *enemy,
    float targetX,
    float targetStageY,
    float entranceSpeed
)
{
    enemy->isEntering = true;
    enemy->hasEnteredStage = false;

    enemy->entranceTargetX = targetX;

    enemy->entranceTargetY =
        targetStageY -
        enemy->stageAnchorOffsetY;

    enemy->entranceSpeed = entranceSpeed;

    enemy->isChasing = false;
    enemy->isAttacking = false;
    enemy->attackTimer = 0.0f;
    enemy->hitPlayerThisAttack = false;
    enemy->attackFrame = 0;
    enemy->attackFrameTimer = 0.0f;
}


bool EnemyUpdateEntrance(
    Enemy *enemy,
    float deltaTime
)
{
    if (!enemy->isEntering)
    {
        return true;
    }

    float differenceX =
        enemy->entranceTargetX -
        enemy->hurtbox.x;

    float differenceY =
        enemy->entranceTargetY -
        enemy->hurtbox.y;

    float absoluteX =
        (differenceX < 0.0f)
        ? -differenceX
        : differenceX;

    float absoluteY =
        (differenceY < 0.0f)
        ? -differenceY
        : differenceY;

    const float arrivalDistance = 3.0f;

    if (
        absoluteX <= arrivalDistance &&
        absoluteY <= arrivalDistance
    )
    {
        enemy->hurtbox.x = enemy->entranceTargetX;
        enemy->hurtbox.y = enemy->entranceTargetY;

        enemy->isEntering = false;
        enemy->hasEnteredStage = true;
        enemy->isChasing = false;

        return true;
    }

    float moveX = 0.0f;
    float moveY = 0.0f;

    if (absoluteX > arrivalDistance)
        moveX = (differenceX > 0.0f) ? 1.0f : -1.0f;

    if (absoluteY > arrivalDistance)
        moveY = (differenceY > 0.0f) ? 1.0f : -1.0f;

    if (moveX != 0.0f && moveY != 0.0f)
    {
        const float diagonalFactor = 0.70710678f;
        moveX *= diagonalFactor;
        moveY *= diagonalFactor;
    }

    enemy->hurtbox.x +=
        moveX *
        enemy->entranceSpeed *
        deltaTime;

    enemy->hurtbox.y +=
        moveY *
        enemy->entranceSpeed *
        deltaTime;

    if (
        (differenceX > 0.0f &&
         enemy->hurtbox.x > enemy->entranceTargetX) ||
        (differenceX < 0.0f &&
         enemy->hurtbox.x < enemy->entranceTargetX)
    )
    {
        enemy->hurtbox.x = enemy->entranceTargetX;
    }

    if (
        (differenceY > 0.0f &&
         enemy->hurtbox.y > enemy->entranceTargetY) ||
        (differenceY < 0.0f &&
         enemy->hurtbox.y < enemy->entranceTargetY)
    )
    {
        enemy->hurtbox.y = enemy->entranceTargetY;
    }

    if (moveX > 0.0f)
        enemy->facingRight = true;
    else if (moveX < 0.0f)
        enemy->facingRight = false;

    return false;
}


void EnemyClampToStage(
    Enemy *enemy,
    float screenWidth,
    float walkAreaTop,
    float walkAreaBottom
)
{
    if (enemy->hurtbox.x < 0.0f)
    {
        enemy->hurtbox.x = 0.0f;
    }

    if (
        enemy->hurtbox.x +
        enemy->hurtbox.width >
        screenWidth
    )
    {
        enemy->hurtbox.x =
            screenWidth -
            enemy->hurtbox.width;
    }

    float stageY =
        enemy->hurtbox.y +
        enemy->stageAnchorOffsetY;

    const float enemyTopExtra = 30.0f;

    float enemyWalkAreaTop =
        walkAreaTop -
        enemyTopExtra;

    if (stageY < enemyWalkAreaTop)
    {
        enemy->hurtbox.y =
            enemyWalkAreaTop -
            enemy->stageAnchorOffsetY;
    }

    if (stageY > walkAreaBottom)
    {
        enemy->hurtbox.y =
            walkAreaBottom -
            enemy->stageAnchorOffsetY;
    }
}


void EnemyUpdateChase(
    Enemy *enemy,
    const Player *player,
    float deltaTime,
    float screenWidth,
    float walkAreaTop,
    float walkAreaBottom,
    EnemyCombatContext *context
)
{
    // Face the player whenever Punk is free to react.
    if (!enemy->isAttacking && !enemy->isHit)
    {
        enemy->facingRight =
            (context->distanceX >= 0.0f);
    }

    enemy->attackDirection =
        (context->distanceX >= 0.0f)
        ? 1
        : -1;

    float currentStopDistance =
        GetNextEnemyAttackStopDistance(enemy);

    enemy->isInAttackRange =
        context->playerDetected &&
        player->isAlive &&
        context->absoluteDistanceX <= currentStopDistance &&
        context->absoluteDepthDifference <= enemy->chaseDepthTolerance;

    enemy->isChasing = false;

    if (
        !enemy->isAttacking &&
        !enemy->isHit &&
        player->isAlive &&
        context->playerDetected &&
        !enemy->isInAttackRange
    )
    {
        float moveX = 0.0f;
        float moveY = 0.0f;

        if (
            context->absoluteDistanceX >
            currentStopDistance
        )
        {
            moveX =
                (context->distanceX > 0.0f)
                ? 1.0f
                : -1.0f;
        }

        if (
            context->absoluteDepthDifference >
            enemy->chaseDepthTolerance
        )
        {
            moveY =
                (context->depthDifference > 0.0f)
                ? 1.0f
                : -1.0f;
        }

        if (moveX != 0.0f || moveY != 0.0f)
        {
            enemy->isChasing = true;

            if (moveX != 0.0f && moveY != 0.0f)
            {
                const float diagonalFactor = 0.70710678f;
                moveX *= diagonalFactor;
                moveY *= diagonalFactor;
            }

            enemy->hurtbox.x +=
                moveX *
                enemy->chaseSpeed *
                deltaTime;

            enemy->hurtbox.y +=
                moveY *
                enemy->chaseSpeed *
                deltaTime;

            EnemyClampToStage(
                enemy,
                screenWidth,
                walkAreaTop,
                walkAreaBottom
            );

            // Refresh all combat distances after movement.
            *context =
                EnemyBuildCombatContext(
                    enemy,
                    player
                );

            enemy->isInAttackRange =
                context->absoluteDistanceX <= currentStopDistance &&
                context->absoluteDepthDifference <= enemy->chaseDepthTolerance;

            enemy->attackDirection =
                (context->distanceX >= 0.0f)
                ? 1
                : -1;

            enemy->facingRight =
                (context->distanceX >= 0.0f);
        }
    }
}