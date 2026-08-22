#include "player.h"
#include "player_attack_data.h"

#include <stdlib.h>

// ============================================================
// 0075 - PLAYER ATTACK TEXTURE LOADING
// ============================================================

static bool IsNearWhiteBackgroundPixel(Color color)
{
    return
        color.r >= 225 &&
        color.g >= 225 &&
        color.b >= 225;
}

static void QueueConnectedWhitePixel(
    Color *pixels,
    int pixelIndex,
    int *queue,
    int *queueTail
)
{
    if (
        pixels[pixelIndex].a != 0 &&
        IsNearWhiteBackgroundPixel(pixels[pixelIndex])
    )
    {
        // Alpha zero doubles as the flood-fill visited marker.
        pixels[pixelIndex].a = 0;
        queue[*queueTail] = pixelIndex;
        (*queueTail)++;
    }
}

// hip_charge3 was supplied as a JPG with a white background while every
// other move frame is a transparent PNG. Remove only the near-white area
// connected to the image edges. The white parts inside Jamber's outlined
// face remain intact because the flood fill cannot cross the dark outline.
static Texture2D LoadHipChargeFrame3(void)
{
    Image image =
        LoadImage("assets/sprites/player/moves/hip_charge3.jpg");

    if (
        image.data == NULL ||
        image.width <= 0 ||
        image.height <= 0
    )
    {
        TraceLog(LOG_WARNING, "PLAYER MOVE FAILED TO LOAD: hip_charge3.jpg");
        return (Texture2D){0};
    }

    ImageFormat(
        &image,
        PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
    );

    int pixelCount = image.width * image.height;
    Color *pixels = (Color *)image.data;
    int *queue = (int *)malloc((size_t)pixelCount * sizeof(int));

    if (queue != NULL)
    {
        int queueHead = 0;
        int queueTail = 0;

        for (int x = 0; x < image.width; x++)
        {
            QueueConnectedWhitePixel(
                pixels,
                x,
                queue,
                &queueTail
            );

            QueueConnectedWhitePixel(
                pixels,
                ((image.height - 1) * image.width) + x,
                queue,
                &queueTail
            );
        }

        for (int y = 1; y < image.height - 1; y++)
        {
            QueueConnectedWhitePixel(
                pixels,
                y * image.width,
                queue,
                &queueTail
            );

            QueueConnectedWhitePixel(
                pixels,
                (y * image.width) + image.width - 1,
                queue,
                &queueTail
            );
        }

        while (queueHead < queueTail)
        {
            int pixelIndex = queue[queueHead++];
            int x = pixelIndex % image.width;
            int y = pixelIndex / image.width;

            if (x > 0)
            {
                QueueConnectedWhitePixel(
                    pixels,
                    pixelIndex - 1,
                    queue,
                    &queueTail
                );
            }

            if (x + 1 < image.width)
            {
                QueueConnectedWhitePixel(
                    pixels,
                    pixelIndex + 1,
                    queue,
                    &queueTail
                );
            }

            if (y > 0)
            {
                QueueConnectedWhitePixel(
                    pixels,
                    pixelIndex - image.width,
                    queue,
                    &queueTail
                );
            }

            if (y + 1 < image.height)
            {
                QueueConnectedWhitePixel(
                    pixels,
                    pixelIndex + image.width,
                    queue,
                    &queueTail
                );
            }
        }

        free(queue);
    }

    Texture2D texture = LoadTextureFromImage(image);
    UnloadImage(image);

    return texture;
}

// A few supplied frames contain a detached speck near the far-right edge
// of the otherwise transparent canvas. Clear only that empty-canvas area;
// the real character pixels end well before x = 900 in these frames.
static Texture2D LoadAttackFrameWithoutRightEdgeArtifact(
    const char *path
)
{
    Image image = LoadImage(path);

    if (
        image.data == NULL ||
        image.width <= 0 ||
        image.height <= 0
    )
    {
        return (Texture2D){0};
    }

    ImageFormat(
        &image,
        PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
    );

    Color *pixels = (Color *)image.data;
    int clearStartX = (image.width * 88) / 100;
    int clearEndY = (image.height * 42) / 100;

    for (int y = 0; y < clearEndY; y++)
    {
        for (int x = clearStartX; x < image.width; x++)
        {
            pixels[(y * image.width) + x].a = 0;
        }
    }

    Texture2D texture = LoadTextureFromImage(image);
    UnloadImage(image);

    return texture;
}

