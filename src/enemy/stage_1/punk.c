#include "punk.h"


// ============================================================
// 0040 - SHARED PUNK TEXTURE CACHE
// ============================================================

static bool punkTexturesLoaded = false;
static Texture2D punkIdleTextures[3];
static Texture2D punkWalkTextures[6];
static Texture2D punkPunchTextures[4];
static Texture2D punkElbowTextures[4];

void LoadPunkSharedTextures(void)
{
    if (punkTexturesLoaded)
    {
        return;
    }

    punkIdleTextures[0] = LoadTexture("assets/sprites/enemy/stage_1/punk/punk_idle_1.png");
    punkIdleTextures[1] = LoadTexture("assets/sprites/enemy/stage_1/punk/punk_idle_2.png");
    punkIdleTextures[2] = LoadTexture("assets/sprites/enemy/stage_1/punk/punk_idle_3.png");

    punkWalkTextures[0] = LoadTexture("assets/sprites/enemy/stage_1/punk/punk_walk_1.png");
    punkWalkTextures[1] = LoadTexture("assets/sprites/enemy/stage_1/punk/punk_walk_2.png");
    punkWalkTextures[2] = LoadTexture("assets/sprites/enemy/stage_1/punk/punk_walk_3.png");
    punkWalkTextures[3] = LoadTexture("assets/sprites/enemy/stage_1/punk/punk_walk_4.png");
    punkWalkTextures[4] = LoadTexture("assets/sprites/enemy/stage_1/punk/punk_walk_5.png");
    punkWalkTextures[5] = LoadTexture("assets/sprites/enemy/stage_1/punk/punk_walk_6.png");

    punkPunchTextures[0] = LoadTexture("assets/sprites/enemy/stage_1/punk/punkp1.png");
    punkPunchTextures[1] = LoadTexture("assets/sprites/enemy/stage_1/punk/punkp2.png");
    punkPunchTextures[2] = LoadTexture("assets/sprites/enemy/stage_1/punk/punkp3.png");
    punkPunchTextures[3] = LoadTexture("assets/sprites/enemy/stage_1/punk/punkp4.png");

    punkElbowTextures[0] = LoadTexture("assets/sprites/enemy/stage_1/punk/PUNKE1.png");
    punkElbowTextures[1] = LoadTexture("assets/sprites/enemy/stage_1/punk/PUNKE2.png");
    punkElbowTextures[2] = LoadTexture("assets/sprites/enemy/stage_1/punk/PUNKE3.png");
    punkElbowTextures[3] = LoadTexture("assets/sprites/enemy/stage_1/punk/PUNKE4.png");

    punkTexturesLoaded = true;
}

void UnloadPunkSharedTextures(void)
{
    if (!punkTexturesLoaded)
    {
        return;
    }

    for (int i = 0; i < 3; i++) UnloadTexture(punkIdleTextures[i]);
    for (int i = 0; i < 6; i++) UnloadTexture(punkWalkTextures[i]);
    for (int i = 0; i < 4; i++) UnloadTexture(punkPunchTextures[i]);
    for (int i = 0; i < 4; i++) UnloadTexture(punkElbowTextures[i]);

    punkTexturesLoaded = false;
}


