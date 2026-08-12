#include "enemy.h"
#include "enemy_internal.h"


Enemy InitEnemyBase(void)
{
    Enemy enemy = {0};

    // ============================================================
    // COMMON ENEMY DEFAULTS
    // ============================================================

    enemy.maxHp = 100;
    enemy.hp = enemy.maxHp;
    enemy.isAlive = true;
    enemy.hitByCurrentAttack = false;

    // ============================================================
    // HIT REACTION / KNOCKBACK DEFAULTS
    // ============================================================

    enemy.isHit = false;
    enemy.hitReactionTimer = 0.0f;
    enemy.knockbackSpeed = 0.0f;
    enemy.knockbackDirection = 0;

    // ============================================================
    // 0030 - BASIC ENEMY ATTACK DEFAULTS
    // ============================================================

    enemy.isAttacking = false;
    enemy.attackTimer = 0.0f;
    enemy.attackCooldownTimer = 0.60f;
    enemy.hitPlayerThisAttack = false;

    enemy.attackDamage = 10;
    enemy.attackRange = 210.0f;
    enemy.attackHitboxWidth = 150.0f;
    enemy.attackHitboxHeight = 120.0f;
    enemy.attackKnockbackSpeed = 190.0f;
    enemy.attackHitReactionTime = 0.16f;
    enemy.attackDirection = -1;

    // 0037 - Per-move attack distance defaults.
    enemy.punchAttackRange = 140.0f;
    enemy.punchStopDistance = 140.0f;

    enemy.elbowAttackRange = 110.0f;
    enemy.elbowStopDistance = 110.0f;

    // 0037 - Per-move attack hitbox defaults.
    // Punk overrides these values in punk.c.
    enemy.punchHitboxWidth = 150.0f;
    enemy.punchHitboxHeight = 120.0f;
    enemy.punchHitboxOffsetX = 0.0f;
    enemy.punchHitboxOffsetY = 0.0f;

    enemy.elbowHitboxWidth = 150.0f;
    enemy.elbowHitboxHeight = 120.0f;
    enemy.elbowHitboxOffsetX = 0.0f;
    enemy.elbowHitboxOffsetY = 0.0f;

    // 0037 - Attack animation defaults.
    enemy.punchFrameCount = 0;
    enemy.elbowFrameCount = 0;
    enemy.attackFrame = 0;
    enemy.attackFrameTimer = 0.0f;
    enemy.attackFrameTime = 0.08f;
    enemy.currentAttackMove = ENEMY_ATTACK_PUNCH;
    enemy.nextAttackMove = ENEMY_ATTACK_PUNCH;

    // ============================================================
    // 0031 - FACING + CHASE AI DEFAULTS
    // ============================================================

    enemy.facingRight = false;
    enemy.isChasing = false;
    enemy.chaseSpeed = 115.0f;
    enemy.chaseStopDistance = 155.0f;
    enemy.chaseDepthTolerance = 8.0f;

    // ============================================================
    // 0035 - ENEMY STOP / ATTACK RANGE DEFAULTS
    // ============================================================

    enemy.attackStopDistance = 210.0f;
    enemy.isInAttackRange = false;
    enemy.aggroRange = 500.0f;

    // ============================================================
    // 0032 - ENEMY STAGE BOUNDARY DEFAULTS
    // ============================================================

    enemy.stageAnchorOffsetY = 0.0f;

    // ============================================================
    // 0034 - ENEMY ENTRANCE / SPAWN DEFAULTS
    // ============================================================

    enemy.isEntering = false;
    enemy.hasEnteredStage = true;
    enemy.entranceTargetX = 0.0f;
    enemy.entranceTargetY = 0.0f;
    enemy.entranceSpeed = 140.0f;

    // ============================================================
    // GENERIC IDLE DEFAULTS
    // ============================================================

    enemy.idleFrameCount = 0;
    enemy.idleFrame = 0;
    enemy.idleDirection = 1;
    enemy.idleTimer = 0.0f;
    enemy.idleFrameTime = 0.19f;

    // ============================================================
    // 0036 - GENERIC WALK DEFAULTS
    // ============================================================

    enemy.walkFrameCount = 0;
    enemy.walkFrame = 0;
    enemy.walkTimer = 0.0f;
    enemy.walkFrameTime = 0.11f;

    // ============================================================
    // GENERIC SPRITE DEFAULTS
    // ============================================================

    enemy.spriteSize = 580.0f;
    enemy.spriteOffsetX = 0.0f;
    enemy.spriteOffsetY = 0.0f;

    return enemy;
}


void UpdateEnemyHit(
    Enemy *enemy,
    Player *player,
    float deltaTime,
    float screenWidth,
    float walkAreaTop,
    float walkAreaBottom
)
{
    if (!enemy->isAlive)
    {
        return;
    }

    // ============================================================
    // ENTRANCE
    // ============================================================

    if (enemy->isEntering)
    {
        bool entranceFinished =
            EnemyUpdateEntrance(enemy, deltaTime);

        if (!entranceFinished)
        {
            EnemyUpdateAnimation(enemy, deltaTime, true);
            return;
        }

        EnemyClampToStage(
            enemy,
            screenWidth,
            walkAreaTop,
            walkAreaBottom
        );
    }

    // ============================================================
    // HIT REACTION / KNOCKBACK
    // ============================================================

    EnemyUpdateHitReaction(
        enemy,
        deltaTime,
        screenWidth,
        walkAreaTop,
        walkAreaBottom
    );

    // ============================================================
    // ATTACK COOLDOWN
    // ============================================================

    if (enemy->attackCooldownTimer > 0.0f)
    {
        enemy->attackCooldownTimer -= deltaTime;
    }

    // ============================================================
    // BUILD CURRENT COMBAT / POSITION DATA
    // ============================================================

    EnemyCombatContext context =
        EnemyBuildCombatContext(enemy, player);

    // ============================================================
    // FACING + CHASE MOVEMENT
    // ============================================================

    EnemyUpdateChase(
        enemy,
        player,
        deltaTime,
        screenWidth,
        walkAreaTop,
        walkAreaBottom,
        &context
    );

    // ============================================================
    // ATTACK STATE + ATTACK DAMAGE
    // ============================================================

    EnemyUpdateAttack(
        enemy,
        player,
        deltaTime,
        &context
    );

    // ============================================================
    // IDLE / WALK ANIMATION
    // ============================================================

    EnemyUpdateAnimation(
        enemy,
        deltaTime,
        enemy->isChasing &&
        !enemy->isAttacking &&
        !enemy->isHit
    );

    // ============================================================
    // FINAL STAGE BOUNDARY SAFETY
    // ============================================================

    EnemyClampToStage(
        enemy,
        screenWidth,
        walkAreaTop,
        walkAreaBottom
    );

    // ============================================================
    // PLAYER ATTACK -> ENEMY DAMAGE
    // ============================================================

    EnemyCheckPlayerAttack(enemy, player);
}


void UnloadEnemy(Enemy *enemy)
{
    for (int i = 0; i < enemy->idleFrameCount; i++)
    {
        UnloadTexture(enemy->idleTextures[i]);
    }

    for (int i = 0; i < enemy->walkFrameCount; i++)
    {
        UnloadTexture(enemy->walkTextures[i]);
    }

    for (int i = 0; i < enemy->punchFrameCount; i++)
    {
        UnloadTexture(enemy->punchTextures[i]);
    }

    for (int i = 0; i < enemy->elbowFrameCount; i++)
    {
        UnloadTexture(enemy->elbowTextures[i]);
    }
}