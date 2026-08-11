#include "player.h"

Player InitPlayer(const char *texturePath)
{
    Player player = {0};

    // ============================================================
    // ORIGINAL PLAYER TEXTURE
    // ============================================================
    player.texture = LoadTexture(texturePath);

    // ============================================================
    // IDLE TEXTURES
    // ============================================================
    player.idleTextures[0] =
        LoadTexture("assets/sprites/player/player_idle_1.png");

    player.idleTextures[1] =
        LoadTexture("assets/sprites/player/player_idle_2.png");

    player.idleTextures[2] =
        LoadTexture("assets/sprites/player/player_idle_3.png");

    player.idleFrame = 0;
    player.idleDirection = 1;
    player.idleTimer = 0.0f;
    player.idleFrameTime = 0.18f;

    // ============================================================
    // 12 WALK TEXTURES
    // ============================================================
    player.walkTextures[0] =
        LoadTexture("assets/sprites/player/step1.png");
    player.walkTextures[1] =
        LoadTexture("assets/sprites/player/step2.png");
    player.walkTextures[2] =
        LoadTexture("assets/sprites/player/step3.png");
    player.walkTextures[3] =
        LoadTexture("assets/sprites/player/step4.png");
    player.walkTextures[4] =
        LoadTexture("assets/sprites/player/step5.png");
    player.walkTextures[5] =
        LoadTexture("assets/sprites/player/step6.png");
    player.walkTextures[6] =
        LoadTexture("assets/sprites/player/step7.png");
    player.walkTextures[7] =
        LoadTexture("assets/sprites/player/step8.png");
    player.walkTextures[8] =
        LoadTexture("assets/sprites/player/step9.png");
    player.walkTextures[9] =
        LoadTexture("assets/sprites/player/step10.png");
    player.walkTextures[10] =
        LoadTexture("assets/sprites/player/step11.png");
    player.walkTextures[11] =
        LoadTexture("assets/sprites/player/step12.png");

    player.walkFrame = 0;
    player.walkTimer = 0.0f;
    player.walkFrameTime = 0.189f;

    // ============================================================
    // ATTACK TEXTURES
    // ============================================================

    // LEFT PUNCH - A
    player.leftPunchTextures[0] =
        LoadTexture("assets/sprites/player/left_punch1.png");
    player.leftPunchTextures[1] =
        LoadTexture("assets/sprites/player/left_punch2.png");
    player.leftPunchTextures[2] =
        LoadTexture("assets/sprites/player/left_punch3.png");

    // RIGHT PUNCH - W
    player.rightPunchTextures[0] =
        LoadTexture("assets/sprites/player/right_punch1.png");
    player.rightPunchTextures[1] =
        LoadTexture("assets/sprites/player/right_punch2.png");
    player.rightPunchTextures[2] =
        LoadTexture("assets/sprites/player/right_punch3.png");

    // LEFT KICK - S
    player.leftKickTextures[0] =
        LoadTexture("assets/sprites/player/left_kick1.png");
    player.leftKickTextures[1] =
        LoadTexture("assets/sprites/player/left_kick2.png");
    player.leftKickTextures[2] =
        LoadTexture("assets/sprites/player/left_kick3.png");

    // RIGHT KICK - D
    player.rightKickTextures[0] =
        LoadTexture("assets/sprites/player/right_kick1.png");
    player.rightKickTextures[1] =
        LoadTexture("assets/sprites/player/right_kick2.png");
    player.rightKickTextures[2] =
        LoadTexture("assets/sprites/player/right_kick3.png");

    player.attackFrame = 0;
    player.attackTimer = 0.0f;
    player.attackFrameTime = 0.10f;
    player.isAttacking = false;

    // ============================================================
    // PLAYER STATE
    // ============================================================
    player.isWalking = false;
    player.currentAttack = ATTACK_NONE;

    // 0026 - No buffered attack when the player is created.
    player.bufferedAttack = ATTACK_NONE;

    player.facingRight = true;

    // ============================================================
    // PLAYER POSITION / SIZE
    // ============================================================
    player.rectangle = (Rectangle){
        150.0f,
        500.0f,
        113.0f,
        180.0f
    };

    player.speed = 300.0f;

    return player;
}

void UnloadPlayer(Player *player)
{
    UnloadTexture(player->texture);

    for (int i = 0; i < IDLE_FRAME_COUNT; i++)
    {
        UnloadTexture(player->idleTextures[i]);
    }

    for (int i = 0; i < WALK_FRAME_COUNT; i++)
    {
        UnloadTexture(player->walkTextures[i]);
    }

    for (int i = 0; i < ATTACK_FRAME_COUNT; i++)
    {
        UnloadTexture(player->leftPunchTextures[i]);
        UnloadTexture(player->rightPunchTextures[i]);
        UnloadTexture(player->leftKickTextures[i]);
        UnloadTexture(player->rightKickTextures[i]);
    }

    player->texture = (Texture2D){0};
}