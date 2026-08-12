#include "enemy_internal.h"


static Texture2D GetEnemyCurrentTexture(
    const Enemy *enemy
)
{
    bool drawWalk =
        (enemy->isEntering || enemy->isChasing) &&
        !enemy->isAttacking &&
        !enemy->isHit &&
        enemy->walkFrameCount > 0;

    if (enemy->isAttacking)
    {
        int frame = enemy->attackFrame;

        if (
            enemy->currentAttackMove == ENEMY_ATTACK_ELBOW &&
            enemy->elbowFrameCount > 0
        )
        {
            if (frame < 0) frame = 0;
            if (frame >= enemy->elbowFrameCount)
                frame = enemy->elbowFrameCount - 1;

            Texture2D texture = enemy->elbowTextures[frame];
            if (texture.id != 0) return texture;
        }
        else if (enemy->punchFrameCount > 0)
        {
            if (frame < 0) frame = 0;
            if (frame >= enemy->punchFrameCount)
                frame = enemy->punchFrameCount - 1;

            Texture2D texture = enemy->punchTextures[frame];
            if (texture.id != 0) return texture;
        }
    }

    if (drawWalk)
    {
        int frame = enemy->walkFrame;
        if (frame < 0) frame = 0;
        if (frame >= enemy->walkFrameCount)
            frame = enemy->walkFrameCount - 1;

        Texture2D texture = enemy->walkTextures[frame];
        if (texture.id != 0) return texture;
    }

    int idleFrame = enemy->idleFrame;
    if (idleFrame < 0) idleFrame = 0;
    if (idleFrame >= enemy->idleFrameCount)
        idleFrame = enemy->idleFrameCount - 1;

    return enemy->idleTextures[idleFrame];
}

void DrawEnemy(const Enemy *enemy)
{
    if (enemy->idleFrameCount <= 0)
    {
        return;
    }

    Texture2D currentTexture =
        GetEnemyCurrentTexture(enemy);

    Rectangle source =
    {
        0.0f,
        0.0f,
        (float)currentTexture.width,
        (float)currentTexture.height
    };

    // Original Punk art faces RIGHT.
    if (!enemy->facingRight)
    {
        source.x =
            (float)currentTexture.width;

        source.width =
            -(float)currentTexture.width;
    }

    float perspectiveScale =
        EnemyGetPerspectiveScale(enemy);

    Rectangle scaledHurtbox =
        EnemyGetScaledHurtbox(enemy);

    float scaledSpriteSize =
        enemy->spriteSize *
        perspectiveScale;

    float hurtboxCenterX =
        scaledHurtbox.x +
        scaledHurtbox.width / 2.0f;

    float hurtboxBottomY =
        scaledHurtbox.y +
        scaledHurtbox.height;

    Rectangle destination =
    {
        hurtboxCenterX -
            scaledSpriteSize / 2.0f +
            (enemy->spriteOffsetX *
             perspectiveScale),

        hurtboxBottomY -
            scaledSpriteSize +
            (enemy->spriteOffsetY *
             perspectiveScale),

        scaledSpriteSize,
        scaledSpriteSize
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
        scaledHurtbox,
        4.0f,
        BLUE
    );

    // ============================================================
    // DEBUG FOOT / GROUND MARKER
    // ============================================================

    Rectangle enemyFeet =
        GetEnemyFootMarker(enemy);

    DrawRectangleRec(
        enemyFeet,
        Fade(ORANGE, 0.35f)
    );

    DrawRectangleLinesEx(
        enemyFeet,
        3.0f,
        ORANGE
    );

    // ============================================================
    // DEBUG ATTACK HITBOX
    // ============================================================

    if (
        enemy->isAttacking &&
        enemy->attackFrame == 2
    )
    {
        Rectangle attackHitbox =
            GetEnemyAttackHitbox(enemy);

        DrawRectangleRec(
            attackHitbox,
            Fade(YELLOW, 0.30f)
        );

        DrawRectangleLinesEx(
            attackHitbox,
            4.0f,
            YELLOW
        );
    }

    // ============================================================
    // HP BAR
    // ============================================================

    float hpPercent =
        (float)enemy->hp /
        (float)enemy->maxHp;

    Rectangle hpBack =
    {
        scaledHurtbox.x,
        scaledHurtbox.y - 24.0f,
        scaledHurtbox.width,
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
    DrawRectangleLinesEx(
        hpBack,
        2.0f,
        WHITE
    );

    DrawText(
        TextFormat("HP: %d", enemy->hp),
        (int)scaledHurtbox.x,
        (int)scaledHurtbox.y - 50,
        20,
        WHITE
    );
}