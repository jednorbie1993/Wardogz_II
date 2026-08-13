#include "raylib.h"
#include "player.h"
#include "enemy.h"
#include "punk.h"

// ============================================================
// 0047 - Y-DEPTH DRAW SORTING
// ============================================================
// Smaller foot/bottom Y is farther back and is drawn first.
// Larger foot/bottom Y is closer to the camera and is drawn last.
typedef enum DrawActorType
{
    DRAW_ACTOR_PLAYER,
    DRAW_ACTOR_ENEMY
} DrawActorType;

typedef struct DrawActor
{
    DrawActorType type;
    float depthY;
    int enemyIndex;
} DrawActor;


// ============================================================
// 0049 - DYNAMIC ENEMY HP HUD
// ============================================================
#define ENEMY_HUD_DEATH_HOLD 1.50f

static void UpdateEnemyHudDeathTimers(
    const Enemy *enemies,
    int enemyCount,
    float *deathTimers,
    float deltaTime
)
{
    for (int i = 0; i < enemyCount; i++)
    {
        if (enemies[i].isAlive)
        {
            deathTimers[i] = ENEMY_HUD_DEATH_HOLD;
        }
        else if (deathTimers[i] > 0.0f)
        {
            deathTimers[i] -= deltaTime;
            if (deathTimers[i] < 0.0f) deathTimers[i] = 0.0f;
        }
    }
}

static void DrawEnemyHud(
    const Enemy *enemies,
    int enemyCount,
    const float *deathTimers
)
{
    const float startY = 24.0f;
    const float rowHeight = 34.0f;
    const float nameWidth = 70.0f;
    const float barWidth = 170.0f;
    const float barHeight = 14.0f;
    const float rightMargin = 24.0f;
    const float startX = (float)GetScreenWidth() - rightMargin - nameWidth - barWidth;

    int visibleRow = 0;

    for (int i = 0; i < enemyCount; i++)
    {
        // Do not reserve HUD space for enemies that have not entered/spawned yet.
        // Show the HUD only after this enemy has actually entered the stage.
        // Future/off-screen enemies do not reserve a row yet.
        bool hasStagePresence = enemies[i].hasEnteredStage;

        bool keepDeadRowBriefly =
            !enemies[i].isAlive &&
            deathTimers[i] > 0.0f;

        if (!hasStagePresence) continue;
        if (!enemies[i].isAlive && !keepDeadRowBriefly) continue;

        float y = startY + (visibleRow * rowHeight);
        float hpPercent = 0.0f;

        if (enemies[i].maxHp > 0)
        {
            hpPercent = (float)enemies[i].hp / (float)enemies[i].maxHp;
        }

        if (hpPercent < 0.0f) hpPercent = 0.0f;
        if (hpPercent > 1.0f) hpPercent = 1.0f;

        DrawText(
            "PUNK",
            (int)startX,
            (int)(y - 4.0f),
            20,
            WHITE
        );

        Rectangle hpBack =
        {
            startX + nameWidth,
            y,
            barWidth,
            barHeight
        };

        Rectangle hpFill =
        {
            hpBack.x,
            hpBack.y,
            hpBack.width * hpPercent,
            hpBack.height
        };

        DrawRectangleRec(hpBack, BLACK);
        DrawRectangleRec(hpFill, RED);
        DrawRectangleLinesEx(hpBack, 2.0f, WHITE);

        visibleRow++;
    }
}

static void SortDrawActorsByDepth(DrawActor *actors, int count)
{
    for (int i = 1; i < count; i++)
    {
        DrawActor current = actors[i];
        int j = i - 1;

        while (j >= 0 && actors[j].depthY > current.depthY)
        {
            actors[j + 1] = actors[j];
            j--;
        }

        actors[j + 1] = current;
    }
}

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

    // TEMPORARY: Disable all Punk gameplay while building/testing Stage 1.
    // Change this to true when the stage/map is ready for enemy placement.
    const bool enemiesEnabled = false;

    Enemy punks[PUNK_COUNT];

    // 0049 - Each enemy HUD row remains briefly after HP reaches zero.
    float enemyHudDeathTimers[PUNK_COUNT] = {0};

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

        if (enemiesEnabled)
        {
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

            // 0044 - Stagger attack starts so the two active Punks do not
            // punch/elbow at the same instant.
            ResolveEnemyAttackTurnTiming(
                punks,
                PUNK_COUNT,
                &player,
                deltaTime
            );

            // 0046 - If an active attacker is blocked behind another Punk,
            // side-step to an upper/lower lane before continuing the approach.
            ResolveEnemyApproachLanes(
                punks,
                PUNK_COUNT,
                &player,
                walkAreaTop,
                walkAreaBottom
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

            // 0049 - Keep a dead enemy's HUD row visible for a short moment.
            UpdateEnemyHudDeathTimers(
                punks,
                PUNK_COUNT,
                enemyHudDeathTimers,
                deltaTime
            );

            // 0040 - Push nearby Punks apart so they do not stack.
            ResolveEnemySpacing(
                punks,
                PUNK_COUNT,
                deltaTime,
                (float)screenWidth,
                walkAreaTop,
                walkAreaBottom
            );

            // 0048 - Give the player and Punks physical body presence.
            // Same-depth actors softly separate instead of passing through.
            ResolvePlayerEnemyBodyCollision(
                &player,
                punks,
                PUNK_COUNT,
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

        }

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

        // ====================================================
        // 0047 - Y-DEPTH DRAW SORTING
        // ====================================================
        // Draw actors from back to front using the bottom/feet Y.
        // This makes MIDDLE overlap TOP, and BOTTOM overlap MIDDLE.
        DrawActor drawActors[PUNK_COUNT + 1];
        int drawActorCount = 0;

        // Use the SAME ground-depth reference used by combat checks:
        // the center Y of each actor's foot marker.
        Rectangle playerFeet = GetPlayerFootMarker(&player);

        drawActors[drawActorCount++] = (DrawActor)
        {
            DRAW_ACTOR_PLAYER,
            playerFeet.y + (playerFeet.height * 0.5f),
            -1
        };

        if (enemiesEnabled)
        {
            for (int i = 0; i < PUNK_COUNT; i++)
            {
                Rectangle enemyFeet = GetEnemyFootMarker(&punks[i]);

                drawActors[drawActorCount++] = (DrawActor)
                {
                    DRAW_ACTOR_ENEMY,
                    enemyFeet.y + (enemyFeet.height * 0.5f),
                    i
                };
            }
        }

        SortDrawActorsByDepth(drawActors, drawActorCount);

        for (int i = 0; i < drawActorCount; i++)
        {
            if (drawActors[i].type == DRAW_ACTOR_PLAYER)
            {
                DrawPlayer(&player);
            }
            else
            {
                DrawEnemy(&punks[drawActors[i].enemyIndex]);
            }
        }

        // 0049 - Active enemy HP list. Dead rows disappear after a short delay.
        if (enemiesEnabled)
        {
            DrawEnemyHud(
                punks,
                PUNK_COUNT,
                enemyHudDeathTimers
            );
        }

        // Controls stay at the bottom; enemy HUD is now at the top-right.
        DrawText(
            "Move: Arrow Keys | Attack: W A S D",
            30,
            screenHeight - 42,
            22,
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