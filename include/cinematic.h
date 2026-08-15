#ifndef CINEMATIC_H
#define CINEMATIC_H

#include "raylib.h"

#define STAGE1_CINEMATIC_SCENE_COUNT 6

#define OPENING_CINEMATIC_SCENE_COUNT 8

typedef enum OpeningCinematicPhase
{
    OPENING_CINEMATIC_INACTIVE,
    OPENING_CINEMATIC_LOADING,
    OPENING_CINEMATIC_FADE_IN,
    OPENING_CINEMATIC_READING,
    OPENING_CINEMATIC_FADE_OUT
} OpeningCinematicPhase;

typedef struct OpeningCinematic
{
    bool active;
    bool finished;
    Texture2D scenes[OPENING_CINEMATIC_SCENE_COUNT];
    int loadedSceneCount;
    int currentScene;
    float loadingTimer;
    float phaseTimer;
    float typedCharacters;
    OpeningCinematicPhase phase;
} OpeningCinematic;

OpeningCinematic InitOpeningCinematic(void);
void StartOpeningCinematic(OpeningCinematic *cinematic);
bool IsOpeningCinematicActive(const OpeningCinematic *cinematic);
bool IsOpeningCinematicFinished(const OpeningCinematic *cinematic);
void UpdateOpeningCinematic(OpeningCinematic *cinematic, float deltaTime);
void DrawOpeningCinematic(
    const OpeningCinematic *cinematic,
    int screenWidth,
    int screenHeight
);
void UnloadOpeningCinematic(OpeningCinematic *cinematic);

typedef enum Stage1CinematicPhase
{
    STAGE1_CINEMATIC_INACTIVE,
    STAGE1_CINEMATIC_LOADING,
    STAGE1_CINEMATIC_FADE_IN,
    STAGE1_CINEMATIC_READING,
    STAGE1_CINEMATIC_FADE_OUT,
    STAGE1_CINEMATIC_FINAL_FADE_IN,
    STAGE1_CINEMATIC_FINAL_SCREEN
} Stage1CinematicPhase;

typedef struct Stage1Cinematic
{
    bool active;
    Texture2D scenes[STAGE1_CINEMATIC_SCENE_COUNT];
    int loadedSceneCount;
    int currentScene;
    float loadingTimer;
    float phaseTimer;
    float typedCharacters;
    Stage1CinematicPhase phase;
} Stage1Cinematic;

Stage1Cinematic InitStage1Cinematic(void);
void StartStage1Cinematic(Stage1Cinematic *cinematic);
bool IsStage1CinematicActive(const Stage1Cinematic *cinematic);
void UpdateStage1Cinematic(Stage1Cinematic *cinematic, float deltaTime);
void DrawStage1Cinematic(
    const Stage1Cinematic *cinematic,
    int screenWidth,
    int screenHeight
);
void UnloadStage1Cinematic(Stage1Cinematic *cinematic);

#endif