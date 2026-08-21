#include "raylib.h"
#include "player.h"
#include "enemy.h"
#include "boss.h"
#include "cinematic.h"
#include "enemy_wave.h"

// Debug/testing switch: 1 = skip title + opening cinematic, 0 = normal flow.
#define DEBUG_SKIP_OPENING 0

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
// 0071 - VARGAS BOSS LOADING / DIALOGUE / FIGHT START
// ============================================================

#define VARGAS_TRIGGER_PROGRESS 0.86f
#define VARGAS_WORLD_X 11452.0f
#define VARGAS_STAGE_Y 390.0f
#define VARGAS_INTRO_LOAD_DELAY 0.35f
#define VARGAS_INTRO_CONTINUE_DELAY 0.50f
#define VARGAS_INTRO_FADE_DURATION 0.75f
#define VARGAS_DIALOGUE_TRIGGER_DISTANCE 500.0f
#define VARGAS_ENDING_DELAY 3.25f

static float ClampIntroValue(float value)
{
    if (value < 0.0f)
        return 0.0f;
    if (value > 1.0f)
        return 1.0f;
    return value;
}

static void DrawVargasBossIntro(
    Texture2D portrait,
    float introTimer,
    bool loadingFinished,
    int dialoguePage,
    bool fadingOut,
    float fadeTimer,
    int screenWidth,
    int screenHeight)
{
    float fadeIn =
        ClampIntroValue(introTimer / 0.25f);
    float overlayAlpha = fadeIn;

    if (fadingOut)
    {
        overlayAlpha *=
            1.0f - ClampIntroValue(
                       fadeTimer / VARGAS_INTRO_FADE_DURATION);
    }

    DrawRectangle(
        0,
        0,
        screenWidth,
        screenHeight,
        Fade(BLACK, 0.88f * overlayAlpha));

    // Thin scanlines keep the reveal alive while Vargas textures load.
    for (int y = 0; y < screenHeight; y += 18)
    {
        DrawLine(
            0,
            y,
            screenWidth,
            y,
            Fade(RED, 0.07f * overlayAlpha));
    }

    DrawRectangle(
        58,
        88,
        7,
        520,
        Fade(RED, overlayAlpha));

    if (portrait.id != 0)
    {
        float portraitReveal =
            ClampIntroValue((introTimer - 0.20f) / 0.65f);

        float portraitSlide =
            (1.0f - portraitReveal) * 110.0f;

        Rectangle source =
            {
                (float)portrait.width,
                0.0f,
                -(float)portrait.width,
                (float)portrait.height};

        Rectangle destination =
            {
                (float)screenWidth - 670.0f + portraitSlide,
                35.0f,
                650.0f,
                650.0f};

        DrawTexturePro(
            portrait,
            source,
            destination,
            (Vector2){0.0f, 0.0f},
            0.0f,
            Fade(
                (Color){255, 150, 150, 255},
                portraitReveal * overlayAlpha));
    }

    if (dialoguePage == 0)
    {
        DrawText(
            "IT'S HIM!",
            78,
            214,
            72,
            Fade(WHITE, overlayAlpha));
    }
    else if (dialoguePage == 1)
    {
        DrawText(
            "HE'S THE ONE WHO",
            78,
            204,
            43,
            Fade(WHITE, overlayAlpha));

        DrawText(
            "RUNS THIS PLACE.",
            78,
            258,
            43,
            Fade(WHITE, overlayAlpha));
    }
    else
    {
        DrawText(
            "WE NEED TO STOP HIM.",
            78,
            228,
            42,
            Fade(WHITE, overlayAlpha));
    }

    if (!fadingOut)
    {
        DrawText(
            loadingFinished
                ? "PRESS ENTER TO CONTINUE"
                : "NOW LOADING... PREPARING THE BOSS BATTLE",
            85,
            screenHeight - 54,
            18,
            Fade(
                loadingFinished ? WHITE : LIGHTGRAY,
                overlayAlpha));
    }

    // Animated red pulse keeps the loading screen visually active.
    int pulseWidth =
        70 + ((int)(introTimer * 220.0f) % 230);

    DrawRectangle(
        85,
        565,
        pulseWidth,
        5,
        Fade(RED, overlayAlpha));
}

