#include "cinematic.h"

#include <string.h>

#define CINEMATIC_LOAD_WAIT 0.35f
#define CINEMATIC_FADE_TIME 0.60f
#define CINEMATIC_FINAL_FADE_TIME 1.20f
#define CINEMATIC_TYPE_SPEED 42.0f

typedef struct CinematicDialogue
{
    const char *speaker;
    const char *line1;
    const char *line2;
    Color speakerColor;
} CinematicDialogue;

static const char *scenePaths[STAGE1_CINEMATIC_SCENE_COUNT] =
{
    "assets/cinematic/stage1/scene1.png",
    "assets/cinematic/stage1/scene2.png",
    "assets/cinematic/stage1/scene3.png",
    "assets/cinematic/stage1/scene4.png",
    "assets/cinematic/stage1/scene5.png",
    "assets/cinematic/stage1/scene6.png"
};

static const CinematicDialogue sceneDialogue[STAGE1_CINEMATIC_SCENE_COUNT] =
{
    {
        "STAGE 1",
        "VARGAS DEFEATED",
        "",
        {230, 41, 55, 255}
    },
    {
        "JAMBER",
        "Where are the prisoners, Vargas?!",
        "",
        {230, 190, 80, 255}
    },
    {
        "VARGAS",
        "You still don't get it...",
        "I don't run this city.",
        {230, 41, 55, 255}
    },
    {
        "SFX",
        "RING... RING...",
        "",
        {230, 41, 55, 255}
    },
    {
        "VARGAS",
        "The shipment already left for the docks.",
        "You're too late.",
        {230, 41, 55, 255}
    },
    {
        "JAMBER",
        "Then that's where I'm going.",
        "",
        {230, 190, 80, 255}
    }
};

static float Clamp01(float value)
{
    if (value < 0.0f) return 0.0f;
    if (value > 1.0f) return 1.0f;
    return value;
}

static int GetDialogueCharacterCount(const CinematicDialogue *dialogue)
{
    return (int)strlen(dialogue->line1) +
        (int)strlen(dialogue->line2);
}

static void DrawCenteredText(
    const char *text,
    int centerX,
    int y,
    int fontSize,
    Color color
)
{
    int width = MeasureText(text, fontSize);
    DrawText(text, centerX - (width / 2), y, fontSize, color);
}

static void DrawTypedText(
    const char *text,
    int visibleCharacters,
    int x,
    int y,
    int fontSize,
    Color color
)
{
    char visibleText[256];
    int length = (int)strlen(text);

    if (visibleCharacters < 0) visibleCharacters = 0;
    if (visibleCharacters > length) visibleCharacters = length;
    if (visibleCharacters > 255) visibleCharacters = 255;

    memcpy(visibleText, text, (size_t)visibleCharacters);
    visibleText[visibleCharacters] = '\0';

    DrawText(visibleText, x, y, fontSize, color);
}

static void DrawLoadingScreen(
    const Stage1Cinematic *cinematic,
    int screenWidth,
    int screenHeight
)
{
    float progress =
        (float)cinematic->loadedSceneCount /
        (float)STAGE1_CINEMATIC_SCENE_COUNT;

    DrawRectangle(0, 0, screenWidth, screenHeight, BLACK);

    for (int y = 0; y < screenHeight; y += 18)
    {
        DrawLine(0, y, screenWidth, y, Fade(RED, 0.07f));
    }

    DrawRectangle(78, 170, 7, 310, RED);
    DrawText("STAGE 1 COMPLETE", 110, 205, 24, RED);
    DrawText("LOADING CINEMATIC...", 110, 250, 52, WHITE);
    DrawText(
        TextFormat(
            "READING SCENE %d OF %d",
            cinematic->loadedSceneCount,
            STAGE1_CINEMATIC_SCENE_COUNT
        ),
        114,
        325,
        22,
        LIGHTGRAY
    );

    Rectangle barBack = {110.0f, 390.0f, 850.0f, 18.0f};
    Rectangle barFill = barBack;
    barFill.width *= progress;

    DrawRectangleRec(barBack, (Color){35, 35, 35, 255});
    DrawRectangleRec(barFill, RED);
    DrawRectangleLinesEx(barBack, 2.0f, WHITE);
}

