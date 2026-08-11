#include "enemy.h"
#include <math.h>


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

    // ============================================================
    // 0031 - FACING + CHASE AI DEFAULTS
    // ============================================================
    enemy.facingRight = false;
    enemy.isChasing = false;
    enemy.chaseSpeed = 115.0f;
    enemy.chaseStopDistance = 155.0f; // Legacy 0031 value kept for compatibility.
    enemy.chaseDepthTolerance = 8.0f;

    // ============================================================
    // 0035 - ENEMY STOP / ATTACK RANGE DEFAULTS
    // ============================================================
    // Punk stops advancing once he is inside this horizontal range.
    // This matches the current attackRange so he does not walk into
    // the player's body before starting an attack.
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


// ============================================================
// 0033 - ENEMY DEPTH SCALE (PLAYER-STYLE)
// ============================================================
// The player keeps a small logical rectangle and derives its visible
// sprite + hurtbox from the same depth formula.  Enemy now follows the
// same rule.  InitPunk() position stays logical; these helpers return
// the scaled combat body used for drawing and collisions.

static float GetEnemyPerspectiveScale(const Enemy *enemy)
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

    // Exact perspective curve used by DrawPlayer().
    float playerStyleScale =
        2.90f +
        (depth * 1.80f);

    // Punk's current 580px art / 148x276 hurtbox were tuned around
    // its original starting stage Y (~470).  Use that point as 1.0
    // so existing size/alignment is preserved there, then grow/shrink
    // with the SAME player perspective curve above/below it.
    const float referenceDepth =
        (470.0f - 345.0f) /
        (700.0f - 270.0f);

    const float referenceScale =
        2.90f +
        (referenceDepth * 1.80f);

    return playerStyleScale / referenceScale;
}

static Rectangle GetEnemyScaledHurtbox(const Enemy *enemy)
{
    float scale = GetEnemyPerspectiveScale(enemy);

    float scaledWidth =
        enemy->hurtbox.width * scale;

    float scaledHeight =
        enemy->hurtbox.height * scale;

    // Same idea as Player: keep the character anchored at the feet.
    float centerX =
        enemy->hurtbox.x +
        (enemy->hurtbox.width / 2.0f);

    float bottomY =
        enemy->hurtbox.y +
        enemy->hurtbox.height;

    return (Rectangle)
    {
        centerX - (scaledWidth / 2.0f),
        bottomY - scaledHeight,
        scaledWidth,
        scaledHeight
    };
}


// ============================================================
// 0034 - ENEMY ENTRANCE / SPAWN SYSTEM
// ============================================================
//
// Spawn position still comes from InitPunk(x, y).
// This function tells the enemy where to WALK TO before normal combat
// AI starts. While isEntering is true, 0032 stage clamping and combat
// are temporarily disabled so the enemy may begin off-screen.

void StartEnemyEntrance(
    Enemy *enemy,
    float targetX,
    float targetStageY,
    float entranceSpeed
)
{
    enemy->isEntering = true;
    enemy->hasEnteredStage = false;

    enemy->entranceTargetX = targetX;
    enemy->entranceTargetY =
        targetStageY -
        enemy->stageAnchorOffsetY;

    enemy->entranceSpeed = entranceSpeed;

    enemy->isChasing = false;
    enemy->isAttacking = false;
    enemy->attackTimer = 0.0f;
    enemy->hitPlayerThisAttack = false;
}


