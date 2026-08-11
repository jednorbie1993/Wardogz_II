#include "enemy.h"


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
    // GENERIC IDLE DEFAULTS
    // ============================================================

    enemy.idleFrameCount = 0;
    enemy.idleFrame = 0;
    enemy.idleDirection = 1;
    enemy.idleTimer = 0.0f;
    enemy.idleFrameTime = 0.19f;

    // ============================================================
    // GENERIC SPRITE DEFAULTS
    // ============================================================

    enemy.spriteSize = 580.0f;
    enemy.spriteOffsetX = 0.0f;
    enemy.spriteOffsetY = 0.0f;

    return enemy;
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

    float enemyTop = enemy->hurtbox.y;
    float enemyBottom =
        enemy->hurtbox.y + enemy->hurtbox.height;

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
    Rectangle feet =
    {
        enemy->hurtbox.x + (enemy->hurtbox.width * -0.80f),
        enemy->hurtbox.y + enemy->hurtbox.height - 34.0f,
        enemy->hurtbox.width * 2.50f,
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

void UpdateEnemyHit(
    Enemy *enemy,
    Player *player,
    float deltaTime,
    float screenWidth
)
{
    // ============================================================
    // GENERIC IDLE ANIMATION
    // ============================================================

    // Kapag patay na ang enemy, hihinto ang animation at movement.
    if (!enemy->isAlive)
    {
        return;
    }

    if (enemy->idleFrameCount > 1)
    {
        enemy->idleTimer += deltaTime;

        if (enemy->idleTimer >= enemy->idleFrameTime)
        {
            enemy->idleTimer -= enemy->idleFrameTime;
            enemy->idleFrame += enemy->idleDirection;

            // Ping-pong:
            // 0 -> 1 -> 2 -> 1 -> 0 -> ...
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

        // Keep the enemy inside the screen.
        if (enemy->hurtbox.x < 0.0f)
        {
            enemy->hurtbox.x = 0.0f;
        }

        if (
            enemy->hurtbox.x +
            enemy->hurtbox.width >
            screenWidth
        )
        {
            enemy->hurtbox.x =
                screenWidth -
                enemy->hurtbox.width;
        }

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

    float enemyCenterX =
        enemy->hurtbox.x +
        enemy->hurtbox.width / 2.0f;

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

    // Start a basic attack when the player is close enough.
    if (
        !enemy->isAttacking &&
        !enemy->isHit &&
        enemy->attackCooldownTimer <= 0.0f &&
        absoluteDistanceX <= enemy->attackRange &&
        IsPlayerInEnemyVerticalRange(enemy, player) &&
        player->isAlive
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
            CheckCollisionRecs(attackHitbox, enemy->hurtbox)
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
    float centerY =
        enemy->hurtbox.y +
        enemy->hurtbox.height * 0.48f;

    Rectangle hitbox =
    {
        0.0f,
        centerY - enemy->attackHitboxHeight / 2.0f,
        enemy->attackHitboxWidth,
        enemy->attackHitboxHeight
    };

    if (enemy->attackDirection > 0)
    {
        hitbox.x =
            enemy->hurtbox.x +
            enemy->hurtbox.width -
            10.0f;
    }
    else
    {
        hitbox.x =
            enemy->hurtbox.x -
            enemy->attackHitboxWidth +
            10.0f;
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

    Texture2D currentTexture =
        enemy->idleTextures[enemy->idleFrame];

    Rectangle source =
    {
        0.0f,
        0.0f,
        (float)currentTexture.width,
        (float)currentTexture.height
    };

    // The hurtbox is the shared position source.
    // Character-specific files only provide spriteSize and offsets.
    float hurtboxCenterX =
        enemy->hurtbox.x +
        enemy->hurtbox.width / 2.0f;

    float hurtboxBottomY =
        enemy->hurtbox.y +
        enemy->hurtbox.height;

    Rectangle destination =
    {
        hurtboxCenterX -
            enemy->spriteSize / 2.0f +
            enemy->spriteOffsetX,

        hurtboxBottomY -
            enemy->spriteSize +
            enemy->spriteOffsetY,

        enemy->spriteSize,
        enemy->spriteSize
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
        enemy->hurtbox,
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
        enemy->hurtbox.x,
        enemy->hurtbox.y - 24.0f,
        enemy->hurtbox.width,
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
        (int)enemy->hurtbox.x,
        (int)enemy->hurtbox.y - 50,
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
}