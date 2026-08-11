#include "punk.h"


Enemy InitPunk(float x, float y)
{
    // Start with the shared/common enemy defaults.
    Enemy enemy = InitEnemyBase();

    // ============================================================
    // PUNK - HURTBOX SETTINGS
    // ============================================================

    // Final measured Punk hurtbox from 0020.
    enemy.hurtbox = (Rectangle)
    {
        x,
        y - 200.0f,
        148.0f,
        276.0f
    };

    // ============================================================
    // PUNK - STATS
    // ============================================================

    enemy.maxHp = 100;
    enemy.hp = enemy.maxHp;

    // ============================================================
    // 0031 - PUNK FACING + CHASE AI
    // ============================================================
    enemy.facingRight = false;
    enemy.chaseSpeed = 115.0f;
    enemy.chaseStopDistance = 155.0f;
    enemy.chaseDepthTolerance = 8.00f;

    // ============================================================
    // 0032 - PUNK STAGE POSITION ANCHOR
    // ============================================================
    // InitPunk(x, y) stores the Punk hurtbox at y - 200.
    // This converts hurtbox.y back to the same logical stage Y used
    // by walkAreaTop / walkAreaBottom in main.c.
    enemy.stageAnchorOffsetY = 220.0f;

    // ============================================================
    // PUNK - IDLE TEXTURES
    // ============================================================

    enemy.idleFrameCount = 3;

    enemy.idleTextures[0] =
        LoadTexture(
            "assets/sprites/enemy/stage_1/punk/punk_idle_1.png"
        );

    enemy.idleTextures[1] =
        LoadTexture(
            "assets/sprites/enemy/stage_1/punk/punk_idle_2.png"
        );

    enemy.idleTextures[2] =
        LoadTexture(
            "assets/sprites/enemy/stage_1/punk/punk_idle_3.png"
        );

    enemy.idleFrame = 0;
    enemy.idleDirection = 1;
    enemy.idleTimer = 0.0f;
    enemy.idleFrameTime = 0.19f;

    // ============================================================
    // 0036 - PUNK WALK TEXTURES
    // ============================================================

    enemy.walkFrameCount = 6;

    enemy.walkTextures[0] = LoadTexture(
        "assets/sprites/enemy/stage_1/punk/punk_walk_1.png"
    );

    enemy.walkTextures[1] = LoadTexture(
        "assets/sprites/enemy/stage_1/punk/punk_walk_2.png"
    );

    enemy.walkTextures[2] = LoadTexture(
        "assets/sprites/enemy/stage_1/punk/punk_walk_3.png"
    );

    enemy.walkTextures[3] = LoadTexture(
        "assets/sprites/enemy/stage_1/punk/punk_walk_4.png"
    );

    enemy.walkTextures[4] = LoadTexture(
        "assets/sprites/enemy/stage_1/punk/punk_walk_5.png"
    );

    enemy.walkTextures[5] = LoadTexture(
        "assets/sprites/enemy/stage_1/punk/punk_walk_6.png"
    );

    enemy.walkFrame = 0;
    enemy.walkTimer = 0.0f;
    enemy.walkFrameTime = 0.11f;

    // ============================================================
    // PUNK - SPRITE SIZE / ALIGNMENT
    // ============================================================

    enemy.spriteSize = 580.0f;

    // These offsets reproduce the final alignment from 0020,
    // but now the shared DrawEnemy() uses normal center/bottom anchors.
    //
    // Old X alignment:
    //   hurtbox.x + hurtbox.width / 1.70
    // New generic center:
    //   hurtbox.x + hurtbox.width / 2
    //
    // Difference is approximately +13.06 pixels.
    enemy.spriteOffsetX = 13.06f;

    // Old Y alignment:
    //   hurtbox.y + hurtbox.height / 0.65
    // New generic bottom:
    //   hurtbox.y + hurtbox.height
    //
    // Difference is approximately +148.62 pixels.
    enemy.spriteOffsetY = 148.62f;

    return enemy;
}