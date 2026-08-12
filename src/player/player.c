#include "player.h"

Player InitPlayer(const char *texturePath)
{
    Player player = {0};

    // ============================================================
    // ORIGINAL PLAYER TEXTURE
    // ============================================================
    player.texture = LoadTexture(texturePath);

    // ============================================================
    // 0037 - NORMAL IDLE BREATH + BATTLE IDLE TEXTURES
    // ============================================================
    for (int i = 0; i < IDLE_BREATH_FRAME_COUNT; i++)
    {
        player.idleBreathTextures[i] = LoadTexture(
            TextFormat("assets/sprites/player/idle_breath/jamber_breath%d.png", i + 1)
        );
    }

    for (int i = 0; i < IDLE_BATTLE_FRAME_COUNT; i++)
    {
        player.idleBattleTextures[i] = LoadTexture(
            TextFormat("assets/sprites/player/idle_battle/Jamber_idle_%d.png", i + 1)
        );
    }

    player.idleFrame = 0;
    player.idleDirection = 1;
    player.idleTimer = 0.0f;
    player.idleFrameTime = 0.17f;

    // Normal state uses idle_breath.
    // Pressing A or W activates idle_battle for 15 seconds.
    player.battleIdleActive = false;
    player.battleIdleTimer = 0.0f;
    player.battleIdleDuration = 15.0f;

    // ============================================================
    // 11 WALK TEXTURES - JWALK1 to JWALK11
    // ============================================================
    for (int i = 0; i < WALK_FRAME_COUNT; i++)
    {
        player.walkTextures[i] = LoadTexture(
            TextFormat("assets/sprites/player/walk/JWALK%d.png", i + 1)
        );
    }

    player.walkFrame = 0;
    player.walkTimer = 0.0f;
    player.walkFrameTime = 0.189f;

    // ============================================================
    // ATTACK TEXTURES
    // ============================================================

    // LEFT PUNCH - A
    player.leftPunchTextures[0] =
        LoadTexture("assets/sprites/player/battle/JLP1.png");
    player.leftPunchTextures[1] =
        LoadTexture("assets/sprites/player/battle/JLP2.png");
    player.leftPunchTextures[2] =
        LoadTexture("assets/sprites/player/battle/JLP3.png");
    player.leftPunchTextures[3] =
        LoadTexture("assets/sprites/player/battle/JLP4.png");

    // RIGHT PUNCH - W
    player.rightPunchTextures[0] =
        LoadTexture("assets/sprites/player/battle/JRP1.png");
    player.rightPunchTextures[1] =
        LoadTexture("assets/sprites/player/battle/JRP2.png");
    player.rightPunchTextures[2] =
        LoadTexture("assets/sprites/player/battle/JRP3.png");
    player.rightPunchTextures[3] =
        LoadTexture("assets/sprites/player/battle/JRP4.png");

    // LEFT KICK - S
    player.leftKickTextures[0] =
        LoadTexture("assets/sprites/player/battle/JLK1.png");
    player.leftKickTextures[1] =
        LoadTexture("assets/sprites/player/battle/JLK2.png");
    player.leftKickTextures[2] =
        LoadTexture("assets/sprites/player/battle/JLK3.png");
    player.leftKickTextures[3] =
        LoadTexture("assets/sprites/player/battle/JLK4.png");
    player.leftKickTextures[4] =
        LoadTexture("assets/sprites/player/battle/JLK5.png");

    // RIGHT KICK - D
    player.rightKickTextures[0] =
        LoadTexture("assets/sprites/player/battle/JRK1.png");
    player.rightKickTextures[1] =
        LoadTexture("assets/sprites/player/battle/JRK2.png");
    player.rightKickTextures[2] =
        LoadTexture("assets/sprites/player/battle/JRK3.png");
    player.rightKickTextures[3] =
        LoadTexture("assets/sprites/player/battle/JRK4.png");
    player.rightKickTextures[4] =
        LoadTexture("assets/sprites/player/battle/JRK5.png");

    player.attackFrame = 0;
    player.attackTimer = 0.0f;
    player.attackFrameTime = 0.10f;
    player.isAttacking = false;

    // ============================================================
    // PLAYER STATE
    // ============================================================
    player.isWalking = false;
    player.currentAttack = ATTACK_NONE;

    // 0026 - No buffered attack when the player is created.
    player.bufferedAttack = ATTACK_NONE;

    // 0027 - Basic combo chain starts empty.
    player.comboStep = 0;
    player.comboTimer = 0.0f;
    player.comboFinisherActive = false;

    // 0028 - No directional command attack at startup.
    player.commandAttackActive = false;
    player.commandAttack = ATTACK_NONE;

    // 0029 - Recovery/cancel state starts inactive.
    player.isRecovering = false;
    player.recoveryTimer = 0.0f;
    player.cancelWindowOpen = false;

    // ============================================================
    // 0030 - PLAYER HP / HURT STATE
    // ============================================================
    player.maxHp = 900;
    player.hp = player.maxHp;
    player.isAlive = true;

    player.isHit = false;
    player.hitReactionTimer = 0.0f;
    player.knockbackSpeed = 0.0f;
    player.knockbackDirection = 0;

    player.facingRight = true;

    // ============================================================
    // PLAYER POSITION / SIZE
    // ============================================================
    player.rectangle = (Rectangle){
        150.0f,
        500.0f,
        113.0f,
        180.0f
    };

    player.speed = 300.0f;

    return player;
}

