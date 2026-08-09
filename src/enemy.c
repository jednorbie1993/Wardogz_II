#include "enemy.h"

Enemy InitEnemy(float x, float y)
{
    Enemy enemy = {0};

    enemy.hurtbox = (Rectangle) // eto ung lapad at laki ng blue rectangle box
    {
        x,
        y - 20, // - pataas + pababa 
        140.0f,
        250.0f
    };

    enemy.maxHp = 100;
    enemy.hp = enemy.maxHp;
    enemy.isAlive = true;
    enemy.hitByCurrentAttack = false;

    // ============================================================
    // 0019 - HIT REACTION / KNOCKBACK DEFAULTS
    // ============================================================

    enemy.isHit = false;
    enemy.hitReactionTimer = 0.0f;
    enemy.knockbackSpeed = 0.0f;
    enemy.knockbackDirection = 0;

    // ============================================================
    // 0020 - PUNK IDLE TEXTURES
    // ============================================================

    enemy.idleTextures[0] =
        LoadTexture("assets/sprites/enemy/stage_1/punk/punk_idle_1.png");

    enemy.idleTextures[1] =
        LoadTexture("assets/sprites/enemy/stage_1/punk/punk_idle_2.png");

    enemy.idleTextures[2] =
        LoadTexture("assets/sprites/enemy/stage_1/punk/punk_idle_3.png");

    enemy.idleFrame = 0;
    enemy.idleDirection = 1;
    enemy.idleTimer = 0.0f;
    enemy.idleFrameTime = 0.19f;

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
    // 0020 - IDLE ANIMATION
    // ============================================================

    // Kapag patay na ang enemy, hihinto ang idle animation at movement.
    if (!enemy->isAlive)
    {
        return;
    }

    enemy->idleTimer += deltaTime;

    if (enemy->idleTimer >= enemy->idleFrameTime)
    {
        enemy->idleTimer -= enemy->idleFrameTime;
        enemy->idleFrame += enemy->idleDirection;

        // Animation order:
        // 0 -> 1 -> 2 -> 1 -> 0 -> repeat
        if (enemy->idleFrame >= ENEMY_IDLE_FRAME_COUNT - 1)
        {
            enemy->idleFrame = ENEMY_IDLE_FRAME_COUNT - 1;
            enemy->idleDirection = -1;
        }
        else if (enemy->idleFrame <= 0)
        {
            enemy->idleFrame = 0;
            enemy->idleDirection = 1;
        }
    }

    // ============================================================
    // 0019 - ACTIVE HIT REACTION / KNOCKBACK
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
            // 0019 - START HIT REACTION / KNOCKBACK
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
    // 0020 - DRAW PUNK SPRITE
    // ============================================================

    Texture2D currentTexture =
        enemy->idleTextures[enemy->idleFrame];

    Rectangle source =
    {
        0.0f,
        0.0f,
        (float)currentTexture.width,
        (float)currentTexture.height
    };

    // The Punk PNG uses a large transparent 1024x1024 canvas.
    // Draw the full canvas larger, then anchor it to the bottom-center
    // of the existing enemy hurtbox.
    float spriteSize = 580.0f;

    float hurtboxCenterX =
        enemy->hurtbox.x +
        enemy->hurtbox.width / 1.50f;

    float hurtboxBottomY =
        enemy->hurtbox.y +
        enemy->hurtbox.height / 0.60f; // mababa pababa mataas paas ung enemy image position

    Rectangle destination =
    {
        hurtboxCenterX - spriteSize / 2.0f,
        hurtboxBottomY - spriteSize,
        spriteSize,
        spriteSize
    };

    Color spriteTint = WHITE;

    if (!enemy->isAlive)
    {
        spriteTint = GRAY;
    }
    else if (enemy->isHit)
    {
        // Keep the visible hit reaction from 0019.
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

    // Keep BLUE BOX for now so we can align the Punk sprite correctly.
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
    for (int i = 0; i < ENEMY_IDLE_FRAME_COUNT; i++)
    {
        UnloadTexture(enemy->idleTextures[i]);
    }
}