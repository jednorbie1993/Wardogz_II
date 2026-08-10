#include "player.h"

Player InitPlayer(const char *texturePath)
{
    Player player = {0};

    // Original player texture
    player.texture = LoadTexture(texturePath);

    // =========================
    // IDLE TEXTURES
    // =========================

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

    // =========================
    // 12 WALK TEXTURES
    // =========================

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

    // 12 frames kaya mas mabilis nang kaunti
    player.walkFrameTime = 0.189f;

    // =========================
    // ATTACK TEXTURES
    // =========================

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

    // 3-frame attack: mabilis at responsive
    player.attackFrameTime = 0.10f;
    player.isAttacking = false;

    // =========================
    // PLAYER STATE
    // =========================

    player.isWalking = false;
    player.currentAttack = ATTACK_NONE;

    // Ang images mo ay nakaharap RIGHT.
    // RIGHT = normal image
    // LEFT  = mirrored image
    player.facingRight = true;

    // =========================
    // PLAYER POSITION / SIZE
    // =========================

    player.rectangle = (Rectangle){
        150.0f, // X position
        500.0f, // Y position
        113.0f, // Width
        180.0f  // Height
    };

    player.speed = 300.0f;

    return player;
}

void UpdatePlayer(
    Player *player,
    float deltaTime,
    float screenWidth,
    float walkAreaTop,
    float walkAreaBottom)
{
    // =========================
    // CHECK MOVEMENT
    // =========================

    bool movingLeft =
        IsKeyDown(KEY_LEFT);

    bool movingRight =
        IsKeyDown(KEY_RIGHT);

    bool movingUp =
        IsKeyDown(KEY_UP);

    bool movingDown =
        IsKeyDown(KEY_DOWN);

    player->isWalking =
        !player->isAttacking &&
        (movingLeft ||
         movingRight ||
         movingUp ||
         movingDown);

    // =========================
    // ATTACK INPUT
    // =========================

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

    // =========================
    // ATTACK ANIMATION
    // =========================

    if (player->isAttacking)
    {
        player->attackTimer += deltaTime;

        if (player->attackTimer >= player->attackFrameTime)
        {
            player->attackTimer -= player->attackFrameTime;
            player->attackFrame++;

            // Pagkatapos ng frame 3, balik sa normal state.
            if (player->attackFrame >= ATTACK_FRAME_COUNT)
            {
                player->attackFrame = 0;
                player->attackTimer = 0.0f;
                player->isAttacking = false;
                player->currentAttack = ATTACK_NONE;
            }
        }
    }

    // Walking is only active when the player is not attacking.
    // Kapag natapos ang attack at naka-hold pa rin ang Arrow Key,
    // automatic babalik ang walking sa parehong frame update.
    player->isWalking =
        !player->isAttacking &&
        (movingLeft ||
         movingRight ||
         movingUp ||
         movingDown);

    // =========================
    // FACING DIRECTION
    // =========================

    // Lock facing direction while attacking.
    if (!player->isAttacking)
    {
        if (movingLeft)
        {
            player->facingRight = false;
        }

        if (movingRight)
        {
            player->facingRight = true;
        }
    }

    // =========================
    // MOVEMENT
    // =========================

    Vector2 movement = {0.0f, 0.0f};

    // Basic Attack System:
    // habang uma-attack, naka-lock muna ang normal Arrow Key movement.
    if (!player->isAttacking)
    {
        if (movingLeft)
        {
            movement.x -= 1.0f;
        }

        if (movingRight)
        {
            movement.x += 1.0f;
        }

        if (movingUp)
        {
            movement.y -= 1.0f;
        }

        if (movingDown)
        {
            movement.y += 1.0f;
        }
    }

    // Para hindi bumilis kapag diagonal
    if (movement.x != 0.0f && movement.y != 0.0f)
    {
        const float diagonalFactor = 0.70710678f;

        movement.x *= diagonalFactor;
        movement.y *= diagonalFactor;
    }

    player->rectangle.x +=
        movement.x * player->speed * deltaTime;

    player->rectangle.y +=
        movement.y * player->speed * deltaTime;

    // =========================
    // WALKING ANIMATION
    // =========================

    if (player->isWalking)
    {
        player->walkTimer += deltaTime;

        if (player->walkTimer >= player->walkFrameTime)
        {
            player->walkTimer -= player->walkFrameTime;

            player->walkFrame++;

            if (player->walkFrame >= WALK_FRAME_COUNT)
            {
                player->walkFrame = 0;
            }
        }
    }

    // =========================
    // IDLE ANIMATION
    // =========================

    else
    {
        // Reset walking animation
        player->walkFrame = 0;
        player->walkTimer = 0.0f;

        player->idleTimer += deltaTime;

        if (player->idleTimer >= player->idleFrameTime)
        {
            player->idleTimer -= player->idleFrameTime;

            player->idleFrame +=
                player->idleDirection;

            if (player->idleFrame >= IDLE_FRAME_COUNT - 1)
            {
                player->idleFrame = IDLE_FRAME_COUNT - 1;
                player->idleDirection = -1;
            }

            if (player->idleFrame <= 0)
            {
                player->idleFrame = 0;
                player->idleDirection = 1;
            }
        }
    }

    // =========================
    // LEFT BOUNDARY
    // =========================

    if (player->rectangle.x < 0.0f)
    {
        player->rectangle.x = 0.0f;
    }

    // =========================
    // RIGHT BOUNDARY
    // =========================

    if (
        player->rectangle.x +
            player->rectangle.width >
        screenWidth)
    {
        player->rectangle.x =
            screenWidth -
            player->rectangle.width;
    }

    // =========================
    // TOP WALKABLE BOUNDARY
    // =========================

    if (player->rectangle.y < walkAreaTop)
    {
        player->rectangle.y =
            walkAreaTop;
    }

    // =========================
    // BOTTOM WALKABLE BOUNDARY
    // =========================

    if (player->rectangle.y > walkAreaBottom)
    {
        player->rectangle.y =
            walkAreaBottom;
    }
}

