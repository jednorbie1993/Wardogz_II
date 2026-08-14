#include "boss.h"

static bool bossTexturesLoaded = false;
static Texture2D bossIdleTextures[2];
static Texture2D bossBattleIdleTextures[4];
static Texture2D bossRunTextures[6];
static Texture2D bossPunchTextures[4];
static Texture2D bossKickTextures[4];
static Texture2D bossDeathTextures[4];
static Texture2D bossComboTextures[4];
static Texture2D bossDashTextures[4];
static Texture2D bossKneeTextures[4];
static Texture2D bossUppercutTextures[4];
static Texture2D bossHeavyBlowTextures[3];

void LoadBossSharedTextures(void)
{
    if (bossTexturesLoaded) return;

    bossIdleTextures[0] = LoadTexture("assets/sprites/enemy/stage_1/boss/boss_idle1.png");
    bossIdleTextures[1] = LoadTexture("assets/sprites/enemy/stage_1/boss/boss_idle2.png");

    bossBattleIdleTextures[0] = LoadTexture("assets/sprites/enemy/stage_1/boss/boss_battle_idle1.png");
    bossBattleIdleTextures[1] = LoadTexture("assets/sprites/enemy/stage_1/boss/boss_battle_idle2.png");
    bossBattleIdleTextures[2] = LoadTexture("assets/sprites/enemy/stage_1/boss/boss_battle_idle3.png");
    bossBattleIdleTextures[3] = LoadTexture("assets/sprites/enemy/stage_1/boss/boss_battle_idle4.png");

    bossRunTextures[0] = LoadTexture("assets/sprites/enemy/stage_1/boss/boss_run1.png");
    bossRunTextures[1] = LoadTexture("assets/sprites/enemy/stage_1/boss/boss_run2.png");
    bossRunTextures[2] = LoadTexture("assets/sprites/enemy/stage_1/boss/boss_run3.png");
    bossRunTextures[3] = LoadTexture("assets/sprites/enemy/stage_1/boss/boss_run4.png");
    bossRunTextures[4] = LoadTexture("assets/sprites/enemy/stage_1/boss/boss_run5.png");
    bossRunTextures[5] = LoadTexture("assets/sprites/enemy/stage_1/boss/boss_run6.png");

    bossPunchTextures[0] = LoadTexture("assets/sprites/enemy/stage_1/boss/boss_punch1.png");
    bossPunchTextures[1] = LoadTexture("assets/sprites/enemy/stage_1/boss/boss_punch2.png");
    bossPunchTextures[2] = LoadTexture("assets/sprites/enemy/stage_1/boss/boss_punch3.png");
    bossPunchTextures[3] = LoadTexture("assets/sprites/enemy/stage_1/boss/boss_punch4.png");

    // The shared second-attack slot is named elbow internally.
    // Vargas uses that same slot for his kick animation.
    bossKickTextures[0] = LoadTexture("assets/sprites/enemy/stage_1/boss/boss_kick1.png");
    bossKickTextures[1] = LoadTexture("assets/sprites/enemy/stage_1/boss/boss_kick2.png");
    bossKickTextures[2] = LoadTexture("assets/sprites/enemy/stage_1/boss/boss_kick3.png");
    bossKickTextures[3] = LoadTexture("assets/sprites/enemy/stage_1/boss/boss_kick4.png");

    bossDeathTextures[0] = LoadTexture("assets/sprites/enemy/stage_1/boss/boss_death1.png");
    bossDeathTextures[1] = LoadTexture("assets/sprites/enemy/stage_1/boss/boss_death2.png");
    bossDeathTextures[2] = LoadTexture("assets/sprites/enemy/stage_1/boss/boss_death3.png");
    bossDeathTextures[3] = LoadTexture("assets/sprites/enemy/stage_1/boss/boss_death4.png");

    for (int i = 0; i < 4; i++)
    {
        bossComboTextures[i] = LoadTexture(TextFormat("assets/sprites/enemy/stage_1/boss/boss_combo%d.png", i + 1));
        bossDashTextures[i] = LoadTexture(TextFormat("assets/sprites/enemy/stage_1/boss/boss_dash%d.png", i + 1));
        bossKneeTextures[i] = LoadTexture(TextFormat("assets/sprites/enemy/stage_1/boss/boss_knee%d.png", i + 1));
        bossUppercutTextures[i] = LoadTexture(TextFormat("assets/sprites/enemy/stage_1/boss/boss_uppercut%d.png", i + 1));
    }

    for (int i = 0; i < 3; i++)
        bossHeavyBlowTextures[i] = LoadTexture(TextFormat("assets/sprites/enemy/stage_1/boss/boss_blow%d.png", i + 1));

    bossTexturesLoaded = true;
}

void UnloadBossSharedTextures(void)
{
    if (!bossTexturesLoaded) return;

    for (int i = 0; i < 2; i++) UnloadTexture(bossIdleTextures[i]);
    for (int i = 0; i < 4; i++) UnloadTexture(bossBattleIdleTextures[i]);
    for (int i = 0; i < 6; i++) UnloadTexture(bossRunTextures[i]);
    for (int i = 0; i < 4; i++) UnloadTexture(bossPunchTextures[i]);
    for (int i = 0; i < 4; i++) UnloadTexture(bossKickTextures[i]);
    for (int i = 0; i < 4; i++) UnloadTexture(bossDeathTextures[i]);
    for (int i = 0; i < 4; i++) UnloadTexture(bossComboTextures[i]);
    for (int i = 0; i < 4; i++) UnloadTexture(bossDashTextures[i]);
    for (int i = 0; i < 4; i++) UnloadTexture(bossKneeTextures[i]);
    for (int i = 0; i < 4; i++) UnloadTexture(bossUppercutTextures[i]);
    for (int i = 0; i < 3; i++) UnloadTexture(bossHeavyBlowTextures[i]);

    bossTexturesLoaded = false;
}

