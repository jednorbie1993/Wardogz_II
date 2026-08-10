#ifndef PLAYER_H
#define PLAYER_H

#include "raylib.h"

#define IDLE_FRAME_COUNT 3
#define WALK_FRAME_COUNT 12
#define ATTACK_FRAME_COUNT 3

// ATTACK TYPES
typedef enum
{
    ATTACK_NONE,
    ATTACK_LEFT_PUNCH,
    ATTACK_RIGHT_PUNCH,
    ATTACK_LEFT_KICK,
    ATTACK_RIGHT_KICK
} AttackType;

// Lahat ng impormasyon tungkol sa player
typedef struct Player
{
    Rectangle rectangle;
    Texture2D texture;
    float speed;

    // IDLE ANIMATION
    Texture2D idleTextures[IDLE_FRAME_COUNT];

    int idleFrame;
    int idleDirection;

    float idleTimer;
    float idleFrameTime;

    // WALKING ANIMATION
    Texture2D walkTextures[WALK_FRAME_COUNT];

    int walkFrame;
    float walkTimer;
    float walkFrameTime;

    // ATTACK ANIMATIONS
    Texture2D leftPunchTextures[ATTACK_FRAME_COUNT];
    Texture2D rightPunchTextures[ATTACK_FRAME_COUNT];
    Texture2D leftKickTextures[ATTACK_FRAME_COUNT];
    Texture2D rightKickTextures[ATTACK_FRAME_COUNT];

    int attackFrame;
    float attackTimer;
    float attackFrameTime;
    bool isAttacking;

    // Player state
    bool isWalking;

    // Current attack input
    AttackType currentAttack;

    // false = LEFT
    // true  = RIGHT
    bool facingRight;

} Player;


// Gumawa ng player at mag-load ng textures
Player InitPlayer(const char *texturePath);


// Basahin ang keyboard at galawin ang player
void UpdatePlayer(
    Player *player,
    float deltaTime,
    float screenWidth,
    float walkAreaTop,
    float walkAreaBottom
);

// ============================================================
// 0017 - ATTACK HITBOX SYSTEM
// ============================================================

// True habang active ang attack hitbox.
// TEMPORARY TEST: buong attack animation muna para madaling makita.
bool IsPlayerAttackHitboxActive(const Player *player);

// Kunin ang rectangle ng current attack hitbox.
Rectangle GetPlayerAttackHitbox(const Player *player);


// I-drawing ang player
void DrawPlayer(const Player *player);


// Alisin ang textures sa memory
void UnloadPlayer(Player *player);

#endif