static void DrawDialogueBox(
    const Stage1Cinematic *cinematic,
    int screenWidth,
    int screenHeight,
    float alpha
)
{
    const CinematicDialogue *dialogue =
        &sceneDialogue[cinematic->currentScene];

    int boxX = 48;
    int boxY = screenHeight - 190;
    int boxWidth = screenWidth - 96;
    int boxHeight = 150;
    int line1Length = (int)strlen(dialogue->line1);
    int visibleCharacters = (int)cinematic->typedCharacters;
    int line1Visible = visibleCharacters;
    int line2Visible = visibleCharacters - line1Length;

    if (line1Visible > line1Length) line1Visible = line1Length;
    if (line2Visible < 0) line2Visible = 0;

    DrawRectangle(
        boxX,
        boxY,
        boxWidth,
        boxHeight,
        Fade(BLACK, 0.88f * alpha)
    );

    DrawRectangleLinesEx(
        (Rectangle)
        {
            (float)boxX,
            (float)boxY,
            (float)boxWidth,
            (float)boxHeight
        },
        3.0f,
        Fade(RED, alpha)
    );

    DrawText(
        dialogue->speaker,
        boxX + 24,
        boxY + 14,
        22,
        Fade(dialogue->speakerColor, alpha)
    );

    DrawTypedText(
        dialogue->line1,
        line1Visible,
        boxX + 24,
        boxY + 48,
        30,
        Fade(WHITE, alpha)
    );

    if (dialogue->line2[0] != '\0')
    {
        DrawTypedText(
            dialogue->line2,
            line2Visible,
            boxX + 24,
            boxY + 83,
            30,
            Fade(WHITE, alpha)
        );
    }

    if (
        cinematic->phase == STAGE1_CINEMATIC_READING &&
        cinematic->typedCharacters >=
            (float)GetDialogueCharacterCount(dialogue)
    )
    {
        DrawText(
            "[ENTER]",
            boxX + boxWidth - 112,
            boxY + boxHeight - 28,
            17,
            Fade(LIGHTGRAY, alpha)
        );
    }
}

static void DrawCurrentScene(
    const Stage1Cinematic *cinematic,
    int screenWidth,
    int screenHeight
)
{
    Texture2D scene = cinematic->scenes[cinematic->currentScene];
    float blackAlpha = 0.0f;
    float contentAlpha = 1.0f;

    if (scene.id != 0 && scene.width > 0 && scene.height > 0)
    {
        DrawTexturePro(
            scene,
            (Rectangle)
            {
                0.0f,
                0.0f,
                (float)scene.width,
                (float)scene.height
            },
            (Rectangle)
            {
                0.0f,
                0.0f,
                (float)screenWidth,
                (float)screenHeight
            },
            (Vector2){0.0f, 0.0f},
            0.0f,
            WHITE
        );
    }
    else
    {
        DrawRectangle(0, 0, screenWidth, screenHeight, BLACK);
        DrawCenteredText(
            TextFormat("MISSING: scene%d.png", cinematic->currentScene + 1),
            screenWidth / 2,
            (screenHeight / 2) - 20,
            30,
            RED
        );
    }

    if (cinematic->phase == STAGE1_CINEMATIC_FADE_IN)
    {
        blackAlpha =
            1.0f - Clamp01(cinematic->phaseTimer / CINEMATIC_FADE_TIME);
        contentAlpha = 1.0f - blackAlpha;
    }
    else if (cinematic->phase == STAGE1_CINEMATIC_FADE_OUT)
    {
        blackAlpha =
            Clamp01(cinematic->phaseTimer / CINEMATIC_FADE_TIME);
        contentAlpha = 1.0f - blackAlpha;
    }

    DrawDialogueBox(
        cinematic,
        screenWidth,
        screenHeight,
        contentAlpha
    );

    if (blackAlpha > 0.0f)
    {
        DrawRectangle(
            0,
            0,
            screenWidth,
            screenHeight,
            Fade(BLACK, blackAlpha)
        );
    }
}

static void DrawFinalScreen(
    const Stage1Cinematic *cinematic,
    int screenWidth,
    int screenHeight
)
{
    float alpha = 1.0f;

    if (cinematic->phase == STAGE1_CINEMATIC_FINAL_FADE_IN)
    {
        alpha = Clamp01(
            cinematic->phaseTimer / CINEMATIC_FINAL_FADE_TIME
        );
    }

    DrawRectangle(0, 0, screenWidth, screenHeight, BLACK);

    DrawRectangle(
        (screenWidth / 2) - 185,
        (screenHeight / 2) - 72,
        370,
        4,
        Fade(RED, alpha)
    );

    DrawCenteredText(
        "TO BE CONTINUED...",
        screenWidth / 2,
        (screenHeight / 2) - 45,
        48,
        Fade(WHITE, alpha)
    );

    DrawCenteredText(
        "NEXT: BLACKFANG DOCKS",
        screenWidth / 2,
        (screenHeight / 2) + 25,
        27,
        Fade(RED, alpha)
    );
}

Stage1Cinematic InitStage1Cinematic(void)
{
    Stage1Cinematic cinematic = {0};
    cinematic.phase = STAGE1_CINEMATIC_INACTIVE;
    return cinematic;
}

void StartStage1Cinematic(Stage1Cinematic *cinematic)
{
    if (cinematic == 0 || cinematic->active) return;

    cinematic->active = true;
    cinematic->loadedSceneCount = 0;
    cinematic->currentScene = 0;
    cinematic->loadingTimer = 0.0f;
    cinematic->phaseTimer = 0.0f;
    cinematic->typedCharacters = 0.0f;
    cinematic->phase = STAGE1_CINEMATIC_LOADING;
}

bool IsStage1CinematicActive(const Stage1Cinematic *cinematic)
{
    return cinematic != 0 && cinematic->active;
}

