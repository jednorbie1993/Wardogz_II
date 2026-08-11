#include "player_draw.h"
#include "player_attack.h"

void DrawPlayer(const Player *player)
{
    // ============================================================
    // DEPTH SCALE
    // ============================================================
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

    // ============================================================
    // CHOOSE TEXTURE
    // ============================================================
    Texture2D currentTexture;

    // Attack has highest drawing priority.
    if (player->isAttacking)
    {
        if (player->currentAttack == ATTACK_LEFT_PUNCH)
        {
            currentTexture =
                player->leftPunchTextures[player->attackFrame];
        }
        else if (player->currentAttack == ATTACK_RIGHT_PUNCH)
        {
            currentTexture =
                player->rightPunchTextures[player->attackFrame];
        }
        else if (player->currentAttack == ATTACK_LEFT_KICK)
        {
            currentTexture =
                player->leftKickTextures[player->attackFrame];
        }
        else
        {
            currentTexture =
                player->rightKickTextures[player->attackFrame];
        }
    }
    else if (player->isWalking)
    {
        currentTexture =
            player->walkTextures[player->walkFrame];
    }
    else
    {
        currentTexture =
            player->idleTextures[player->idleFrame];
    }

    // ============================================================
    // SOURCE RECTANGLE
    // ============================================================
    Rectangle source =
    {
        0.0f,
        0.0f,
        (float)currentTexture.width,
        (float)currentTexture.height
    };

    // ============================================================
    // MIRROR WHEN FACING LEFT
    // ============================================================
    if (!player->facingRight)
    {
        source.x = (float)currentTexture.width;
        source.width = -(float)currentTexture.width;
    }

    // ============================================================
    // DESTINATION
    // ============================================================
    Rectangle destination =
    {
        player->rectangle.x +
            (player->rectangle.width / 2.0f),

        player->rectangle.y +
            player->rectangle.height,

        scaledWidth,
        scaledHeight
    };

    // ============================================================
    // ORIGIN
    // ============================================================
    Vector2 origin =
    {
        scaledWidth / 2.0f,
        scaledHeight
    };

    // ============================================================
    // DRAW PLAYER
    // ============================================================
    DrawTexturePro(
        currentTexture,
        source,
        destination,
        origin,
        0.0f,
        WHITE
    );

    // ============================================================
    // 0017 - ATTACK HITBOX DEBUG DRAW
    // ============================================================
    if (IsPlayerAttackHitboxActive(player))
    {
        Rectangle attackHitbox =
            GetPlayerAttackHitbox(player);

        DrawRectangleRec(
            attackHitbox,
            Fade(RED, 0.35f)
        );

        DrawRectangleLinesEx(
            attackHitbox,
            4.0f,
            RED
        );
    }

    // ============================================================
    // 0030 - PLAYER HURTBOX DEBUG
    // ============================================================
    Rectangle playerHurtbox =
        GetPlayerHurtbox(player);

    DrawRectangleLinesEx(
        playerHurtbox,
        3.0f,
        GREEN
    );

    // ============================================================
    // 0030 FIX 3 - PLAYER FOOT MARKER DEBUG
    // ============================================================
    Rectangle playerFeet =
        GetPlayerFootMarker(player);

    DrawRectangleRec(
        playerFeet,
        Fade(PURPLE, 0.35f)
    );

    DrawRectangleLinesEx(
        playerFeet,
        3.0f,
        PURPLE
    );

    // ============================================================
    // 0030 - PLAYER HP DISPLAY
    // ============================================================
    DrawText(
        TextFormat("Player HP: %d", player->hp),
        30,
        65,
        25,
        GREEN
    );

}