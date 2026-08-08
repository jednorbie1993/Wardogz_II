#include "player.h"

Player InitPlayer(const char *texturePath)
{
    Player player = {0};

    // Load player image
    player.texture = LoadTexture(texturePath);

    player.idleTextures[0] = LoadTexture("assets/sprites/player/player_idle_1.png");
    player.idleTextures[1] = LoadTexture("assets/sprites/player/player_idle_2.png");
    player.idleTextures[2] = LoadTexture("assets/sprites/player/player_idle_3.png");

    player.idleFrame = 0;
    player.idleDirection = 1;
    player.idleTimer = 0.0f;
    player.idleFrameTime = 0.18f;

    // Player position and size
    player.rectangle = (Rectangle)
    {
        150.0f,   // X position
        500.0f,   // Y position
        113.0f,    // Width
        180.0f     // Height
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
    // Idle breathing animation
    player->idleTimer += GetFrameTime();

    if (player->idleTimer >= player->idleFrameTime)
    {
        player->idleTimer = 0.0f;

        player->idleFrame += player->idleDirection;

        if (player->idleFrame >= 2)
        {
            player->idleFrame = 2;
            player->idleDirection = -1;
        }

        if (player->idleFrame <= 0)
        {
            player->idleFrame = 0;
            player->idleDirection = 1;
        }
    }

    // Left movement
    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))
    {
        player->rectangle.x -= player->speed * deltaTime;
    }

    // Right movement
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D))
    {
        player->rectangle.x += player->speed * deltaTime;
    }

    // Up movement
    if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W))
    {
        player->rectangle.y -= player->speed * deltaTime;
    }

    // Down movement
    if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S))
    {
        player->rectangle.y += player->speed * deltaTime;
    }

    // Left boundary
    if (player->rectangle.x < 0.0f)
    {
        player->rectangle.x = 0.0f;
    }

    // Right boundary
    if (player->rectangle.x + player->rectangle.width > screenWidth)
    {
        player->rectangle.x =
            screenWidth - player->rectangle.width;
    }

    // Top walkable boundary
    if (player->rectangle.y < walkAreaTop)
    {
        player->rectangle.y = walkAreaTop;
    }

    // Bottom walkable boundary
    if (player->rectangle.y > walkAreaBottom)
    {
        player->rectangle.y = walkAreaBottom;
    }
}

void DrawPlayer(const Player *player)
{
    float depth = (player->rectangle.y - 345.0f) / (700.0f - 270.0f);

    if (depth < 0.0f)
        depth = 0.0f;

    if (depth > 1.0f)
        depth = 1.0f;

    float scale = 2.90f + (depth * 1.80f);    
    float scaledWidth  = player->rectangle.width * scale * 1.30f;
    float scaledHeight = player->rectangle.height * scale;

    Texture2D currentTexture = player->idleTextures[player->idleFrame];
    //player image
    Rectangle source =
    {
        0.0f,
        0.0f,
        (float)currentTexture.width,
        (float)currentTexture.height
    };

    // Scaled size depende sa lalim
    //float scaledWidth = player->rectangle.width * scale;
    //float scaledHeight = player->rectangle.height * scale;

    // Position and size sa screen
    Rectangle destination =
    {
        player->rectangle.x + (player->rectangle.width / 2.0f),
        player->rectangle.y + player->rectangle.height,
        scaledWidth,
        scaledHeight
    };

    Vector2 origin =
    {
        scaledWidth / 2.0f,
        scaledHeight
    };

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
    UnloadTexture(player->texture);

    for (int i = 0; i < 3; i++)
    {
        UnloadTexture(player->idleTextures[i]);
    }

    player->texture = (Texture2D){0};
}