#include "enemy_internal.h"


void EnemyUpdateHitReaction(
    Enemy *enemy,
    float deltaTime,
    float screenWidth,
    float walkAreaTop,
    float walkAreaBottom
)
{
    if (!enemy->isHit)
    {
        return;
    }

    enemy->hurtbox.x +=
        enemy->knockbackDirection *
        enemy->knockbackSpeed *
        deltaTime;

    enemy->hitReactionTimer -=
        deltaTime;

    EnemyClampToStage(
        enemy,
        screenWidth,
        walkAreaTop,
        walkAreaBottom
    );

    if (enemy->hitReactionTimer <= 0.0f)
    {
        enemy->hitReactionTimer = 0.0f;
        enemy->knockbackSpeed = 0.0f;
        enemy->knockbackDirection = 0;
        enemy->isHit = false;
    }
}


void EnemyCheckPlayerAttack(
    Enemy *enemy,
    Player *player
)
{
    // Reset once the player's attack is completely finished.
    if (!player->isAttacking)
    {
        enemy->hitByCurrentAttack = false;
    }

    if (
        !IsPlayerAttackHitboxActive(player) ||
        enemy->hitByCurrentAttack
    )
    {
        return;
    }

    Rectangle attackHitbox =
        GetPlayerAttackHitbox(player);

    if (
        !EnemyIsSameGroundDepth(enemy, player) ||
        !CheckCollisionRecs(
            attackHitbox,
            EnemyGetScaledHurtbox(enemy)
        )
    )
    {
        return;
    }

    // ============================================================
    // DAMAGE
    // ============================================================

    enemy->hp -=
        GetPlayerAttackDamage(player);

    enemy->hitByCurrentAttack = true;

    // ============================================================
    // HIT REACTION / KNOCKBACK
    // ============================================================

    enemy->isHit = true;

    // Cancel the enemy attack immediately when Punk gets hit.
    enemy->isAttacking = false;
    enemy->attackTimer = 0.0f;
    enemy->hitPlayerThisAttack = false;
    enemy->attackFrame = 0;
    enemy->attackFrameTimer = 0.0f;

    enemy->hitReactionTimer =
        GetPlayerAttackHitReactionTime(player);

    enemy->knockbackSpeed =
        GetPlayerAttackKnockbackSpeed(player);

    enemy->knockbackDirection =
        player->facingRight
        ? 1
        : -1;

    // ============================================================
    // DEATH
    // ============================================================

    if (enemy->hp <= 0)
    {
        enemy->hp = 0;
        enemy->isAlive = false;

        enemy->isHit = false;
        enemy->hitReactionTimer = 0.0f;
        enemy->knockbackSpeed = 0.0f;
        enemy->knockbackDirection = 0;
    }
}