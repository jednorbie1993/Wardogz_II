#include "raylib.h"
#include "player.h"
#include "enemy.h"
#include "boss.h"

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
    const float nameWidth = 110.0f;
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
            enemies[i].displayName,
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

    // ============================================================
    // 0054 - STAGE 1 WORLD LENGTH / WALK TEST
    // ============================================================
    // Stage width is calculated automatically after all background
    // textures are loaded. This prevents black/empty world space when
    // panel widths or file formats change.
    float stageWorldWidth = 0.0f;

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

    // 0054 - Horizontal camera for the long Stage 1 walk test.
    Camera2D camera = {0};
    camera.offset = (Vector2){screenWidth * 0.5f, screenHeight * 0.5f};
    camera.target = (Vector2){screenWidth * 0.5f, screenHeight * 0.5f};
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    // ============================================================
    // 0061 - VARGAS-ONLY BOSS TEST SETUP
    // ============================================================
    //
    // Punk, Hooligan, and Gangster are temporarily hidden.
    // Vargas appears by himself so his complete sprite set and
    // normal-idle -> battle-idle transition can be tested safely.
    //
    #define ENEMY_COUNT 1

    // Temporary Vargas gameplay switch.
    const bool enemiesEnabled = true;

    Enemy enemies[ENEMY_COUNT];

    // 0049 - Each enemy HUD row remains briefly after HP reaches zero.
    float enemyHudDeathTimers[ENEMY_COUNT] = {0};

    LoadBossSharedTextures();

    enemies[0] = InitBoss(1380.0f, 540.0f);
    StartEnemyEntrance(&enemies[0], 1000.0f, 540.0f, 140.0f);

    // ============================================================
    // 0054 - STAGE 1 BACKGROUND SECTIONS
    // ============================================================
    // Final Stage 1 background sequence.
    // Full panels keep their original aspect ratio at 720 px height.
    // BA10 is a narrow final strip and is drawn at its proportional width.
    #define STAGE_BACKGROUND_COUNT 10

    Texture2D stageBackgrounds[STAGE_BACKGROUND_COUNT] =
    {
        LoadTexture("assets/background/B1.png"),
        LoadTexture("assets/background/B2.png"),
        LoadTexture("assets/background/BA3.png"),
        LoadTexture("assets/background/BA4.png"),
        LoadTexture("assets/background/BA5.png"),
        LoadTexture("assets/background/BA6.png"),
        LoadTexture("assets/background/BA7.png"),
        LoadTexture("assets/background/BA8.png"),
        LoadTexture("assets/background/BA9.png"),
        LoadTexture("assets/background/BA10.png")
    };

    // Calculate the real Stage 1 width from every valid texture.
    // Each panel is scaled proportionally to the 720 px game height.
    for (int i = 0; i < STAGE_BACKGROUND_COUNT; i++)
    {
        if (stageBackgrounds[i].id == 0 ||
            stageBackgrounds[i].width <= 0 ||
            stageBackgrounds[i].height <= 0)
        {
            TraceLog(LOG_WARNING,
                "STAGE BACKGROUND FAILED TO LOAD: index %d", i);
            continue;
        }

        float panelScale =
            (float)screenHeight / (float)stageBackgrounds[i].height;

        stageWorldWidth +=
            (float)stageBackgrounds[i].width * panelScale;
    }

    // Never allow a world narrower than the game window.
    if (stageWorldWidth < (float)screenWidth)
    {
        stageWorldWidth = (float)screenWidth;
    }

    while (!WindowShouldClose())
    {
        float deltaTime = GetFrameTime();

        // ========================================================
        // UPDATE
        // ========================================================
        UpdatePlayer(
            &player,
            deltaTime,
            stageWorldWidth,
            walkAreaTop,
            walkAreaBottom
        );

        if (enemiesEnabled)
        {
            // 0042 - Keep free Punks distributed around the player.
            // Attack-slot owners still chase the player directly.
            ResolveEnemySurroundFormation(
                enemies,
                ENEMY_COUNT
            );

            // 0043 - Choose/swap the two active attackers dynamically.
            ResolveEnemyAttackSlot(
                enemies,
                ENEMY_COUNT,
                &player
            );

            // 0044 - Stagger attack starts so the two active Punks do not
            // punch/elbow at the same instant.
            ResolveEnemyAttackTurnTiming(
                enemies,
                ENEMY_COUNT,
                &player,
                deltaTime
            );

            // 0046 - If an active attacker is blocked behind another Punk,
            // side-step to an upper/lower lane before continuing the approach.
            ResolveEnemyApproachLanes(
                enemies,
                ENEMY_COUNT,
                &player,
                walkAreaTop,
                walkAreaBottom
            );

            // Update each Punk independently.
            for (int i = 0; i < ENEMY_COUNT; i++)
            {
                UpdateEnemyHit(
                    &enemies[i],
                    &player,
                    deltaTime,
                    stageWorldWidth,
                    walkAreaTop,
                    walkAreaBottom
                );
            }

            // 0049 - Keep a dead enemy's HUD row visible for a short moment.
            UpdateEnemyHudDeathTimers(
                enemies,
                ENEMY_COUNT,
                enemyHudDeathTimers,
                deltaTime
            );

            // 0040 - Push nearby Punks apart so they do not stack.
            ResolveEnemySpacing(
                enemies,
                ENEMY_COUNT,
                deltaTime,
                stageWorldWidth,
                walkAreaTop,
                walkAreaBottom
            );

            // 0048 - Give the player and Punks physical body presence.
            // Same-depth actors softly separate instead of passing through.
            ResolvePlayerEnemyBodyCollision(
                &player,
                enemies,
                ENEMY_COUNT,
                stageWorldWidth,
                walkAreaTop,
                walkAreaBottom
            );

            // 0043 - Refresh after attacks/cancellations so a nearby waiting
            // Punk can take over a released attack slot immediately.
            ResolveEnemyAttackSlot(
                enemies,
                ENEMY_COUNT,
                &player
            );

        }

        // ========================================================
        // 0054 - CAMERA FOLLOW / WORLD CLAMP
        // ========================================================
        // Follow the player's horizontal position, but do not show
        // anything before x=0 or after the Stage 1 background end.
        float playerCenterX =
            player.rectangle.x + (player.rectangle.width * 0.5f);

        float cameraTargetX = playerCenterX;
        float halfScreenWidth = screenWidth * 0.5f;

        if (cameraTargetX < halfScreenWidth)
        {
            cameraTargetX = halfScreenWidth;
        }

        if (cameraTargetX > stageWorldWidth - halfScreenWidth)
        {
            cameraTargetX = stageWorldWidth - halfScreenWidth;
        }

        camera.target = (Vector2)
        {
            cameraTargetX,
            screenHeight * 0.5f
        };

        // ========================================================
        // DRAW
        // ========================================================
        BeginDrawing();

        ClearBackground(BLACK);

        BeginMode2D(camera);

        // 0054 - Draw every Stage 1 panel at the same 720 px height.
        // Width is calculated from each texture's real aspect ratio,
        // so BA10 can stay narrow without being stretched.
        float backgroundX = 0.0f;

        for (int i = 0; i < STAGE_BACKGROUND_COUNT; i++)
        {
            Texture2D currentBackground = stageBackgrounds[i];

            // Safety: a failed texture must not corrupt backgroundX.
            // Without this guard, height == 0 causes division by zero and
            // every panel after it can disappear into a black screen.
            if (currentBackground.id == 0 ||
                currentBackground.width <= 0 ||
                currentBackground.height <= 0)
            {
                continue;
            }

            float backgroundScale =
                (float)screenHeight / (float)currentBackground.height;

            float backgroundWidth =
                (float)currentBackground.width * backgroundScale;

            Rectangle source =
            {
                0.0f,
                0.0f,
                (float)currentBackground.width,
                (float)currentBackground.height
            };

            Rectangle destination =
            {
                backgroundX,
                0.0f,
                backgroundWidth,
                (float)screenHeight
            };

            DrawTexturePro(
                currentBackground,
                source,
                destination,
                (Vector2){0.0f, 0.0f},
                0.0f,
                WHITE
            );

            backgroundX += backgroundWidth;
        }

        // Temporary visual marker so the end of the walk test is obvious.
        DrawLineEx(
            (Vector2){stageWorldWidth - 8.0f, 0.0f},
            (Vector2){stageWorldWidth - 8.0f, (float)screenHeight},
            8.0f,
            RED
        );

        DrawText(
            "STAGE 1 END",
            (int)stageWorldWidth - 430,
            80,
            30,
            RED
        );

        // ====================================================
        // 0047 - Y-DEPTH DRAW SORTING
        // ====================================================
        // Draw actors from back to front using the bottom/feet Y.
        // This makes MIDDLE overlap TOP, and BOTTOM overlap MIDDLE.
        DrawActor drawActors[ENEMY_COUNT + 1];
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
            for (int i = 0; i < ENEMY_COUNT; i++)
            {
                Rectangle enemyFeet = GetEnemyFootMarker(&enemies[i]);

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
                DrawEnemy(&enemies[drawActors[i].enemyIndex]);
            }
        }

        EndMode2D();

        // 0049 - Active enemy HP list. Dead rows disappear after a short delay.
        if (enemiesEnabled)
        {
            DrawEnemyHud(
                enemies,
                ENEMY_COUNT,
                enemyHudDeathTimers
            );
        }

        // 0054 - Walk-test progress HUD (screen-space, not affected by camera).
        float stageProgress = playerCenterX / stageWorldWidth;
        if (stageProgress < 0.0f) stageProgress = 0.0f;
        if (stageProgress > 1.0f) stageProgress = 1.0f;

        DrawText(
            TextFormat("Stage X: %.0f / %.0f px  (%.0f%%)",
                playerCenterX,
                stageWorldWidth,
                stageProgress * 100.0f),
            30,
            25,
            22,
            YELLOW
        );

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
    for (int i = 0; i < STAGE_BACKGROUND_COUNT; i++)
    {
        if (stageBackgrounds[i].id != 0)
        {
            UnloadTexture(stageBackgrounds[i]);
        }
    }

    for (int i = 0; i < ENEMY_COUNT; i++)
    {
        UnloadEnemy(&enemies[i]);
    }

    // Vargas' shared textures are released exactly once.
    UnloadBossSharedTextures();

    UnloadPlayer(&player);

    CloseWindow();

    return 0;
}