#include "enemy_internal.h"


static int GetCurrentAttackFrameCount(
    const Enemy *enemy
)
{
    int frameCount =
        (enemy->currentAttackMove == ENEMY_ATTACK_ELBOW)
        ? enemy->elbowFrameCount
        : enemy->punchFrameCount;

    if (frameCount <= 0)
    {
        frameCount = 1;
    }

    return frameCount;
}


static void StartEnemyAttack(Enemy *enemy)
{
    enemy->isAttacking = true;
    enemy->hitPlayerThisAttack = false;

    // 0037 FIX - Do NOT switch moves at attack start.
    // currentAttackMove is the move that must finish first.
    // The Punch/Elbow switch happens only after the full 4-frame
    // animation completes, so a cancelled Punch will not skip to Elbow.

    enemy->attackFrame = 0;
    enemy->attackFrameTimer = 0.0f;

    int frameCount =
        GetCurrentAttackFrameCount(enemy);

    enemy->attackTimer =
        enemy->attackFrameTime *
        (float)frameCount;
}


static void AdvanceEnemyAttackAnimation(
    Enemy *enemy,
    float deltaTime
)
{
    int frameCount =
        GetCurrentAttackFrameCount(enemy);

    enemy->attackFrameTimer += deltaTime;

    while (
        enemy->attackFrameTimer >=
            enemy->attackFrameTime &&
        enemy->attackFrame <
            frameCount - 1
    )
    {
        enemy->attackFrameTimer -=
            enemy->attackFrameTime;

        enemy->attackFrame++;
    }
}


Rectangle GetEnemyAttackHitbox(
    const Enemy *enemy
)
{
    Rectangle hurtbox =
        EnemyGetScaledHurtbox(enemy);

    float scale =
        EnemyGetPerspectiveScale(enemy);

    // ============================================================
    // 0037 - SELECT PUNCH OR ELBOW HITBOX
    // ============================================================

    float hitboxWidth;
    float hitboxHeight;
    float hitboxOffsetX;
    float hitboxOffsetY;

    if (
        enemy->currentAttackMove ==
        ENEMY_ATTACK_ELBOW
    )
    {
        hitboxWidth =
            enemy->elbowHitboxWidth;

        hitboxHeight =
            enemy->elbowHitboxHeight;

        hitboxOffsetX =
            enemy->elbowHitboxOffsetX;

        hitboxOffsetY =
            enemy->elbowHitboxOffsetY;
    }
    else
    {
        hitboxWidth =
            enemy->punchHitboxWidth;

        hitboxHeight =
            enemy->punchHitboxHeight;

        hitboxOffsetX =
            enemy->punchHitboxOffsetX;

        hitboxOffsetY =
            enemy->punchHitboxOffsetY;
    }

    float attackWidth =
        hitboxWidth *
        scale;

    float attackHeight =
        hitboxHeight *
        scale;

    // Base vertical position used by the old shared hitbox.
    // OffsetY now lets Punch and Elbow move independently.
    float centerY =
        hurtbox.y +
        hurtbox.height * 0.48f +
        (hitboxOffsetY * scale);

    Rectangle hitbox =
    {
        0.0f,
        centerY - attackHeight / 2.0f,
        attackWidth,
        attackHeight
    };

    // OffsetX follows the attack direction:
    // positive = farther forward
    // negative = closer toward the enemy.
    if (enemy->attackDirection > 0)
    {
        hitbox.x =
            hurtbox.x +
            hurtbox.width -
            (10.0f * scale) +
            (hitboxOffsetX * scale);
    }
    else
    {
        hitbox.x =
            hurtbox.x -
            attackWidth +
            (10.0f * scale) -
            (hitboxOffsetX * scale);
    }

    return hitbox;
}

void EnemyUpdateAttack(
    Enemy *enemy,
    Player *player,
    float deltaTime,
    const EnemyCombatContext *context
)
{
    if (enemy->isHit)
    {
        enemy->isAttacking = false;
        enemy->attackTimer = 0.0f;
        enemy->hitPlayerThisAttack = false;
        enemy->attackFrame = 0;
        enemy->attackFrameTimer = 0.0f;
        return;
    }

    if (
        !enemy->isAttacking &&
        enemy->attackCooldownTimer <= 0.0f &&
        enemy->isInAttackRange &&
        context->absoluteDistanceX <= enemy->attackRange &&
        EnemyIsPlayerInVerticalRange(enemy, player) &&
        player->isAlive &&
        context->playerDetected
    )
    {
        StartEnemyAttack(enemy);
    }

    if (!enemy->isAttacking)
    {
        return;
    }

    AdvanceEnemyAttackAnimation(enemy, deltaTime);

    // Frame 3 (index 2) is the actual strike frame.
    const bool attackHitboxActive =
        (enemy->attackFrame == 2);

    if (attackHitboxActive)
    {
        Rectangle enemyAttackHitbox =
            GetEnemyAttackHitbox(enemy);

        if (
            !enemy->hitPlayerThisAttack &&
            player->isAlive &&
            EnemyIsSameGroundDepth(enemy, player) &&
            CheckCollisionRecs(
                enemyAttackHitbox,
                context->playerHurtbox
            )
        )
        {
            DamagePlayer(
                player,
                enemy->attackDamage,
                enemy->attackDirection,
                enemy->attackKnockbackSpeed,
                enemy->attackHitReactionTime
            );

            enemy->hitPlayerThisAttack = true;
        }
    }

    enemy->attackTimer -= deltaTime;

    if (enemy->attackTimer <= 0.0f)
    {
        enemy->isAttacking = false;
        enemy->attackTimer = 0.0f;
        enemy->attackCooldownTimer = 1.10f;
        enemy->hitPlayerThisAttack = false;
        enemy->attackFrame = 0;
        enemy->attackFrameTimer = 0.0f;

        // 0037 FIX - Alternate only after a completed attack.
        if (enemy->currentAttackMove == ENEMY_ATTACK_PUNCH)
        {
            enemy->currentAttackMove = ENEMY_ATTACK_ELBOW;
        }
        else
        {
            enemy->currentAttackMove = ENEMY_ATTACK_PUNCH;
        }

        // Keep legacy field synchronized for compatibility.
        enemy->nextAttackMove = enemy->currentAttackMove;
    }
}