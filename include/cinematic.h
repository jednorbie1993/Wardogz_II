#ifndef CINEMATIC_H
#define CINEMATIC_H

#include "raylib.h"

#define STAGE1_CINEMATIC_SCENE_COUNT 6

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