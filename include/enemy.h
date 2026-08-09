#ifndef ENEMY_H
#define ENEMY_H

#include "raylib.h"
#include "player.h"

typedef struct Enemy
{
    Rectangle hurtbox;
    int hp;
    int maxHp;
    bool isAlive;

    // Prevents one attack from damaging the enemy more than once.
    bool hitByCurrentAttack;

} Enemy;

Enemy InitEnemy(float x, float y);
void UpdateEnemyHit(Enemy *enemy, const Player *player);
void DrawEnemy(const Enemy *enemy);

#endif