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
    enemy.displayName = "ENEMY";
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
    // 0050 - ENEMY DEATH FREEZE + DEAD SPRITE DEFAULTS
    // ============================================================

    enemy.isDying = false;
    enemy.deathFinished = false;

    enemy.deathFreezeTimer = 0.0f;
    enemy.deathFreezeDuration = 0.10f;
    enemy.deathFreezeTexture = (Texture2D){0};

    // Keep the final body on screen briefly before it disappears.
    enemy.deathTimer = 0.0f;
    enemy.deathDuration = 3.00f; //death duration

    enemy.deathFrameCount = 0;
    enemy.deathFrame = 0;
    enemy.deathFrameTimer = 0.0f;
    enemy.deathFrameTime = 0.50f;

    // ============================================================
    // 0030 - BASIC ENEMY ATTACK DEFAULTS
    // ============================================================

    enemy.isAttacking = false;
    enemy.attackTimer = 0.0f;
    enemy.attackCooldownTimer = 0.60f;
    enemy.hitPlayerThisAttack = false;
    enemy.attackSlotGranted = false;

    // 0044 - Shared turn controller grants this before a new attack starts.
    enemy.attackTurnAllowed = false;

    // 0045 - Post-attack backward retreat defaults.
    enemy.isRetreating = false;
    enemy.retreatTimer = 0.0f;
    enemy.retreatDuration = 1.50f;
    enemy.retreatSpeed = 105.0f;
    enemy.retreatDirection = 0;
    enemy.retreatPauseTimer = 0.0f;
    enemy.retreatPauseDuration = 0.55f;

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

    // 0046 - Blocked-approach lane bypass defaults.
    enemy.isLaneBypassing = false;
    enemy.laneBypassTargetY = 0.0f;
    enemy.laneBypassDirection = 0;

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
        // 0050 - Dead enemies leave combat immediately, but their
        // freeze/death sprite sequence continues updating.
        EnemyUpdateDeath(
            enemy,
            deltaTime,
            screenWidth,
            walkAreaTop,
            walkAreaBottom
        );
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


// ============================================================
// 0044 - ATTACK COOLDOWN / TURN TIMING SYSTEM
// ============================================================
// Two Punks may still own the 0043 attack slots, but only one Punk is
// allowed to START a new attack at a time. After any Punk begins an attack,
// the group waits briefly before another Punk may start. This prevents the
// two active attackers from looking synchronized or spammy.
void ResolveEnemyAttackTurnTiming(
    Enemy *enemies,
    int enemyCount,
    const Player *player,
    float deltaTime
)
{
    if (enemies == 0 || enemyCount <= 0 || player == 0)
    {
        return;
    }

    // Small gap between attack STARTS. Existing attacks keep playing.
    const float attackStartGap = 0.55f;

    // This controller is for the current enemy group used by the stage.
    // MAX_TRACKED_ENEMIES is intentionally larger than the current 4 Punks.
    enum { MAX_TRACKED_ENEMIES = 32 };
    static bool previousAttackState[MAX_TRACKED_ENEMIES] = {0};
    static float groupStartCooldown = 0.0f;
    static int turnCursor = 0;

    int trackedCount = enemyCount;
    if (trackedCount > MAX_TRACKED_ENEMIES)
    {
        trackedCount = MAX_TRACKED_ENEMIES;
    }

    // By default nobody may START a fresh attack this frame.
    // Punks already attacking do not need this permission to finish.
    for (int i = 0; i < enemyCount; i++)
    {
        enemies[i].attackTurnAllowed = false;
    }

    // Detect a new attack that started since the previous frame.
    // As soon as one Punk begins, restart the shared start-gap timer.
    bool sawNewAttackStart = false;

    for (int i = 0; i < trackedCount; i++)
    {
        if (enemies[i].isAttacking && !previousAttackState[i])
        {
            sawNewAttackStart = true;
        }

        previousAttackState[i] = enemies[i].isAttacking;
    }

    if (sawNewAttackStart)
    {
        groupStartCooldown = attackStartGap;
    }
    else if (groupStartCooldown > 0.0f)
    {
        groupStartCooldown -= deltaTime;

        if (groupStartCooldown < 0.0f)
        {
            groupStartCooldown = 0.0f;
        }
    }

    // During the shared gap, no second Punk may begin yet.
    if (groupStartCooldown > 0.0f || !player->isAlive)
    {
        return;
    }

    // Pick one eligible slot owner, rotating the search start so the same
    // Punk does not always win when both are ready at the same time.
    if (turnCursor < 0 || turnCursor >= enemyCount)
    {
        turnCursor = 0;
    }

    for (int step = 0; step < enemyCount; step++)
    {
        int i = (turnCursor + step) % enemyCount;
        Enemy *enemy = &enemies[i];

        if (
            !enemy->isAlive ||
            enemy->isEntering ||
            !enemy->hasEnteredStage ||
            enemy->isHit ||
            enemy->isAttacking ||
            !enemy->attackSlotGranted ||
            enemy->attackCooldownTimer > 0.0f ||
            !enemy->isInAttackRange
        )
        {
            continue;
        }

        EnemyCombatContext context =
            EnemyBuildCombatContext(enemy, player);

        float requiredAttackRange =
            (enemy->nextAttackMove == ENEMY_ATTACK_ELBOW)
            ? enemy->elbowAttackRange
            : enemy->punchAttackRange;

        if (
            !context.playerDetected ||
            context.absoluteDistanceX > requiredAttackRange ||
            !EnemyIsPlayerInVerticalRange(enemy, player)
        )
        {
            continue;
        }

        enemy->attackTurnAllowed = true;
        turnCursor = (i + 1) % enemyCount;
        break;
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

    for (int i = 0; i < enemy->deathFrameCount; i++)
    {
        UnloadTexture(enemy->deathTextures[i]);
    }
}