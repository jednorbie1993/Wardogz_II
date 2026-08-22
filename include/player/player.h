#ifndef PLAYER_H
#define PLAYER_H

#include "raylib.h"

#define IDLE_BREATH_FRAME_COUNT 4
#define IDLE_BATTLE_FRAME_COUNT 8
#define WALK_FRAME_COUNT 11
#define RUN_FRAME_COUNT 12
#define MAX_PLAYER_ATTACK_FRAMES 6

// ============================================================
// ATTACK TYPES
// ============================================================
typedef enum
{
    ATTACK_NONE,
    ATTACK_LEFT_PUNCH,
    ATTACK_RIGHT_PUNCH,
    ATTACK_LEFT_KICK,
    ATTACK_RIGHT_KICK,

    // 0075 - JAMBER ADVANCED MOVE SET
    ATTACK_HAMMER_PUNCH,
    ATTACK_BACK_BLOW,
    ATTACK_ELBOW_DASH,
    ATTACK_DOWNWARD_FIST,
    ATTACK_SLIDE_KICK,
    ATTACK_ROUND_KICK,
    ATTACK_HAMMER_CHARGE,
    ATTACK_PUNCH_CHARGE,
    ATTACK_UPPERCUT,
    ATTACK_CHOP,
    ATTACK_DROP_KICK,
    ATTACK_ELBOW_RISE,
    ATTACK_HIP_CHECK,
    ATTACK_HEADBUTT,

    PLAYER_ATTACK_TYPE_COUNT
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
    Texture2D idleBreathTextures[IDLE_BREATH_FRAME_COUNT];
    Texture2D idleBattleTextures[IDLE_BATTLE_FRAME_COUNT];
    int idleFrame;
    int idleDirection;
    float idleTimer;
    float idleFrameTime;

    // 0037 - NORMAL/BATTLE IDLE MODE
    bool battleIdleActive;
    float battleIdleTimer;
    float battleIdleDuration;

    // WALKING ANIMATION
    Texture2D walkTextures[WALK_FRAME_COUNT];
    int walkFrame;
    float walkTimer;
    float walkFrameTime;

    // 0076 - RUNNING ANIMATION - run1.png to run12.png
    Texture2D runTextures[RUN_FRAME_COUNT];
    int runFrame;
    float runTimer;
    float runFrameTime;

    // ATTACK ANIMATIONS
    // Every basic and advanced move uses the same indexed storage.
    Texture2D attackTextures
        [PLAYER_ATTACK_TYPE_COUNT]
        [MAX_PLAYER_ATTACK_FRAMES];
    Texture2D crouchTexture;

    int attackFrame;
    float attackTimer;
    float attackFrameTime;
    bool isAttacking;

    // PLAYER STATE
    bool isWalking;
    bool isRunning;
    float runSpeedMultiplier;
    AttackType currentAttack;

    // 0026 - COMBO INPUT BUFFER
    // Stores one attack pressed while another attack is still playing.
    AttackType bufferedAttack;

    // 0027 - BASIC COMBO CHAIN STATE
    int comboStep;              // Current step in A -> W -> D
    float comboTimer;           // Time left to continue the combo
    bool comboFinisherActive;   // True only while a combo finisher is playing
    bool bufferedComboFinisher; // Finisher waiting behind the current attack

    // 0028 - DIRECTION + ATTACK COMMAND STATE
    bool commandAttackActive;   // True when a directional command starts
    AttackType commandAttack;   // Attack used by the directional command

    // 0029 - RECOVERY / CANCEL WINDOW STATE
    bool isRecovering;          // True during post-attack recovery
    float recoveryTimer;        // Remaining recovery time
    bool cancelWindowOpen;      // True when next attack may be buffered/cancelled

    // 0075 - Input chord / tap-versus-hold state.
    unsigned int attackButtonMask;
    float attackButtonChordTimer;
    bool aHoldPending;
    float aHoldTimer;

    // Hold-A Punch Charge. Frame 2 remains frozen until A is released.
    bool punchChargeHolding;
    float punchChargeTimer;
    int punchChargeLevel;       // 0 = weak, 1 = 1.5 sec, 2 = 3 sec

    // Per-move forward/backward slide, measured from scaled sprite width.
    float attackSlideRemaining;
    float attackSlideSpeed;
    int attackSlideDirection;

    // Hold Ctrl to stay in the one-frame crouch pose.
    bool isCrouching;
    bool showHitboxes;          // F1 debug toggle, off by default

    // ============================================================
    // 0030 - PLAYER HURT / HP STATE
    // ============================================================
    float maxHp;
    float hp;
    bool isAlive;

    // 0073 - PASSIVE HP REGENERATION
    // Jamber restores 1.5 HP after each full second while below max HP.
    float hpRegenTimer;
    float hpRegenInterval;
    float hpRegenAmount;

    bool isHit;
    float hitReactionTimer;
    float knockbackSpeed;
    int knockbackDirection;

    // 0069 - Alternating player hit sounds.
    Sound enemyHitSound;
    Sound enemyHitSoundAlternate;
    bool useAlternateEnemyHitSound;

    // false = LEFT
    // true  = RIGHT
    bool facingRight;

    // 0051 - DISTANCE-BASED HORIZONTAL TURN
    // When moving opposite the current facing direction, the player
    // keeps the old facing until enough horizontal distance is travelled.
    float turnDirectionTravel;
    float turnDirectionDistance;
    int pendingFacingDirection; // -1 = LEFT, 0 = NONE, 1 = RIGHT

    // 0052 - FORWARD / BACK DASH
    // Facing RIGHT + RIGHT, RIGHT = forward dash RIGHT.
    // Facing RIGHT + LEFT, LEFT   = back dash LEFT.
    // Facing LEFT  + LEFT, LEFT   = forward dash LEFT.
    // Facing LEFT  + RIGHT, RIGHT = back dash RIGHT.
    float dashTapTimer;
    float dashTapWindow;
    int lastDashTapDirection;   // -1 = LEFT, 0 = NONE, 1 = RIGHT
    bool dashTapFacingRight;    // Facing captured on the first tap
    bool isDashing;
    int dashDirection;          // -1 = LEFT, 1 = RIGHT
    bool dashLockedFacingRight; // Facing stays locked during any dash
    float dashTimer;
    float dashDuration;
    float dashSpeed;

    // A just-completed double tap remains available briefly for an
    // F,F+button or B,B+button command. A recognized command cancels
    // the ordinary dash before the special move takes over.
    float dashCommandTimer;
    float dashCommandWindow;
    int dashCommandDirection;
    bool dashCommandFacingRight;
    float dashCommandStartX;

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
void PlayPlayerAttackHitSound(Player *player);

// 0030 - Player hurtbox / hit reaction
Rectangle GetPlayerHurtbox(const Player *player);
Rectangle GetPlayerFootMarker(const Player *player);
void DamagePlayer(
    Player *player,
    int damage,
    int knockbackDirection,
    float knockbackSpeed,
    float hitReactionTime
);
void UpdatePlayerHitReaction(
    Player *player,
    float deltaTime,
    float screenWidth
);

void DrawPlayer(const Player *player);
void DrawPlayerHud(const Player *player);
void UnloadPlayer(Player *player);

#endif // PLAYER_H