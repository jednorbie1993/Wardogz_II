#ifndef HOOLIGAN_H
#define HOOLIGAN_H

#include "enemy.h"

// Load/unload the Hooligan texture set once for all instances.
void LoadHooliganSharedTextures(void);
void UnloadHooliganSharedTextures(void);

// Stage 1 Hooligan enemy.
Enemy InitHooligan(float x, float y);

#endif