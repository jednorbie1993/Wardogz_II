#include "enemy_internal.h"

// Set to 1 when you want to see combat debug boxes again.
#define SHOW_HITBOXES 0


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

    if (
        enemy->battleIdleActive &&
        enemy->battleIdleFrameCount > 0
    )
    {
        int battleFrame = enemy->battleIdleFrame;
        if (battleFrame < 0) battleFrame = 0;
        if (battleFrame >= enemy->battleIdleFrameCount)
            battleFrame = enemy->battleIdleFrameCount - 1;

        Texture2D texture = enemy->battleIdleTextures[battleFrame];
        if (texture.id != 0) return texture;
    }

    int idleFrame = enemy->idleFrame;
    if (idleFrame < 0) idleFrame = 0;
    if (idleFrame >= enemy->idleFrameCount)
        idleFrame = enemy->idleFrameCount - 1;

    return enemy->idleTextures[idleFrame];
}


static Texture2D GetEnemyDeathTexture(
    const Enemy *enemy
)
{
    if (enemy->deathFrameCount <= 0)
    {
        return GetEnemyCurrentTexture(enemy);
    }

    int frame = enemy->deathFrame;
    if (frame < 0) frame = 0;
    if (frame >= enemy->deathFrameCount)
        frame = enemy->deathFrameCount - 1;

    Texture2D texture = enemy->deathTextures[frame];

    // Safe fallback while the new PNG files are still being prepared.
    if (texture.id == 0)
    {
        return GetEnemyCurrentTexture(enemy);
    }

    return texture;
}

void DrawEnemy(const Enemy *enemy)
{
    // 0050 - Remove the body only after the full death hold finishes.
    if (enemy->deathFinished)
    {
        return;
    }

    if (enemy->idleFrameCount <= 0)
    {
        return;
    }

    bool drawDeathSprite =
        enemy->isDying &&
        enemy->deathFreezeTimer <= 0.0f;

    Texture2D currentTexture;

    if (
        enemy->isDying &&
        enemy->deathFreezeTimer > 0.0f &&
        enemy->deathFreezeTexture.id != 0
    )
    {
        currentTexture = enemy->deathFreezeTexture;
    }
    else
    {
        currentTexture =
            drawDeathSprite
            ? GetEnemyDeathTexture(enemy)
            : GetEnemyCurrentTexture(enemy);
    }

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

    if (enemy->isDying)
    {
        if (enemy->deathTimer <= 1.3f)
        {
            // Gray during final 1 second
            spriteTint = GRAY;

            // Final 0.5 sec = fade out
            if (enemy->deathTimer <= 0.5f)
            {
                float alpha =
                    enemy->deathTimer / 0.7f;

                spriteTint =
                    Fade(GRAY, alpha);
            }
        }
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
    // COMBAT DEBUG BOXES - HIDDEN BY DEFAULT
    // ============================================================
#if SHOW_HITBOXES
    // DEBUG HURTBOX
    DrawRectangleLinesEx(
        scaledHurtbox,
        4.0f,
        BLUE
    );

    // DEBUG FOOT / GROUND MARKER
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

    // DEBUG ATTACK HITBOX
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
#endif

}