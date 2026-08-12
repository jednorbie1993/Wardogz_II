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