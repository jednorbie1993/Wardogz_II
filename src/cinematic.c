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


// ============================================================
// OPENING CINEMATIC - TITLE STORY -> TUTORIAL -> STAGE 1
// ============================================================
#define OPENING_LOAD_WAIT 0.20f
#define OPENING_FADE_TIME 0.70f
#define OPENING_TYPE_SPEED 44.0f

typedef struct OpeningDialogue
{
    const char *speaker;
    const char *line1;
    const char *line2;
    Color speakerColor;
} OpeningDialogue;

static const char *openingScenePaths[OPENING_CINEMATIC_SCENE_COUNT] =
{
    "assets/cinematic/intro/intro1.png",
    "assets/cinematic/intro/intro2.png",
    "assets/cinematic/intro/intro3.png",
    "assets/cinematic/intro/intro4.png",
    "assets/cinematic/intro/intro5.png",
    "assets/cinematic/intro/intro6.png",
    "assets/cinematic/intro/intro7.png",
    "assets/cinematic/intro/intro_tutorial.png"
};

static const OpeningDialogue openingDialogue[OPENING_CINEMATIC_SCENE_COUNT] =
{
    {
        "THE CAPTIVE CITY",
        "Night after night, trucks entered the industrial district.",
        "Inside were prisoners the city had forgotten.",
        {230, 190, 80, 255}
    },
    {
        "FORCED LABOR",
        "Those who survived were forced to work.",
        "No names. No freedom. No way out.",
        {230, 190, 80, 255}
    },
    {
        "VARGAS",
        "From above, Vargas watched everything.",
        "To him, they were only assets.",
        {230, 41, 55, 255}
    },
    {
        "PROJECT CERBERUS",
        "The strongest were forced to fight.",
        "Others were taken deeper inside.",
        {230, 41, 55, 255}
    },
    {
        "JAMBER",
        "Then Jamber felt something familiar.",
        "His own kind was inside.",
        {230, 190, 80, 255}
    },
    {
        "THE TERRITORY",
        "Vargas' men stood between him and the prisoners.",
        "Jamber kept walking.",
        {230, 190, 80, 255}
    },
    {
        "JAMBER",
        "He wasn't here for glory.",
        "No one gets left behind.",
        {230, 190, 80, 255}
    },
    {
        "TUTORIAL",
        "",
        "",
        {230, 190, 80, 255}
    }
};

static int GetOpeningDialogueCharacterCount(const OpeningDialogue *dialogue)
{
    return (int)strlen(dialogue->line1) + (int)strlen(dialogue->line2);
}

static void DrawOpeningLoadingScreen(
    const OpeningCinematic *cinematic,
    int screenWidth,
    int screenHeight)
{
    float progress =
        (float)cinematic->loadedSceneCount /
        (float)OPENING_CINEMATIC_SCENE_COUNT;

    DrawRectangle(0, 0, screenWidth, screenHeight, BLACK);

    for (int y = 0; y < screenHeight; y += 18)
    {
        DrawLine(0, y, screenWidth, y, Fade(RED, 0.06f));
    }

    DrawCenteredText("WARDOGZ II", screenWidth / 2, 220, 48, WHITE);
    DrawCenteredText("NOW LOADING...", screenWidth / 2, 285, 30, LIGHTGRAY);

    Rectangle barBack = {(screenWidth - 700.0f) * 0.5f, 355.0f, 700.0f, 18.0f};
    Rectangle barFill = barBack;
    barFill.width *= progress;

    DrawRectangleRec(barBack, (Color){35, 35, 35, 255});
    DrawRectangleRec(barFill, RED);
    DrawRectangleLinesEx(barBack, 2.0f, WHITE);

    DrawCenteredText(
        TextFormat("PREPARING STORY %d / %d",
                   cinematic->loadedSceneCount,
                   OPENING_CINEMATIC_SCENE_COUNT),
        screenWidth / 2,
        395,
        20,
        LIGHTGRAY);
}