static void DrawVargasApproachDialogue(
    int dialoguePage,
    int screenWidth,
    int screenHeight)
{
    int boxX = 48;
    int boxY = screenHeight - 190;
    int boxWidth = screenWidth - 96;
    int boxHeight = 132;

    DrawRectangle(
        boxX,
        boxY,
        boxWidth,
        boxHeight,
        Fade(BLACK, 0.90f));

    DrawRectangleLinesEx(
        (Rectangle){
            (float)boxX,
            (float)boxY,
            (float)boxWidth,
            (float)boxHeight},
        3.0f,
        dialoguePage == 0 ? WHITE : RED);

    DrawText(
        dialoguePage == 0 ? "PLAYER" : "VARGAS",
        boxX + 24,
        boxY + 16,
        21,
        dialoguePage == 0 ? LIGHTGRAY : RED);

    DrawText(
        dialoguePage == 0
            ? "IT'S OVER, VARGAS!"
            : "AMATEUR... YOUR TIME IS UP!",
        boxX + 24,
        boxY + 49,
        32,
        WHITE);

    DrawText(
        "PRESS ENTER",
        boxX + boxWidth - 174,
        boxY + boxHeight - 27,
        16,
        LIGHTGRAY);
}

// ============================================================
// 0049 - DYNAMIC ENEMY HP HUD
// ============================================================
#define ENEMY_HUD_DEATH_HOLD 1.50f

static void UpdateEnemyHudDeathTimers(
    const Enemy *enemies,
    int enemyCount,
    float *deathTimers,
    float deltaTime)
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
            if (deathTimers[i] < 0.0f)
                deathTimers[i] = 0.0f;
        }
    }
}

static void DrawEnemyHud(
    const Enemy *enemies,
    int enemyCount,
    const float *deathTimers)
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

        if (!hasStagePresence)
            continue;
        if (!enemies[i].isAlive && !keepDeadRowBriefly)
            continue;

        float y = startY + (visibleRow * rowHeight);
        float hpPercent = 0.0f;

        if (enemies[i].maxHp > 0)
        {
            hpPercent = (float)enemies[i].hp / (float)enemies[i].maxHp;
        }

        if (hpPercent < 0.0f)
            hpPercent = 0.0f;
        if (hpPercent > 1.0f)
            hpPercent = 1.0f;

        DrawText(
            enemies[i].displayName,
            (int)startX,
            (int)(y - 4.0f),
            20,
            WHITE);

        Rectangle hpBack =
            {
                startX + nameWidth,
                y,
                barWidth,
                barHeight};

        const float nameBarGap = 12.0f;

        Rectangle hpFill =
            {
                hpBack.x,
                hpBack.y,
                hpBack.width * hpPercent,
                hpBack.height};

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

