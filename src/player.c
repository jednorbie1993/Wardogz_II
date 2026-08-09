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
    // PLAYER STATE
    // =========================

    player.isWalking = false;

    // Ang step images mo ay nakaharap RIGHT.
    // RIGHT = normal image
    // LEFT  = mirrored image
    player.facingRight = true;


    // =========================
    // PLAYER POSITION / SIZE
    // =========================

    player.rectangle = (Rectangle)
    {
        150.0f,   // X position
        500.0f,   // Y position
        113.0f,   // Width
        180.0f    // Height
    };


    player.speed = 300.0f;


    return player;
}



void UpdatePlayer(
    Player *player,
    float deltaTime,
    float screenWidth,
    float walkAreaTop,
    float walkAreaBottom
)
{
    // =========================
    // CHECK MOVEMENT
    // =========================

    bool movingLeft =
        IsKeyDown(KEY_LEFT) ||
        IsKeyDown(KEY_A);

    bool movingRight =
        IsKeyDown(KEY_RIGHT) ||
        IsKeyDown(KEY_D);

    bool movingUp =
        IsKeyDown(KEY_UP) ||
        IsKeyDown(KEY_W);

    bool movingDown =
        IsKeyDown(KEY_DOWN) ||
        IsKeyDown(KEY_S);


    player->isWalking =
        movingLeft ||
        movingRight ||
        movingUp ||
        movingDown;


    // =========================
    // FACING DIRECTION
    // =========================

    if (movingLeft)
    {
        player->facingRight = false;
    }

    if (movingRight)
    {
        player->facingRight = true;
    }


    // =========================
    // MOVEMENT
    // =========================

    if (movingLeft)
    {
        player->rectangle.x -=
            player->speed * deltaTime;
    }


    if (movingRight)
    {
        player->rectangle.x +=
            player->speed * deltaTime;
    }


    if (movingUp)
    {
        player->rectangle.y -=
            player->speed * deltaTime;
    }


    if (movingDown)
    {
        player->rectangle.y +=
            player->speed * deltaTime;
    }


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
        screenWidth
    )
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


    if (player->isWalking)
    {
        currentTexture =
            player->walkTextures[
                player->walkFrame
            ];
    }
    else
    {
        currentTexture =
            player->idleTextures[
                player->idleFrame
            ];
    }


    // =========================
    // SOURCE RECTANGLE
    // =========================

    Rectangle source =
    {
        0.0f,
        0.0f,
        (float)currentTexture.width,
        (float)currentTexture.height
    };


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
        scaledHeight
    };


    // =========================
    // ORIGIN
    // =========================

    Vector2 origin =
    {
        scaledWidth / 2.0f,
        scaledHeight
    };


    // =========================
    // DRAW PLAYER
    // =========================

    DrawTexturePro(
        currentTexture,
        source,
        destination,
        origin,
        0.0f,
        WHITE
    );
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
            player->idleTextures[i]
        );
    }


    // =========================
    // UNLOAD WALK
    // =========================

    for (int i = 0; i < WALK_FRAME_COUNT; i++)
    {
        UnloadTexture(
            player->walkTextures[i]
        );
    }


    player->texture =
        (Texture2D){0};
}