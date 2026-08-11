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
    // 0030 - BASIC ENEMY ATTACK
    // ============================================================
    bool isAttacking;
    float attackTimer;
    float attackCooldownTimer;
    bool hitPlayerThisAttack;

    int attackDamage;
    float attackRange;
    float attackHitboxWidth;
    float attackHitboxHeight;
    float attackKnockbackSpeed;
    float attackHitReactionTime;
    int attackDirection;

    // ============================================================
    // 0031 - FACING + CHASE AI
    // ============================================================
    bool facingRight;
    bool isChasing;
    float chaseSpeed;
    float chaseStopDistance;
    float chaseDepthTolerance;

    // ============================================================
    // 0032 - ENEMY STAGE BOUNDARY / WALK AREA
    // ============================================================
    // Y offset from hurtbox.y to the character's stage-position anchor.
    // For Punk, InitPunk(x, y) uses y as the stage position and places
    // the hurtbox 200 px above it, so this value is 200.0f.
    float stageAnchorOffsetY;

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
    Player *player,
    float deltaTime,
    float screenWidth,
    float walkAreaTop,
    float walkAreaBottom
);


// 0030 - Current enemy attack hitbox.
Rectangle GetEnemyAttackHitbox(const Enemy *enemy);
Rectangle GetEnemyFootMarker(const Enemy *enemy);


// Shared drawing system.
// Draws the current enemy sprite, debug hurtbox, and HP display.
void DrawEnemy(const Enemy *enemy);


// Unload textures belonging to the enemy.
void UnloadEnemy(Enemy *enemy);

#endif