static void DrawOpeningDialogueBox(
    const OpeningCinematic *cinematic,
    int screenWidth,
    int screenHeight,
    float alpha)
{
    const OpeningDialogue *dialogue =
        &openingDialogue[cinematic->currentScene];

    // Tutorial has its controls baked into the artwork. Only show the start prompt.
    if (cinematic->currentScene == OPENING_CINEMATIC_SCENE_COUNT - 1)
    {
        const char *prompt = "PRESS ENTER TO START";
        int promptWidth = MeasureText(prompt, 22);
        int boxWidth = promptWidth + 54;
        int boxX = (screenWidth - boxWidth) / 2;
        int boxY = screenHeight - 74;

        DrawRectangle(boxX, boxY, boxWidth, 42, Fade(BLACK, 0.78f * alpha));
        DrawRectangleLinesEx((Rectangle){(float)boxX, (float)boxY, (float)boxWidth, 42.0f},
                             2.0f, Fade(WHITE, alpha));
        DrawText(prompt, boxX + 27, boxY + 10, 22, Fade(WHITE, alpha));
        return;
    }

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

    DrawRectangle(boxX, boxY, boxWidth, boxHeight, Fade(BLACK, 0.86f * alpha));
    DrawRectangleLinesEx(
        (Rectangle){(float)boxX, (float)boxY, (float)boxWidth, (float)boxHeight},
        3.0f,
        Fade(RED, alpha));

    DrawText(dialogue->speaker, boxX + 24, boxY + 14, 22,
             Fade(dialogue->speakerColor, alpha));

    DrawTypedText(dialogue->line1, line1Visible,
                  boxX + 24, boxY + 48, 28, Fade(WHITE, alpha));

    if (dialogue->line2[0] != '\0')
    {
        DrawTypedText(dialogue->line2, line2Visible,
                      boxX + 24, boxY + 83, 28, Fade(WHITE, alpha));
    }

    if (cinematic->phase == OPENING_CINEMATIC_READING &&
        cinematic->typedCharacters >=
            (float)GetOpeningDialogueCharacterCount(dialogue))
    {
        DrawText("[ENTER]", boxX + boxWidth - 112,
                 boxY + boxHeight - 28, 17, Fade(LIGHTGRAY, alpha));
    }
}

static void DrawOpeningCurrentScene(
    const OpeningCinematic *cinematic,
    int screenWidth,
    int screenHeight)
{
    Texture2D scene = cinematic->scenes[cinematic->currentScene];
    float blackAlpha = 0.0f;
    float contentAlpha = 1.0f;

    if (scene.id != 0 && scene.width > 0 && scene.height > 0)
    {
        DrawTexturePro(
            scene,
            (Rectangle){0.0f, 0.0f, (float)scene.width, (float)scene.height},
            (Rectangle){0.0f, 0.0f, (float)screenWidth, (float)screenHeight},
            (Vector2){0.0f, 0.0f},
            0.0f,
            WHITE);
    }
    else
    {
        DrawRectangle(0, 0, screenWidth, screenHeight, BLACK);
        DrawCenteredText(
            TextFormat("MISSING: intro%d.png", cinematic->currentScene + 1),
            screenWidth / 2,
            (screenHeight / 2) - 20,
            30,
            RED);
    }

    if (cinematic->phase == OPENING_CINEMATIC_FADE_IN)
    {
        blackAlpha = 1.0f - Clamp01(cinematic->phaseTimer / OPENING_FADE_TIME);
        contentAlpha = 1.0f - blackAlpha;
    }
    else if (cinematic->phase == OPENING_CINEMATIC_FADE_OUT)
    {
        blackAlpha = Clamp01(cinematic->phaseTimer / OPENING_FADE_TIME);
        contentAlpha = 1.0f - blackAlpha;
    }

    DrawOpeningDialogueBox(cinematic, screenWidth, screenHeight, contentAlpha);

    if (blackAlpha > 0.0f)
    {
        DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, blackAlpha));
    }
}

OpeningCinematic InitOpeningCinematic(void)
{
    OpeningCinematic cinematic = {0};
    cinematic.phase = OPENING_CINEMATIC_INACTIVE;
    return cinematic;
}

void StartOpeningCinematic(OpeningCinematic *cinematic)
{
    if (cinematic == 0 || cinematic->active) return;

    cinematic->active = true;
    cinematic->finished = false;
    cinematic->loadedSceneCount = 0;
    cinematic->currentScene = 0;
    cinematic->loadingTimer = 0.0f;
    cinematic->phaseTimer = 0.0f;
    cinematic->typedCharacters = 0.0f;
    cinematic->phase = OPENING_CINEMATIC_LOADING;
}

bool IsOpeningCinematicActive(const OpeningCinematic *cinematic)
{
    return cinematic != 0 && cinematic->active;
}

bool IsOpeningCinematicFinished(const OpeningCinematic *cinematic)
{
    return cinematic != 0 && cinematic->finished;
}

