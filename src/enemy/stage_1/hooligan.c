#include "hooligan.h"

static bool hooliganTexturesLoaded = false;
static Texture2D hooliganIdleTextures[3];
static Texture2D hooliganRunTextures[6];
static Texture2D hooliganPunchTextures[4];
static Texture2D hooliganKickTextures[4];
static Texture2D hooliganDeathTextures[2];

void LoadHooliganSharedTextures(void)
{
    if (hooliganTexturesLoaded) return;

    hooliganIdleTextures[0] = LoadTexture("assets/sprites/enemy/stage_1/hooligan/hooligan_idle1.png");
    hooliganIdleTextures[1] = LoadTexture("assets/sprites/enemy/stage_1/hooligan/hooligan_idle2.png");
    hooliganIdleTextures[2] = LoadTexture("assets/sprites/enemy/stage_1/hooligan/hooligan_idle3.png");

    hooliganRunTextures[0] = LoadTexture("assets/sprites/enemy/stage_1/hooligan/hooligan_run1.png");
    hooliganRunTextures[1] = LoadTexture("assets/sprites/enemy/stage_1/hooligan/hooligan_run2.png");
    hooliganRunTextures[2] = LoadTexture("assets/sprites/enemy/stage_1/hooligan/hooligan_run3.png");
    hooliganRunTextures[3] = LoadTexture("assets/sprites/enemy/stage_1/hooligan/hooligan_run4.png");
    hooliganRunTextures[4] = LoadTexture("assets/sprites/enemy/stage_1/hooligan/hooligan_run5.png");
    hooliganRunTextures[5] = LoadTexture("assets/sprites/enemy/stage_1/hooligan/hooligan_run6.png");

    hooliganPunchTextures[0] = LoadTexture("assets/sprites/enemy/stage_1/hooligan/hooligan_punch1.png");
    hooliganPunchTextures[1] = LoadTexture("assets/sprites/enemy/stage_1/hooligan/hooligan_punch2.png");
    hooliganPunchTextures[2] = LoadTexture("assets/sprites/enemy/stage_1/hooligan/hooligan_punch3.png");
    hooliganPunchTextures[3] = LoadTexture("assets/sprites/enemy/stage_1/hooligan/hooligan_punch4.png");

    // The shared enemy system calls its second attack "elbow" internally.
    // For Hooligan, that same slot displays the four kick frames.
    hooliganKickTextures[0] = LoadTexture("assets/sprites/enemy/stage_1/hooligan/hooligan_kick1.png");
    hooliganKickTextures[1] = LoadTexture("assets/sprites/enemy/stage_1/hooligan/hooligan_kick2.png");
    hooliganKickTextures[2] = LoadTexture("assets/sprites/enemy/stage_1/hooligan/hooligan_kick3.png");
    hooliganKickTextures[3] = LoadTexture("assets/sprites/enemy/stage_1/hooligan/hooligan_kick4.png");

    hooliganDeathTextures[0] = LoadTexture("assets/sprites/enemy/stage_1/hooligan/hooligan_death1.png");
    hooliganDeathTextures[1] = LoadTexture("assets/sprites/enemy/stage_1/hooligan/hooligan_death2.png");

    hooliganTexturesLoaded = true;
}

void UnloadHooliganSharedTextures(void)
{
    if (!hooliganTexturesLoaded) return;

    for (int i = 0; i < 3; i++) UnloadTexture(hooliganIdleTextures[i]);
    for (int i = 0; i < 6; i++) UnloadTexture(hooliganRunTextures[i]);
    for (int i = 0; i < 4; i++) UnloadTexture(hooliganPunchTextures[i]);
    for (int i = 0; i < 4; i++) UnloadTexture(hooliganKickTextures[i]);
    for (int i = 0; i < 2; i++) UnloadTexture(hooliganDeathTextures[i]);

    hooliganTexturesLoaded = false;
}

Enemy InitHooligan(float x, float y)
{
    Enemy enemy = InitEnemyBase();

    enemy.displayName = "HOOLIGAN";
    enemy.hurtbox = (Rectangle){x, y - 200.0f, 148.0f, 276.0f};
    enemy.maxHp = 14;
    enemy.hp = enemy.maxHp;

    enemy.facingRight = false;
    enemy.chaseSpeed = 180.0f;
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

    // Hooligan kick uses the shared second-attack (elbow) settings.
    enemy.elbowAttackRange = 220.0f;
    enemy.elbowStopDistance = 180.0f;
    enemy.elbowLungeDistance = 70.0f;
    enemy.elbowLungeRemaining = 0.0f;

    enemy.stageAnchorOffsetY = 220.0f;

    LoadHooliganSharedTextures();
    enemy.ownsTextures = false;

    enemy.idleFrameCount = 3;
    for (int i = 0; i < enemy.idleFrameCount; i++)
        enemy.idleTextures[i] = hooliganIdleTextures[i];
    enemy.idleFrameTime = 0.19f;

    enemy.walkFrameCount = 6;
    for (int i = 0; i < enemy.walkFrameCount; i++)
        enemy.walkTextures[i] = hooliganRunTextures[i];
    enemy.walkFrameTime = 0.10f;

    enemy.punchFrameCount = 4;
    for (int i = 0; i < enemy.punchFrameCount; i++)
        enemy.punchTextures[i] = hooliganPunchTextures[i];

    enemy.elbowFrameCount = 4;
    for (int i = 0; i < enemy.elbowFrameCount; i++)
        enemy.elbowTextures[i] = hooliganKickTextures[i];

    enemy.deathFrameCount = 2;
    for (int i = 0; i < enemy.deathFrameCount; i++)
        enemy.deathTextures[i] = hooliganDeathTextures[i];
    enemy.deathFrameTime = 0.28f;
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

    // Initial alignment follows Punk. These are the two values to tune
    // if Hooligan's cutout has different transparent margins.
    enemy.spriteSize = 580.0f;
    enemy.spriteOffsetX = 13.06f;
    enemy.spriteOffsetY = 148.62f;

    return enemy;
}