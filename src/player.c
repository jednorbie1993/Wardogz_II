#include "player.h"

Player InitPlayer(const char *texturePath)
{
    Player player = {0};

    // Load player image
    player.texture = LoadTexture(texturePath);

    // Player position and size
    player.rectangle = (Rectangle)
    {
        150.0f,   // X position
        500.0f,   // Y position
        64.0f,    // Width
        64.0f     // Height
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
    if (player->rectangle.y + player->rectangle.height > walkAreaBottom)
    {
        player->rectangle.y =
            walkAreaBottom - player->rectangle.height;
    }
}

void DrawPlayer(const Player *player)
{
    // Buong player image
    Rectangle source =
    {
        0.0f,
        0.0f,
        (float)player->texture.width,
        (float)player->texture.height
    };

    // Position and size sa screen
    Rectangle destination = player->rectangle;

    // Drawing origin
    Vector2 origin =
    {
        0.0f,
        0.0f
    };

    DrawTexturePro(
        player->texture,
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

    player->texture = (Texture2D){0};
}