// ============================================================
// 0017 - ATTACK HITBOX SYSTEM
// ============================================================

bool IsPlayerAttackHitboxActive(const Player *player)
{
    // TEMPORARY DEBUG MODE:
    // buong 3-frame attack muna ang active para madaling makita
    // ang red hitbox habang tine-test natin ang position.
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
        bottomY -
        scaledHeight;

    Rectangle hitbox =
        {0.0f, 0.0f, 0.0f, 0.0f};

    // ============================================================
    // INDIVIDUAL ATTACK HITBOX SETTINGS
    // ============================================================

    // LEFT PUNCH - A
    if (player->currentAttack == ATTACK_LEFT_PUNCH)
    {
        hitbox.width = scaledWidth * 0.25f; //haba ng box
        hitbox.height = scaledHeight * 0.10f; //taba ng redbox
        hitbox.y = topY + (scaledHeight * 0.32f); //mataas na number baba, mababa na number tatass

        if (player->facingRight)
            hitbox.x = centerX + (scaledWidth * 0.04f);
        else
            hitbox.x = centerX - (scaledWidth * 0.04f) - hitbox.width;
    }

    // RIGHT PUNCH - W
    else if (player->currentAttack == ATTACK_RIGHT_PUNCH)
    {
        // Mas maikli ang reach ng sprite nito.
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
        hitbox.width = scaledWidth * 0.25f; //haba ng box
        hitbox.height = scaledHeight * 0.29f; //taba ng redbox
        hitbox.y = topY + (scaledHeight * 0.35f); //mataas na number baba, mababa na number tatass

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

void DrawPlayer(const Player *player)
{
    // =========================
    // DEPTH SCALE
    // =========================

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

    // =========================
    // CHOOSE TEXTURE
    // =========================

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

    // =========================
    // SOURCE RECTANGLE
    // =========================

    Rectangle source =
        {
            0.0f,
            0.0f,
            (float)currentTexture.width,
            (float)currentTexture.height};

    // =========================
    // MIRROR WHEN FACING LEFT
    // =========================

    if (!player->facingRight)
    {
        source.x =
            (float)currentTexture.width;

        source.width =
            -(float)currentTexture.width;
    }

    // =========================
    // DESTINATION
    // =========================

    Rectangle destination =
        {
            player->rectangle.x +
                (player->rectangle.width / 2.0f),

            player->rectangle.y +
                player->rectangle.height,

            scaledWidth,
            scaledHeight};

    // =========================
    // ORIGIN
    // =========================

    Vector2 origin =
        {
            scaledWidth / 2.0f,
            scaledHeight};

    // =========================
    // DRAW PLAYER
    // =========================

    DrawTexturePro(
        currentTexture,
        source,
        destination,
        origin,
        0.0f,
        WHITE);

    // ========================================================
    // 0017 - ATTACK HITBOX DEBUG DRAW
    // ========================================================
    // RED BOX = attack hitbox.
    // Temporary: visible for the whole attack animation.
    if (IsPlayerAttackHitboxActive(player))
    {
        Rectangle attackHitbox =
            GetPlayerAttackHitbox(player);

        DrawRectangleRec(
            attackHitbox,
            Fade(RED, 0.35f));

        DrawRectangleLinesEx(
            attackHitbox,
            4.0f,
            RED);
    }
}

void UnloadPlayer(Player *player)
{
    // Original texture
    UnloadTexture(player->texture);

    // =========================
    // UNLOAD IDLE
    // =========================

    for (int i = 0; i < IDLE_FRAME_COUNT; i++)
    {
        UnloadTexture(
            player->idleTextures[i]);
    }

    // =========================
    // UNLOAD WALK
    // =========================

    for (int i = 0; i < WALK_FRAME_COUNT; i++)
    {
        UnloadTexture(
            player->walkTextures[i]);
    }

    // =========================
    // UNLOAD ATTACKS
    // =========================

    for (int i = 0; i < ATTACK_FRAME_COUNT; i++)
    {
        UnloadTexture(player->leftPunchTextures[i]);
        UnloadTexture(player->rightPunchTextures[i]);
        UnloadTexture(player->leftKickTextures[i]);
        UnloadTexture(player->rightKickTextures[i]);
    }

    player->texture =
        (Texture2D){0};
}