static bool UpdateEnemyEntrance(
    Enemy *enemy,
    float deltaTime
)
{
    if (!enemy->isEntering)
    {
        return true;
    }

    float differenceX =
        enemy->entranceTargetX -
        enemy->hurtbox.x;

    float differenceY =
        enemy->entranceTargetY -
        enemy->hurtbox.y;

    float absoluteX = differenceX;
    float absoluteY = differenceY;

    if (absoluteX < 0.0f)
        absoluteX = -absoluteX;

    if (absoluteY < 0.0f)
        absoluteY = -absoluteY;

    const float arrivalDistance = 3.0f;

    if (
        absoluteX <= arrivalDistance &&
        absoluteY <= arrivalDistance
    )
    {
        enemy->hurtbox.x = enemy->entranceTargetX;
        enemy->hurtbox.y = enemy->entranceTargetY;

        enemy->isEntering = false;
        enemy->hasEnteredStage = true;
        enemy->isChasing = false;

        return true;
    }

    float moveX = 0.0f;
    float moveY = 0.0f;

    if (absoluteX > arrivalDistance)
        moveX = (differenceX > 0.0f) ? 1.0f : -1.0f;

    if (absoluteY > arrivalDistance)
        moveY = (differenceY > 0.0f) ? 1.0f : -1.0f;

    if (moveX != 0.0f && moveY != 0.0f)
    {
        const float diagonalFactor = 0.70710678f;
        moveX *= diagonalFactor;
        moveY *= diagonalFactor;
    }

    enemy->hurtbox.x +=
        moveX *
        enemy->entranceSpeed *
        deltaTime;

    enemy->hurtbox.y +=
        moveY *
        enemy->entranceSpeed *
        deltaTime;

    if (
        (differenceX > 0.0f && enemy->hurtbox.x > enemy->entranceTargetX) ||
        (differenceX < 0.0f && enemy->hurtbox.x < enemy->entranceTargetX)
    )
    {
        enemy->hurtbox.x = enemy->entranceTargetX;
    }

    if (
        (differenceY > 0.0f && enemy->hurtbox.y > enemy->entranceTargetY) ||
        (differenceY < 0.0f && enemy->hurtbox.y < enemy->entranceTargetY)
    )
    {
        enemy->hurtbox.y = enemy->entranceTargetY;
    }

    if (moveX > 0.0f)
        enemy->facingRight = true;
    else if (moveX < 0.0f)
        enemy->facingRight = false;

    return false;
}


// ============================================================
// 0032 - ENEMY STAGE BOUNDARY / WALK AREA CLAMP
// ============================================================
//
// The player already uses walkAreaTop / walkAreaBottom in main.c.
// Enemy characters use the same stage limits, but their hurtbox may
// be positioned above the logical stage Y. stageAnchorOffsetY converts
// hurtbox.y back to the character's stage-position anchor.
//
// This clamp is for NORMAL combat movement. A future entrance/spawn
// system can intentionally skip this clamp while an enemy is entering.

static void ClampEnemyToStage(
    Enemy *enemy,
    float screenWidth,
    float walkAreaTop,
    float walkAreaBottom
)
{
    // Horizontal stage limits.
    if (enemy->hurtbox.x < 0.0f)
    {
        enemy->hurtbox.x = 0.0f;
    }

    if (enemy->hurtbox.x + enemy->hurtbox.width > screenWidth)
    {
        enemy->hurtbox.x = screenWidth - enemy->hurtbox.width;
    }

    // Convert hurtbox Y to the same logical stage-Y system used by
    // InitPunk(x, y) and the player's walk-area limits.
    float stageY =
        enemy->hurtbox.y +
        enemy->stageAnchorOffsetY;

    // 0035 - Allow Punk to move higher than Player top boundary.
    const float enemyTopExtra = 30.0f;

    float enemyWalkAreaTop =
        walkAreaTop - enemyTopExtra;

    if (stageY < enemyWalkAreaTop)
    {
        enemy->hurtbox.y =
            enemyWalkAreaTop -
            enemy->stageAnchorOffsetY;
    }

    if (stageY > walkAreaBottom)
    {
        enemy->hurtbox.y =
            walkAreaBottom -
            enemy->stageAnchorOffsetY;
    }
}