static void LoadPlayerAttackFrames(
    Player *player,
    AttackType attack,
    const char *pathPattern
)
{
    const PlayerAttackData *attackData =
        GetPlayerAttackData(attack);

    for (int i = 0; i < attackData->frameCount; i++)
    {
        bool hasRightEdgeArtifact =
            (
                (
                    attack == ATTACK_LEFT_PUNCH ||
                    attack == ATTACK_RIGHT_PUNCH ||
                    attack == ATTACK_PUNCH_CHARGE
                ) &&
                i == 0
            ) ||
            (
                attack == ATTACK_UPPERCUT &&
                i == 2
            );

        if (attack == ATTACK_HIP_CHECK && i == 2)
        {
            player->attackTextures[attack][i] =
                LoadHipChargeFrame3();
        }
        else if (hasRightEdgeArtifact)
        {
            player->attackTextures[attack][i] =
                LoadAttackFrameWithoutRightEdgeArtifact(
                    TextFormat(pathPattern, i + 1)
                );
        }
        else
        {
            player->attackTextures[attack][i] =
                LoadTexture(TextFormat(pathPattern, i + 1));
        }

        if (player->attackTextures[attack][i].id == 0)
        {
            TraceLog(
                LOG_WARNING,
                "PLAYER ATTACK TEXTURE FAILED: attack %d frame %d",
                attack,
                i + 1
            );
        }
    }
}

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
    // 0076 - 12 RUN TEXTURES - run1 to run12
    // Hold Shift + Arrow to use this animation and faster movement.
    // ============================================================
    for (int i = 0; i < RUN_FRAME_COUNT; i++)
    {
        player.runTextures[i] = LoadTexture(
            TextFormat("assets/sprites/player/walk/run%d.png", i + 1)
        );
    }

    player.runFrame = 0;
    player.runTimer = 0.0f;
    player.runFrameTime = 0.075f;

    // ============================================================
    // 0075 - BASIC + ADVANCED ATTACK TEXTURES
    // ============================================================
    LoadPlayerAttackFrames(
        &player,
        ATTACK_LEFT_PUNCH,
        "assets/sprites/player/battle/JLP%d.png"
    );
    LoadPlayerAttackFrames(
        &player,
        ATTACK_RIGHT_PUNCH,
        "assets/sprites/player/battle/JRP%d.png"
    );
    LoadPlayerAttackFrames(
        &player,
        ATTACK_LEFT_KICK,
        "assets/sprites/player/battle/JLK%d.png"
    );
    LoadPlayerAttackFrames(
        &player,
        ATTACK_RIGHT_KICK,
        "assets/sprites/player/battle/JRK%d.png"
    );

    LoadPlayerAttackFrames(
        &player,
        ATTACK_HAMMER_PUNCH,
        "assets/sprites/player/moves/hammer%d.png"
    );
    LoadPlayerAttackFrames(
        &player,
        ATTACK_BACK_BLOW,
        "assets/sprites/player/moves/back_blow%d.png"
    );
    LoadPlayerAttackFrames(
        &player,
        ATTACK_ELBOW_DASH,
        "assets/sprites/player/moves/elbow_slide%d.png"
    );
    LoadPlayerAttackFrames(
        &player,
        ATTACK_DOWNWARD_FIST,
        "assets/sprites/player/moves/high_blow%d.png"
    );
    LoadPlayerAttackFrames(
        &player,
        ATTACK_SLIDE_KICK,
        "assets/sprites/player/moves/sidekick%d.png"
    );
    LoadPlayerAttackFrames(
        &player,
        ATTACK_ROUND_KICK,
        "assets/sprites/player/moves/roundkick%d.png"
    );
    LoadPlayerAttackFrames(
        &player,
        ATTACK_HAMMER_CHARGE,
        "assets/sprites/player/moves/charge%d.png"
    );
    LoadPlayerAttackFrames(
        &player,
        ATTACK_PUNCH_CHARGE,
        "assets/sprites/player/moves/punch_charge%d.png"
    );
    LoadPlayerAttackFrames(
        &player,
        ATTACK_UPPERCUT,
        "assets/sprites/player/moves/uppercut%d.png"
    );
    LoadPlayerAttackFrames(
        &player,
        ATTACK_CHOP,
        "assets/sprites/player/moves/chop%d.png"
    );
    LoadPlayerAttackFrames(
        &player,
        ATTACK_DROP_KICK,
        "assets/sprites/player/moves/drop_kick%d.png"
    );
    LoadPlayerAttackFrames(
        &player,
        ATTACK_ELBOW_RISE,
        "assets/sprites/player/moves/elbow_rise%d.png"
    );
    LoadPlayerAttackFrames(
        &player,
        ATTACK_HIP_CHECK,
        "assets/sprites/player/moves/hip_charge%d.png"
    );
    LoadPlayerAttackFrames(
        &player,
        ATTACK_HEADBUTT,
        "assets/sprites/player/moves/head_butt%d.png"
    );

    player.crouchTexture =
        LoadTexture("assets/sprites/player/moves/crouch.png");

    player.attackFrame = 0;
    player.attackTimer = 0.0f;
    player.attackFrameTime = 0.10f;
    player.isAttacking = false;

    // ============================================================
    // PLAYER STATE
    // ============================================================
    player.isWalking = false;
    player.isRunning = false;
    player.runSpeedMultiplier = 1.60f;
    player.currentAttack = ATTACK_NONE;

    // 0026 - No buffered attack when the player is created.
    player.bufferedAttack = ATTACK_NONE;

    // 0027 - Basic combo chain starts empty.
    player.comboStep = 0;
    player.comboTimer = 0.0f;
    player.comboFinisherActive = false;
    player.bufferedComboFinisher = false;

    // 0028 - No directional command attack at startup.
    player.commandAttackActive = false;
    player.commandAttack = ATTACK_NONE;

    // 0029 - Recovery/cancel state starts inactive.
    player.isRecovering = false;
    player.recoveryTimer = 0.0f;
    player.cancelWindowOpen = false;

    // 0075 - Advanced input / charge / slide state.
    player.attackButtonMask = 0;
    player.attackButtonChordTimer = 0.0f;
    player.aHoldPending = false;
    player.aHoldTimer = 0.0f;
    player.punchChargeHolding = false;
    player.punchChargeTimer = 0.0f;
    player.punchChargeLevel = 0;
    player.attackSlideRemaining = 0.0f;
    player.attackSlideSpeed = 0.0f;
    player.attackSlideDirection = 0;
    player.isCrouching = false;
    player.showHitboxes = false;

    // ============================================================
    // 0030 - PLAYER HP / HURT STATE
    // ============================================================
    player.maxHp = 1000.0f;
    player.hp = player.maxHp;
    player.isAlive = true;

    // 0073 - Restore 1.5 HP after every full second below max HP.
    player.hpRegenTimer = 0.0f;
    player.hpRegenInterval = 0.60f;
    player.hpRegenAmount = 3.0f;

    player.isHit = false;
    player.hitReactionTimer = 0.0f;
    player.knockbackSpeed = 0.0f;
    player.knockbackDirection = 0;

    // 0069 - These play only after an enemy attack successfully damages Jamber.
    player.enemyHitSound =
        LoadSound("assets/music/punch.wav");
    player.enemyHitSoundAlternate =
        LoadSound("assets/music/punch1.wav");
    player.useAlternateEnemyHitSound = false;

    player.facingRight = true;
    player.turnDirectionTravel = 0.0f;
    // Horizontal backward distance before the sprite turns.
    // Increase this value if you want the player to stay facing
    // the old direction for a longer backward movement.
    player.turnDirectionDistance = 50.0f;
    player.pendingFacingDirection = 0;

    // ============================================================
    // 0052 - FORWARD / BACK DASH
    // ============================================================
    player.dashTapTimer = 0.0f;
    player.dashTapWindow = 0.30f;
    player.lastDashTapDirection = 0;
    player.dashTapFacingRight = player.facingRight;
    player.isDashing = false;
    player.dashDirection = 0;
    player.dashLockedFacingRight = player.facingRight;
    player.dashTimer = 0.0f;

    // Approximate slide distance = dashSpeed * dashDuration.
    // 900 * 0.10 = about 90 pixels.
    player.dashDuration = 0.12f;
    player.dashSpeed = 900.0f;
    player.dashCommandTimer = 0.0f;
    player.dashCommandWindow = 0.15f;
    player.dashCommandDirection = 0;
    player.dashCommandFacingRight = player.facingRight;

    // ============================================================
    // PLAYER POSITION / SIZE
    // ============================================================
    player.rectangle = (Rectangle){
        150.0f,
        500.0f,
        113.0f,
        180.0f
    };

    player.dashCommandStartX = player.rectangle.x;

    player.speed = 300.0f;

    return player;
}