static void DrawStartupLoading(
    int screenWidth,
    int screenHeight,
    float progress)
{
    if (progress < 0.0f)
        progress = 0.0f;
    if (progress > 1.0f)
        progress = 1.0f;

    int barWidth = 500;
    int barHeight = 24;
    int barX = (screenWidth - barWidth) / 2;
    int barY = (screenHeight / 2) + 45;

    BeginDrawing();

    ClearBackground(BLACK);

    DrawText(
        "WARDOGZ II",
        screenWidth / 2 - MeasureText("WARDOGZ II", 50) / 2,
        screenHeight / 2 - 80,
        50,
        WHITE);

    DrawText(
        TextFormat("NOW LOADING... %.0f%%", progress * 100.0f),
        screenWidth / 2 -
            MeasureText(
                TextFormat("NOW LOADING... %.0f%%", progress * 100.0f),
                24) /
                2,
        screenHeight / 2,
        24,
        LIGHTGRAY);

    // Bar background
    DrawRectangle(
        barX,
        barY,
        barWidth,
        barHeight,
        DARKGRAY);

    // RED loading progress
    DrawRectangle(
        barX,
        barY,
        (int)(barWidth * progress),
        barHeight,
        RED);

    // Border
    DrawRectangleLines(
        barX,
        barY,
        barWidth,
        barHeight,
        WHITE);

    EndDrawing();
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
        "Wardogz II");

    // 0069 - Audio must be ready before InitPlayer() loads the hit sounds.
    InitAudioDevice();

    SetTargetFPS(60);

    DrawStartupLoading(
        screenWidth,
        screenHeight,
        0.05f);

    // ============================================================
    // STARTUP LOADING SCREEN
    // ============================================================
    BeginDrawing();

    ClearBackground(BLACK);

    DrawText(
        "WARDOGZ II",
        screenWidth / 2 - MeasureText("WARDOGZ II", 50) / 2,
        screenHeight / 2 - 60,
        50,
        WHITE);

    DrawText(
        "NOW LOADING...",
        screenWidth / 2 - MeasureText("NOW LOADING...", 24) / 2,
        screenHeight / 2 + 20,
        24,
        LIGHTGRAY);

    EndDrawing();

    // Load player and background
    Player player =
        InitPlayer("assets/sprites/player/player.png");

    DrawStartupLoading(
        screenWidth,
        screenHeight,
        0.20f);

    // ============================================================
    // OPENING TITLE SCREEN - PLAYER FIRST, ENEMIES LOAD AFTER ENTER
    // ============================================================
    Texture2D introStart = (Texture2D){0};
    if (!DEBUG_SKIP_OPENING)
    {
        introStart =
            LoadTexture("assets/cinematic/intro/intro_start.png");
    }

    bool titleAccepted = DEBUG_SKIP_OPENING;
    bool titleFadingOut = false;
    float titleFadeTimer = 0.0f;
    float titlePulseTimer = 0.0f;
    const float titleFadeDuration = 0.70f;

    while (!WindowShouldClose() && !titleAccepted)
    {
        float titleDeltaTime = GetFrameTime();
        if (titleDeltaTime > 0.10f) titleDeltaTime = 0.10f;
        titlePulseTimer += titleDeltaTime;

        if (!titleFadingOut && IsKeyPressed(KEY_ENTER))
        {
            titleFadingOut = true;
            titleFadeTimer = 0.0f;
        }

        if (titleFadingOut)
        {
            titleFadeTimer += titleDeltaTime;
            if (titleFadeTimer >= titleFadeDuration)
            {
                titleAccepted = true;
            }
        }

        BeginDrawing();
        ClearBackground(BLACK);

        if (introStart.id != 0 && introStart.width > 0 && introStart.height > 0)
        {
            DrawTexturePro(
                introStart,
                (Rectangle){0.0f, 0.0f, (float)introStart.width, (float)introStart.height},
                (Rectangle){0.0f, 0.0f, (float)screenWidth, (float)screenHeight},
                (Vector2){0.0f, 0.0f},
                0.0f,
                WHITE);
        }

        // The prompt is code-driven so it can blink and be repositioned later.
        if (!titleFadingOut && ((int)(titlePulseTimer * 2.0f) % 2 == 0))
        {
            const char *titlePrompt = "PRESS ENTER TO PLAY";
            int promptWidth = MeasureText(titlePrompt, 26);
            int promptX = (screenWidth - promptWidth) / 2;
            int promptY = screenHeight - 72;
            DrawRectangle(promptX - 22, promptY - 9, promptWidth + 44, 46, Fade(BLACK, 0.68f));
            DrawText(titlePrompt, promptX, promptY, 26, WHITE);
        }

        if (titleFadingOut)
        {
            float alpha = titleFadeTimer / titleFadeDuration;
            if (alpha > 1.0f) alpha = 1.0f;
            DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, alpha));
        }

        EndDrawing();
    }

    if (introStart.id != 0)
    {
        UnloadTexture(introStart);
    }

    if (WindowShouldClose())
    {
        UnloadPlayer(&player);
        CloseAudioDevice();
        CloseWindow();
        return 0;
    }

    // Second loading phase begins only after the title screen is accepted.
    DrawStartupLoading(screenWidth, screenHeight, 0.05f);
    LoadEnemyWaveSharedTextures();
    DrawStartupLoading(screenWidth, screenHeight, 0.15f);

    // 0054 - Horizontal camera for the long Stage 1 walk test.
    Camera2D camera = {0};
    camera.offset = (Vector2){screenWidth * 0.5f, screenHeight * 0.5f};
    camera.target = (Vector2){screenWidth * 0.5f, screenHeight * 0.5f};
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

