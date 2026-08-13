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

    bool runHeld =
        IsKeyDown(KEY_LEFT_SHIFT) ||
        IsKeyDown(KEY_RIGHT_SHIFT);

    // ============================================================
    // 0052 - FORWARD / BACK DASH INPUT
    // ============================================================
    // Double-tap LEFT / RIGHT starts a horizontal dash.
    // Same direction as facing = FORWARD DASH.
    // Opposite direction       = BACK DASH.
    // Facing is captured on the first tap and stays locked
    // for the full dash so a back dash does not turn the player.
    if (player->dashTapTimer > 0.0f)
    {
        player->dashTapTimer -= deltaTime;

        if (player->dashTapTimer <= 0.0f)
        {
            player->dashTapTimer = 0.0f;
            player->lastDashTapDirection = 0;
        }
    }

    if (!player->isAttacking && !player->isDashing)
    {
        int pressedDashDirection = 0;

        if (IsKeyPressed(KEY_LEFT))
        {
            pressedDashDirection = -1;
        }
        else if (IsKeyPressed(KEY_RIGHT))
        {
            pressedDashDirection = 1;
        }

        if (pressedDashDirection != 0)
        {
            bool sameSecondTap =
                player->lastDashTapDirection == pressedDashDirection &&
                player->dashTapTimer > 0.0f;

            if (sameSecondTap)
            {
                player->isDashing = true;
                player->dashDirection = pressedDashDirection;
                player->dashTimer = player->dashDuration;

                // Lock the ORIGINAL facing for the entire dash.
                player->dashLockedFacingRight = player->dashTapFacingRight;
                player->facingRight = player->dashLockedFacingRight;

                // Cancel the normal delayed turn so it cannot flip the
                // sprite in the middle of the dash.
                player->pendingFacingDirection = 0;
                player->turnDirectionTravel = 0.0f;

                // Consume the double-tap.
                player->lastDashTapDirection = 0;
                player->dashTapTimer = 0.0f;
            }
            else
            {
                // First tap: remember both the direction and current facing.
                player->lastDashTapDirection = pressedDashDirection;
                player->dashTapFacingRight = player->facingRight;
                player->dashTapTimer = player->dashTapWindow;
            }
        }
    }

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
        (movingLeft || movingRight || movingUp || movingDown || player->isDashing);

    // 0051 - Hold Shift while moving to run.
    player->isRunning =
        player->isWalking &&
        runHeld;

    // ============================================================
    // 0051 - DISTANCE-BASED HORIZONTAL TURN
    // ============================================================
    // Facing is updated after movement is calculated below so the turn
    // uses actual horizontal travel distance instead of elapsed time.

    // ============================================================
    // MOVEMENT
    // ============================================================
    Vector2 movement = {0.0f, 0.0f};

    // Habang uma-attack o nagba-back-dash, naka-lock muna ang normal movement.
    if (!player->isAttacking && !player->isDashing)
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

    float movementSpeed = player->speed;

    if (player->isRunning)
    {
        movementSpeed *= player->runSpeedMultiplier;
    }

    player->rectangle.x +=
        movement.x * movementSpeed * deltaTime;

    player->rectangle.y +=
        movement.y * movementSpeed * deltaTime;

    // ============================================================
    // 0051 - DISTANCE-BASED FACING TURN
    // ============================================================
    if (!player->isAttacking && !player->isDashing)
    {
        int oppositeDirection = 0;

        if (movement.x < 0.0f && player->facingRight)
        {
            oppositeDirection = -1;
        }
        else if (movement.x > 0.0f && !player->facingRight)
        {
            oppositeDirection = 1;
        }

        if (oppositeDirection != 0)
        {
            if (player->pendingFacingDirection != oppositeDirection)
            {
                player->pendingFacingDirection = oppositeDirection;
                player->turnDirectionTravel = 0.0f;
            }

            float horizontalTravel = movement.x * movementSpeed * deltaTime;
            if (horizontalTravel < 0.0f)
            {
                horizontalTravel = -horizontalTravel;
            }

            player->turnDirectionTravel += horizontalTravel;

            if (player->turnDirectionTravel >= player->turnDirectionDistance)
            {
                player->facingRight =
                    (player->pendingFacingDirection == 1);

                player->pendingFacingDirection = 0;
                player->turnDirectionTravel = 0.0f;
            }
        }
        else
        {
            // The player stopped moving backward or returned to the
            // current facing direction before reaching the threshold.
            player->pendingFacingDirection = 0;
            player->turnDirectionTravel = 0.0f;
        }
    }

    // ============================================================
    // 0052 - FORWARD / BACK DASH SLIDE
    // ============================================================
    if (player->isDashing && !player->isAttacking)
    {
        // Never change facing during the dash.
        player->facingRight = player->dashLockedFacingRight;
        player->pendingFacingDirection = 0;
        player->turnDirectionTravel = 0.0f;

        player->rectangle.x +=
            player->dashDirection * player->dashSpeed * deltaTime;

        player->dashTimer -= deltaTime;

        if (player->dashTimer <= 0.0f)
        {
            player->dashTimer = 0.0f;
            player->isDashing = false;
            player->dashDirection = 0;

            // Stay facing the same way after the dash.
            player->facingRight = player->dashLockedFacingRight;
        }
    }

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

        // Running temporarily reuses the walk sprites, but cycles them faster.
        if (player->isRunning)
        {
            currentWalkFrameTime *= 0.65f;
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
        player->isRunning = false;
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