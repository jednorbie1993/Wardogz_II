#include "player_attack.h"

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
        player->attackTimer += deltaTime;

        if (player->attackTimer >= player->attackFrameTime)
        {
            player->attackTimer -= player->attackFrameTime;
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
// 0017 - ATTACK HITBOX SYSTEM
// ============================================================
bool IsPlayerAttackHitboxActive(const Player *player)
{
    // TEMPORARY DEBUG MODE:
    // buong 3-frame attack muna ang active para madaling makita.
    return player->isAttacking &&
           player->currentAttack != ATTACK_NONE;
}

Rectangle GetPlayerAttackHitbox(const Player *player)
{
    if (!IsPlayerAttackHitboxActive(player))
    {
        return (Rectangle){0.0f, 0.0f, 0.0f, 0.0f};
    }

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
        {0.0f, 0.0f, 0.0f, 0.0f};

    // ============================================================
    // INDIVIDUAL ATTACK HITBOX SETTINGS
    // ============================================================

    // LEFT PUNCH - A
    if (player->currentAttack == ATTACK_LEFT_PUNCH)
    {
        hitbox.width = scaledWidth * 0.25f;
        hitbox.height = scaledHeight * 0.10f;
        hitbox.y = topY + (scaledHeight * 0.32f);

        if (player->facingRight)
            hitbox.x = centerX + (scaledWidth * 0.04f);
        else
            hitbox.x = centerX - (scaledWidth * 0.04f) - hitbox.width;
    }

    // RIGHT PUNCH - W
    else if (player->currentAttack == ATTACK_RIGHT_PUNCH)
    {
        hitbox.width = scaledWidth * 0.24f;
        hitbox.height = scaledHeight * 0.10f;
        hitbox.y = topY + (scaledHeight * 0.32f);

        if (player->facingRight)
            hitbox.x = centerX + (scaledWidth * 0.01f);
        else
            hitbox.x = centerX - (scaledWidth * 0.01f) - hitbox.width;
    }

    // LEFT KICK - S
    else if (player->currentAttack == ATTACK_LEFT_KICK)
    {
        hitbox.width = scaledWidth * 0.25f;
        hitbox.height = scaledHeight * 0.29f;
        hitbox.y = topY + (scaledHeight * 0.35f);

        if (player->facingRight)
            hitbox.x = centerX + (scaledWidth * 0.02f);
        else
            hitbox.x = centerX - (scaledWidth * 0.02f) - hitbox.width;
    }

    // RIGHT KICK - D
    else if (player->currentAttack == ATTACK_RIGHT_KICK)
    {
        hitbox.width = scaledWidth * 0.26f;
        hitbox.height = scaledHeight * 0.32f;
        hitbox.y = topY + (scaledHeight * 0.33f);

        if (player->facingRight)
            hitbox.x = centerX + (scaledWidth * 0.02f);
        else
            hitbox.x = centerX - (scaledWidth * 0.02f) - hitbox.width;
    }

    return hitbox;
}