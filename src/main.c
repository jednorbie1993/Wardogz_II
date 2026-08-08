#include "raylib.h"
#include "player.h"

int main(void)
{
    const int screenWidth = 1280;
    const int screenHeight = 720;

    // Walkable area ng stage
    const float walkAreaTop = 400.0f;
    const float walkAreaBottom = 640.0f;

    InitWindow(
        screenWidth,
        screenHeight,
        "Wardogz II"
    );

    SetTargetFPS(60);

    // Gumawa at mag-load ng player
    Player player = InitPlayer("assets/sprites/player/player.png");

    while (!WindowShouldClose())
    {
        float deltaTime = GetFrameTime();

        // Update player movement and boundaries
        UpdatePlayer(
            &player,
            deltaTime,
            (float)screenWidth,
            walkAreaTop,
            walkAreaBottom
        );

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

        // Draw player
        DrawPlayer(&player);

        DrawText(
            "Move: Arrow Keys or WASD",
            30,
            30,
            25,
            WHITE
        );

        EndDrawing();
    }

    // Alisin ang player texture sa memory
    UnloadPlayer(&player);

    CloseWindow();

    return 0;
}