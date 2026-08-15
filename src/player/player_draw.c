#include "player_draw.h"
#include "player_attack.h"

#include <math.h>

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
    Texture2D currentTexture = player->texture;

    // Attack has highest drawing priority.
    if (player->isAttacking)
    {
        currentTexture =
            player->attackTextures
                [player->currentAttack]
                [player->attackFrame];
    }
    else if (player->isCrouching)
    {
        currentTexture = player->crouchTexture;
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

    // Hold-A Punch Charge feedback. Frame 2 stays frozen while the
    // character shake becomes stronger at the 1.5 and 3.0 sec levels.
    if (
        player->isAttacking &&
        player->currentAttack == ATTACK_PUNCH_CHARGE &&
        player->punchChargeHolding &&
        player->attackFrame == 1 &&
        player->punchChargeLevel > 0
    )
    {
        float shakeAmount =
            player->punchChargeLevel >= 2
            ? 4.0f
            : 2.0f;

        destination.x +=
            sinf(player->punchChargeTimer * 52.0f) * shakeAmount;

        destination.y +=
            sinf(player->punchChargeTimer * 37.0f) * (shakeAmount * 0.45f);
    }

    // Adjust idle breath position only
    if (!player->isAttacking &&
        !player->isWalking &&
        !player->isCrouching &&
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
    if (player->showHitboxes)
    {
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
    }

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