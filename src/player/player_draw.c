#include "player_draw.h"
#include "player_attack.h"

// Set to 1 when you want to see combat debug boxes again.
#define SHOW_HITBOXES 0

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
        if (player->battleIdleActive)
        {
            currentTexture =
                player->idleBattleTextures[player->idleFrame];
        }
        else
        {
            currentTexture =
                player->idleBreathTextures[player->idleFrame];
        }
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

    // Adjust idle breath position only
    if (!player->isAttacking &&
        !player->isWalking &&
        !player->battleIdleActive)
    {
        destination.y += 20.0f;
    }
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
    Color playerTint = player->isHit ? RED : WHITE;

    DrawTexturePro(
        currentTexture,
        source,
        destination,
        origin,
        0.0f,
        playerTint
    );

    // ============================================================
    // COMBAT DEBUG BOXES - HIDDEN BY DEFAULT
    // ============================================================
#if SHOW_HITBOXES
    // 0017 - ATTACK HITBOX DEBUG DRAW
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

    // 0030 - PLAYER HURTBOX DEBUG
    Rectangle playerHurtbox =
        GetPlayerHurtbox(player);

    DrawRectangleLinesEx(
        playerHurtbox,
        3.0f,
        GREEN
    );

    // 0030 FIX 3 - PLAYER FOOT MARKER DEBUG
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
#endif

}

// ============================================================
// 0073 - SCREEN-SPACE JAMBER HP HUD
// ============================================================
// Call this after EndMode2D() so it remains fixed on the screen while
// the camera follows Jamber across the long stage.
void DrawPlayerHud(const Player *player)
{
    const float startX = 24.0f;
    const float y = 24.0f;
    const float nameWidth = 110.0f;
    const float barWidth = 170.0f;
    const float barHeight = 14.0f;

    float hpPercent = 0.0f;

    if (player->maxHp > 0.0f)
    {
        hpPercent = player->hp / player->maxHp;
    }

    if (hpPercent < 0.0f) hpPercent = 0.0f;
    if (hpPercent > 1.0f) hpPercent = 1.0f;

    DrawText(
        "JAMBER",
        (int)startX,
        (int)(y - 4.0f),
        20,
        WHITE
    );

    Rectangle hpBack =
    {
        startX + nameWidth,
        y,
        barWidth,
        barHeight
    };

    Rectangle hpFill =
    {
        hpBack.x,
        hpBack.y,
        hpBack.width * hpPercent,
        hpBack.height
    };

    DrawRectangleRec(hpBack, BLACK);
    DrawRectangleRec(hpFill, GREEN);
    DrawRectangleLinesEx(hpBack, 2.0f, WHITE);
}