void UpdateStage1Cinematic(Stage1Cinematic *cinematic, float deltaTime)
{
    if (cinematic == 0 || !cinematic->active) return;

    if (deltaTime > 0.10f) deltaTime = 0.10f;

    if (cinematic->phase == STAGE1_CINEMATIC_LOADING)
    {
        cinematic->loadingTimer += deltaTime;

        if (
            cinematic->loadingTimer >= CINEMATIC_LOAD_WAIT &&
            cinematic->loadedSceneCount < STAGE1_CINEMATIC_SCENE_COUNT
        )
        {
            int index = cinematic->loadedSceneCount;
            cinematic->scenes[index] = LoadTexture(scenePaths[index]);

            if (cinematic->scenes[index].id == 0)
            {
                TraceLog(
                    LOG_WARNING,
                    "CINEMATIC IMAGE FAILED TO LOAD: %s",
                    scenePaths[index]
                );
            }

            cinematic->loadedSceneCount++;

            if (
                cinematic->loadedSceneCount >=
                    STAGE1_CINEMATIC_SCENE_COUNT
            )
            {
                cinematic->phase = STAGE1_CINEMATIC_FADE_IN;
                cinematic->phaseTimer = 0.0f;
                cinematic->typedCharacters = 0.0f;
            }
        }

        return;
    }

    if (cinematic->phase == STAGE1_CINEMATIC_FADE_IN)
    {
        cinematic->phaseTimer += deltaTime;

        if (cinematic->phaseTimer >= CINEMATIC_FADE_TIME)
        {
            cinematic->phase = STAGE1_CINEMATIC_READING;
            cinematic->phaseTimer = 0.0f;
        }

        return;
    }

    if (cinematic->phase == STAGE1_CINEMATIC_READING)
    {
        const CinematicDialogue *dialogue =
            &sceneDialogue[cinematic->currentScene];
        int totalCharacters = GetDialogueCharacterCount(dialogue);

        if (cinematic->typedCharacters < (float)totalCharacters)
        {
            cinematic->typedCharacters +=
                CINEMATIC_TYPE_SPEED * deltaTime;

            if (cinematic->typedCharacters > (float)totalCharacters)
            {
                cinematic->typedCharacters = (float)totalCharacters;
            }
        }

        if (IsKeyPressed(KEY_ENTER))
        {
            if (cinematic->typedCharacters < (float)totalCharacters)
            {
                // First Enter completes a line that is still typing.
                cinematic->typedCharacters = (float)totalCharacters;
            }
            else
            {
                // The next Enter begins the fade to the following scene.
                cinematic->phase = STAGE1_CINEMATIC_FADE_OUT;
                cinematic->phaseTimer = 0.0f;
            }
        }

        return;
    }

    if (cinematic->phase == STAGE1_CINEMATIC_FADE_OUT)
    {
        cinematic->phaseTimer += deltaTime;

        if (cinematic->phaseTimer >= CINEMATIC_FADE_TIME)
        {
            if (
                cinematic->currentScene <
                    STAGE1_CINEMATIC_SCENE_COUNT - 1
            )
            {
                cinematic->currentScene++;
                cinematic->phase = STAGE1_CINEMATIC_FADE_IN;
            }
            else
            {
                cinematic->phase = STAGE1_CINEMATIC_FINAL_FADE_IN;
            }

            cinematic->phaseTimer = 0.0f;
            cinematic->typedCharacters = 0.0f;
        }

        return;
    }

    if (cinematic->phase == STAGE1_CINEMATIC_FINAL_FADE_IN)
    {
        cinematic->phaseTimer += deltaTime;

        if (cinematic->phaseTimer >= CINEMATIC_FINAL_FADE_TIME)
        {
            cinematic->phase = STAGE1_CINEMATIC_FINAL_SCREEN;
            cinematic->phaseTimer = CINEMATIC_FINAL_FADE_TIME;
        }
    }
}

void DrawStage1Cinematic(
    const Stage1Cinematic *cinematic,
    int screenWidth,
    int screenHeight
)
{
    if (cinematic == 0 || !cinematic->active) return;

    if (cinematic->phase == STAGE1_CINEMATIC_LOADING)
    {
        DrawLoadingScreen(cinematic, screenWidth, screenHeight);
        return;
    }

    if (
        cinematic->phase == STAGE1_CINEMATIC_FINAL_FADE_IN ||
        cinematic->phase == STAGE1_CINEMATIC_FINAL_SCREEN
    )
    {
        DrawFinalScreen(cinematic, screenWidth, screenHeight);
        return;
    }

    DrawCurrentScene(cinematic, screenWidth, screenHeight);
}

void UnloadStage1Cinematic(Stage1Cinematic *cinematic)
{
    if (cinematic == 0) return;

    for (int i = 0; i < STAGE1_CINEMATIC_SCENE_COUNT; i++)
    {
        if (cinematic->scenes[i].id != 0)
        {
            UnloadTexture(cinematic->scenes[i]);
            cinematic->scenes[i] = (Texture2D){0};
        }
    }

    cinematic->active = false;
    cinematic->loadedSceneCount = 0;
    cinematic->phase = STAGE1_CINEMATIC_INACTIVE;
}