// ============================================================
// 0030 FIX - VERTICAL / DEPTH RANGE CHECK
// ============================================================
//
// Do not divide the floor into artificial TOP/MIDDLE/BOTTOM zones.
// Combat now uses the actual rectangles shown on screen.
// If the player hurtbox is vertically near the enemy body, the
// enemy is allowed to start an attack. Actual damage still requires
// the YELLOW attack hitbox to overlap the GREEN player hurtbox.

static bool IsPlayerInEnemyVerticalRange(
    const Enemy *enemy,
    const Player *player
)
{
    Rectangle playerHurtbox = GetPlayerHurtbox(player);

    Rectangle enemyHurtbox =
        GetEnemyScaledHurtbox(enemy);

    float enemyTop = enemyHurtbox.y;
    float enemyBottom =
        enemyHurtbox.y + enemyHurtbox.height;

    float playerTop = playerHurtbox.y;
    float playerBottom =
        playerHurtbox.y + playerHurtbox.height;

    // Small forgiveness around the visible boxes.
    const float verticalMargin = 35.0f;

    return
        playerBottom >= enemyTop - verticalMargin &&
        playerTop <= enemyBottom + verticalMargin;
}


// ============================================================
// 0030 FIX 3 - ENEMY FOOT / GROUND MARKER
// ============================================================

Rectangle GetEnemyFootMarker(const Enemy *enemy)
{
    // Same pattern as GetPlayerFootMarker(): derive feet from the
    // already depth-scaled body hurtbox, not from the logical box.
    Rectangle hurtbox =
        GetEnemyScaledHurtbox(enemy);

    Rectangle feet =
    {
        hurtbox.x + (hurtbox.width * -0.80f),
        hurtbox.y + hurtbox.height - 34.0f,
        hurtbox.width * 2.50f,
        44.0f
    };

    return feet;
}


// ============================================================
// 0030 FIX 3 - GROUND DEPTH CHECK
// ============================================================
//
// Body overlap is NOT enough.
// Player and enemy must also overlap at the foot/ground marker.

