#include "player_attack.h"
#include "player_attack_data.h"

// ============================================================
// 0026 - COMBO INPUT BUFFER
// ============================================================
// This is NOT the final combo-chain system yet.
//
// Purpose:
// - If the player presses A/W/S/D during an attack,
//   remember ONE next attack.
// - When the current 3-frame attack finishes,
//   automatically start the buffered attack.
//
// Example:
// Press A.
// Before A finishes, press W.
// A finishes -> W starts automatically.
//
// This makes fast command input possible later for Tekken-style
// move strings and combo chains.

static AttackType GetPressedAttack(void)
{
    if (IsKeyPressed(KEY_A))
    {
        return ATTACK_LEFT_PUNCH;
    }

    if (IsKeyPressed(KEY_W))
    {
        return ATTACK_RIGHT_PUNCH;
    }

    if (IsKeyPressed(KEY_S))
    {
        return ATTACK_LEFT_KICK;
    }

    if (IsKeyPressed(KEY_D))
    {
        return ATTACK_RIGHT_KICK;
    }

    return ATTACK_NONE;
}

static void StartPlayerAttack(Player *player, AttackType attack)
{
    if (attack == ATTACK_NONE)
    {
        return;
    }

    player->currentAttack = attack;
    player->isAttacking = true;
    player->attackFrame = 0;
    player->attackTimer = 0.0f;
}

void UpdatePlayerAttack(Player *player, float deltaTime)
{
    AttackType pressedAttack = GetPressedAttack();

    // ============================================================
    // ATTACK INPUT / BUFFER
    // ============================================================
    if (!player->isAttacking)
    {
        // No attack is playing, so start the input immediately.
        StartPlayerAttack(player, pressedAttack);
    }
    else if (
        pressedAttack != ATTACK_NONE &&
        player->bufferedAttack == ATTACK_NONE
    )
    {
        // One attack is already playing.
        // Save ONE next attack instead of ignoring the input.
        player->bufferedAttack = pressedAttack;
    }

    // ============================================================
    // ATTACK ANIMATION
    // ============================================================
    if (player->isAttacking)
    {
        const PlayerAttackData *attackData =
            GetPlayerAttackData(player->currentAttack);

        player->attackTimer += deltaTime;

        if (player->attackTimer >= attackData->frameTime)
        {
            player->attackTimer -= attackData->frameTime;
            player->attackFrame++;

            if (player->attackFrame >= ATTACK_FRAME_COUNT)
            {
                // Current attack finished.
                // If 0026 stored another attack, consume it now.
                if (player->bufferedAttack != ATTACK_NONE)
                {
                    AttackType nextAttack = player->bufferedAttack;

                    player->bufferedAttack = ATTACK_NONE;
                    StartPlayerAttack(player, nextAttack);
                }
                else
                {
                    player->attackFrame = 0;
                    player->attackTimer = 0.0f;
                    player->isAttacking = false;
                    player->currentAttack = ATTACK_NONE;
                }
            }
        }
    }
}

// ============================================================
// 0017 + 0023 - ATTACK HITBOX SYSTEM USING MOVE DATA
// ============================================================
bool IsPlayerAttackHitboxActive(const Player *player)
{
    if (
        !player->isAttacking ||
        player->currentAttack == ATTACK_NONE
    )
    {
        return false;
    }

    const PlayerAttackData *attackData =
        GetPlayerAttackData(player->currentAttack);

    return
        player->attackFrame >= attackData->activeStartFrame &&
        player->attackFrame <= attackData->activeEndFrame;
}

Rectangle GetPlayerAttackHitbox(const Player *player)
{
    if (!IsPlayerAttackHitboxActive(player))
    {
        return (Rectangle){0.0f, 0.0f, 0.0f, 0.0f};
    }

    const PlayerAttackData *attackData =
        GetPlayerAttackData(player->currentAttack);

    // Same depth scaling as DrawPlayer().
    float depth =
        (player->rectangle.y - 345.0f) /
        (700.0f - 270.0f);

    if (depth < 0.0f)
    {
        depth = 0.0f;
    }

    if (depth > 1.0f)
    {
        depth = 1.0f;
    }

    float scale =
        2.90f +
        (depth * 1.80f);

    float scaledWidth =
        player->rectangle.width *
        scale *
        1.30f;

    float scaledHeight =
        player->rectangle.height *
        scale;

    float centerX =
        player->rectangle.x +
        (player->rectangle.width / 2.0f);

    float bottomY =
        player->rectangle.y +
        player->rectangle.height;

    float topY =
        bottomY - scaledHeight;

    Rectangle hitbox =
    {
        0.0f,
        0.0f,
        scaledWidth * attackData->hitboxWidthScale,
        scaledHeight * attackData->hitboxHeightScale
    };

    hitbox.y =
        topY +
        (scaledHeight * attackData->hitboxOffsetYScale);

    if (player->facingRight)
    {
        hitbox.x =
            centerX +
            (scaledWidth * attackData->hitboxOffsetXScale);
    }
    else
    {
        hitbox.x =
            centerX -
            (scaledWidth * attackData->hitboxOffsetXScale) -
            hitbox.width;
    }

    return hitbox;
}

int GetPlayerAttackDamage(const Player *player)
{
    if (player->currentAttack == ATTACK_NONE)
    {
        return 0;
    }

    return GetPlayerAttackData(player->currentAttack)->damage;
}

float GetPlayerAttackKnockbackSpeed(const Player *player)
{
    if (player->currentAttack == ATTACK_NONE)
    {
        return 0.0f;
    }

    return GetPlayerAttackData(player->currentAttack)->knockbackSpeed;
}

float GetPlayerAttackHitReactionTime(const Player *player)
{
    if (player->currentAttack == ATTACK_NONE)
    {
        return 0.0f;
    }

    return GetPlayerAttackData(player->currentAttack)->hitReactionTime;
}