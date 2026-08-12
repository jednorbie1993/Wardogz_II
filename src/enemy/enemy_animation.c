#include "enemy_internal.h"


void EnemyUpdateAnimation(
    Enemy *enemy,
    float deltaTime,
    bool isWalking
)
{
    if (
        isWalking &&
        enemy->walkFrameCount > 0
    )
    {
        enemy->walkTimer += deltaTime;

        while (
            enemy->walkTimer >=
            enemy->walkFrameTime
        )
        {
            enemy->walkTimer -=
                enemy->walkFrameTime;

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

    if (enemy->idleFrameCount > 1)
    {
        enemy->idleTimer += deltaTime;

        if (
            enemy->idleTimer >=
            enemy->idleFrameTime
        )
        {
            enemy->idleTimer -=
                enemy->idleFrameTime;

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