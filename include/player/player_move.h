#ifndef PLAYER_MOVE_H
#define PLAYER_MOVE_H

#include "player.h"

void UpdatePlayer(
    Player *player,
    float deltaTime,
    float screenWidth,
    float walkAreaTop,
    float walkAreaBottom
);

#endif