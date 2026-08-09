#ifndef PLAYER_H
#define PLAYER_H

#include "raylib.h"

#define IDLE_FRAME_COUNT 3
#define WALK_FRAME_COUNT 12

// Lahat ng impormasyon tungkol sa player
typedef struct Player
{
    Rectangle rectangle;
    Texture2D texture;
    float speed;

    // IDLE ANIMATION
    Texture2D idleTextures[IDLE_FRAME_COUNT];

    int idleFrame;
    int idleDirection;

    float idleTimer;
    float idleFrameTime;

    // WALKING ANIMATION
    Texture2D walkTextures[WALK_FRAME_COUNT];

    int walkFrame;
    float walkTimer;
    float walkFrameTime;

    // Player state
    bool isWalking;

    // false = LEFT
    // true  = RIGHT
    bool facingRight;

} Player;


// Gumawa ng player at mag-load ng textures
Player InitPlayer(const char *texturePath);


// Basahin ang keyboard at galawin ang player
void UpdatePlayer(
    Player *player,
    float deltaTime,
    float screenWidth,
    float walkAreaTop,
    float walkAreaBottom
);


// I-drawing ang player
void DrawPlayer(const Player *player);


// Alisin ang textures sa memory
void UnloadPlayer(Player *player);

#endif