void UpdateOpeningCinematic(OpeningCinematic *cinematic, float deltaTime)
{
    if (cinematic == 0 || !cinematic->active) return;
    if (deltaTime > 0.10f) deltaTime = 0.10f;

    if (cinematic->phase == OPENING_CINEMATIC_LOADING)
    {
        cinematic->loadingTimer += deltaTime;

        if (cinematic->loadingTimer >= OPENING_LOAD_WAIT &&
            cinematic->loadedSceneCount < OPENING_CINEMATIC_SCENE_COUNT)
        {
            int index = cinematic->loadedSceneCount;
            cinematic->scenes[index] = LoadTexture(openingScenePaths[index]);

            if (cinematic->scenes[index].id == 0)
            {
                TraceLog(LOG_WARNING,
                         "OPENING IMAGE FAILED TO LOAD: %s",
                         openingScenePaths[index]);
            }

            cinematic->loadedSceneCount++;
            cinematic->loadingTimer = 0.0f;

            if (cinematic->loadedSceneCount >= OPENING_CINEMATIC_SCENE_COUNT)
            {
                cinematic->phase = OPENING_CINEMATIC_FADE_IN;
                cinematic->phaseTimer = 0.0f;
                cinematic->typedCharacters = 0.0f;
            }
        }
        return;
    }

    if (cinematic->phase == OPENING_CINEMATIC_FADE_IN)
    {
        cinematic->phaseTimer += deltaTime;
        if (cinematic->phaseTimer >= OPENING_FADE_TIME)
        {
            cinematic->phase = OPENING_CINEMATIC_READING;
            cinematic->phaseTimer = 0.0f;
        }
        return;
    }

    if (cinematic->phase == OPENING_CINEMATIC_READING)
    {
        const OpeningDialogue *dialogue = &openingDialogue[cinematic->currentScene];
        int totalCharacters = GetOpeningDialogueCharacterCount(dialogue);
        bool tutorialScene = cinematic->currentScene == OPENING_CINEMATIC_SCENE_COUNT - 1;

        if (!tutorialScene && cinematic->typedCharacters < (float)totalCharacters)
        {
            cinematic->typedCharacters += OPENING_TYPE_SPEED * deltaTime;
            if (cinematic->typedCharacters > (float)totalCharacters)
            {
                cinematic->typedCharacters = (float)totalCharacters;
            }
        }

        if (IsKeyPressed(KEY_ENTER))
        {
            if (!tutorialScene && cinematic->typedCharacters < (float)totalCharacters)
            {
                cinematic->typedCharacters = (float)totalCharacters;
            }
            else
            {
                cinematic->phase = OPENING_CINEMATIC_FADE_OUT;
                cinematic->phaseTimer = 0.0f;
            }
        }
        return;
    }

    if (cinematic->phase == OPENING_CINEMATIC_FADE_OUT)
    {
        cinematic->phaseTimer += deltaTime;

        if (cinematic->phaseTimer >= OPENING_FADE_TIME)
        {
            if (cinematic->currentScene < OPENING_CINEMATIC_SCENE_COUNT - 1)
            {
                cinematic->currentScene++;
                cinematic->phase = OPENING_CINEMATIC_FADE_IN;
                cinematic->phaseTimer = 0.0f;
                cinematic->typedCharacters = 0.0f;
            }
            else
            {
                cinematic->active = false;
                cinematic->finished = true;
                cinematic->phase = OPENING_CINEMATIC_INACTIVE;
                cinematic->phaseTimer = 0.0f;
            }
        }
    }
}

void DrawOpeningCinematic(
    const OpeningCinematic *cinematic,
    int screenWidth,
    int screenHeight)
{
    if (cinematic == 0 || !cinematic->active) return;

    if (cinematic->phase == OPENING_CINEMATIC_LOADING)
    {
        DrawOpeningLoadingScreen(cinematic, screenWidth, screenHeight);
        return;
    }

    DrawOpeningCurrentScene(cinematic, screenWidth, screenHeight);
}

void UnloadOpeningCinematic(OpeningCinematic *cinematic)
{
    if (cinematic == 0) return;

    for (int i = 0; i < OPENING_CINEMATIC_SCENE_COUNT; i++)
    {
        if (cinematic->scenes[i].id != 0)
        {
            UnloadTexture(cinematic->scenes[i]);
            cinematic->scenes[i] = (Texture2D){0};
        }
    }

    cinematic->active = false;
    cinematic->finished = false;
    cinematic->loadedSceneCount = 0;
    cinematic->phase = OPENING_CINEMATIC_INACTIVE;
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