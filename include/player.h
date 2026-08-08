#ifndef PLAYER_H
#define PLAYER_H

#include "raylib.h"

// Lahat ng impormasyon tungkol sa player
typedef struct Player
{
    Rectangle rectangle;
    Texture2D texture;
    float speed;

    Texture2D idleTextures[3];

    int idleFrame;
    int idleDirection;

    float idleTimer;
    float idleFrameTime;

} Player;

// Gumawa ng player at mag-load ng texture
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

// Alisin ang texture sa memory
void UnloadPlayer(Player *player);

#endif