void UnloadPlayer(Player *player)
{
    UnloadTexture(player->texture);

    for (int i = 0; i < IDLE_BREATH_FRAME_COUNT; i++)
    {
        UnloadTexture(player->idleBreathTextures[i]);
    }

    for (int i = 0; i < IDLE_BATTLE_FRAME_COUNT; i++)
    {
        UnloadTexture(player->idleBattleTextures[i]);
    }

    for (int i = 0; i < WALK_FRAME_COUNT; i++)
    {
        UnloadTexture(player->walkTextures[i]);
    }

    for (int i = 0; i < LEFT_PUNCH_FRAME_COUNT; i++)
    {
        UnloadTexture(player->leftPunchTextures[i]);
    }

    for (int i = 0; i < ATTACK_FRAME_COUNT; i++)
    {
        UnloadTexture(player->leftKickTextures[i]);
        UnloadTexture(player->rightKickTextures[i]);
    }

    for (int i = 0; i < RIGHT_PUNCH_FRAME_COUNT; i++)
    {
        UnloadTexture(player->rightPunchTextures[i]);
    }

    player->texture = (Texture2D){0};
}

// ============================================================
// 0030 - PLAYER HURTBOX
// ============================================================

Rectangle GetPlayerHurtbox(const Player *player)
{
    // ============================================================
    // 0030 FIX - HURTBOX FOLLOWS THE ACTUAL SCALED PLAYER SPRITE
    // ============================================================

    float depth =
        (player->rectangle.y - 345.0f) /
        (700.0f - 270.0f);

    if (depth < 0.0f)
        depth = 0.0f;

    if (depth > 1.0f)
        depth = 1.0f;

    float scale =
        2.90f +
        (depth * 1.80f);

    float scaledWidth =
        player->rectangle.width *
        scale *
        1.30f;

    float scaledHeight =
        player->rectangle.height *
        scale;

    // Same bottom-center anchor used by DrawPlayer().
    float spriteLeft =
        player->rectangle.x +
        (player->rectangle.width / 2.0f) -
        (scaledWidth / 2.0f);

    float spriteTop =
        player->rectangle.y +
        player->rectangle.height -
        scaledHeight;

    // Body hurtbox inside the visible sprite.
    Rectangle hurtbox =
    {
        spriteLeft + (scaledWidth * 0.37f),   // X
        spriteTop  + (scaledHeight * 0.26f),  // Y
        scaledWidth  * 0.26f,                 // WIDTH
        scaledHeight * 0.50f                  // HEIGHT
    };

    return hurtbox;
}

// ============================================================
// 0030 FIX 3 - PLAYER FOOT / GROUND MARKER
// ============================================================
//
// This small box represents the player's real ground position.
// It is used for depth/lane checks instead of the full body.

Rectangle GetPlayerFootMarker(const Player *player)
{
    Rectangle hurtbox =
        GetPlayerHurtbox(player);

    Rectangle feet =
    {
        hurtbox.x + (hurtbox.width * -0.80f),
        hurtbox.y + hurtbox.height - 34.0f,
        hurtbox.width * 2.50f,
        44.0f
    };

    return feet;
}



// ============================================================
// 0030 - DAMAGE PLAYER
// ============================================================

void DamagePlayer(
    Player *player,
    int damage,
    int knockbackDirection,
    float knockbackSpeed,
    float hitReactionTime
)
{
    // 0030 FIX:
    // hitPlayerThisAttack already prevents repeated damage from the SAME
    // enemy attack, so a later enemy attack may damage the player again.
    if (!player->isAlive)
    {
        return;
    }

    player->hp -= damage;

    if (player->hp < 0)
    {
        player->hp = 0;
    }

    player->isHit = true;
    player->hitReactionTimer = hitReactionTime;
    player->knockbackSpeed = knockbackSpeed;
    player->knockbackDirection = knockbackDirection;

    if (player->hp <= 0)
    {
        player->isAlive = false;
        player->isAttacking = false;
        player->currentAttack = ATTACK_NONE;
        player->bufferedAttack = ATTACK_NONE;
    }
}


// ============================================================
// 0030 - PLAYER HIT REACTION / KNOCKBACK UPDATE
// ============================================================

void UpdatePlayerHitReaction(
    Player *player,
    float deltaTime,
    float screenWidth
)
{
    if (!player->isHit)
    {
        return;
    }

    player->rectangle.x +=
        player->knockbackDirection *
        player->knockbackSpeed *
        deltaTime;

    if (player->rectangle.x < 0.0f)
    {
        player->rectangle.x = 0.0f;
    }

    if (
        player->rectangle.x +
        player->rectangle.width >
        screenWidth
    )
    {
        player->rectangle.x =
            screenWidth -
            player->rectangle.width;
    }

    player->hitReactionTimer -= deltaTime;

    if (player->hitReactionTimer <= 0.0f)
    {
        player->hitReactionTimer = 0.0f;
        player->knockbackSpeed = 0.0f;
        player->knockbackDirection = 0;
        player->isHit = false;
    }
}