#include "player_attack.h"
#include "player_attack_data.h"

void UpdatePlayerAttack(Player *player, float deltaTime)
{
    // ============================================================
    // ATTACK INPUT
    // ============================================================
    // Habang uma-attack, hindi muna puwedeng mag-start
    // ng panibagong attack hanggang matapos ang 3 frames.
    if (!player->isAttacking)
    {
        if (IsKeyPressed(KEY_A))
        {
            player->currentAttack = ATTACK_LEFT_PUNCH;
            player->isAttacking = true;
            player->attackFrame = 0;
            player->attackTimer = 0.0f;
        }
        else if (IsKeyPressed(KEY_W))
        {
            player->currentAttack = ATTACK_RIGHT_PUNCH;
            player->isAttacking = true;
            player->attackFrame = 0;
            player->attackTimer = 0.0f;
        }
        else if (IsKeyPressed(KEY_S))
        {
            player->currentAttack = ATTACK_LEFT_KICK;
            player->isAttacking = true;
            player->attackFrame = 0;
            player->attackTimer = 0.0f;
        }
        else if (IsKeyPressed(KEY_D))
        {
            player->currentAttack = ATTACK_RIGHT_KICK;
            player->isAttacking = true;
            player->attackFrame = 0;
            player->attackTimer = 0.0f;
        }
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
                player->attackFrame = 0;
                player->attackTimer = 0.0f;
                player->isAttacking = false;
                player->currentAttack = ATTACK_NONE;
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