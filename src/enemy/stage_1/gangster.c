#include "gangster.h"

static bool gangsterTexturesLoaded = false;
static Texture2D gangsterIdleTextures[3];
static Texture2D gangsterRunTextures[6];
static Texture2D gangsterPunchTextures[4];
static Texture2D gangsterKickTextures[4];
static Texture2D gangsterDeathTextures[3];

void LoadGangsterSharedTextures(void)
{
    if (gangsterTexturesLoaded) return;

    gangsterIdleTextures[0] = LoadTexture("assets/sprites/enemy/stage_1/gangster/gangster_idle1.png");
    gangsterIdleTextures[1] = LoadTexture("assets/sprites/enemy/stage_1/gangster/gangster_idle2.png");
    gangsterIdleTextures[2] = LoadTexture("assets/sprites/enemy/stage_1/gangster/gangster_idle3.png");

    gangsterRunTextures[0] = LoadTexture("assets/sprites/enemy/stage_1/gangster/gangster_run1.png");
    gangsterRunTextures[1] = LoadTexture("assets/sprites/enemy/stage_1/gangster/gangster_run2.png");
    gangsterRunTextures[2] = LoadTexture("assets/sprites/enemy/stage_1/gangster/gangster_run3.png");
    gangsterRunTextures[3] = LoadTexture("assets/sprites/enemy/stage_1/gangster/gangster_run4.png");
    gangsterRunTextures[4] = LoadTexture("assets/sprites/enemy/stage_1/gangster/gangster_run5.png");
    gangsterRunTextures[5] = LoadTexture("assets/sprites/enemy/stage_1/gangster/gangster_run6.png");

    gangsterPunchTextures[0] = LoadTexture("assets/sprites/enemy/stage_1/gangster/gangster_punch1.png");
    gangsterPunchTextures[1] = LoadTexture("assets/sprites/enemy/stage_1/gangster/gangster_punch2.png");
    gangsterPunchTextures[2] = LoadTexture("assets/sprites/enemy/stage_1/gangster/gangster_punch3.png");
    gangsterPunchTextures[3] = LoadTexture("assets/sprites/enemy/stage_1/gangster/gangster_punch4.png");

    // The common enemy system uses the second-attack slot for this kick.
    gangsterKickTextures[0] = LoadTexture("assets/sprites/enemy/stage_1/gangster/gangster_kick1.png");
    gangsterKickTextures[1] = LoadTexture("assets/sprites/enemy/stage_1/gangster/gangster_kick2.png");
    gangsterKickTextures[2] = LoadTexture("assets/sprites/enemy/stage_1/gangster/gangster_kick3.png");
    gangsterKickTextures[3] = LoadTexture("assets/sprites/enemy/stage_1/gangster/gangster_kick4.png");

    gangsterDeathTextures[0] = LoadTexture("assets/sprites/enemy/stage_1/gangster/gangster_death1.png");
    gangsterDeathTextures[1] = LoadTexture("assets/sprites/enemy/stage_1/gangster/gangster_death2.png");
    gangsterDeathTextures[2] = LoadTexture("assets/sprites/enemy/stage_1/gangster/gangster_death3.png");

    gangsterTexturesLoaded = true;
}

void UnloadGangsterSharedTextures(void)
{
    if (!gangsterTexturesLoaded) return;

    for (int i = 0; i < 3; i++) UnloadTexture(gangsterIdleTextures[i]);
    for (int i = 0; i < 6; i++) UnloadTexture(gangsterRunTextures[i]);
    for (int i = 0; i < 4; i++) UnloadTexture(gangsterPunchTextures[i]);
    for (int i = 0; i < 4; i++) UnloadTexture(gangsterKickTextures[i]);
    for (int i = 0; i < 3; i++) UnloadTexture(gangsterDeathTextures[i]);

    gangsterTexturesLoaded = false;
}

Enemy InitGangster(float x, float y)
{
    Enemy enemy = InitEnemyBase();

    enemy.displayName = "GANGSTER";
    enemy.hurtbox = (Rectangle){x, y - 200.0f, 148.0f, 276.0f};
    enemy.maxHp = 200;
    enemy.hp = enemy.maxHp;
    enemy.attackDamage = 30;

    enemy.facingRight = false;
    enemy.chaseSpeed = 175.0f;
    enemy.chaseStopDistance = 155.0f;
    enemy.chaseDepthTolerance = 8.0f;

    enemy.separationRadiusX = 135.0f;
    enemy.separationDepthTolerance = 75.0f;
    enemy.separationPushSpeed = 300.0f;

    enemy.retreatDuration = 0.24f;
    enemy.retreatSpeed = 480.0f;
    enemy.retreatPauseDuration = 0.55f;

    enemy.punchAttackRange = 230.0f;
    enemy.punchStopDistance = 158.0f;

    // Gangster kick uses the shared second-attack settings.
    enemy.elbowAttackRange = 220.0f;
    enemy.elbowStopDistance = 180.0f;
    enemy.elbowLungeDistance = 70.0f;
    enemy.elbowLungeRemaining = 0.0f;

    enemy.stageAnchorOffsetY = 220.0f;

    LoadGangsterSharedTextures();
    enemy.ownsTextures = false;

    enemy.idleFrameCount = 3;
    for (int i = 0; i < enemy.idleFrameCount; i++)
        enemy.idleTextures[i] = gangsterIdleTextures[i];
    enemy.idleFrameTime = 0.19f;

    enemy.walkFrameCount = 6;
    for (int i = 0; i < enemy.walkFrameCount; i++)
        enemy.walkTextures[i] = gangsterRunTextures[i];
    enemy.walkFrameTime = 0.10f;

    enemy.punchFrameCount = 4;
    for (int i = 0; i < enemy.punchFrameCount; i++)
        enemy.punchTextures[i] = gangsterPunchTextures[i];

    enemy.elbowFrameCount = 4;
    for (int i = 0; i < enemy.elbowFrameCount; i++)
        enemy.elbowTextures[i] = gangsterKickTextures[i];

    enemy.deathFrameCount = 3;
    for (int i = 0; i < enemy.deathFrameCount; i++)
        enemy.deathTextures[i] = gangsterDeathTextures[i];
    enemy.deathFrameTime = 0.24f;
    enemy.deathFreezeDuration = 0.0f;
    enemy.deathDuration = 2.70f;

    enemy.attackFrameTime = 0.19f;
    enemy.currentAttackMove = ENEMY_ATTACK_PUNCH;
    enemy.nextAttackMove = ENEMY_ATTACK_PUNCH;

    enemy.punchHitboxWidth = 120.0f;
    enemy.punchHitboxHeight = 80.0f;
    enemy.punchHitboxOffsetX = -50.0f;
    enemy.punchHitboxOffsetY = -55.0f;

    enemy.elbowHitboxWidth = 145.0f;
    enemy.elbowHitboxHeight = 100.0f;
    enemy.elbowHitboxOffsetX = -35.0f;
    enemy.elbowHitboxOffsetY = -45.0f;

    enemy.spriteSize = 580.0f;
    enemy.spriteOffsetX = 13.06f;
    enemy.spriteOffsetY = 148.62f;

    return enemy;
}