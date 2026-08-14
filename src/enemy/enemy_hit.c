#include "enemy_internal.h"


static Texture2D EnemyGetCurrentTextureForDeathFreeze(
    const Enemy *enemy
)
{
    if (enemy->isAttacking)
    {
        int frame = enemy->attackFrame;

        if (
            enemy->currentAttackMove == ENEMY_ATTACK_ELBOW &&
            enemy->elbowFrameCount > 0
        )
        {
            if (frame < 0) frame = 0;
            if (frame >= enemy->elbowFrameCount)
                frame = enemy->elbowFrameCount - 1;

            if (enemy->elbowTextures[frame].id != 0)
                return enemy->elbowTextures[frame];
        }
        else if (enemy->punchFrameCount > 0)
        {
            if (frame < 0) frame = 0;
            if (frame >= enemy->punchFrameCount)
                frame = enemy->punchFrameCount - 1;

            if (enemy->punchTextures[frame].id != 0)
                return enemy->punchTextures[frame];
        }
    }

    if (
        (enemy->isEntering || enemy->isChasing) &&
        enemy->walkFrameCount > 0
    )
    {
        int frame = enemy->walkFrame;
        if (frame < 0) frame = 0;
        if (frame >= enemy->walkFrameCount)
            frame = enemy->walkFrameCount - 1;

        if (enemy->walkTextures[frame].id != 0)
            return enemy->walkTextures[frame];
    }

    int idleFrame = enemy->idleFrame;
    if (idleFrame < 0) idleFrame = 0;
    if (idleFrame >= enemy->idleFrameCount)
        idleFrame = enemy->idleFrameCount - 1;

    if (enemy->idleFrameCount > 0)
        return enemy->idleTextures[idleFrame];

    return (Texture2D){0};
}


// ============================================================
// 0050 - ENEMY DEATH FREEZE + 2-FRAME DEAD SPRITE SYSTEM
// ============================================================
void EnemyUpdateDeath(
    Enemy *enemy,
    float deltaTime,
    float screenWidth,
    float walkAreaTop,
    float walkAreaBottom
)
{
    (void)screenWidth;
    (void)walkAreaTop;
    (void)walkAreaBottom;

    if (!enemy->isDying || enemy->deathFinished)
    {
        return;
    }

    // First: freeze the exact pose from the lethal hit and draw it black.
    if (enemy->deathFreezeTimer > 0.0f)
    {
        enemy->deathFreezeTimer -= deltaTime;
        if (enemy->deathFreezeTimer < 0.0f)
        {
            enemy->deathFreezeTimer = 0.0f;
        }
    }
    else if (enemy->deathFrameCount > 0)
    {
        // Then: punk_dead1.png -> punk_death2.png.
        if (enemy->deathFrame < enemy->deathFrameCount - 1)
        {
            enemy->deathFrameTimer += deltaTime;

            if (enemy->deathFrameTimer >= enemy->deathFrameTime)
            {
                enemy->deathFrameTimer = 0.0f;
                enemy->deathFrame++;
            }
        }
    }

    enemy->deathTimer -= deltaTime;

    if (enemy->deathTimer <= 0.0f)
    {
        enemy->deathTimer = 0.0f;
        enemy->isDying = false;
        enemy->deathFinished = true;
    }
}


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

    // Receiving a non-lethal hit no longer cancels or restarts an attack.
    // Damage, hit reaction, and the small knockback still apply normally.

    enemy->hitReactionTimer =
        GetPlayerAttackHitReactionTime(player);

    enemy->knockbackSpeed =
        GetPlayerAttackKnockbackSpeed(player) * 0.15f;

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

        // Leave all normal combat systems immediately.
        enemy->deathFreezeTexture =
            EnemyGetCurrentTextureForDeathFreeze(enemy);

        enemy->isAlive = false;
        enemy->isDying = true;
        enemy->deathFinished = false;

        // 0050 timing:
        // 0.10 sec black freeze -> death frame 1 -> death frame 2 -> disappear.
        enemy->deathFreezeTimer = enemy->deathFreezeDuration;
        enemy->deathTimer = enemy->deathDuration;
        enemy->deathFrame = 0;
        enemy->deathFrameTimer = 0.0f;

        enemy->isHit = false;
        enemy->hitReactionTimer = 0.0f;
        enemy->knockbackSpeed = 0.0f;
        enemy->knockbackDirection = 0;

        enemy->isAttacking = false;
        enemy->attackTimer = 0.0f;
        enemy->attackFrame = 0;
        enemy->attackFrameTimer = 0.0f;
        enemy->hitPlayerThisAttack = false;
        enemy->attackSlotGranted = false;
        enemy->attackTurnAllowed = false;

        enemy->isChasing = false;
        enemy->isRetreating = false;
        enemy->isLaneBypassing = false;
        enemy->attackCooldownTimer = 0.0f;
    }
}