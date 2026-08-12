#ifndef ENEMY_H
#define ENEMY_H

#include "raylib.h"
#include "player.h"

#define MAX_ENEMY_IDLE_FRAMES 8
#define MAX_ENEMY_WALK_FRAMES 8
#define MAX_ENEMY_ATTACK_FRAMES 4

typedef enum EnemyAttackMove
{
    ENEMY_ATTACK_PUNCH = 0,
    ENEMY_ATTACK_ELBOW = 1
} EnemyAttackMove;

typedef struct Enemy
{
    // ============================================================
    // COMMON ENEMY DATA
    // ============================================================

    Rectangle hurtbox;

    int hp;
    int maxHp;
    bool isAlive;

    // Prevents one player attack from damaging the enemy more than once.
    bool hitByCurrentAttack;

    // 0040 - true only when this Enemy instance owns its texture handles.
    bool ownsTextures;

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
    // 0037 - PER-MOVE ATTACK DISTANCE
    // ============================================================

    float punchAttackRange;
    float punchStopDistance;

    float elbowAttackRange;
    float elbowStopDistance;

    // 0038 - ELBOW FRAME 3 LUNGE / SLIDE
    float elbowLungeDistance;
    float elbowLungeRemaining;

    // ============================================================
    // 0037 - PER-MOVE ATTACK HITBOX SETTINGS
    // ============================================================

    float punchHitboxWidth;
    float punchHitboxHeight;
    float punchHitboxOffsetX;
    float punchHitboxOffsetY;

    float elbowHitboxWidth;
    float elbowHitboxHeight;
    float elbowHitboxOffsetX;
    float elbowHitboxOffsetY;

    // ============================================================
    // 0037 - ENEMY ATTACK ANIMATION
    // ============================================================

    Texture2D punchTextures[MAX_ENEMY_ATTACK_FRAMES];
    Texture2D elbowTextures[MAX_ENEMY_ATTACK_FRAMES];

    int punchFrameCount;
    int elbowFrameCount;

    int attackFrame;
    float attackFrameTimer;
    float attackFrameTime;

    EnemyAttackMove currentAttackMove;
    EnemyAttackMove nextAttackMove;

    // ============================================================
    // 0031 - FACING + CHASE AI
    // ============================================================

    bool facingRight;
    bool isChasing;
    float chaseSpeed;
    float chaseStopDistance;
    float chaseDepthTolerance;

    // 0040 - MULTI-ENEMY SPACING / ANTI-OVERLAP
    float separationRadiusX;
    float separationDepthTolerance;
    float separationPushSpeed;

    // ============================================================
    // 0035 - ENEMY STOP / ATTACK RANGE
    // ============================================================

    float attackStopDistance;
    bool isInAttackRange;
    float aggroRange;

    // ============================================================
    // 0032 - ENEMY STAGE BOUNDARY / WALK AREA
    // ============================================================

    float stageAnchorOffsetY;

    // ============================================================
    // 0034 - ENEMY ENTRANCE / SPAWN
    // ============================================================

    bool isEntering;
    bool hasEnteredStage;

    float entranceTargetX;
    float entranceTargetY;
    float entranceSpeed;

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
    // 0036 - GENERIC WALK ANIMATION
    // ============================================================

    Texture2D walkTextures[MAX_ENEMY_WALK_FRAMES];
    int walkFrameCount;

    int walkFrame;
    float walkTimer;
    float walkFrameTime;

    // ============================================================
    // GENERIC SPRITE SETTINGS
    // ============================================================

    float spriteSize;
    float spriteOffsetX;
    float spriteOffsetY;

} Enemy;


// Common/default enemy state.
Enemy InitEnemyBase(void);


// Main shared enemy update/orchestrator.
void UpdateEnemyHit(
    Enemy *enemy,
    Player *player,
    float deltaTime,
    float screenWidth,
    float walkAreaTop,
    float walkAreaBottom
);


// 0034 - Entrance control.
void StartEnemyEntrance(
    Enemy *enemy,
    float targetX,
    float targetStageY,
    float entranceSpeed
);


// 0040 - Keep multiple enemies from stacking on top of each other.
void ResolveEnemySpacing(
    Enemy *enemies,
    int enemyCount,
    float deltaTime,
    float screenWidth,
    float walkAreaTop,
    float walkAreaBottom
);


// Public debug/combat helpers.
Rectangle GetEnemyAttackHitbox(const Enemy *enemy);
Rectangle GetEnemyFootMarker(const Enemy *enemy);


// Shared enemy drawing.
void DrawEnemy(const Enemy *enemy);


// Shared enemy texture cleanup.
void UnloadEnemy(Enemy *enemy);

#endif