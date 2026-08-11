#include "raylib.h"
#include "player.h"
#include "enemy.h"
#include "punk.h"

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
    Player player =
        InitPlayer("assets/sprites/player/player.png");

    // ============================================================
    // 0034 - PUNK ENTRANCE / SPAWN
    // ============================================================
    //
    // InitPunk(x, y) = START / SPAWN LOCATION.
    //
    // Current setup:
    // 1380 = outside the RIGHT side of the 1280 screen.
    // 470  = stage/depth Y.
    //
    Enemy enemy = InitPunk(1380.0f, 470.0f);

    // Entrance target inside the stage:
    // targetX, targetStageY, entranceSpeed
    StartEnemyEntrance(
        &enemy,
        1000.0f,
        470.0f,
        140.0f
    );

    Texture2D background =
        LoadTexture("assets/background/back_alley.png");

    while (!WindowShouldClose())
    {
        float deltaTime = GetFrameTime();

        // ========================================================
        // UPDATE
        // ========================================================
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
            (float)screenWidth,
            walkAreaTop,
            walkAreaBottom
        );

        // ========================================================
        // DRAW
        // ========================================================
        BeginDrawing();

        ClearBackground(BLACK);

        Rectangle source =
        {
            0.0f,
            0.0f,
            (float)background.width,
            (float)background.height
        };

        Rectangle destination =
        {
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

        DrawEnemy(&enemy);
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

    // ============================================================
    // UNLOAD
    // ============================================================
    UnloadTexture(background);
    UnloadPlayer(&player);

    CloseWindow();

    return 0;
}