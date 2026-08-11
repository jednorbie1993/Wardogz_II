#ifndef ENEMY_H
#define ENEMY_H

#include "raylib.h"
#include "player.h"

#define MAX_ENEMY_IDLE_FRAMES 8

typedef struct Enemy
{
    // ============================================================
    // COMMON ENEMY DATA
    // ============================================================

    Rectangle hurtbox;

    int hp;
    int maxHp;
    bool isAlive;

    // Prevents one attack from damaging the enemy more than once.
    bool hitByCurrentAttack;

    // ============================================================
    // HIT REACTION / KNOCKBACK
    // ============================================================

    bool isHit;
    float hitReactionTimer;
    float knockbackSpeed;
    int knockbackDirection;

    // ============================================================
    // GENERIC IDLE ANIMATION
    // ============================================================

    Texture2D idleTextures[MAX_ENEMY_IDLE_FRAMES];
    int idleFrameCount;

    int idleFrame;
    int idleDirection;
    float idleTimer;
    float idleFrameTime;

    // ============================================================
    // GENERIC SPRITE SETTINGS
    // ============================================================

    float spriteSize;

    // Image adjustment relative to the hurtbox.
    float spriteOffsetX;
    float spriteOffsetY;

} Enemy;


// Creates common/default enemy state.
// Character-specific files such as punk.c customize the returned Enemy.
Enemy InitEnemyBase(void);


// Shared enemy update system.
// Handles idle animation, hit detection, damage, knockback, and death.
void UpdateEnemyHit(
    Enemy *enemy,
    const Player *player,
    float deltaTime,
    float screenWidth
);


// Shared drawing system.
// Draws the current enemy sprite, debug hurtbox, and HP display.
void DrawEnemy(const Enemy *enemy);


// Unload textures belonging to the enemy.
void UnloadEnemy(Enemy *enemy);

#endif