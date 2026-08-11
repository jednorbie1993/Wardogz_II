#ifndef PLAYER_H
#define PLAYER_H

#include "raylib.h"

#define IDLE_FRAME_COUNT 3
#define WALK_FRAME_COUNT 12
#define ATTACK_FRAME_COUNT 3

// ============================================================
// ATTACK TYPES
// ============================================================
typedef enum
{
    ATTACK_NONE,
    ATTACK_LEFT_PUNCH,
    ATTACK_RIGHT_PUNCH,
    ATTACK_LEFT_KICK,
    ATTACK_RIGHT_KICK
} AttackType;

// ============================================================
// PLAYER DATA
// ============================================================
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

    // PLAYER STATE
    bool isWalking;
    AttackType currentAttack;

    // 0026 - COMBO INPUT BUFFER
    // Stores one attack pressed while another attack is still playing.
    AttackType bufferedAttack;

    // 0027 - BASIC COMBO CHAIN STATE
    int comboStep;              // Current step in A -> W -> D
    float comboTimer;           // Time left to continue the combo
    bool comboFinisherActive;   // True when A -> W -> D reaches D

    // 0028 - DIRECTION + ATTACK COMMAND STATE
    bool commandAttackActive;   // True when a directional command starts
    AttackType commandAttack;   // Attack used by the directional command

    // 0029 - RECOVERY / CANCEL WINDOW STATE
    bool isRecovering;          // True during post-attack recovery
    float recoveryTimer;        // Remaining recovery time
    bool cancelWindowOpen;      // True when next attack may be buffered/cancelled

    // false = LEFT
    // true  = RIGHT
    bool facingRight;

} Player;

// ============================================================
// PUBLIC PLAYER FUNCTIONS
// ============================================================
// Implementations are separated into player.c, player_move.c,
// player_attack.c, and player_draw.c. Keeping the declarations
// here means old files that include only player.h still work.

Player InitPlayer(const char *texturePath);

void UpdatePlayer(
    Player *player,
    float deltaTime,
    float screenWidth,
    float walkAreaTop,
    float walkAreaBottom
);

void UpdatePlayerAttack(Player *player, float deltaTime);

bool IsPlayerAttackHitboxActive(const Player *player);
Rectangle GetPlayerAttackHitbox(const Player *player);

int GetPlayerAttackDamage(const Player *player);
float GetPlayerAttackKnockbackSpeed(const Player *player);
float GetPlayerAttackHitReactionTime(const Player *player);

void DrawPlayer(const Player *player);
void UnloadPlayer(Player *player);

#endif // PLAYER_H