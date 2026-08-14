#ifndef BOSS_H
#define BOSS_H

#include "enemy.h"

void LoadBossSharedTextures(void);
void UnloadBossSharedTextures(void);
Enemy InitBoss(float x, float y);

#endif