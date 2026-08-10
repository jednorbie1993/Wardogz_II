#include "raylib.h"
#include "player.h"
#include "enemy.h"

int main(void)
{
    const int screenWidth = 1280;
    const int screenHeight = 720;

    // Walkable area ng stage
    const float walkAreaTop = 410.0f;
    const float walkAreaBottom = 820.0f;

    InitWindow(
        screenWidth,
        screenHeight,
        "Wardogz II"
    );

    SetTargetFPS(60);

    // Load player and background
    Player player = InitPlayer("assets/sprites/player/player.png");

    // 0018 - Stationary enemy dummy
    Enemy enemy = InitEnemy(850.0f, 470.0f);

    Texture2D background = LoadTexture("assets/background/back_alley.png");

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

        // 0019 - Damage + enemy hit reaction / knockback.
        UpdateEnemyHit(
            &enemy,
            &player,
            deltaTime,
            (float)screenWidth
        );

        BeginDrawing();

        // Clear first, then draw the background
        ClearBackground(BLACK);

        // Scale background to exactly fit 1280x720
        Rectangle source = {
            0.0f,
            0.0f,
            (float)background.width,
            (float)background.height
        };

        Rectangle destination = {
            0.0f,
            0.0f,
            (float)screenWidth,
            (float)screenHeight
        };

        DrawTexturePro(
            background,
            source,
            destination,
            (Vector2){0.0f, 0.0f},
            0.0f,
            WHITE
        );

        // 0019 - Draw enemy dummy, hurtbox, HP, and hit reaction.
        DrawEnemy(&enemy);

        // Draw player on top of the background
        DrawPlayer(&player);

        DrawText(
            "Move: Arrow Keys | Attack: W A S D",
            30,
            30,
            25,
            WHITE
        );

        EndDrawing();
    }

    // Unload textures from memory
    UnloadTexture(background);
    UnloadPlayer(&player);

    CloseWindow();

    return 0;
}