#include "raylib.h"

int main(void)
{
    const int screenWidth = 1280;
    const int screenHeight = 720;

    InitWindow(screenWidth, screenHeight, "Wardogz II");
    SetTargetFPS(60);

    // Load player image
    Texture2D playerTexture = LoadTexture("assets/player.png");

    // Player position at size
    Rectangle player = {
        150.0f,   // X position
        500.0f,   // Y position
        64.0f,    // Width ng player sa screen
        64.0f     // Height ng player sa screen
    };

    float playerSpeed = 300.0f;

    // Walkable area ng stage
    const float walkAreaTop = 400.0f;
    const float walkAreaBottom = 640.0f;

    while (!WindowShouldClose())
    {
        float deltaTime = GetFrameTime();

        // Movement
        if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))
        {
            player.x -= playerSpeed * deltaTime;
        }

        if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D))
        {
            player.x += playerSpeed * deltaTime;
        }

        if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W))
        {
            player.y -= playerSpeed * deltaTime;
        }

        if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S))
        {
            player.y += playerSpeed * deltaTime;
        }

        // Huwag palabasin sa left at right ng screen
        if (player.x < 0)
        {
            player.x = 0;
        }

        if (player.x + player.width > screenWidth)
        {
            player.x = screenWidth - player.width;
        }

        // Huwag palabasin sa walkable area
        if (player.y < walkAreaTop)
        {
            player.y = walkAreaTop;
        }

        if (player.y + player.height > walkAreaBottom)
        {
            player.y = walkAreaBottom - player.height;
        }

        BeginDrawing();

        ClearBackground(SKYBLUE);

        // Background area
        DrawRectangle(
            0,
            0,
            screenWidth,
            (int)walkAreaTop,
            DARKBLUE
        );

        DrawText(
            "BACKGROUND AREA",
            500,
            180,
            30,
            WHITE
        );

        // Walkable floor
        DrawRectangle(
            0,
            (int)walkAreaTop,
            screenWidth,
            (int)(walkAreaBottom - walkAreaTop),
            GRAY
        );

        // Foreground
        DrawRectangle(
            0,
            (int)walkAreaBottom,
            screenWidth,
            screenHeight - (int)walkAreaBottom,
            DARKGRAY
        );

        // Buong image source
        Rectangle source = {
            0.0f,
            0.0f,
            (float)playerTexture.width,
            (float)playerTexture.height
        };

        // Player image destination
        Rectangle destination = {
            player.x,
            player.y,
            player.width,
            player.height
        };

        // Center/origin ng image
        Vector2 origin = {
            0.0f,
            0.0f
        };

        // Draw player image
        DrawTexturePro(
            playerTexture,
            source,
            destination,
            origin,
            0.0f,
            WHITE
        );

        DrawText(
            "Move: Arrow Keys or WASD",
            30,
            30,
            25,
            WHITE
        );

        EndDrawing();
    }

    // Alisin sa memory ang image
    UnloadTexture(playerTexture);

    CloseWindow();

    return 0;
}