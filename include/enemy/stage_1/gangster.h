#ifndef GANGSTER_H
#define GANGSTER_H

#include "enemy.h"

void LoadGangsterSharedTextures(void);
void UnloadGangsterSharedTextures(void);

Enemy InitGangster(float x, float y);

#endif