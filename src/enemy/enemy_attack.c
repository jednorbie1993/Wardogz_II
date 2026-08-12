#include "enemy_internal.h"


static float GetNextEnemyAttackRange(
    const Enemy *enemy
)
{
    if (enemy->nextAttackMove == ENEMY_ATTACK_ELBOW)
    {
        return enemy->elbowAttackRange;
    }

    return enemy->punchAttackRange;
}


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


static float GetEnemyAttackFrameDuration(
    const Enemy *enemy,
    int frame
)
{
    // Punk Elbow timing:
    // Frame 1 = wind-up
    // Frame 2 = freeze / hold (no hitbox)
    // Frame 3 = strike (yellow hitbox active)
    // Frame 4 = recovery
    if (
        enemy->currentAttackMove == ENEMY_ATTACK_ELBOW &&
        frame == 1
    )
    {
        return 0.32f;
    }

    // Visible Frame 3 (index 2) stays held while Punk slides forward.
    if (
        enemy->currentAttackMove == ENEMY_ATTACK_ELBOW &&
        frame == 2
    )
    {
        return 0.30f;
    }

    return enemy->attackFrameTime;
}


static void StartEnemyAttack(Enemy *enemy)
{
    enemy->isAttacking = true;
    enemy->hitPlayerThisAttack = false;

    // 0037 FIX - Do NOT switch moves at attack start.
    // currentAttackMove is the move that must finish first.
    // The Punch/Elbow switch happens only after the full animation
    // completes, so a cancelled Punch will not skip to Elbow.

    enemy->attackFrame = 0;
    enemy->attackFrameTimer = 0.0f;

    // Prepare the elbow lunge once at attack start.
    enemy->elbowLungeRemaining =
        (enemy->currentAttackMove == ENEMY_ATTACK_ELBOW)
        ? enemy->elbowLungeDistance
        : 0.0f;

    int frameCount =
        GetCurrentAttackFrameCount(enemy);

    enemy->attackTimer = 0.0f;

    for (int frame = 0; frame < frameCount; frame++)
    {
        enemy->attackTimer +=
            GetEnemyAttackFrameDuration(enemy, frame);
    }
}


static void AdvanceEnemyAttackAnimation(
    Enemy *enemy,
    float deltaTime
)
{
    int frameCount =
        GetCurrentAttackFrameCount(enemy);

    enemy->attackFrameTimer += deltaTime;

    while (enemy->attackFrame < frameCount - 1)
    {
        float currentFrameDuration =
            GetEnemyAttackFrameDuration(
                enemy,
                enemy->attackFrame
            );

        if (enemy->attackFrameTimer < currentFrameDuration)
        {
            break;
        }

        enemy->attackFrameTimer -= currentFrameDuration;
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
        enemy->elbowLungeRemaining = 0.0f;
        return;
    }

    if (
        !enemy->isAttacking &&
        enemy->attackCooldownTimer <= 0.0f &&
        enemy->isInAttackRange &&
        context->absoluteDistanceX <= GetNextEnemyAttackRange(enemy) &&
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
    // ============================================================
    // 0038 - ELBOW FRAME 3 FORWARD SLIDE
    // ============================================================
    // attackFrame == 2 means visible Frame 3.
    // Move the actual Punk hurtbox/body forward while Frame 3 is held.
    if (
        enemy->currentAttackMove == ENEMY_ATTACK_ELBOW &&
        enemy->attackFrame == 2 &&
        enemy->elbowLungeRemaining > 0.0f
    )
    {
        const float frame3HoldTime = 0.30f;

        float slideAmount =
            (enemy->elbowLungeDistance / frame3HoldTime) *
            deltaTime;

        if (slideAmount > enemy->elbowLungeRemaining)
        {
            slideAmount = enemy->elbowLungeRemaining;
        }

        enemy->hurtbox.x +=
            slideAmount * (float)enemy->attackDirection;

        enemy->elbowLungeRemaining -= slideAmount;
    }

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
        enemy->elbowLungeRemaining = 0.0f;

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