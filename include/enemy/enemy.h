#ifndef ENEMY_H
#define ENEMY_H

#include "raylib.h"
#include "player.h"

#define ENEMY_IDLE_FRAME_COUNT 3

typedef struct Enemy
{
    Rectangle hurtbox;
    int hp;
    int maxHp;
    bool isAlive;

    // Prevents one attack from damaging the enemy more than once.
    bool hitByCurrentAttack;

    // 0019 - Enemy hit reaction / knockback state.
    bool isHit;
    float hitReactionTimer;
    float knockbackSpeed;
    int knockbackDirection;

    // 0020 - Enemy Punk idle animation.
    Texture2D idleTextures[ENEMY_IDLE_FRAME_COUNT];
    int idleFrame;
    int idleDirection;
    float idleTimer;
    float idleFrameTime;

} Enemy;

Enemy InitEnemy(float x, float y);

void UpdateEnemyHit(
    Enemy *enemy,
    const Player *player,
    float deltaTime,
    float screenWidth
);

void DrawEnemy(const Enemy *enemy);
void UnloadEnemy(Enemy *enemy);

#endif