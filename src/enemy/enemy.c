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


void UpdateEnemyHit(
    Enemy *enemy,
    const Player *player,
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

        if (CheckCollisionRecs(attackHitbox, enemy->hurtbox))
        {
            // ====================================================
            // DAMAGE
            // ====================================================

            enemy->hp -= 10;
            enemy->hitByCurrentAttack = true;

            // ====================================================
            // START HIT REACTION / KNOCKBACK
            // ====================================================

            enemy->isHit = true;
            enemy->hitReactionTimer = 0.12f;
            enemy->knockbackSpeed = 360.0f;

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