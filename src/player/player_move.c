#include "player_move.h"
#include "player_attack.h"

void UpdatePlayer(
    Player *player,
    float deltaTime,
    float screenWidth,
    float walkAreaTop,
    float walkAreaBottom)
{
    // ============================================================
    // CHECK MOVEMENT INPUT
    // ============================================================
    bool movingLeft = IsKeyDown(KEY_LEFT);
    bool movingRight = IsKeyDown(KEY_RIGHT);
    bool movingUp = IsKeyDown(KEY_UP);
    bool movingDown = IsKeyDown(KEY_DOWN);

    // ============================================================
    // ATTACK UPDATE
    // ============================================================
    UpdatePlayerAttack(player, deltaTime);

    // ============================================================
    // 0037 - BATTLE IDLE TIMER
    // ============================================================
    if (player->battleIdleActive)
    {
        player->battleIdleTimer -= deltaTime;

        if (player->battleIdleTimer <= 0.0f)
        {
            player->battleIdleTimer = 0.0f;
            player->battleIdleActive = false;

            // Restart the normal breathing loop cleanly.
            player->idleFrame = 0;
            player->idleDirection = 1;
            player->idleTimer = 0.0f;
        }
    }

    // Walking only works when the player is not attacking.
    player->isWalking =
        !player->isAttacking &&
        (movingLeft || movingRight || movingUp || movingDown);

    // ============================================================
    // FACING DIRECTION
    // ============================================================
    // Lock facing direction while attacking.
    if (!player->isAttacking)
    {
        if (movingLeft)
        {
            player->facingRight = false;
        }

        if (movingRight)
        {
            player->facingRight = true;
        }
    }

    // ============================================================
    // MOVEMENT
    // ============================================================
    Vector2 movement = {0.0f, 0.0f};

    // Habang uma-attack, naka-lock muna ang normal movement.
    if (!player->isAttacking)
    {
        if (movingLeft)
        {
            movement.x -= 1.0f;
        }

        if (movingRight)
        {
            movement.x += 1.0f;
        }

        if (movingUp)
        {
            movement.y -= 1.0f;
        }

        if (movingDown)
        {
            movement.y += 1.0f;
        }
    }

    // Para hindi bumilis kapag diagonal.
    if (movement.x != 0.0f && movement.y != 0.0f)
    {
        const float diagonalFactor = 0.70710678f;
        movement.x *= diagonalFactor;
        movement.y *= diagonalFactor;
    }

    player->rectangle.x +=
        movement.x * player->speed * deltaTime;

    player->rectangle.y +=
        movement.y * player->speed * deltaTime;

    // ============================================================
    // WALKING ANIMATION
    // ============================================================
    if (player->isWalking)
    {
        player->walkTimer += deltaTime;

        float currentWalkFrameTime;

        // JWALK1 - JWALK5 = mabilis na bwelo
        if (player->walkFrame <= 4)
        {
            currentWalkFrameTime = 0.08f;
        }
        else
        {
            // JWALK6 - JWALK11 = normal walking speed
            currentWalkFrameTime = player->walkFrameTime;
        }

        if (player->walkTimer >= currentWalkFrameTime)
        {
            player->walkTimer -= currentWalkFrameTime;
            player->walkFrame++;

            if (player->walkFrame >= WALK_FRAME_COUNT)
            {
                // JWALK11 -> balik sa JWALK6
                player->walkFrame = 5;
            }
        }
    }
    else
    {
        // IDLE ANIMATION
        player->walkFrame = 0;
        player->walkTimer = 0.0f;

        player->idleTimer += deltaTime;

        if (player->idleTimer >= player->idleFrameTime)
        {
            player->idleTimer -= player->idleFrameTime;

            int idleFrameCount =
                player->battleIdleActive
                    ? IDLE_BATTLE_FRAME_COUNT
                    : IDLE_BREATH_FRAME_COUNT;

            player->idleFrame += player->idleDirection;

            if (player->idleFrame >= idleFrameCount - 1)
            {
                player->idleFrame = idleFrameCount - 1;
                player->idleDirection = -1;
            }

            if (player->idleFrame <= 0)
            {
                player->idleFrame = 0;
                player->idleDirection = 1;
            }
        }
    }

    // ============================================================
    // PLAYER BOUNDARIES
    // ============================================================
    if (player->rectangle.x < 0.0f)
    {
        player->rectangle.x = 0.0f;
    }

    if (player->rectangle.x + player->rectangle.width > screenWidth)
    {
        player->rectangle.x =
            screenWidth - player->rectangle.width;
    }

    if (player->rectangle.y < walkAreaTop)
    {
        player->rectangle.y = walkAreaTop;
    }

    if (player->rectangle.y > walkAreaBottom)
    {
        player->rectangle.y = walkAreaBottom;
    }
}