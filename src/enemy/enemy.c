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
    enemy.ownsTextures = true;

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
    enemy.attackSlotGranted = false;

    // 0044 rollback/fix:
    // Keep attack permission open so the working 0043 attack flow is restored.
    // We will add shared turn timing again only after the basic attacks work.
    enemy.attackTurnAllowed = true;

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

    // 0040 - Multi-enemy spacing defaults.
    enemy.separationRadiusX = 120.0f;
    enemy.separationDepthTolerance = 70.0f;
    enemy.separationPushSpeed = 260.0f;

    // 0042 - Surround / formation defaults.
    enemy.surroundEnabled = false;
    enemy.surroundOffsetX = 0.0f;
    enemy.surroundOffsetY = 0.0f;
    enemy.surroundArrivalTolerance = 18.0f;

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


// ============================================================
// 0043 - DYNAMIC ATTACK SLOT SWAP / TAKEOVER SYSTEM
// ============================================================
// Maximum two active attackers are still allowed.
//
// New behavior:
// - A Punk that is already attacking keeps its slot until the attack ends.
// - Current non-attacking slot owners get a small stability bonus so the
//   slots do not flicker between enemies every frame.
// - A waiting/surround Punk that the player moves close to gets a strong
//   takeover bonus. This lets that Punk become one of the two attackers,
//   while a farther non-attacking attacker returns to the surround role.
void ResolveEnemyAttackSlot(
    Enemy *enemies,
    int enemyCount,
    const Player *player
)
{
    if (enemies == 0 || enemyCount <= 0 || player == 0)
    {
        return;
    }

    const int maxActiveAttackers = 2;

    // 0043 tuning values.
    // A waiting Punk inside this area is considered close enough to
    // challenge an existing non-attacking slot owner.
    const float takeoverDistanceX = 190.0f;
    const float takeoverDepth = 95.0f;

    // Lower score = higher priority.
    const float currentOwnerHoldBonus = 25.0f;
    const float waitingTakeoverBonus = 140.0f;

    // Remember who owned a slot before rebuilding the two slots.
    // This is what lets us tell an old attacker from a waiting Punk.
    bool previousSlotOwners[enemyCount];

    for (int i = 0; i < enemyCount; i++)
    {
        previousSlotOwners[i] = enemies[i].attackSlotGranted;
        enemies[i].attackSlotGranted = false;
    }

    int granted = 0;

    // A Punk already performing an attack cannot lose its slot halfway
    // through the animation. It finishes first, then it may be swapped out.
    for (int i = 0; i < enemyCount && granted < maxActiveAttackers; i++)
    {
        if (enemies[i].isAlive && enemies[i].isAttacking)
        {
            enemies[i].attackSlotGranted = true;
            granted++;
        }
    }

    // Fill the remaining slots using distance + 0043 swap priority.
    while (granted < maxActiveAttackers)
    {
        int bestIndex = -1;
        float bestScore = 1000000000.0f;

        float playerCenterX =
            player->rectangle.x + player->rectangle.width * 0.5f;

        float playerGroundY =
            player->rectangle.y + player->rectangle.height;

        for (int i = 0; i < enemyCount; i++)
        {
            Enemy *enemy = &enemies[i];

            if (
                !enemy->isAlive ||
                enemy->isEntering ||
                !enemy->hasEnteredStage ||
                enemy->isHit ||
                enemy->attackSlotGranted
            )
            {
                continue;
            }

            float enemyCenterX =
                enemy->hurtbox.x + enemy->hurtbox.width * 0.5f;

            float enemyStageY =
                enemy->hurtbox.y + enemy->stageAnchorOffsetY;

            float dx = playerCenterX - enemyCenterX;
            float dy = playerGroundY - enemyStageY;

            if (dx < 0.0f) dx = -dx;
            if (dy < 0.0f) dy = -dy;

            // Horizontal distance matters most; depth is a smaller penalty.
            float score = dx + dy * 0.75f;

            // Keep existing non-attacking owners slightly stable.
            if (previousSlotOwners[i])
            {
                score -= currentOwnerHoldBonus;
            }
            else if (
                dx <= takeoverDistanceX &&
                dy <= takeoverDepth
            )
            {
                // 0043 TAKEOVER:
                // The player approached a waiting Punk, so give that Punk
                // strong priority to take one of the two attack slots.
                score -= waitingTakeoverBonus;
            }

            if (score < bestScore)
            {
                bestScore = score;
                bestIndex = i;
            }
        }

        if (bestIndex < 0)
        {
            break;
        }

        enemies[bestIndex].attackSlotGranted = true;
        granted++;
    }
}

void UnloadEnemy(Enemy *enemy)
{
    // 0040 - Shared Punk textures are unloaded once by UnloadPunkSharedTextures().
    if (!enemy->ownsTextures)
    {
        return;
    }

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