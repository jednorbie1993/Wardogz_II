#include "enemy_internal.h"
#include <math.h>


float EnemyGetPerspectiveScale(const Enemy *enemy)
{
    float stageY =
        enemy->hurtbox.y +
        enemy->stageAnchorOffsetY;

    float depth =
        (stageY - 345.0f) /
        (700.0f - 270.0f);

    if (depth < 0.0f)
        depth = 0.0f;

    if (depth > 1.0f)
        depth = 1.0f;

    float playerStyleScale =
        2.90f +
        (depth * 1.80f);

    const float referenceDepth =
        (470.0f - 345.0f) /
        (700.0f - 270.0f);

    const float referenceScale =
        2.90f +
        (referenceDepth * 1.80f);

    return playerStyleScale / referenceScale;
}


Rectangle EnemyGetScaledHurtbox(const Enemy *enemy)
{
    float scale =
        EnemyGetPerspectiveScale(enemy);

    float scaledWidth =
        enemy->hurtbox.width * scale;

    float scaledHeight =
        enemy->hurtbox.height * scale;

    float centerX =
        enemy->hurtbox.x +
        enemy->hurtbox.width / 2.0f;

    float bottomY =
        enemy->hurtbox.y +
        enemy->hurtbox.height;

    return (Rectangle)
    {
        centerX - scaledWidth / 2.0f,
        bottomY - scaledHeight,
        scaledWidth,
        scaledHeight
    };
}


Rectangle GetEnemyFootMarker(const Enemy *enemy)
{
    Rectangle hurtbox =
        EnemyGetScaledHurtbox(enemy);

    return (Rectangle)
    {
        hurtbox.x + (hurtbox.width * -0.80f),
        hurtbox.y + hurtbox.height - 34.0f,
        hurtbox.width * 2.50f,
        44.0f
    };
}


EnemyCombatContext EnemyBuildCombatContext(
    const Enemy *enemy,
    const Player *player
)
{
    EnemyCombatContext context = {0};

    context.enemyHurtbox =
        EnemyGetScaledHurtbox(enemy);

    context.playerHurtbox =
        GetPlayerHurtbox(player);

    context.enemyCenterX =
        context.enemyHurtbox.x +
        context.enemyHurtbox.width / 2.0f;

    context.playerCenterX =
        context.playerHurtbox.x +
        context.playerHurtbox.width / 2.0f;

    context.distanceX =
        context.playerCenterX -
        context.enemyCenterX;

    context.absoluteDistanceX =
        fabsf(context.distanceX);

    context.enemyFeet =
        GetEnemyFootMarker(enemy);

    context.playerFeet =
        GetPlayerFootMarker(player);

    context.enemyGroundY =
        context.enemyFeet.y +
        context.enemyFeet.height / 2.0f;

    context.playerGroundY =
        context.playerFeet.y +
        context.playerFeet.height / 2.0f;

    context.depthDifference =
        context.playerGroundY -
        context.enemyGroundY;

    context.absoluteDepthDifference =
        fabsf(context.depthDifference);

    context.aggroDistance =
        sqrtf(
            context.distanceX * context.distanceX +
            context.depthDifference * context.depthDifference
        );

    context.playerDetected =
        context.aggroDistance <= enemy->aggroRange;

    return context;
}


bool EnemyIsPlayerInVerticalRange(
    const Enemy *enemy,
    const Player *player
)
{
    Rectangle playerHurtbox =
        GetPlayerHurtbox(player);

    Rectangle enemyHurtbox =
        EnemyGetScaledHurtbox(enemy);

    float enemyTop = enemyHurtbox.y;
    float enemyBottom =
        enemyHurtbox.y + enemyHurtbox.height;

    float playerTop = playerHurtbox.y;
    float playerBottom =
        playerHurtbox.y + playerHurtbox.height;

    const float verticalMargin = 35.0f;

    return
        playerBottom >= enemyTop - verticalMargin &&
        playerTop <= enemyBottom + verticalMargin;
}