Enemy InitBoss(float x, float y)
{
    Enemy enemy = InitEnemyBase();

    enemy.displayName = "VARGAS";
    enemy.hurtbox = (Rectangle){x, y - 200.0f, 160.0f, 290.0f};
    enemy.maxHp = 60;
    enemy.hp = enemy.maxHp;

    enemy.facingRight = false;
    enemy.chaseSpeed = 155.0f;
    enemy.chaseStopDistance = 165.0f;
    enemy.chaseDepthTolerance = 8.0f;

    enemy.separationRadiusX = 150.0f;
    enemy.separationDepthTolerance = 80.0f;
    enemy.separationPushSpeed = 300.0f;

    enemy.retreatDuration = 0.28f;
    enemy.retreatSpeed = 430.0f;
    enemy.retreatPauseDuration = 0.65f;

    enemy.punchAttackRange = 190.0f;
    enemy.punchStopDistance = 189.0f;
    enemy.elbowAttackRange = 231.0f;
    enemy.elbowStopDistance = 230.0f;
    enemy.bossKneeAttackRange = 405.0f;
    enemy.bossKneeStopDistance = 390.0f;
    enemy.bossKneeLungeDistance = 300.0f;
    enemy.elbowLungeDistance = 75.0f;
    enemy.elbowLungeRemaining = 0.0f;

    // Vargas forward-slide distances. Adjust each move separately here.
    enemy.bossComboLungeDistance = 200.0f;
    enemy.bossKneeLungeDistance = 450.0f;
    enemy.bossUppercutLungeDistance = 240.0f;
    enemy.bossLungeRemaining = 0.0f;

    enemy.attackDamage = 14;
    enemy.attackKnockbackSpeed = 230.0f;
    enemy.attackHitReactionTime = 0.17f;
    enemy.stageAnchorOffsetY = 220.0f;

    LoadBossSharedTextures();
    enemy.ownsTextures = false;

    enemy.idleFrameCount = 2;
    for (int i = 0; i < enemy.idleFrameCount; i++)
        enemy.idleTextures[i] = bossIdleTextures[i];
    enemy.idleFrameTime = 0.24f;

    enemy.battleIdleFrameCount = 4;
    for (int i = 0; i < enemy.battleIdleFrameCount; i++)
        enemy.battleIdleTextures[i] = bossBattleIdleTextures[i];
    enemy.battleIdleFrameTime = 0.15f;
    enemy.battleIdleActive = false;

    enemy.walkFrameCount = 6;
    for (int i = 0; i < enemy.walkFrameCount; i++)
        enemy.walkTextures[i] = bossRunTextures[i];
    enemy.walkFrameTime = 0.10f;

    enemy.punchFrameCount = 4;
    for (int i = 0; i < enemy.punchFrameCount; i++)
        enemy.punchTextures[i] = bossPunchTextures[i];

    enemy.elbowFrameCount = 4;
    for (int i = 0; i < enemy.elbowFrameCount; i++)
        enemy.elbowTextures[i] = bossKickTextures[i];

    enemy.bossComboFrameCount = 4;
    enemy.bossKneeFrameCount = 4;
    enemy.bossUppercutFrameCount = 4;
    enemy.bossHeavyBlowFrameCount = 3;
    enemy.retreatFrameCount = 4;

    for (int i = 0; i < 4; i++)
    {
        enemy.bossComboTextures[i] = bossComboTextures[i];
        enemy.bossKneeTextures[i] = bossKneeTextures[i];
        enemy.bossUppercutTextures[i] = bossUppercutTextures[i];
        enemy.retreatTextures[i] = bossDashTextures[i];
    }

    for (int i = 0; i < 3; i++)
        enemy.bossHeavyBlowTextures[i] = bossHeavyBlowTextures[i];

    enemy.deathFrameCount = 4;
    for (int i = 0; i < enemy.deathFrameCount; i++)
        enemy.deathTextures[i] = bossDeathTextures[i];
    enemy.deathFrameTime = 0.24f;
    enemy.deathFreezeDuration = 0.0f;
    enemy.deathDuration = 3.20f;

    enemy.attackFrameTime = 0.16f;
    // Vargas attack order:
    // Punch -> Kick -> Combo -> Knee -> Uppercut -> Heavy Blow -> repeat.
    // enemy_attack.c advances the move only after the full attack finishes.
    enemy.currentAttackMove = ENEMY_ATTACK_PUNCH;
    enemy.nextAttackMove = ENEMY_ATTACK_PUNCH;
    enemy.lockAttackMove = false;
    enemy.retreatFrameTime = 0.07f;

    enemy.punchHitboxWidth = 135.0f;
    enemy.punchHitboxHeight = 90.0f;
    enemy.punchHitboxOffsetX = -45.0f;
    enemy.punchHitboxOffsetY = -50.0f;

    enemy.elbowHitboxWidth = 155.0f;
    enemy.elbowHitboxHeight = 110.0f;
    enemy.elbowHitboxOffsetX = -30.0f;
    enemy.elbowHitboxOffsetY = -42.0f;

    // First visual-test values for Vargas' 1024x1024 frames.
    enemy.spriteSize = 650.0f;
    enemy.spriteOffsetX = 13.06f;
    enemy.spriteOffsetY = 166.0f;

    return enemy;
}