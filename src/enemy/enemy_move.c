#include "enemy_internal.h"
#include <stddef.h>


static float GetNextEnemyAttackStopDistance(
    const Enemy *enemy
)
{
    switch (enemy->nextAttackMove)
    {
        case ENEMY_ATTACK_ELBOW:
            // Boss Kick / Punk Elbow
            return enemy->elbowStopDistance;

        case ENEMY_ATTACK_BOSS_KNEE:
            return enemy->bossKneeStopDistance;

        default:
            return enemy->punchStopDistance;
    }
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
    // ============================================================
    // 0045 - ENEMY RETREAT AFTER ATTACK + FACING HOLD
    // ============================================================
    // After a completed Punch/Elbow, Punk backs away in the saved
    // retreat direction. Do NOT recalculate facing or attackDirection
    // during this state; this keeps him looking at the player while
    // physically moving backward for the full retreat duration.
    if (enemy->isRetreating && !enemy->isHit)
    {
        enemy->isChasing = true;
        enemy->isInAttackRange = false;

        enemy->hurtbox.x +=
            (float)enemy->retreatDirection *
            enemy->retreatSpeed *
            deltaTime;

        EnemyClampToStage(
            enemy,
            screenWidth,
            walkAreaTop,
            walkAreaBottom
        );

        enemy->retreatTimer -= deltaTime;

        if (enemy->retreatTimer <= 0.0f)
        {
            enemy->retreatTimer = 0.0f;
            enemy->isRetreating = false;
            enemy->isChasing = false;
            enemy->retreatPauseTimer = enemy->retreatPauseDuration;
        }

        // Refresh position data for the rest of this frame, but return
        // without changing facing. Normal facing resumes next frame.
        *context = EnemyBuildCombatContext(enemy, player);
        return;
    }

    // 0045 - Short stand/stance pause after the backward dash.
    // During this pause Punk stays still before normal chase resumes.
    if (enemy->retreatPauseTimer > 0.0f && !enemy->isHit)
    {
        enemy->isChasing = false;
        enemy->isInAttackRange = false;

        enemy->retreatPauseTimer -= deltaTime;
        if (enemy->retreatPauseTimer < 0.0f)
        {
            enemy->retreatPauseTimer = 0.0f;
        }

        *context = EnemyBuildCombatContext(enemy, player);
        return;
    }

    // Always face the real player when Punk is free to react.
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

    // Attack range is always measured against the real player,
    // never against a formation point.
    enemy->isInAttackRange =
        context->playerDetected &&
        player->isAlive &&
        context->absoluteDistanceX <= currentStopDistance &&
        context->absoluteDepthDifference <= enemy->chaseDepthTolerance;

    enemy->isChasing = false;

    if (
        enemy->isAttacking ||
        enemy->isHit ||
        !player->isAlive ||
        !context->playerDetected
    )
    {
        return;
    }

    float moveX = 0.0f;
    float moveY = 0.0f;

    // ============================================================
    // 0046 - BLOCKED ATTACKER UPPER/LOWER BYPASS
    // ============================================================
    // If another Punk is directly in front of this active attacker,
    // ResolveEnemyApproachLanes() gives it a temporary depth target.
    // Reach that side lane first; then normal chase resumes.
    if (enemy->isLaneBypassing && enemy->attackSlotGranted)
    {
        float enemyStageY =
            enemy->hurtbox.y + enemy->stageAnchorOffsetY;

        float laneDifferenceY =
            enemy->laneBypassTargetY - enemyStageY;

        float absoluteLaneDifferenceY =
            (laneDifferenceY < 0.0f)
            ? -laneDifferenceY
            : laneDifferenceY;

        const float laneArrivalTolerance = 12.0f;

        if (absoluteLaneDifferenceY > laneArrivalTolerance)
        {
            moveY = (laneDifferenceY > 0.0f) ? 1.0f : -1.0f;
        }
        else
        {
            enemy->isLaneBypassing = false;
            enemy->laneBypassDirection = 0;
        }
    }

    // ============================================================
    // 0042 FIX - SOFT SURROUND / WAITING AI
    // ============================================================
    // Waiting Punks DO NOT run away from the player to force a perfect
    // formation. If they are already near the player, they simply hold
    // position and remain hittable. Formation offsets only guide enemies
    // that are genuinely far away.
    if (!enemy->isLaneBypassing && enemy->surroundEnabled && !enemy->attackSlotGranted)
    {
        const float waitingMaxDistanceX = 260.0f;
        const float waitingMaxDepth = 115.0f;

        // Already close enough: hold position instead of retreating.
        if (
            context->absoluteDistanceX > waitingMaxDistanceX ||
            context->absoluteDepthDifference > waitingMaxDepth
        )
        {
            float playerCenterX =
                player->rectangle.x +
                player->rectangle.width * 0.5f;

            float playerGroundY =
                player->rectangle.y +
                player->rectangle.height;

            float enemyCenterX =
                enemy->hurtbox.x +
                enemy->hurtbox.width * 0.5f;

            float enemyStageY =
                enemy->hurtbox.y +
                enemy->stageAnchorOffsetY;

            float targetCenterX =
                playerCenterX + enemy->surroundOffsetX;

            float targetStageY =
                playerGroundY + enemy->surroundOffsetY;

            float formationDifferenceX =
                targetCenterX - enemyCenterX;

            float formationDifferenceY =
                targetStageY - enemyStageY;

            // Only correct X while the Punk is truly far from the player.
            if (context->absoluteDistanceX > waitingMaxDistanceX)
            {
                moveX =
                    (formationDifferenceX > 0.0f)
                    ? 1.0f
                    : -1.0f;
            }

            // Depth correction is softer and only used when clearly far.
            if (context->absoluteDepthDifference > waitingMaxDepth)
            {
                moveY =
                    (formationDifferenceY > 0.0f)
                    ? 1.0f
                    : -1.0f;
            }
        }
    }
    else if (!enemy->isLaneBypassing && !enemy->isInAttackRange)
    {
        // 0031/0035 behavior for a Punk that currently owns an attack slot:
        // close the real distance to the player until attack position.
        if (context->absoluteDistanceX > currentStopDistance)
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

        // Refresh combat distance after movement.
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


// ============================================================
// 0046 - BLOCKED ATTACKER APPROACH LANE SYSTEM
// ============================================================
// An active attacker that is behind another Punk on the same side of the
// player does not keep pushing straight forward. It picks a stable upper or
// lower depth lane, side-steps there first, then resumes its normal chase.
void ResolveEnemyApproachLanes(
    Enemy *enemies,
    int enemyCount,
    const Player *player,
    float walkAreaTop,
    float walkAreaBottom
)
{
    if (enemies == NULL || enemyCount <= 1 || player == NULL)
    {
        return;
    }

    float playerCenterX =
        player->rectangle.x + player->rectangle.width * 0.5f;

    float playerGroundY =
        player->rectangle.y + player->rectangle.height;

    const float blockerHorizontalGap = 185.0f;
    const float blockerDepthGap = 58.0f;
    const float laneDepthOffset = 92.0f;
    const float laneOccupancyDepth = 55.0f;
    const float laneOccupancyX = 280.0f;

    for (int i = 0; i < enemyCount; i++)
    {
        Enemy *enemy = &enemies[i];

        if (
            !enemy->isAlive ||
            enemy->isEntering ||
            !enemy->hasEnteredStage ||
            !enemy->attackSlotGranted ||
            enemy->isAttacking ||
            enemy->isHit ||
            enemy->isRetreating ||
            enemy->retreatPauseTimer > 0.0f
        )
        {
            if (!enemy->attackSlotGranted)
            {
                enemy->isLaneBypassing = false;
                enemy->laneBypassDirection = 0;
            }
            continue;
        }

        // Keep an already chosen lane stable until EnemyUpdateChase reaches it.
        if (enemy->isLaneBypassing)
        {
            continue;
        }

        float enemyCenterX =
            enemy->hurtbox.x + enemy->hurtbox.width * 0.5f;

        float enemyStageY =
            enemy->hurtbox.y + enemy->stageAnchorOffsetY;

        float enemyFromPlayerX = enemyCenterX - playerCenterX;
        float enemyAbsPlayerX =
            (enemyFromPlayerX < 0.0f)
            ? -enemyFromPlayerX
            : enemyFromPlayerX;

        bool blocked = false;

        for (int j = 0; j < enemyCount; j++)
        {
            if (j == i)
            {
                continue;
            }

            Enemy *blocker = &enemies[j];

            if (
                !blocker->isAlive ||
                blocker->isEntering ||
                !blocker->hasEnteredStage
            )
            {
                continue;
            }

            float blockerCenterX =
                blocker->hurtbox.x + blocker->hurtbox.width * 0.5f;

            float blockerStageY =
                blocker->hurtbox.y + blocker->stageAnchorOffsetY;

            float blockerFromPlayerX = blockerCenterX - playerCenterX;
            float blockerAbsPlayerX =
                (blockerFromPlayerX < 0.0f)
                ? -blockerFromPlayerX
                : blockerFromPlayerX;

            bool samePlayerSide =
                (enemyFromPlayerX >= 0.0f && blockerFromPlayerX >= 0.0f) ||
                (enemyFromPlayerX < 0.0f && blockerFromPlayerX < 0.0f);

            float centerGapX = blockerCenterX - enemyCenterX;
            if (centerGapX < 0.0f) centerGapX = -centerGapX;

            float depthGap = blockerStageY - enemyStageY;
            if (depthGap < 0.0f) depthGap = -depthGap;

            // The blocker must be on the same side, physically ahead toward
            // the player, and close enough to occupy the same approach lane.
            if (
                samePlayerSide &&
                blockerAbsPlayerX + 12.0f < enemyAbsPlayerX &&
                centerGapX <= blockerHorizontalGap &&
                depthGap <= blockerDepthGap
            )
            {
                blocked = true;
                break;
            }
        }

        if (!blocked)
        {
            continue;
        }

        float upperTargetY = playerGroundY - laneDepthOffset;
        float lowerTargetY = playerGroundY + laneDepthOffset;

        // Keep both candidate lanes inside the playable stage depth.
        const float laneEdgeMargin = 28.0f;
        float minimumLaneY = walkAreaTop + laneEdgeMargin;
        float maximumLaneY = walkAreaBottom - laneEdgeMargin;

        if (upperTargetY < minimumLaneY) upperTargetY = minimumLaneY;
        if (upperTargetY > maximumLaneY) upperTargetY = maximumLaneY;
        if (lowerTargetY < minimumLaneY) lowerTargetY = minimumLaneY;
        if (lowerTargetY > maximumLaneY) lowerTargetY = maximumLaneY;

        int upperOccupancy = 0;
        int lowerOccupancy = 0;

        // Prefer the less crowded side lane so several Punks do not all
        // choose the same bypass route.
        for (int j = 0; j < enemyCount; j++)
        {
            if (j == i || !enemies[j].isAlive || enemies[j].isEntering)
            {
                continue;
            }

            float otherCenterX =
                enemies[j].hurtbox.x + enemies[j].hurtbox.width * 0.5f;

            float otherStageY =
                enemies[j].hurtbox.y + enemies[j].stageAnchorOffsetY;

            float otherPlayerX = otherCenterX - playerCenterX;
            if (otherPlayerX < 0.0f) otherPlayerX = -otherPlayerX;

            if (otherPlayerX > laneOccupancyX)
            {
                continue;
            }

            float upperGap = otherStageY - upperTargetY;
            if (upperGap < 0.0f) upperGap = -upperGap;

            float lowerGap = otherStageY - lowerTargetY;
            if (lowerGap < 0.0f) lowerGap = -lowerGap;

            if (upperGap <= laneOccupancyDepth) upperOccupancy++;
            if (lowerGap <= laneOccupancyDepth) lowerOccupancy++;
        }

        int direction;

        if (upperOccupancy < lowerOccupancy)
        {
            direction = -1;
        }
        else if (lowerOccupancy < upperOccupancy)
        {
            direction = 1;
        }
        else
        {
            // Stable tie-breaker: alternate by enemy index, no frame jitter.
            direction = (i % 2 == 0) ? -1 : 1;
        }

        enemy->isLaneBypassing = true;
        enemy->laneBypassDirection = direction;
        enemy->laneBypassTargetY =
            (direction < 0) ? upperTargetY : lowerTargetY;
    }
}


// ============================================================
// 0042 - ENEMY SURROUND / FORMATION SLOT ASSIGNMENT
// ============================================================
// Alive Punks are distributed across four relative positions around
// the player. The offsets move with the player because EnemyUpdateChase()
// interprets them relative to the player's current position every frame.
void ResolveEnemySurroundFormation(
    Enemy *enemies,
    int enemyCount
)
{
    if (enemies == NULL || enemyCount <= 0)
    {
        return;
    }

    // Beat-em-up style surround offsets, not a perfect circle.
    // Slot 0 = left, slot 1 = right, slot 2 = upper/back,
    // slot 3 = lower/front.
    static const float slotOffsetX[4] =
    {
        -190.0f,
         190.0f,
         -85.0f,
          85.0f
    };

    static const float slotOffsetY[4] =
    {
          0.0f,
          0.0f,
         -80.0f,
          80.0f
    };

    int formationSlot = 0;

    for (int i = 0; i < enemyCount; i++)
    {
        Enemy *enemy = &enemies[i];

        if (!enemy->isAlive)
        {
            enemy->surroundEnabled = false;
            continue;
        }

        int slot = formationSlot % 4;

        enemy->surroundEnabled = true;
        enemy->surroundOffsetX = slotOffsetX[slot];
        enemy->surroundOffsetY = slotOffsetY[slot];

        formationSlot++;
    }
}


// ============================================================
// 0040 - MULTI-ENEMY SPACING / ANTI-OVERLAP
// ============================================================
//
// Keeps enemies that are standing on nearly the same depth line from
// stacking on top of each other. Attacking / hit enemies act as anchors;
// free enemies are pushed away from them instead of interrupting attacks.
void ResolveEnemySpacing(
    Enemy *enemies,
    int enemyCount,
    float deltaTime,
    float screenWidth,
    float walkAreaTop,
    float walkAreaBottom
)
{
    if (enemies == NULL || enemyCount <= 1)
    {
        return;
    }

    for (int i = 0; i < enemyCount; i++)
    {
        Enemy *a = &enemies[i];

        if (!a->isAlive || a->isEntering)
        {
            continue;
        }

        for (int j = i + 1; j < enemyCount; j++)
        {
            Enemy *b = &enemies[j];

            if (!b->isAlive || b->isEntering)
            {
                continue;
            }

            float aCenterX = a->hurtbox.x + a->hurtbox.width * 0.5f;
            float bCenterX = b->hurtbox.x + b->hurtbox.width * 0.5f;

            float aStageY = a->hurtbox.y + a->stageAnchorOffsetY;
            float bStageY = b->hurtbox.y + b->stageAnchorOffsetY;

            float differenceX = bCenterX - aCenterX;
            float differenceY = bStageY - aStageY;

            float absoluteX = (differenceX < 0.0f) ? -differenceX : differenceX;
            float absoluteY = (differenceY < 0.0f) ? -differenceY : differenceY;

            float requiredX =
                (a->separationRadiusX + b->separationRadiusX) * 0.5f;

            float allowedDepth =
                (a->separationDepthTolerance + b->separationDepthTolerance) * 0.5f;

            // Different depth lanes may overlap visually in X.
            if (absoluteY > allowedDepth || absoluteX >= requiredX)
            {
                continue;
            }

            float overlapX = requiredX - absoluteX;
            float direction = (differenceX >= 0.0f) ? 1.0f : -1.0f;

            // If their centers are exactly equal, use array order to choose
            // a stable left/right direction instead of jittering each frame.
            if (absoluteX < 0.01f)
            {
                direction = (i < j) ? 1.0f : -1.0f;
            }

            // 0045 - Retreating or post-retreat-paused enemies are anchors.
            // Other enemies move away from them instead of pushing them.
            bool aLocked =
                a->isAttacking ||
                a->isHit ||
                a->isRetreating ||
                a->retreatPauseTimer > 0.0f;

            bool bLocked =
                b->isAttacking ||
                b->isHit ||
                b->isRetreating ||
                b->retreatPauseTimer > 0.0f;

            float pushSpeed =
                (a->separationPushSpeed + b->separationPushSpeed) * 0.5f;

            float maxPush = pushSpeed * deltaTime;

            if (aLocked && bLocked)
            {
                // Do not disturb two enemies that are both inside a committed
                // attack/hit reaction. They will separate once one becomes free.
                continue;
            }
            else if (aLocked)
            {
                float push = overlapX;
                if (push > maxPush) push = maxPush;
                b->hurtbox.x += direction * push;
            }
            else if (bLocked)
            {
                float push = overlapX;
                if (push > maxPush) push = maxPush;
                a->hurtbox.x -= direction * push;
            }
            else
            {
                float halfPush = overlapX * 0.5f;
                if (halfPush > maxPush) halfPush = maxPush;

                a->hurtbox.x -= direction * halfPush;
                b->hurtbox.x += direction * halfPush;
            }

            EnemyClampToStage(
                a,
                screenWidth,
                walkAreaTop,
                walkAreaBottom
            );

            EnemyClampToStage(
                b,
                screenWidth,
                walkAreaTop,
                walkAreaBottom
            );
        }
    }
}