bool EnemyIsSameGroundDepth(
    const Enemy *enemy,
    const Player *player
)
{
    Rectangle enemyFeet =
        GetEnemyFootMarker(enemy);

    Rectangle playerFeet =
        GetPlayerFootMarker(player);

    return CheckCollisionRecs(
        enemyFeet,
        playerFeet
    );
}
// ============================================================
// 0048 - PLAYER / ENEMY BODY COLLISION + SOFT PUSH
// ============================================================
// Uses a compact ground-contact area instead of the full sprite/hurtbox.
// This prevents the player from walking straight through a Punk while
// still allowing normal top/bottom movement around the enemy.
void ResolvePlayerEnemyBodyCollision(
    Player *player,
    Enemy *enemies,
    int enemyCount,
    float screenWidth,
    float walkAreaTop,
    float walkAreaBottom
)
{
    if (!player || !enemies || enemyCount <= 0 || !player->isAlive)
    {
        return;
    }

    for (int i = 0; i < enemyCount; i++)
    {
        Enemy *enemy = &enemies[i];

        if (!enemy->isAlive || enemy->isEntering || !enemy->hasEnteredStage)
        {
            continue;
        }

        Rectangle playerHurtbox = GetPlayerHurtbox(player);
        Rectangle enemyHurtbox = EnemyGetScaledHurtbox(enemy);
        Rectangle playerFeet = GetPlayerFootMarker(player);
        Rectangle enemyFeet = GetEnemyFootMarker(enemy);

        float playerCenterX =
            playerHurtbox.x + playerHurtbox.width * 0.5f;

        float enemyCenterX =
            enemyHurtbox.x + enemyHurtbox.width * 0.5f;

        float playerGroundY =
            playerFeet.y + playerFeet.height * 0.5f;

        float enemyGroundY =
            enemyFeet.y + enemyFeet.height * 0.5f;

        float differenceX = playerCenterX - enemyCenterX;
        float differenceY = playerGroundY - enemyGroundY;

        float absoluteX = fabsf(differenceX);
        float absoluteY = fabsf(differenceY);

        // Compact body footprint. The full hurtboxes are intentionally
        // NOT used because that would block the player too far away.
        float minimumDistanceX =
            playerHurtbox.width * 0.28f +
            enemyHurtbox.width * 0.28f;

        const float minimumDepthDistance = 32.0f;

        if (
            absoluteX >= minimumDistanceX ||
            absoluteY >= minimumDepthDistance
        )
        {
            continue;
        }

        float overlapX = minimumDistanceX - absoluteX;
        float overlapY = minimumDepthDistance - absoluteY;

        // Normally both actors give a little. During an enemy attack,
        // retreat, or retreat pause, keep the Punk more planted while
        // still allowing a small nudge instead of making him immovable.
        float playerShare = 0.50f;
        float enemyShare = 0.50f;

        if (
            enemy->isAttacking ||
            enemy->isRetreating ||
            enemy->retreatPauseTimer > 0.0f
        )
        {
            playerShare = 0.75f;
            enemyShare = 0.25f;
        }

        // Resolve through the axis requiring the smaller relative move.
        float normalizedX = overlapX / minimumDistanceX;
        float normalizedY = overlapY / minimumDepthDistance;

        if (normalizedX <= normalizedY)
        {
            float directionX;

            if (absoluteX < 0.001f)
            {
                directionX = player->facingRight ? -1.0f : 1.0f;
            }
            else
            {
                directionX = (differenceX > 0.0f) ? 1.0f : -1.0f;
            }

            player->rectangle.x +=
                directionX * overlapX * playerShare;

            enemy->hurtbox.x -=
                directionX * overlapX * enemyShare;
        }
        else
        {
            float directionY;

            if (absoluteY < 0.001f)
            {
                directionY = (player->rectangle.y >= enemy->hurtbox.y)
                    ? 1.0f
                    : -1.0f;
            }
            else
            {
                directionY = (differenceY > 0.0f) ? 1.0f : -1.0f;
            }

            player->rectangle.y +=
                directionY * overlapY * playerShare;

            enemy->hurtbox.y -=
                directionY * overlapY * enemyShare;
        }

        // Keep both actors inside the playable stage after separation.
        if (player->rectangle.x < 0.0f)
        {
            player->rectangle.x = 0.0f;
        }

        if (player->rectangle.x + player->rectangle.width > screenWidth)
        {
            player->rectangle.x =
                screenWidth - player->rectangle.width;
        }

        if (player->rectangle.y < walkAreaTop)
        {
            player->rectangle.y = walkAreaTop;
        }

        if (player->rectangle.y > walkAreaBottom)
        {
            player->rectangle.y = walkAreaBottom;
        }

        EnemyClampToStage(
            enemy,
            screenWidth,
            walkAreaTop,
            walkAreaBottom
        );
    }
}