static bool IsSameGroundDepth(
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

// ============================================================
// 0036 - ENEMY IDLE / WALK ANIMATION UPDATE
// ============================================================

static void UpdateEnemyAnimation(
    Enemy *enemy,
    float deltaTime,
    bool isWalking
)
{
    if (isWalking && enemy->walkFrameCount > 0)
    {
        enemy->walkTimer += deltaTime;

        while (enemy->walkTimer >= enemy->walkFrameTime)
        {
            enemy->walkTimer -= enemy->walkFrameTime;
            enemy->walkFrame++;

            if (enemy->walkFrame >= enemy->walkFrameCount)
            {
                enemy->walkFrame = 0;
            }
        }

        return;
    }

    // Restart the next walk cycle from frame 1.
    enemy->walkFrame = 0;
    enemy->walkTimer = 0.0f;

    if (enemy->idleFrameCount > 1)
    {
        enemy->idleTimer += deltaTime;

        if (enemy->idleTimer >= enemy->idleFrameTime)
        {
            enemy->idleTimer -= enemy->idleFrameTime;
            enemy->idleFrame += enemy->idleDirection;

            if (enemy->idleFrame >= enemy->idleFrameCount - 1)
            {
                enemy->idleFrame = enemy->idleFrameCount - 1;
                enemy->idleDirection = -1;
            }
            else if (enemy->idleFrame <= 0)
            {
                enemy->idleFrame = 0;
                enemy->idleDirection = 1;
            }
        }
    }
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
    // Kapag patay na ang enemy, hihinto ang animation at movement.
    if (!enemy->isAlive)
    {
        return;
    }

    // ============================================================
    // 0034 - ENEMY ENTRANCE UPDATE
    // ============================================================
    //
    // During entrance:
    // - enemy may remain outside the 0032 boundary
    // - normal chase is disabled
    // - attacks / damage collision are disabled
    //
    // Once the target is reached, normal combat resumes.
    if (enemy->isEntering)
    {
        bool entranceFinished =
            UpdateEnemyEntrance(
                enemy,
                deltaTime
            );

        if (!entranceFinished)
        {
            // 0036 - Entrance movement uses the walk animation.
            UpdateEnemyAnimation(enemy, deltaTime, true);
            return;
        }

        ClampEnemyToStage(
            enemy,
            screenWidth,
            walkAreaTop,
            walkAreaBottom
        );
    }

    // ============================================================
    // ACTIVE HIT REACTION / KNOCKBACK
    // ============================================================

    if (enemy->isHit)
    {
        enemy->hurtbox.x +=
            enemy->knockbackDirection *
            enemy->knockbackSpeed *
            deltaTime;

        enemy->hitReactionTimer -= deltaTime;

        // 0032 - Keep knockback inside the normal combat area.
        ClampEnemyToStage(
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

    // ============================================================
    // 0030 - ENEMY ATTACK UPDATE
    // ============================================================

    if (enemy->attackCooldownTimer > 0.0f)
    {
        enemy->attackCooldownTimer -= deltaTime;
    }

    Rectangle scaledEnemyHurtbox =
        GetEnemyScaledHurtbox(enemy);

    float enemyCenterX =
        scaledEnemyHurtbox.x +
        scaledEnemyHurtbox.width / 2.0f;

    Rectangle playerHurtbox =
        GetPlayerHurtbox(player);

    float playerCenterX =
        playerHurtbox.x +
        playerHurtbox.width / 2.0f;

    float distanceX =
        playerCenterX - enemyCenterX;

    if (distanceX >= 0.0f)
    {
        enemy->attackDirection = 1;
    }
    else
    {
        enemy->attackDirection = -1;
    }

    float absoluteDistanceX = distanceX;

    if (absoluteDistanceX < 0.0f)
    {
        absoluteDistanceX = -absoluteDistanceX;
    }

    // ============================================================
    // 0031 - ENEMY FACING + CHASE AI
    // ============================================================
    // Face the player whenever the Punk is free to react.
    if (!enemy->isAttacking && !enemy->isHit)
    {
        enemy->facingRight = (distanceX >= 0.0f);
    }

    Rectangle enemyFeet = GetEnemyFootMarker(enemy);
    Rectangle playerFeet = GetPlayerFootMarker(player);

    float enemyGroundY = enemyFeet.y + enemyFeet.height / 2.0f;
    float playerGroundY = playerFeet.y + playerFeet.height / 2.0f;
    float depthDifference = playerGroundY - enemyGroundY;

    float absoluteDepthDifference = depthDifference;
    if (absoluteDepthDifference < 0.0f)
    {
        absoluteDepthDifference = -absoluteDepthDifference;
    }

    // ============================================================
    // 0034 - PLAYER DETECTION / AGGRO RANGE
    // ============================================================
    // Use both horizontal distance and stage-depth distance.
    // This prevents Punk from chasing when the player is still far away.
    float aggroDistance =
        sqrtf(
            (distanceX * distanceX) +
            (depthDifference * depthDifference)
        );

    bool playerDetected =
        aggroDistance <= enemy->aggroRange;

    // ============================================================
    // 0035 - ENEMY STOP / ATTACK RANGE
    // ============================================================
    //
    // Punk must satisfy BOTH conditions before he fully stops:
    // 1. Close enough on X.
    // 2. Feet/depth aligned with the player.
    //
    // This prevents Punk from walking into the player's body and also
    // prevents him from stopping too early while still on another lane.
    enemy->isInAttackRange =
        playerDetected &&
        player->isAlive &&
        absoluteDistanceX <= enemy->attackStopDistance &&
        absoluteDepthDifference <= enemy->chaseDepthTolerance;

    enemy->isChasing = false;

    if (
        !enemy->isAttacking &&
        !enemy->isHit &&
        player->isAlive &&
        playerDetected &&
        !enemy->isInAttackRange
    )
    {
        float moveX = 0.0f;
        float moveY = 0.0f;

        // 0035:
        // Stop horizontal advance at attackStopDistance.
        if (absoluteDistanceX > enemy->attackStopDistance)
        {
            moveX = (distanceX > 0.0f) ? 1.0f : -1.0f;
        }

        // Continue aligning on the depth axis until the feet match.
        if (absoluteDepthDifference > enemy->chaseDepthTolerance)
        {
            moveY = (depthDifference > 0.0f) ? 1.0f : -1.0f;
        }

        if (moveX != 0.0f || moveY != 0.0f)
        {
            enemy->isChasing = true;

            // Normalize diagonal chase so diagonal movement is not faster.
            if (moveX != 0.0f && moveY != 0.0f)
            {
                const float diagonalFactor = 0.70710678f;
                moveX *= diagonalFactor;
                moveY *= diagonalFactor;
            }

            enemy->hurtbox.x += moveX * enemy->chaseSpeed * deltaTime;
            enemy->hurtbox.y += moveY * enemy->chaseSpeed * deltaTime;

            // 0032 - Clamp normal chase to the stage walk area.
            ClampEnemyToStage(
                enemy,
                screenWidth,
                walkAreaTop,
                walkAreaBottom
            );

            // Recalculate after movement so 0035 and attack logic
            // use the enemy's fresh position this frame.
            scaledEnemyHurtbox = GetEnemyScaledHurtbox(enemy);
            enemyCenterX =
                scaledEnemyHurtbox.x +
                scaledEnemyHurtbox.width / 2.0f;

            distanceX = playerCenterX - enemyCenterX;
            absoluteDistanceX = distanceX;

            if (absoluteDistanceX < 0.0f)
            {
                absoluteDistanceX = -absoluteDistanceX;
            }

            enemyFeet = GetEnemyFootMarker(enemy);
            enemyGroundY =
                enemyFeet.y +
                enemyFeet.height / 2.0f;

            depthDifference =
                playerGroundY -
                enemyGroundY;

            absoluteDepthDifference = depthDifference;

            if (absoluteDepthDifference < 0.0f)
            {
                absoluteDepthDifference = -absoluteDepthDifference;
            }

            enemy->isInAttackRange =
                absoluteDistanceX <= enemy->attackStopDistance &&
                absoluteDepthDifference <= enemy->chaseDepthTolerance;

            enemy->attackDirection = (distanceX >= 0.0f) ? 1 : -1;
            enemy->facingRight = (distanceX >= 0.0f);
        }
    }

    // Start a basic attack only after Punk has reached the 0035
    // stop/attack position.
    if (
        !enemy->isAttacking &&
        !enemy->isHit &&
        enemy->attackCooldownTimer <= 0.0f &&
        enemy->isInAttackRange &&
        absoluteDistanceX <= enemy->attackRange &&
        IsPlayerInEnemyVerticalRange(enemy, player) &&
        player->isAlive &&
        playerDetected
    )
    {
        enemy->isAttacking = true;
        enemy->attackTimer = 0.18f;
        enemy->hitPlayerThisAttack = false;
    }

    if (enemy->isAttacking)
    {
        Rectangle enemyAttackHitbox =
            GetEnemyAttackHitbox(enemy);

        if (
            !enemy->hitPlayerThisAttack &&
            player->isAlive &&
            IsSameGroundDepth(enemy, player) &&
            CheckCollisionRecs(
                enemyAttackHitbox,
                playerHurtbox
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

        enemy->attackTimer -= deltaTime;

        if (enemy->attackTimer <= 0.0f)
        {
            enemy->isAttacking = false;
            enemy->attackTimer = 0.0f;
            enemy->attackCooldownTimer = 1.10f;
            enemy->hitPlayerThisAttack = false;
        }
    }

    // ============================================================
    // 0036 - IDLE / WALK ANIMATION STATE
    // ============================================================
    UpdateEnemyAnimation(
        enemy,
        deltaTime,
        enemy->isChasing &&
        !enemy->isAttacking &&
        !enemy->isHit
    );

    // ============================================================
    // 0032 - FINAL NORMAL-COMBAT BOUNDARY SAFETY
    // ============================================================
    ClampEnemyToStage(
        enemy,
        screenWidth,
        walkAreaTop,
        walkAreaBottom
    );

    // ============================================================
    // PLAYER ATTACK COLLISION
    // ============================================================

    // Reset once the player's attack is completely finished.
    if (!player->isAttacking)
    {
        enemy->hitByCurrentAttack = false;
    }

    if (
        IsPlayerAttackHitboxActive(player) &&
        !enemy->hitByCurrentAttack
    )
    {
        Rectangle attackHitbox =
            GetPlayerAttackHitbox(player);

        if (
            IsSameGroundDepth(enemy, player) &&
            CheckCollisionRecs(
                attackHitbox,
                GetEnemyScaledHurtbox(enemy)
            )
        )
        {
            // ====================================================
            // DAMAGE
            // ====================================================

            enemy->hp -= GetPlayerAttackDamage(player);
            enemy->hitByCurrentAttack = true;

            // ====================================================
            // START HIT REACTION / KNOCKBACK
            // ====================================================

            enemy->isHit = true;
            enemy->hitReactionTimer =
                GetPlayerAttackHitReactionTime(player);

            enemy->knockbackSpeed =
                GetPlayerAttackKnockbackSpeed(player);

            if (player->facingRight)
            {
                enemy->knockbackDirection = 1;
            }
            else
            {
                enemy->knockbackDirection = -1;
            }

            // ====================================================
            // DEATH
            // ====================================================

            if (enemy->hp <= 0)
            {
                enemy->hp = 0;
                enemy->isAlive = false;

                // Stop knockback immediately when defeated.
                enemy->isHit = false;
                enemy->hitReactionTimer = 0.0f;
                enemy->knockbackSpeed = 0.0f;
                enemy->knockbackDirection = 0;
            }
        }
    }
}



// ============================================================
// 0030 - ENEMY ATTACK HITBOX
// ============================================================

Rectangle GetEnemyAttackHitbox(const Enemy *enemy)
{
    Rectangle hurtbox =
        GetEnemyScaledHurtbox(enemy);

    float scale =
        GetEnemyPerspectiveScale(enemy);

    float attackWidth =
        enemy->attackHitboxWidth * scale;

    float attackHeight =
        enemy->attackHitboxHeight * scale;

    float centerY =
        hurtbox.y +
        hurtbox.height * 0.48f;

    Rectangle hitbox =
    {
        0.0f,
        centerY - attackHeight / 2.0f,
        attackWidth,
        attackHeight
    };

    if (enemy->attackDirection > 0)
    {
        hitbox.x =
            hurtbox.x +
            hurtbox.width -
            (10.0f * scale);
    }
    else
    {
        hitbox.x =
            hurtbox.x -
            attackWidth +
            (10.0f * scale);
    }

    return hitbox;
}

void DrawEnemy(const Enemy *enemy)
{
    // ============================================================
    // GENERIC ENEMY SPRITE
    // ============================================================

    if (enemy->idleFrameCount <= 0)
    {
        return;
    }

    // 0036 - Draw walking frames while Punk is moving.
    bool drawWalk =
        (enemy->isEntering || enemy->isChasing) &&
        !enemy->isAttacking &&
        !enemy->isHit &&
        enemy->walkFrameCount > 0;

    Texture2D currentTexture = drawWalk
        ? enemy->walkTextures[enemy->walkFrame]
        : enemy->idleTextures[enemy->idleFrame];

    Rectangle source =
    {
        0.0f,
        0.0f,
        (float)currentTexture.width,
        (float)currentTexture.height
    };

    // 0031 FIX - Mirror the Punk sprite when facing LEFT.
    // The original Punk art faces RIGHT.
    if (!enemy->facingRight)
    {
        source.x = (float)currentTexture.width;
        source.width = -(float)currentTexture.width;
    }

    // ============================================================
    // 0033 - PLAYER-STYLE ENEMY DEPTH SCALING
    // ============================================================
    float perspectiveScale =
        GetEnemyPerspectiveScale(enemy);

    Rectangle scaledHurtbox =
        GetEnemyScaledHurtbox(enemy);

    float scaledSpriteSize =
        enemy->spriteSize *
        perspectiveScale;

    // Sprite and BLUE hurtbox use the SAME scaled body anchor.
    // This keeps them as one unit from TOP -> BOTTOM.
    float hurtboxCenterX =
        scaledHurtbox.x +
        scaledHurtbox.width / 2.0f;

    float hurtboxBottomY =
        scaledHurtbox.y +
        scaledHurtbox.height;

    Rectangle destination =
    {
        hurtboxCenterX -
            scaledSpriteSize / 2.0f +
            (enemy->spriteOffsetX * perspectiveScale),

        hurtboxBottomY -
            scaledSpriteSize +
            (enemy->spriteOffsetY * perspectiveScale),

        scaledSpriteSize,
        scaledSpriteSize
    };

    Color spriteTint = WHITE;

    if (!enemy->isAlive)
    {
        spriteTint = GRAY;
    }
    else if (enemy->isHit)
    {
        spriteTint = ORANGE;
    }

    DrawTexturePro(
        currentTexture,
        source,
        destination,
        (Vector2){0.0f, 0.0f},
        0.0f,
        spriteTint
    );

    // ============================================================
    // DEBUG HURTBOX
    // ============================================================

    DrawRectangleLinesEx(
        scaledHurtbox,
        4.0f,
        BLUE
    );

    // ============================================================
    // 0030 FIX 3 - ENEMY FOOT MARKER DEBUG
    // ============================================================
    Rectangle enemyFeet =
        GetEnemyFootMarker(enemy);

    DrawRectangleRec(
        enemyFeet,
        Fade(ORANGE, 0.35f)
    );

    DrawRectangleLinesEx(
        enemyFeet,
        3.0f,
        ORANGE
    );

    // ============================================================
    // 0030 - ENEMY ATTACK HITBOX DEBUG
    // ============================================================

    if (enemy->isAttacking)
    {
        Rectangle attackHitbox =
            GetEnemyAttackHitbox(enemy);

        DrawRectangleRec(
            attackHitbox,
            Fade(YELLOW, 0.30f)
        );

        DrawRectangleLinesEx(
            attackHitbox,
            4.0f,
            YELLOW
        );
    }

    // ============================================================
    // HP BAR
    // ============================================================

    float hpPercent =
        (float)enemy->hp /
        (float)enemy->maxHp;

    Rectangle hpBack =
    {
        scaledHurtbox.x,
        scaledHurtbox.y - 24.0f,
        scaledHurtbox.width,
        12.0f
    };

    Rectangle hpFill =
    {
        hpBack.x,
        hpBack.y,
        hpBack.width * hpPercent,
        hpBack.height
    };

    DrawRectangleRec(hpBack, BLACK);
    DrawRectangleRec(hpFill, RED);
    DrawRectangleLinesEx(hpBack, 2.0f, WHITE);

    DrawText(
        TextFormat("HP: %d", enemy->hp),
        (int)scaledHurtbox.x,
        (int)scaledHurtbox.y - 50,
        20,
        WHITE
    );
}


void UnloadEnemy(Enemy *enemy)
{
    for (int i = 0; i < enemy->idleFrameCount; i++)
    {
        UnloadTexture(enemy->idleTextures[i]);
    }

    // 0036 - Unload walking textures too.
    for (int i = 0; i < enemy->walkFrameCount; i++)
    {
        UnloadTexture(enemy->walkTextures[i]);
    }
}
