#ifndef PUNK_H
#define PUNK_H

#include "enemy.h"

// 0040 - Load/unload the Punk texture set only once for all Punk instances.
void LoadPunkSharedTextures(void);
void UnloadPunkSharedTextures(void);

// Stage 1 Punk enemy.
Enemy InitPunk(float x, float y);

#endif