void PlayPlayerAttackHitSound(Player *player)
{
    if (player->enemyHitSound.frameCount > 0)
    {
        PlaySound(player->enemyHitSound);
    }
}

void UnloadPlayer(Player *player)
{
    if (player->enemyHitSound.frameCount > 0)
    {
        UnloadSound(player->enemyHitSound);
    }

    if (player->enemyHitSoundAlternate.frameCount > 0)
    {
        UnloadSound(player->enemyHitSoundAlternate);
    }

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

    for (int i = 0; i < RUN_FRAME_COUNT; i++)
    {
        if (player->runTextures[i].id != 0)
        {
            UnloadTexture(player->runTextures[i]);
        }
    }

    for (
        int attack = ATTACK_LEFT_PUNCH;
        attack < PLAYER_ATTACK_TYPE_COUNT;
        attack++
    )
    {
        const PlayerAttackData *attackData =
            GetPlayerAttackData((AttackType)attack);

        for (int frame = 0; frame < attackData->frameCount; frame++)
        {
            if (player->attackTextures[attack][frame].id != 0)
            {
                UnloadTexture(player->attackTextures[attack][frame]);
            }
        }
    }

    if (player->crouchTexture.id != 0)
    {
        UnloadTexture(player->crouchTexture);
    }

    player->texture = (Texture2D){0};
    player->enemyHitSound = (Sound){0};
    player->enemyHitSoundAlternate = (Sound){0};
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

    Rectangle hurtbox;

    if (player->isCrouching && !player->isAttacking)
    {
        // The crouch art occupies a shorter, wider body area.
        hurtbox = (Rectangle)
        {
            spriteLeft + (scaledWidth * 0.28f),
            spriteTop  + (scaledHeight * 0.45f),
            scaledWidth  * 0.38f,
            scaledHeight * 0.31f
        };
    }
    else
    {
        // Normal body hurtbox inside the visible sprite.
        hurtbox = (Rectangle)
        {
            spriteLeft + (scaledWidth * 0.37f),
            spriteTop  + (scaledHeight * 0.26f),
            scaledWidth  * 0.26f,
            scaledHeight * 0.50f
        };
    }

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

    player->hp -= (float)damage;

    // The sound is triggered here, not when the enemy merely starts attacking.
    // DamagePlayer() is called only after the enemy hitbox connects with Jamber.
    Sound hitSound =
        player->useAlternateEnemyHitSound
        ? player->enemyHitSoundAlternate
        : player->enemyHitSound;

    if (hitSound.frameCount > 0)
    {
        PlaySound(hitSound);
    }

    player->useAlternateEnemyHitSound =
        !player->useAlternateEnemyHitSound;

    if (player->hp < 0)
    {
        player->hp = 0.0f;
    }

    player->isHit = true;
    player->hitReactionTimer = hitReactionTime;
    player->aHoldPending = false;
    player->aHoldTimer = 0.0f;
    player->isCrouching = false;
    player->isDashing = false;
    player->dashDirection = 0;
    player->dashTimer = 0.0f;
    // Match the enemy's light hit movement: show impact without sending
    // Jamber too far away from the fight.
    player->knockbackSpeed = knockbackSpeed * 0.15f;
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