// ============================================================
// 0074 - STAGE 1 NORMAL ENEMY WAVES + VARGAS BOSS SLOT
// ============================================================
// Slots 0-4 are reserved for normal Stage 1 enemies/reinforcements.
// Slot 5 is reserved for Vargas so normal-wave enemies never collide
// with the boss instance in the shared Enemy array.
#define ENEMY_COUNT 6
#define VARGAS_ENEMY_INDEX 5

    // Vargas does not exist yet. At 86% the intro covers his real loading,
    // then he becomes active at world x = 11452 (about 98% of this stage).
    bool enemiesEnabled = false;
    bool vargasIntroStarted = false;
    bool vargasIntroActive = false;
    bool vargasIntroFadingOut = false;
    bool vargasLoaded = false;
    bool vargasCameraRevealDone = false;
    float vargasIntroTimer = 0.0f;
    float vargasIntroFadeTimer = 0.0f;
    int vargasIntroDialoguePage = 0;

    bool vargasApproachDialogueStarted = false;
    bool vargasApproachDialogueActive = false;
    int vargasApproachDialoguePage = 0;
    bool vargasFightStarted = false;
    float vargasEndingTimer = 0.0f;

    // Opening cinematic uses intro1.png-intro7.png plus intro_tutorial.png.
    // Its images are loaded after the player accepts the title screen.
    OpeningCinematic opening = (OpeningCinematic){0};
    if (!DEBUG_SKIP_OPENING)
    {
        opening = InitOpeningCinematic();
    }

    // 0072 - Stage 1 ending cinematic.
    // The six scene textures are intentionally NOT loaded at startup.
    // They are loaded only after Vargas' death animation has finished.
    Stage1Cinematic stage1Ending = InitStage1Cinematic();

    Enemy enemies[ENEMY_COUNT] = {0};

    // 0074 - Stage 1 normal-enemy wave manager.
    EnemyWaveSystem enemyWaves = InitEnemyWaveSystem();

    // 0049 - Each enemy HUD row remains briefly after HP reaches zero.
    float enemyHudDeathTimers[ENEMY_COUNT] = {0};

    // Only this single portrait is loaded at startup. The remaining Vargas
    // images are delayed until the player reaches the 86% trigger.
    Texture2D vargasIntroPortrait =
        LoadTexture(
            "assets/sprites/enemy/stage_1/boss/boss_battle_idle1.png");
    DrawStartupLoading(
        screenWidth,
        screenHeight,
        0.30f);

