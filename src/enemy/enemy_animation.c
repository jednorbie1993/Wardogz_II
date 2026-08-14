#include "enemy_internal.h"


void EnemyUpdateAnimation(
    Enemy *enemy,
    float deltaTime,
    bool isWalking
)
{
    float animationSpeed =
        EnemyGetAnimationSpeedMultiplier(enemy);

    float retreatFrameDuration =
        enemy->retreatFrameTime / animationSpeed;

    float walkFrameDuration =
        enemy->walkFrameTime / animationSpeed;

    float battleIdleFrameDuration =
        enemy->battleIdleFrameTime / animationSpeed;

    float idleFrameDuration =
        enemy->idleFrameTime / animationSpeed;

    if (enemy->isRetreating && enemy->retreatFrameCount > 0)
    {
        enemy->retreatFrameTimer += deltaTime;

        while (enemy->retreatFrameTimer >= retreatFrameDuration)
        {
            enemy->retreatFrameTimer -= retreatFrameDuration;
            enemy->retreatFrame++;

            if (enemy->retreatFrame >= enemy->retreatFrameCount)
            {
                enemy->retreatFrame = enemy->retreatFrameCount - 1;
                break;
            }
        }

        return;
    }

    if (
        isWalking &&
        enemy->walkFrameCount > 0
    )
    {
        enemy->walkTimer += deltaTime;

        if (
            enemy->walkTimer >=
            walkFrameDuration
        )
        {
            enemy->walkTimer -=
                walkFrameDuration;

            enemy->walkFrame++;

            if (
                enemy->walkFrame >=
                enemy->walkFrameCount
            )
            {
                enemy->walkFrame = 0;
            }
        }

        return;
    }

    // Restart the next walk cycle from frame 1.
    enemy->walkFrame = 0;
    enemy->walkTimer = 0.0f;

    if (
        enemy->battleIdleActive &&
        enemy->battleIdleFrameCount > 1
    )
    {
        enemy->battleIdleTimer += deltaTime;

        if (enemy->battleIdleTimer >= battleIdleFrameDuration)
        {
            enemy->battleIdleTimer -= battleIdleFrameDuration;
            enemy->battleIdleFrame += enemy->battleIdleDirection;

            if (enemy->battleIdleFrame >= enemy->battleIdleFrameCount - 1)
            {
                enemy->battleIdleFrame = enemy->battleIdleFrameCount - 1;
                enemy->battleIdleDirection = -1;
            }
            else if (enemy->battleIdleFrame <= 0)
            {
                enemy->battleIdleFrame = 0;
                enemy->battleIdleDirection = 1;
            }
        }

        return;
    }

    if (enemy->idleFrameCount > 1)
    {
        enemy->idleTimer += deltaTime;

        if (
            enemy->idleTimer >=
            idleFrameDuration
        )
        {
            enemy->idleTimer -=
                idleFrameDuration;

            enemy->idleFrame +=
                enemy->idleDirection;

            if (
                enemy->idleFrame >=
                enemy->idleFrameCount - 1
            )
            {
                enemy->idleFrame =
                    enemy->idleFrameCount - 1;

                enemy->idleDirection = -1;
            }
            else if (enemy->idleFrame <= 0)
            {
                enemy->idleFrame = 0;
                enemy->idleDirection = 1;
            }
        }
    }
}