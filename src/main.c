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
    // 0040 - MULTI-PUNK TEST SETUP
    // ============================================================
    //
    // Stage 1 test limit: 4 Punks at the same time.
    // Each Punk keeps its own AI, HP, attack state, and animation.
    //
    #define PUNK_COUNT 4

    Enemy punks[PUNK_COUNT];

    // 0040 - Load the 17 Punk textures once, then share them across all Punks.
    LoadPunkSharedTextures();

    punks[0] = InitPunk(1380.0f, 470.0f);
    punks[1] = InitPunk(-180.0f, 540.0f);
    punks[2] = InitPunk(1460.0f, 620.0f);
    punks[3] = InitPunk(-260.0f, 430.0f);

    StartEnemyEntrance(&punks[0], 1000.0f, 470.0f, 140.0f);
    StartEnemyEntrance(&punks[1],  220.0f, 540.0f, 140.0f);
    StartEnemyEntrance(&punks[2], 1080.0f, 620.0f, 140.0f);
    StartEnemyEntrance(&punks[3],  320.0f, 430.0f, 140.0f);

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

        // 0042 - Keep free Punks distributed around the player.
        // Attack-slot owners still chase the player directly.
        ResolveEnemySurroundFormation(
            punks,
            PUNK_COUNT
        );

        // 0043 - Choose/swap the two active attackers dynamically.
        ResolveEnemyAttackSlot(
            punks,
            PUNK_COUNT,
            &player
        );

        // Update each Punk independently.
        for (int i = 0; i < PUNK_COUNT; i++)
        {
            UpdateEnemyHit(
                &punks[i],
                &player,
                deltaTime,
                (float)screenWidth,
                walkAreaTop,
                walkAreaBottom
            );
        }

        // 0040 - Push nearby Punks apart so they do not stack.
        ResolveEnemySpacing(
            punks,
            PUNK_COUNT,
            deltaTime,
            (float)screenWidth,
            walkAreaTop,
            walkAreaBottom
        );

        // 0043 - Refresh after attacks/cancellations so a nearby waiting
        // Punk can take over a released attack slot immediately.
        ResolveEnemyAttackSlot(
            punks,
            PUNK_COUNT,
            &player
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

        for (int i = 0; i < PUNK_COUNT; i++)
        {
            DrawEnemy(&punks[i]);
        }

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

    for (int i = 0; i < PUNK_COUNT; i++)
    {
        UnloadEnemy(&punks[i]);
    }

    // Shared Punk textures are released exactly once.
    UnloadPunkSharedTextures();

    UnloadPlayer(&player);

    CloseWindow();

    return 0;
}