// ============================================================
// 0054 - STAGE 1 BACKGROUND SECTIONS
// ============================================================
// Final Stage 1 background sequence.
// Full panels keep their original aspect ratio at 720 px height.
// BA10 is a narrow final strip and is drawn at its proportional width.
#define STAGE_BACKGROUND_COUNT 10

    Texture2D stageBackgrounds[STAGE_BACKGROUND_COUNT] = {0};

    stageBackgrounds[0] = LoadTexture("assets/background/B1.png");
    DrawStartupLoading(screenWidth, screenHeight, 0.41f);

    stageBackgrounds[1] = LoadTexture("assets/background/B2.png");
    DrawStartupLoading(screenWidth, screenHeight, 0.47f);

    stageBackgrounds[2] = LoadTexture("assets/background/BA3.png");
    DrawStartupLoading(screenWidth, screenHeight, 0.53f);

    stageBackgrounds[3] = LoadTexture("assets/background/BA4.png");
    DrawStartupLoading(screenWidth, screenHeight, 0.59f);

    stageBackgrounds[4] = LoadTexture("assets/background/BA5.png");
    DrawStartupLoading(screenWidth, screenHeight, 0.65f);

    stageBackgrounds[5] = LoadTexture("assets/background/BA6.png");
    DrawStartupLoading(screenWidth, screenHeight, 0.72f);

    stageBackgrounds[6] = LoadTexture("assets/background/BA7.png");
    DrawStartupLoading(screenWidth, screenHeight, 0.79f);

    stageBackgrounds[7] = LoadTexture("assets/background/BA8.png");
    DrawStartupLoading(screenWidth, screenHeight, 0.86f);

    stageBackgrounds[8] = LoadTexture("assets/background/BA9.png");
    DrawStartupLoading(screenWidth, screenHeight, 0.93f);

    stageBackgrounds[9] = LoadTexture("assets/background/BA10.png");
    DrawStartupLoading(screenWidth, screenHeight, 1.00f);

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

    const float gameplayFadeInDuration = 0.90f;

    if (!DEBUG_SKIP_OPENING)
    {
        StartOpeningCinematic(&opening);
    }

    bool openingWasActive = !DEBUG_SKIP_OPENING;
    float gameplayFadeInTimer =
        DEBUG_SKIP_OPENING ? gameplayFadeInDuration : 0.0f;

    while (!WindowShouldClose())
    {
        float deltaTime = GetFrameTime();

        // ========================================================
        // OPENING STORY + TUTORIAL
        // ========================================================
        if (!DEBUG_SKIP_OPENING && IsOpeningCinematicActive(&opening))
        {
            UpdateOpeningCinematic(&opening, deltaTime);

            BeginDrawing();
            ClearBackground(BLACK);
            DrawOpeningCinematic(&opening, screenWidth, screenHeight);
            EndDrawing();

            openingWasActive = true;
            continue;
        }

        if (!DEBUG_SKIP_OPENING &&
            openingWasActive &&
            IsOpeningCinematicFinished(&opening))
        {
            openingWasActive = false;
            gameplayFadeInTimer = 0.0f;
        }

        // Wait for Vargas' 3.20-second death animation before leaving gameplay.
        // Once the cinematic starts, gameplay is no longer updated or drawn.
        if (
            !IsStage1CinematicActive(&stage1Ending) &&
            enemiesEnabled &&
            vargasFightStarted &&
            vargasLoaded &&
            !enemies[VARGAS_ENEMY_INDEX].isAlive)
        {
            vargasEndingTimer += deltaTime;

            if (vargasEndingTimer >= VARGAS_ENDING_DELAY)
            {
                StartStage1Cinematic(&stage1Ending);
            }
        }

        if (IsStage1CinematicActive(&stage1Ending))
        {
            UpdateStage1Cinematic(&stage1Ending, deltaTime);

            BeginDrawing();
            ClearBackground(BLACK);
            DrawStage1Cinematic(
                &stage1Ending,
                screenWidth,
                screenHeight);
            EndDrawing();

            continue;
        }

        // ========================================================
        // UPDATE
        // ========================================================
        if (
            !vargasIntroActive &&
            !vargasApproachDialogueActive)
        {
            UpdatePlayer(
                &player,
                deltaTime,
                stageWorldWidth,
                walkAreaTop,
                walkAreaBottom);
        }

        float playerCenterX =
            player.rectangle.x +
            (player.rectangle.width * 0.5f);

        // ========================================================
        // 0074 - NORMAL ENEMY WAVE UPDATE
        // ========================================================
        // Wave 1 currently triggers at 15% and spawns one Hooligan
        // off-screen ahead of Jamber. Forward progress stays locked
        // until that encounter is defeated.
        if (
            !vargasIntroActive &&
            !vargasApproachDialogueActive &&
            !vargasFightStarted)
        {
            UpdateEnemyWaveSystem(
                &enemyWaves,
                &player,
                enemies,
                ENEMY_COUNT - 1,
                stageWorldWidth,
                &camera,
                screenWidth,
                walkAreaTop,
                walkAreaBottom);

            // The wave manager may clamp Jamber to the fight boundary,
            // so refresh his center before camera and boss trigger checks.
            playerCenterX =
                player.rectangle.x +
                (player.rectangle.width * 0.5f);
        }

        float vargasTriggerX =
            stageWorldWidth *
            VARGAS_TRIGGER_PROGRESS;

        if (
            !vargasIntroStarted &&
            playerCenterX >= vargasTriggerX)
        {
            vargasIntroStarted = true;
            vargasIntroActive = true;
            vargasIntroFadingOut = false;
            vargasIntroTimer = 0.0f;
            vargasIntroFadeTimer = 0.0f;
            vargasIntroDialoguePage = 0;
        }

        if (vargasIntroActive)
        {
            // A texture-loading stall must not skip the whole reveal.
            float introDeltaTime = deltaTime;
            if (introDeltaTime > 0.10f)
            {
                introDeltaTime = 0.10f;
            }

            vargasIntroTimer += introDeltaTime;

            // The overlay has already been visible for several frames before
            // InitBoss() performs the one-time loading of all Vargas textures.
            if (
                !vargasLoaded &&
                vargasIntroTimer >= VARGAS_INTRO_LOAD_DELAY)
            {
                enemies[VARGAS_ENEMY_INDEX] =
                    InitBoss(
                        VARGAS_WORLD_X,
                        VARGAS_STAGE_Y);

                vargasLoaded = true;
            }

            bool introDialogueReady =
                vargasLoaded &&
                vargasIntroTimer >= VARGAS_INTRO_CONTINUE_DELAY;

            if (
                introDialogueReady &&
                !vargasIntroFadingOut &&
                IsKeyPressed(KEY_ENTER))
            {
                if (vargasIntroDialoguePage < 2)
                {
                    vargasIntroDialoguePage++;
                }
                else
                {
                    vargasIntroFadingOut = true;
                    vargasIntroFadeTimer = 0.0f;
                }
            }

            if (vargasIntroFadingOut)
            {
                vargasIntroFadeTimer += introDeltaTime;

                if (
                    vargasIntroFadeTimer >=
                    VARGAS_INTRO_FADE_DURATION)
                {
                    vargasIntroActive = false;
                    enemiesEnabled = true;

                    // The normal Vargas sprite set now owns its own copy.
                    // Release the temporary intro-only portrait.
                    if (vargasIntroPortrait.id != 0)
                    {
                        UnloadTexture(vargasIntroPortrait);
                        vargasIntroPortrait = (Texture2D){0};
                    }
                }
            }
        }

        if (
            enemiesEnabled &&
            !vargasApproachDialogueStarted &&
            playerCenterX >=
                (VARGAS_WORLD_X - VARGAS_DIALOGUE_TRIGGER_DISTANCE))
        {
            vargasApproachDialogueStarted = true;
            vargasApproachDialogueActive = true;
            vargasApproachDialoguePage = 0;
        }

        if (
            vargasApproachDialogueActive &&
            IsKeyPressed(KEY_ENTER))
        {
            if (vargasApproachDialoguePage == 0)
            {
                vargasApproachDialoguePage = 1;
            }
            else
            {
                vargasApproachDialogueActive = false;
                vargasFightStarted = true;
            }
        }

        bool normalWavePresent =
            GetEnemyWaveActiveCount(&enemyWaves) > 0;

        bool bossCombatActive =
            enemiesEnabled &&
            vargasFightStarted;

        if (
            !vargasIntroActive &&
            !vargasApproachDialogueActive &&
            (normalWavePresent || bossCombatActive))
        {
            // 0042 - Keep free Punks distributed around the player.
            // Attack-slot owners still chase the player directly.
            ResolveEnemySurroundFormation(
                enemies,
                ENEMY_COUNT);

            // 0043 - Choose/swap the two active attackers dynamically.
            ResolveEnemyAttackSlot(
                enemies,
                ENEMY_COUNT,
                &player);

            // 0044 - Stagger attack starts so the two active Punks do not
            // punch/elbow at the same instant.
            ResolveEnemyAttackTurnTiming(
                enemies,
                ENEMY_COUNT,
                &player,
                deltaTime);

            // 0046 - If an active attacker is blocked behind another Punk,
            // side-step to an upper/lower lane before continuing the approach.
            ResolveEnemyApproachLanes(
                enemies,
                ENEMY_COUNT,
                &player,
                walkAreaTop,
                walkAreaBottom);

            // Update every active/shared enemy slot independently.
            for (int i = 0; i < ENEMY_COUNT; i++)
            {
                UpdateEnemyHit(
                    &enemies[i],
                    &player,
                    deltaTime,
                    stageWorldWidth,
                    walkAreaTop,
                    walkAreaBottom);
            }

            // 0049 - Keep a dead enemy's HUD row visible for a short moment.
            UpdateEnemyHudDeathTimers(
                enemies,
                ENEMY_COUNT,
                enemyHudDeathTimers,
                deltaTime);

            // 0040 - Push nearby Punks apart so they do not stack.
            ResolveEnemySpacing(
                enemies,
                ENEMY_COUNT,
                deltaTime,
                stageWorldWidth,
                walkAreaTop,
                walkAreaBottom);

            // 0048 - Give the player and Punks physical body presence.
            // Same-depth actors softly separate instead of passing through.
            ResolvePlayerEnemyBodyCollision(
                &player,
                enemies,
                ENEMY_COUNT,
                stageWorldWidth,
                walkAreaTop,
                walkAreaBottom);

            // 0043 - Refresh after attacks/cancellations so a nearby waiting
            // Punk can take over a released attack slot immediately.
            ResolveEnemyAttackSlot(
                enemies,
                ENEMY_COUNT,
                &player);
        }

        // ========================================================
        // CAMERA FOLLOW / 25%-75% DEAD ZONE + VARGAS REVEAL
        // ========================================================

        float halfScreenWidth = screenWidth * 0.5f;

        float leftScreenLimit = screenWidth * 0.25f;
        float rightScreenLimit = screenWidth * 0.75f;

        float cameraWorldLeft =
            camera.target.x - halfScreenWidth + leftScreenLimit;

        float cameraWorldRight =
            camera.target.x - halfScreenWidth + rightScreenLimit;

        // Vargas special reveal starts at 93% of the map.
        float cameraRevealTriggerX = stageWorldWidth * 0.93f;

        if (
            !vargasCameraRevealDone &&
            vargasLoaded &&
            playerCenterX >= cameraRevealTriggerX)
        {
            // Pan toward the end of the stage.
            float revealTargetX = stageWorldWidth * 0.99f;

            camera.target.x +=
                (revealTargetX - camera.target.x) * 3.0f * deltaTime;

            // Once close enough, mark the reveal as finished.
            if (camera.target.x >= revealTargetX - 10.0f)
            {
                camera.target.x = revealTargetX;
                vargasCameraRevealDone = true;
            }
        }
        else
        {
            // Normal 25%-75% screen dead zone.
            if (playerCenterX < cameraWorldLeft)
            {
                camera.target.x -= cameraWorldLeft - playerCenterX;
            }
            else if (playerCenterX > cameraWorldRight)
            {
                camera.target.x += playerCenterX - cameraWorldRight;
            }
        }

        // Keep camera inside Stage 1.
        if (camera.target.x < halfScreenWidth)
        {
            camera.target.x = halfScreenWidth;
        }

        if (camera.target.x > stageWorldWidth - halfScreenWidth)
        {
            camera.target.x = stageWorldWidth - halfScreenWidth;
        }

        camera.target.y = screenHeight * 0.5f;

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
                    (float)currentBackground.height};

            Rectangle destination =
                {
                    backgroundX,
                    0.0f,
                    backgroundWidth,
                    (float)screenHeight};

            DrawTexturePro(
                currentBackground,
                source,
                destination,
                (Vector2){0.0f, 0.0f},
                0.0f,
                WHITE);

            backgroundX += backgroundWidth;
        }

        // Temporary visual marker so the end of the walk test is obvious.
        DrawLineEx(
            (Vector2){stageWorldWidth - 8.0f, 0.0f},
            (Vector2){stageWorldWidth - 8.0f, (float)screenHeight},
            8.0f,
            RED);

        DrawText(
            "STAGE 1 END",
            (int)stageWorldWidth - 430,
            80,
            30,
            RED);

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

        drawActors[drawActorCount++] = (DrawActor){
            DRAW_ACTOR_PLAYER,
            playerFeet.y + (playerFeet.height * 0.5f),
            -1};

        // Screen-space enemy HUD follows the screen during normal waves
        // and during the Vargas fight.
        bool enemyHudVisible =
            normalWavePresent ||
            (enemiesEnabled && vargasFightStarted);

        // Draw normal-wave enemies before Vargas, then keep Vargas drawable
        // after his reveal has been accepted.
        bool anyEnemyDrawable =
            normalWavePresent ||
            enemiesEnabled;

        if (anyEnemyDrawable)
        {
            for (int i = 0; i < ENEMY_COUNT; i++)
            {
                Rectangle enemyFeet = GetEnemyFootMarker(&enemies[i]);

                drawActors[drawActorCount++] = (DrawActor){
                    DRAW_ACTOR_ENEMY,
                    enemyFeet.y + (enemyFeet.height * 0.5f),
                    i};
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

        // 0073 - Screen-fixed Jamber name and HP bar.
        DrawPlayerHud(&player);

        // 0049 - Active enemy HP list. Dead rows disappear after a short delay.
        if (enemyHudVisible)
        {
            DrawEnemyHud(
                enemies,
                ENEMY_COUNT,
                enemyHudDeathTimers);
        }

        // 0054 - Walk-test progress HUD (screen-space, not affected by camera).
        float stageProgress = playerCenterX / stageWorldWidth;
        if (stageProgress < 0.0f)
            stageProgress = 0.0f;
        if (stageProgress > 1.0f)
            stageProgress = 1.0f;

        DrawText(
            TextFormat("Stage X: %.0f / %.0f px  (%.0f%%)",
                       playerCenterX,
                       stageWorldWidth,
                       stageProgress * 100.0f),
            30,
            65,
            22,
            YELLOW);

        // Controls stay at the bottom; enemy HUD is now at the top-right.
        DrawText(
            "Move: Arrow Keys | Attack: W A S D",
            30,
            screenHeight - 42,
            22,
            WHITE);

        if (vargasIntroActive)
        {
            DrawVargasBossIntro(
                vargasIntroPortrait,
                vargasIntroTimer,
                vargasLoaded &&
                    vargasIntroTimer >= VARGAS_INTRO_CONTINUE_DELAY,
                vargasIntroDialoguePage,
                vargasIntroFadingOut,
                vargasIntroFadeTimer,
                screenWidth,
                screenHeight);
        }

        if (vargasApproachDialogueActive)
        {
            DrawVargasApproachDialogue(
                vargasApproachDialoguePage,
                screenWidth,
                screenHeight);
        }

        // Fade Stage 1 in from black after the tutorial closes.
        if (!DEBUG_SKIP_OPENING &&
            IsOpeningCinematicFinished(&opening) &&
            gameplayFadeInTimer < gameplayFadeInDuration)
        {
            gameplayFadeInTimer += deltaTime;
            float fadeAlpha = 1.0f - (gameplayFadeInTimer / gameplayFadeInDuration);
            if (fadeAlpha < 0.0f) fadeAlpha = 0.0f;
            DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, fadeAlpha));
        }

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

    if (vargasLoaded)
    {
        for (int i = 0; i < ENEMY_COUNT; i++)
        {
            UnloadEnemy(&enemies[i]);
        }
    }

    // 0074 - Normal enemy sprite caches are shared across clones.
    // Safe to call even when a type was never loaded.
    UnloadEnemyWaveSharedTextures();

    // Vargas' shared textures are released exactly once.
    UnloadBossSharedTextures();

    if (vargasIntroPortrait.id != 0)
    {
        UnloadTexture(vargasIntroPortrait);
    }

    if (!DEBUG_SKIP_OPENING)
    {
        UnloadOpeningCinematic(&opening);
    }
    UnloadStage1Cinematic(&stage1Ending);

    UnloadPlayer(&player);

    CloseAudioDevice();
    CloseWindow();

    return 0;
}