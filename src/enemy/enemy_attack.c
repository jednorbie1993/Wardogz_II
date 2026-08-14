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


static bool IsBossAttackMove(EnemyAttackMove move)
{
    return
        move == ENEMY_ATTACK_BOSS_COMBO ||
        move == ENEMY_ATTACK_BOSS_KNEE ||
        move == ENEMY_ATTACK_BOSS_UPPERCUT ||
        move == ENEMY_ATTACK_BOSS_HEAVY_BLOW;
}


static EnemyAttackMove GetNextBossAttackMove(EnemyAttackMove move)
{
    switch (move)
    {
        case ENEMY_ATTACK_BOSS_COMBO:
            return ENEMY_ATTACK_BOSS_KNEE;
        case ENEMY_ATTACK_BOSS_KNEE:
            return ENEMY_ATTACK_BOSS_UPPERCUT;
        case ENEMY_ATTACK_BOSS_UPPERCUT:
            return ENEMY_ATTACK_BOSS_HEAVY_BLOW;
        case ENEMY_ATTACK_BOSS_HEAVY_BLOW:
        default:
            return ENEMY_ATTACK_BOSS_COMBO;
    }
}


static int GetCurrentAttackFrameCount(
    const Enemy *enemy
)
{
    int frameCount = enemy->punchFrameCount;

    switch (enemy->currentAttackMove)
    {
        case ENEMY_ATTACK_ELBOW:
            frameCount = enemy->elbowFrameCount;
            break;
        case ENEMY_ATTACK_BOSS_COMBO:
            frameCount = enemy->bossComboFrameCount;
            break;
        case ENEMY_ATTACK_BOSS_KNEE:
            frameCount = enemy->bossKneeFrameCount;
            break;
        case ENEMY_ATTACK_BOSS_UPPERCUT:
            frameCount = enemy->bossUppercutFrameCount;
            break;
        case ENEMY_ATTACK_BOSS_HEAVY_BLOW:
            frameCount = enemy->bossHeavyBlowFrameCount;
            break;
        default:
            break;
    }

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
    enemy->attackHitFrames = 0u;

    // 0061 - After the first attack, an enemy with an optional battle-idle
    // set remains in its fighting stance for the rest of the encounter.
    if (enemy->battleIdleFrameCount > 0)
    {
        enemy->battleIdleActive = true;
        enemy->battleIdleFrame = 0;
        enemy->battleIdleDirection = 1;
        enemy->battleIdleTimer = 0.0f;
    }

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
        enemy->isRetreating = false;
        enemy->retreatTimer = 0.0f;
        enemy->retreatDirection = 0;
        enemy->retreatPauseTimer = 0.0f;
        return;
    }

    if (
        !enemy->isAttacking &&
        enemy->attackSlotGranted &&
        enemy->attackTurnAllowed &&
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

    // Normal attacks hit once. Vargas' Combo hits separately on
    // visible Frames 2 and 4 (indices 1 and 3).
    bool attackHitboxActive = (enemy->attackFrame == 2);
    if (enemy->currentAttackMove == ENEMY_ATTACK_BOSS_COMBO)
    {
        attackHitboxActive =
            (enemy->attackFrame == 1 || enemy->attackFrame == 3);
    }
    else if (enemy->currentAttackMove == ENEMY_ATTACK_BOSS_HEAVY_BLOW)
    {
        // Heavy Blow has three images; visible Frame 2 is its impact.
        attackHitboxActive = (enemy->attackFrame == 1);
    }

    unsigned int currentFrameBit =
        1u << (unsigned int)enemy->attackFrame;

    if (attackHitboxActive)
    {
        Rectangle enemyAttackHitbox =
            GetEnemyAttackHitbox(enemy);

        if (
            (enemy->attackHitFrames & currentFrameBit) == 0u &&
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

            enemy->attackHitFrames |= currentFrameBit;
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

        // ========================================================
        // 0045 - RETREAT AFTER A COMPLETED ATTACK
        // ========================================================
        // Save the backward direction NOW, before normal chase AI can
        // recalculate attackDirection. Facing remains unchanged while
        // retreating, so Punk appears to back away from the player.
        enemy->isRetreating = true;
        enemy->retreatTimer = enemy->retreatDuration;
        enemy->retreatDirection = -enemy->attackDirection;
        enemy->retreatPauseTimer = 0.0f;
        enemy->retreatFrame = 0;
        enemy->retreatFrameTimer = 0.0f;

        // Vargas uses every completed special move once before repeating:
        // Combo -> Knee -> Uppercut -> Heavy Blow -> Combo.
        if (!enemy->lockAttackMove && IsBossAttackMove(enemy->currentAttackMove))
        {
            enemy->currentAttackMove =
                GetNextBossAttackMove(enemy->currentAttackMove);
        }
        // 0037 FIX - Normal enemies alternate only after a completed attack.
        else if (!enemy->lockAttackMove && enemy->currentAttackMove == ENEMY_ATTACK_PUNCH)
        {
            enemy->currentAttackMove = ENEMY_ATTACK_ELBOW;
        }
        else if (!enemy->lockAttackMove)
        {
            enemy->currentAttackMove = ENEMY_ATTACK_PUNCH;
        }

        // Keep legacy field synchronized for compatibility.
        enemy->nextAttackMove = enemy->currentAttackMove;
    }
}