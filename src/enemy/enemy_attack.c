#include "enemy_internal.h"


static float GetNextEnemyAttackRange(
    const Enemy *enemy
)
{
    switch (enemy->nextAttackMove)
    {
        case ENEMY_ATTACK_ELBOW:
            // Boss Kick / Punk Elbow
            return enemy->elbowAttackRange;

        case ENEMY_ATTACK_BOSS_KNEE:
            return enemy->bossKneeAttackRange;

        default:
            return enemy->punchAttackRange;
    }
}

static bool IsVargas(const Enemy *enemy)
{
    return
        enemy->bossComboFrameCount > 0 &&
        enemy->bossKneeFrameCount > 0 &&
        enemy->bossUppercutFrameCount > 0 &&
        enemy->bossHeavyBlowFrameCount > 0;
}


static EnemyAttackMove GetNextBossAttackMove(EnemyAttackMove move)
{
    switch (move)
    {
        case ENEMY_ATTACK_PUNCH:
            return ENEMY_ATTACK_ELBOW;
        case ENEMY_ATTACK_ELBOW:
            return ENEMY_ATTACK_BOSS_COMBO;
        case ENEMY_ATTACK_BOSS_COMBO:
            return ENEMY_ATTACK_BOSS_KNEE;
        case ENEMY_ATTACK_BOSS_KNEE:
            return ENEMY_ATTACK_BOSS_UPPERCUT;
        case ENEMY_ATTACK_BOSS_UPPERCUT:
            return ENEMY_ATTACK_BOSS_HEAVY_BLOW;
        case ENEMY_ATTACK_BOSS_HEAVY_BLOW:
        default:
            return ENEMY_ATTACK_PUNCH;
    }
}


static float GetBossLungeDistance(const Enemy *enemy)
{
    switch (enemy->currentAttackMove)
    {
        case ENEMY_ATTACK_BOSS_COMBO:
            return enemy->bossComboLungeDistance;
        case ENEMY_ATTACK_BOSS_KNEE:
            return enemy->bossKneeLungeDistance;
        case ENEMY_ATTACK_BOSS_UPPERCUT:
            return enemy->bossUppercutLungeDistance;
        default:
            return 0.0f;
    }
}


static bool IsBossLungeFrame(const Enemy *enemy)
{
    if (enemy->currentAttackMove == ENEMY_ATTACK_BOSS_COMBO)
    {
        // Visible Frames 2-4.
        return enemy->attackFrame >= 1 && enemy->attackFrame <= 3;
    }

    if (
        enemy->currentAttackMove == ENEMY_ATTACK_BOSS_KNEE ||
        enemy->currentAttackMove == ENEMY_ATTACK_BOSS_UPPERCUT
    )
    {
        // Visible Frames 2-3.
        return enemy->attackFrame >= 1 && enemy->attackFrame <= 2;
    }

    return false;
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
    // Vargas Combo, Knee, and Uppercut hold visible Frame 1 for 0.70s.
    // The forward slide begins only after this freeze finishes.
    if (
        frame == 0 &&
        (
            enemy->currentAttackMove == ENEMY_ATTACK_BOSS_COMBO ||
            enemy->currentAttackMove == ENEMY_ATTACK_BOSS_KNEE ||
            enemy->currentAttackMove == ENEMY_ATTACK_BOSS_UPPERCUT
        )
    )
    {
        return 0.70f;
    }

    // Punk Elbow timing:
    // Frame 1 = wind-up
    // Frame 2 = freeze / hold (no hitbox)
    // Frame 3 = strike (yellow hitbox active)
    // Frame 4 = recovery
    if (
        !IsVargas(enemy) &&
        enemy->currentAttackMove == ENEMY_ATTACK_ELBOW &&
        frame == 1
    )
    {
        return 0.32f;
    }

    // Visible Frame 3 (index 2) stays held while Punk slides forward.
    if (
        !IsVargas(enemy) &&
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
        (!IsVargas(enemy) && enemy->currentAttackMove == ENEMY_ATTACK_ELBOW)
        ? enemy->elbowLungeDistance
        : 0.0f;

    enemy->bossLungeRemaining = GetBossLungeDistance(enemy);

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

    float attackDeltaTime =
        deltaTime *
        EnemyGetAnimationSpeedMultiplier(enemy);

    AdvanceEnemyAttackAnimation(enemy, attackDeltaTime);
    // ============================================================
    // 0038 - ELBOW FRAME 3 FORWARD SLIDE
    // ============================================================
    // attackFrame == 2 means visible Frame 3.
    // Move the actual Punk hurtbox/body forward while Frame 3 is held.
    if (
        !IsVargas(enemy) &&
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

    // Combo slides on visible Frames 2-4.
    // Knee and Uppercut slide on visible Frames 2-3.
    if (IsBossLungeFrame(enemy) && enemy->bossLungeRemaining > 0.0f)
    {
        int slideFrameCount =
            (enemy->currentAttackMove == ENEMY_ATTACK_BOSS_COMBO) ? 3 : 2;

        float fullDistance = GetBossLungeDistance(enemy);
        float slideDuration = enemy->attackFrameTime * (float)slideFrameCount;
        float slideAmount =
            (fullDistance / slideDuration) *
            attackDeltaTime;

        if (slideAmount > enemy->bossLungeRemaining)
        {
            slideAmount = enemy->bossLungeRemaining;
        }

        enemy->hurtbox.x +=
            slideAmount * (float)enemy->attackDirection;
        enemy->bossLungeRemaining -= slideAmount;
    }

    // Default impact is visible Frame 3. This keeps Boss Kick damage
    // on Frame 3 only. Vargas Punch is a two-hit left/right attack,
    // so visible Frames 2 and 3 both deal damage.
    bool attackHitboxActive = (enemy->attackFrame == 2);
    if (
        IsVargas(enemy) &&
        enemy->currentAttackMove == ENEMY_ATTACK_PUNCH
    )
    {
        attackHitboxActive =
            (enemy->attackFrame == 1 || enemy->attackFrame == 2);
    }
    else if (enemy->currentAttackMove == ENEMY_ATTACK_BOSS_COMBO)
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

    enemy->attackTimer -= attackDeltaTime;

    if (enemy->attackTimer <= 0.0f)
    {
        enemy->isAttacking = false;
        enemy->attackTimer = 0.0f;
        enemy->attackCooldownTimer = 1.10f;
        enemy->hitPlayerThisAttack = false;
        enemy->attackFrame = 0;
        enemy->attackFrameTimer = 0.0f;
        enemy->elbowLungeRemaining = 0.0f;
        enemy->bossLungeRemaining = 0.0f;

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

        // Vargas cycle:
        // Punch -> Kick -> Combo -> Knee -> Uppercut -> Heavy Blow -> repeat.
        if (!enemy->lockAttackMove && IsVargas(enemy))
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