Enemy InitPunk(float x, float y)
{
    // Start with the shared/common enemy defaults.
    Enemy enemy = InitEnemyBase();

    // ============================================================
    // PUNK - HURTBOX SETTINGS
    // ============================================================

    // Final measured Punk hurtbox from 0020.
    enemy.hurtbox = (Rectangle)
    {
        x,
        y - 200.0f,
        148.0f,
        276.0f
    };

    // ============================================================
    // PUNK - STATS
    // ============================================================

    enemy.maxHp = 100;
    enemy.hp = enemy.maxHp;

    // ============================================================
    // 0031 - PUNK FACING + CHASE AI
    // ============================================================
    enemy.facingRight = false;
    enemy.chaseSpeed = 165.0f;
    enemy.chaseStopDistance = 155.0f;
    enemy.chaseDepthTolerance = 8.00f;

    // 0040 - MULTI-PUNK SPACING / ANTI-OVERLAP
    // Adjust these if multiple Punks look too close or too spread out.
    enemy.separationRadiusX = 135.0f;
    enemy.separationDepthTolerance = 75.0f;
    enemy.separationPushSpeed = 300.0f;

    // ============================================================
    // 0037 - PUNK PUNCH / ELBOW DISTANCE SETTINGS
    // ============================================================

    // PUNCH
    // Maximum distance where Punch may START.
    enemy.punchAttackRange = 230.0f;

    // Distance where Punk stops walking before Punch.
    enemy.punchStopDistance = 190.0f;

    // ELBOW
    // Maximum distance where Elbow may START.
    enemy.elbowAttackRange = 200.0f;

    // Distance where Punk stops walking before Elbow.
    enemy.elbowStopDistance = 120.0f;

    // 0038 - Frame 3 forward lunge / slide.
    // Adjust this value to control how far Punk slides.
    enemy.elbowLungeDistance = 70.0f;
    enemy.elbowLungeRemaining = 0.0f;

    // ============================================================
    // 0032 - PUNK STAGE POSITION ANCHOR
    // ============================================================
    // InitPunk(x, y) stores the Punk hurtbox at y - 200.
    // This converts hurtbox.y back to the same logical stage Y used
    // by walkAreaTop / walkAreaBottom in main.c.
    enemy.stageAnchorOffsetY = 220.0f;

    // ============================================================
    // PUNK - IDLE TEXTURES (SHARED)
    // ============================================================

    LoadPunkSharedTextures();
    enemy.ownsTextures = false;

    enemy.idleFrameCount = 3;
    for (int i = 0; i < enemy.idleFrameCount; i++)
    {
        enemy.idleTextures[i] = punkIdleTextures[i];
    }

    enemy.idleFrame = 0;
    enemy.idleDirection = 1;
    enemy.idleTimer = 0.0f;
    enemy.idleFrameTime = 0.19f;

    // ============================================================
    // 0036 - PUNK WALK TEXTURES (SHARED)
    // ============================================================

    enemy.walkFrameCount = 6;
    for (int i = 0; i < enemy.walkFrameCount; i++)
    {
        enemy.walkTextures[i] = punkWalkTextures[i];
    }

    enemy.walkFrame = 0;
    enemy.walkTimer = 0.0f;
    enemy.walkFrameTime = 0.11f;

    // ============================================================
    // 0037 - PUNK ATTACK TEXTURES (SHARED)
    // ============================================================

    enemy.punchFrameCount = 4;
    for (int i = 0; i < enemy.punchFrameCount; i++)
    {
        enemy.punchTextures[i] = punkPunchTextures[i];
    }

    enemy.elbowFrameCount = 4;
    for (int i = 0; i < enemy.elbowFrameCount; i++)
    {
        enemy.elbowTextures[i] = punkElbowTextures[i];
    }

    enemy.attackFrame = 0;
    enemy.attackFrameTimer = 0.0f;
    enemy.attackFrameTime = 0.19f;
    // First completed attack is Punch, then Elbow, then Punch...
    enemy.currentAttackMove = ENEMY_ATTACK_PUNCH;
    enemy.nextAttackMove = ENEMY_ATTACK_PUNCH;

    // ============================================================
    // 0037 - PUNK PUNCH HITBOX SETTINGS
    // ============================================================
    //
    // Width / Height = yellow box size.
    // OffsetX:
    //   positive = farther forward
    //   negative = closer to Punk
    //
    // OffsetY:
    //   positive = lower
    //   negative = higher

    enemy.punchHitboxWidth = 120.0f;
    enemy.punchHitboxHeight = 80.0f;
    enemy.punchHitboxOffsetX = -50.0f;
    enemy.punchHitboxOffsetY = -55.0f; //height

    // ============================================================
    // 0037 - PUNK ELBOW HITBOX SETTINGS
    // ============================================================

    enemy.elbowHitboxWidth = 120.0f;
    enemy.elbowHitboxHeight = 80.0f;
    enemy.elbowHitboxOffsetX = -45.90f; // Frame 3 reach: ~1-2 inches farther forward
    enemy.elbowHitboxOffsetY = -65.0f; //height

    // ============================================================
    // PUNK - SPRITE SIZE / ALIGNMENT
    // ============================================================

    enemy.spriteSize = 580.0f;

    // These offsets reproduce the final alignment from 0020,
    // but now the shared DrawEnemy() uses normal center/bottom anchors.
    //
    // Old X alignment:
    //   hurtbox.x + hurtbox.width / 1.70
    // New generic center:
    //   hurtbox.x + hurtbox.width / 2
    //
    // Difference is approximately +13.06 pixels.
    enemy.spriteOffsetX = 13.06f;

    // Old Y alignment:
    //   hurtbox.y + hurtbox.height / 0.65
    // New generic bottom:
    //   hurtbox.y + hurtbox.height
    //
    // Difference is approximately +148.62 pixels.
    enemy.spriteOffsetY = 148.62f;

    return enemy;
}