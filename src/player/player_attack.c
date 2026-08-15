#include "player_attack.h"
#include "player_attack_data.h"

// ============================================================
// 0075 - JAMBER ADVANCED COMMAND / COMBO SYSTEM
// ============================================================

#define COMBO_WINDOW 0.85f
#define ATTACK_RECOVERY_TIME 0.01f
#define ATTACK_CHORD_WINDOW 0.10f
#define A_HOLD_THRESHOLD 0.18f
#define PUNCH_CHARGE_LEVEL_ONE_TIME 1.50f
#define PUNCH_CHARGE_LEVEL_TWO_TIME 3.00f

#define ATTACK_BUTTON_A 0x01u
#define ATTACK_BUTTON_W 0x02u
#define ATTACK_BUTTON_S 0x04u
#define ATTACK_BUTTON_D 0x08u

typedef struct AttackRequest
{
    AttackType attack;
    bool comboFinisher;
    bool directionalCommand;
} AttackRequest;

static AttackRequest NoAttackRequest(void)
{
    return (AttackRequest)
    {
        ATTACK_NONE,
        false,
        false
    };
}

static AttackRequest MakeAttackRequest(
    AttackType attack,
    bool comboFinisher,
    bool directionalCommand
)
{
    return (AttackRequest)
    {
        attack,
        comboFinisher,
        directionalCommand
    };
}

static unsigned int GetPressedAttackButtons(void)
{
    unsigned int buttons = 0;

    if (IsKeyPressed(KEY_A)) buttons |= ATTACK_BUTTON_A;
    if (IsKeyPressed(KEY_W)) buttons |= ATTACK_BUTTON_W;
    if (IsKeyPressed(KEY_S)) buttons |= ATTACK_BUTTON_S;
    if (IsKeyPressed(KEY_D)) buttons |= ATTACK_BUTTON_D;

    return buttons;
}

static bool IsControlHeld(void)
{
    return
        IsKeyDown(KEY_LEFT_CONTROL) ||
        IsKeyDown(KEY_RIGHT_CONTROL);
}

static bool IsBackwardHeld(const Player *player)
{
    if (player->facingRight)
    {
        return IsKeyDown(KEY_LEFT);
    }

    return IsKeyDown(KEY_RIGHT);
}

static bool IsRecentDashCommandForward(const Player *player)
{
    if (
        player->dashCommandTimer <= 0.0f ||
        player->dashCommandDirection == 0
    )
    {
        return false;
    }

    int forwardDirection =
        player->dashCommandFacingRight
        ? 1
        : -1;

    return player->dashCommandDirection == forwardDirection;
}

static bool IsRecentDashCommandBackward(const Player *player)
{
    if (
        player->dashCommandTimer <= 0.0f ||
        player->dashCommandDirection == 0
    )
    {
        return false;
    }

    int forwardDirection =
        player->dashCommandFacingRight
        ? 1
        : -1;

    return player->dashCommandDirection == -forwardDirection;
}

static int CountAttackButtons(unsigned int buttons)
{
    int count = 0;

    if ((buttons & ATTACK_BUTTON_A) != 0) count++;
    if ((buttons & ATTACK_BUTTON_W) != 0) count++;
    if ((buttons & ATTACK_BUTTON_S) != 0) count++;
    if ((buttons & ATTACK_BUTTON_D) != 0) count++;

    return count;
}

static float GetScaledPlayerSpriteWidth(const Player *player)
{
    float depth =
        (player->rectangle.y - 345.0f) /
        (700.0f - 270.0f);

    if (depth < 0.0f) depth = 0.0f;
    if (depth > 1.0f) depth = 1.0f;

    float scale =
        2.90f +
        (depth * 1.80f);

    return
        player->rectangle.width *
        scale *
        1.30f;
}

static float GetAttackSlideDuration(const PlayerAttackData *attackData)
{
    if (
        attackData->slideStartFrame < 0 ||
        attackData->slideEndFrame < attackData->slideStartFrame
    )
    {
        return 0.0f;
    }

    float duration = 0.0f;

    for (
        int frame = attackData->slideStartFrame;
        frame <= attackData->slideEndFrame &&
            frame < attackData->frameCount;
        frame++
    )
    {
        duration += attackData->frameTimes[frame];
    }

    return duration;
}

static void PrepareAttackSlide(
    Player *player,
    const PlayerAttackData *attackData,
    float distanceScale
)
{
    player->attackSlideRemaining = 0.0f;
    player->attackSlideSpeed = 0.0f;
    player->attackSlideDirection = 0;

    if (
        distanceScale <= 0.0f ||
        attackData->slideFacingDirection == 0
    )
    {
        return;
    }

    float slideDuration =
        GetAttackSlideDuration(attackData);

    if (slideDuration <= 0.0f)
    {
        return;
    }

    player->attackSlideRemaining =
        GetScaledPlayerSpriteWidth(player) *
        distanceScale;

    player->attackSlideSpeed =
        player->attackSlideRemaining /
        slideDuration;

    int facingDirection =
        player->facingRight
        ? 1
        : -1;

    player->attackSlideDirection =
        facingDirection *
        attackData->slideFacingDirection;
}

static void ResetCombo(Player *player)
{
    player->comboStep = 0;
    player->comboTimer = 0.0f;
    player->comboFinisherActive = false;
    player->bufferedComboFinisher = false;
}

static void RegisterStartedAttack(
    Player *player,
    AttackType attack,
    bool comboFinisher
)
{
    if (comboFinisher)
    {
        player->comboStep = 3;
        player->comboTimer = COMBO_WINDOW;
        return;
    }

    if (attack == ATTACK_LEFT_PUNCH)
    {
        player->comboStep = 1;
        player->comboTimer = COMBO_WINDOW;
        return;
    }

    if (
        attack == ATTACK_RIGHT_PUNCH &&
        player->comboStep == 1 &&
        player->comboTimer > 0.0f
    )
    {
        player->comboStep = 2;
        player->comboTimer = COMBO_WINDOW;
        return;
    }

    // Standalone moves and invalid branches close the old combo route.
    ResetCombo(player);
}

static void StartPlayerAttack(
    Player *player,
    AttackRequest request
)
{
    if (request.attack == ATTACK_NONE)
    {
        return;
    }

    const PlayerAttackData *attackData =
        GetPlayerAttackData(request.attack);

    player->currentAttack = request.attack;
    player->isAttacking = true;
    player->isRecovering = false;
    player->recoveryTimer = 0.0f;
    player->cancelWindowOpen = true;

    player->attackFrame = 0;
    player->attackTimer = 0.0f;
    player->comboFinisherActive = request.comboFinisher;
    player->bufferedComboFinisher = false;

    player->commandAttackActive = request.directionalCommand;
    player->commandAttack =
        request.directionalCommand
        ? request.attack
        : ATTACK_NONE;

    player->aHoldPending = false;
    player->aHoldTimer = 0.0f;
    player->isCrouching = false;

    // Every attack cancels the ordinary dash. A recognized F,F/B,B
    // command therefore uses only its own measured move slide.
    player->isDashing = false;
    player->dashTimer = 0.0f;
    player->dashDirection = 0;

    player->punchChargeHolding =
        request.attack == ATTACK_PUNCH_CHARGE &&
        IsKeyDown(KEY_A);

    if (request.attack == ATTACK_PUNCH_CHARGE)
    {
        player->punchChargeTimer = 0.0f;
        player->punchChargeLevel = 0;

        // The Punch Charge slide is prepared only when A is released.
        player->attackSlideRemaining = 0.0f;
        player->attackSlideSpeed = 0.0f;
        player->attackSlideDirection = 0;
    }
    else
    {
        player->punchChargeTimer = 0.0f;
        player->punchChargeLevel = 0;

        PrepareAttackSlide(
            player,
            attackData,
            attackData->slideDistanceScale
        );
    }

    player->battleIdleActive = true;
    player->battleIdleTimer = player->battleIdleDuration;

    RegisterStartedAttack(
        player,
        request.attack,
        request.comboFinisher
    );
}

static void QueueOrStartAttack(
    Player *player,
    AttackRequest request,
    unsigned int commandButtons
)
{
    if (request.attack == ATTACK_NONE)
    {
        return;
    }

    bool multiButtonCommand =
        CountAttackButtons(commandButtons) >= 2;

    // A second chord key may arrive one or two rendered frames later.
    // Upgrade a just-started single move before its first image finishes.
    if (
        player->isAttacking &&
        multiButtonCommand &&
        player->attackButtonChordTimer > 0.0f &&
        player->attackFrame == 0 &&
        player->currentAttack != request.attack
    )
    {
        StartPlayerAttack(player, request);
        return;
    }

    if (!player->isAttacking && !player->isRecovering)
    {
        StartPlayerAttack(player, request);
        return;
    }

    if (
        player->bufferedAttack == ATTACK_NONE &&
        (
            player->cancelWindowOpen ||
            player->isRecovering
        )
    )
    {
        player->bufferedAttack = request.attack;
        player->bufferedComboFinisher = request.comboFinisher;
    }
}

static AttackRequest ResolveAttackRequest(
    const Player *player,
    unsigned int buttons
)
{
    if (buttons == 0)
    {
        return NoAttackRequest();
    }

    bool buttonA = (buttons & ATTACK_BUTTON_A) != 0;
    bool buttonW = (buttons & ATTACK_BUTTON_W) != 0;
    bool buttonS = (buttons & ATTACK_BUTTON_S) != 0;
    bool buttonD = (buttons & ATTACK_BUTTON_D) != 0;

    // Ctrl commands have priority over crouch and all neutral attacks.
    if (IsControlHeld())
    {
        if (buttonA)
        {
            return MakeAttackRequest(
                ATTACK_UPPERCUT,
                false,
                true
            );
        }

        if (buttonW)
        {
            return MakeAttackRequest(
                ATTACK_ELBOW_RISE,
                false,
                true
            );
        }

        return NoAttackRequest();
    }

    bool forwardCommand =
        IsRecentDashCommandForward(player);

    bool backwardCommand =
        IsRecentDashCommandBackward(player);

    // Multi-button double-forward commands must win over their
    // single-button versions.
    if (forwardCommand)
    {
        if (buttonA && buttonW)
        {
            return MakeAttackRequest(
                ATTACK_HAMMER_CHARGE,
                false,
                true
            );
        }

        if (buttonS && buttonD)
        {
            return MakeAttackRequest(
                ATTACK_DROP_KICK,
                false,
                true
            );
        }

        if (buttonA)
        {
            return MakeAttackRequest(
                ATTACK_UPPERCUT,
                false,
                true
            );
        }

        if (buttonW)
        {
            return MakeAttackRequest(
                ATTACK_CHOP,
                false,
                true
            );
        }

        if (buttonS)
        {
            return MakeAttackRequest(
                ATTACK_SLIDE_KICK,
                false,
                true
            );
        }

        if (buttonD)
        {
            return MakeAttackRequest(
                ATTACK_ROUND_KICK,
                false,
                true
            );
        }
    }

    if (backwardCommand && buttonW)
    {
        return MakeAttackRequest(
            ATTACK_ELBOW_DASH,
            false,
            true
        );
    }

    // Hold-back moves come after B,B+W so Elbow Dash is not mistaken
    // for the ordinary Downward Fist.
    if (IsBackwardHeld(player))
    {
        if (buttonA)
        {
            return MakeAttackRequest(
                ATTACK_BACK_BLOW,
                false,
                true
            );
        }

        if (buttonW)
        {
            return MakeAttackRequest(
                ATTACK_DOWNWARD_FIST,
                false,
                true
            );
        }
    }

    // A, W opens three explicit finisher branches.
    if (
        player->comboStep == 2 &&
        player->comboTimer > 0.0f
    )
    {
        if (buttonA && buttonW)
        {
            return MakeAttackRequest(
                ATTACK_HAMMER_PUNCH,
                true,
                false
            );
        }

        if (buttonA)
        {
            return MakeAttackRequest(
                ATTACK_DOWNWARD_FIST,
                true,
                false
            );
        }

        if (buttonS)
        {
            return MakeAttackRequest(
                ATTACK_ROUND_KICK,
                true,
                false
            );
        }
    }

    // Neutral simultaneous-button moves.
    if (buttonA && buttonW)
    {
        return MakeAttackRequest(
            ATTACK_HAMMER_PUNCH,
            false,
            false
        );
    }

    if (buttonA && buttonD)
    {
        return MakeAttackRequest(
            ATTACK_HIP_CHECK,
            false,
            false
        );
    }

    if (buttonW && buttonS)
    {
        return MakeAttackRequest(
            ATTACK_HEADBUTT,
            false,
            false
        );
    }

    if (buttonA)
    {
        return MakeAttackRequest(
            ATTACK_LEFT_PUNCH,
            false,
            false
        );
    }

    if (buttonW)
    {
        return MakeAttackRequest(
            ATTACK_RIGHT_PUNCH,
            false,
            false
        );
    }

    if (buttonS)
    {
        return MakeAttackRequest(
            ATTACK_LEFT_KICK,
            false,
            false
        );
    }

    if (buttonD)
    {
        return MakeAttackRequest(
            ATTACK_RIGHT_KICK,
            false,
            false
        );
    }

    return NoAttackRequest();
}

static bool ShouldWaitForAHold(
    const Player *player,
    unsigned int buttons
)
{
    return
        buttons == ATTACK_BUTTON_A &&
        !IsControlHeld() &&
        !IsRecentDashCommandForward(player) &&
        !IsRecentDashCommandBackward(player) &&
        !IsBackwardHeld(player);
}

static void ConsumeDirectionalCommand(
    Player *player,
    const AttackRequest *request,
    unsigned int buttons
)
{
    if (!request->directionalCommand)
    {
        return;
    }

    bool usesDoubleTapMotion =
        IsRecentDashCommandForward(player) ||
        IsRecentDashCommandBackward(player);

    if (usesDoubleTapMotion)
    {
        // Remove any part of the ordinary dash that already happened
        // during the short button window. The special move then travels
        // exactly its own configured slide distance.
        player->rectangle.x = player->dashCommandStartX;
    }

    player->isDashing = false;
    player->dashTimer = 0.0f;
    player->dashDirection = 0;

    // Preserve only a tiny chord-upgrade period. For example, F,F+A
    // may become F,F+A+W if W arrives within 0.10 sec.
    if (CountAttackButtons(buttons) < 2)
    {
        player->dashCommandTimer = ATTACK_CHORD_WINDOW;
    }
    else
    {
        player->dashCommandTimer = 0.0f;
        player->dashCommandDirection = 0;
    }
}

static void HandleAttackButtonPresses(
    Player *player,
    unsigned int pressedButtons
)
{
    if (pressedButtons == 0)
    {
        return;
    }

    if (player->attackButtonChordTimer > 0.0f)
    {
        player->attackButtonMask |= pressedButtons;
    }
    else
    {
        player->attackButtonMask = pressedButtons;
    }

    player->attackButtonChordTimer = ATTACK_CHORD_WINDOW;

    unsigned int commandButtons =
        player->attackButtonMask;

    if (ShouldWaitForAHold(player, commandButtons))
    {
        player->aHoldPending = true;
        player->aHoldTimer = 0.0f;
        return;
    }

    player->aHoldPending = false;
    player->aHoldTimer = 0.0f;

    AttackRequest request =
        ResolveAttackRequest(player, commandButtons);

    ConsumeDirectionalCommand(
        player,
        &request,
        commandButtons
    );

    QueueOrStartAttack(
        player,
        request,
        commandButtons
    );
}

static void UpdateAHoldInput(
    Player *player,
    float deltaTime
)
{
    if (!player->aHoldPending)
    {
        return;
    }

    if (IsKeyReleased(KEY_A))
    {
        player->aHoldPending = false;
        player->aHoldTimer = 0.0f;
        player->attackButtonMask = 0;
        player->attackButtonChordTimer = 0.0f;

        AttackRequest tapRequest =
            ResolveAttackRequest(
                player,
                ATTACK_BUTTON_A
            );

        QueueOrStartAttack(
            player,
            tapRequest,
            ATTACK_BUTTON_A
        );

        return;
    }

    if (!IsKeyDown(KEY_A))
    {
        player->aHoldPending = false;
        player->aHoldTimer = 0.0f;
        return;
    }

    player->aHoldTimer += deltaTime;

    if (player->aHoldTimer >= A_HOLD_THRESHOLD)
    {
        player->aHoldPending = false;
        player->aHoldTimer = 0.0f;

        AttackRequest chargeRequest =
            MakeAttackRequest(
                ATTACK_PUNCH_CHARGE,
                false,
                false
            );

        QueueOrStartAttack(
            player,
            chargeRequest,
            ATTACK_BUTTON_A
        );
    }
}

static void BeginPunchChargeRelease(Player *player)
{
    const PlayerAttackData *attackData =
        GetPlayerAttackData(ATTACK_PUNCH_CHARGE);

    player->punchChargeHolding = false;
    player->attackFrame = 2;
    player->attackTimer = 0.0f;

    float slideScale = attackData->slideDistanceScale;

    if (player->punchChargeLevel == 1)
    {
        slideScale = 0.17f;
    }
    else if (player->punchChargeLevel >= 2)
    {
        slideScale = 0.30f;
    }

    PrepareAttackSlide(
        player,
        attackData,
        slideScale
    );
}

static bool UpdatePunchChargeHold(
    Player *player,
    float deltaTime
)
{
    if (
        player->currentAttack != ATTACK_PUNCH_CHARGE ||
        player->attackFrame != 1 ||
        !player->punchChargeHolding
    )
    {
        return false;
    }

    if (!IsKeyDown(KEY_A))
    {
        BeginPunchChargeRelease(player);
        return false;
    }

    player->punchChargeTimer += deltaTime;

    if (player->punchChargeTimer >= PUNCH_CHARGE_LEVEL_TWO_TIME)
    {
        player->punchChargeTimer = PUNCH_CHARGE_LEVEL_TWO_TIME;
        player->punchChargeLevel = 2;
    }
    else if (player->punchChargeTimer >= PUNCH_CHARGE_LEVEL_ONE_TIME)
    {
        player->punchChargeLevel = 1;
    }

    // Keep visible Frame 2 frozen for as long as A stays held.
    player->attackTimer = 0.0f;
    return true;
}

static void UpdateAttackSlide(
    Player *player,
    float deltaTime
)
{
    if (
        !player->isAttacking ||
        player->attackSlideRemaining <= 0.0f ||
        player->attackSlideSpeed <= 0.0f ||
        player->attackSlideDirection == 0
    )
    {
        return;
    }

    const PlayerAttackData *attackData =
        GetPlayerAttackData(player->currentAttack);

    if (
        player->attackFrame < attackData->slideStartFrame ||
        player->attackFrame > attackData->slideEndFrame
    )
    {
        return;
    }

    float slideAmount =
        player->attackSlideSpeed *
        deltaTime;

    if (slideAmount > player->attackSlideRemaining)
    {
        slideAmount = player->attackSlideRemaining;
    }

    player->rectangle.x +=
        player->attackSlideDirection *
        slideAmount;

    player->attackSlideRemaining -= slideAmount;
}

static void StartRecovery(Player *player)
{
    player->isRecovering = true;
    player->recoveryTimer = ATTACK_RECOVERY_TIME;
    player->cancelWindowOpen = false;
}

static void FinishCurrentAttack(Player *player)
{
    bool finishedCombo =
        player->comboFinisherActive;

    player->attackFrame = 0;
    player->attackTimer = 0.0f;
    player->isAttacking = false;
    player->currentAttack = ATTACK_NONE;
    player->comboFinisherActive = false;

    player->commandAttackActive = false;
    player->commandAttack = ATTACK_NONE;

    player->punchChargeHolding = false;
    player->attackSlideRemaining = 0.0f;
    player->attackSlideSpeed = 0.0f;
    player->attackSlideDirection = 0;

    if (finishedCombo)
    {
        ResetCombo(player);
    }

    StartRecovery(player);
}

static void UpdateAttackAnimation(
    Player *player,
    float deltaTime
)
{
    if (!player->isAttacking)
    {
        return;
    }

    if (UpdatePunchChargeHold(player, deltaTime))
    {
        return;
    }

    UpdateAttackSlide(player, deltaTime);

    const PlayerAttackData *attackData =
        GetPlayerAttackData(player->currentAttack);

    player->attackTimer += deltaTime;

    float frameTime =
        attackData->frameTimes[player->attackFrame];

    if (frameTime <= 0.0f)
    {
        frameTime = 0.10f;
    }

    if (player->attackTimer < frameTime)
    {
        return;
    }

    player->attackTimer -= frameTime;

    if (
        player->currentAttack == ATTACK_PUNCH_CHARGE &&
        player->attackFrame == 0
    )
    {
        if (IsKeyDown(KEY_A))
        {
            player->attackFrame = 1;
            player->punchChargeHolding = true;
        }
        else
        {
            BeginPunchChargeRelease(player);
        }

        return;
    }

    player->attackFrame++;

    if (player->attackFrame >= attackData->frameCount)
    {
        FinishCurrentAttack(player);
    }
}

void UpdatePlayerAttack(Player *player, float deltaTime)
{
    if (IsKeyPressed(KEY_F1))
    {
        player->showHitboxes = !player->showHitboxes;
    }

    // Input history timers.
    if (player->attackButtonChordTimer > 0.0f)
    {
        player->attackButtonChordTimer -= deltaTime;

        if (player->attackButtonChordTimer <= 0.0f)
        {
            player->attackButtonChordTimer = 0.0f;
            player->attackButtonMask = 0;
        }
    }

    if (player->comboTimer > 0.0f)
    {
        player->comboTimer -= deltaTime;

        if (
            player->comboTimer <= 0.0f &&
            !player->comboFinisherActive &&
            !player->bufferedComboFinisher
        )
        {
            ResetCombo(player);
        }
    }

    // Finish the one-frame recovery gap, then start a buffered move.
    if (player->isRecovering)
    {
        player->recoveryTimer -= deltaTime;

        if (player->recoveryTimer <= 0.0f)
        {
            player->isRecovering = false;
            player->recoveryTimer = 0.0f;

            if (player->bufferedAttack != ATTACK_NONE)
            {
                AttackRequest bufferedRequest =
                    MakeAttackRequest(
                        player->bufferedAttack,
                        player->bufferedComboFinisher,
                        false
                    );

                player->bufferedAttack = ATTACK_NONE;
                player->bufferedComboFinisher = false;

                StartPlayerAttack(
                    player,
                    bufferedRequest
                );
            }
        }
    }

    unsigned int pressedButtons =
        GetPressedAttackButtons();

    HandleAttackButtonPresses(
        player,
        pressedButtons
    );

    UpdateAHoldInput(
        player,
        deltaTime
    );

    player->isCrouching =
        IsControlHeld() &&
        !player->isAttacking &&
        !player->isRecovering &&
        !player->isDashing;

    UpdateAttackAnimation(
        player,
        deltaTime
    );
}

// ============================================================
// FRAME-MEASURED ATTACK HITBOX SYSTEM
// ============================================================

bool IsPlayerAttackHitboxActive(const Player *player)
{
    if (
        !player->isAttacking ||
        player->currentAttack == ATTACK_NONE
    )
    {
        return false;
    }

    const PlayerAttackData *attackData =
        GetPlayerAttackData(player->currentAttack);

    if (
        player->attackFrame < 0 ||
        player->attackFrame >= attackData->frameCount
    )
    {
        return false;
    }

    Rectangle frameHitbox =
        attackData->frameHitboxes[player->attackFrame];

    return
        frameHitbox.width > 0.0f &&
        frameHitbox.height > 0.0f;
}

Rectangle GetPlayerAttackHitbox(const Player *player)
{
    if (!IsPlayerAttackHitboxActive(player))
    {
        return (Rectangle){0.0f, 0.0f, 0.0f, 0.0f};
    }

    const PlayerAttackData *attackData =
        GetPlayerAttackData(player->currentAttack);

    Rectangle localHitbox =
        attackData->frameHitboxes[player->attackFrame];

    float depth =
        (player->rectangle.y - 345.0f) /
        (700.0f - 270.0f);

    if (depth < 0.0f) depth = 0.0f;
    if (depth > 1.0f) depth = 1.0f;

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

    float centerX =
        player->rectangle.x +
        (player->rectangle.width * 0.5f);

    float spriteLeft =
        centerX -
        (scaledWidth * 0.5f);

    float spriteTop =
        player->rectangle.y +
        player->rectangle.height -
        scaledHeight;

    float normalizedX = localHitbox.x;

    if (!player->facingRight)
    {
        normalizedX =
            1.0f -
            localHitbox.x -
            localHitbox.width;
    }

    return (Rectangle)
    {
        spriteLeft + (normalizedX * scaledWidth),
        spriteTop + (localHitbox.y * scaledHeight),
        localHitbox.width * scaledWidth,
        localHitbox.height * scaledHeight
    };
}

int GetPlayerAttackDamage(const Player *player)
{
    if (player->currentAttack == ATTACK_NONE)
    {
        return 0;
    }

    const PlayerAttackData *attackData =
        GetPlayerAttackData(player->currentAttack);

    if (player->currentAttack == ATTACK_PUNCH_CHARGE)
    {
        if (player->punchChargeLevel >= 2)
        {
            return 48;
        }

        if (player->punchChargeLevel == 1)
        {
            return 32;
        }
    }

    return attackData->damage;
}

float GetPlayerAttackKnockbackSpeed(const Player *player)
{
    if (player->currentAttack == ATTACK_NONE)
    {
        return 0.0f;
    }

    float knockback =
        GetPlayerAttackData(player->currentAttack)->knockbackSpeed;

    if (player->currentAttack == ATTACK_PUNCH_CHARGE)
    {
        if (player->punchChargeLevel >= 2)
        {
            knockback *= 1.80f;
        }
        else if (player->punchChargeLevel == 1)
        {
            knockback *= 1.35f;
        }
    }

    return knockback;
}

float GetPlayerAttackHitReactionTime(const Player *player)
{
    if (player->currentAttack == ATTACK_NONE)
    {
        return 0.0f;
    }

    float hitReactionTime =
        GetPlayerAttackData(player->currentAttack)->hitReactionTime;

    if (
        player->currentAttack == ATTACK_PUNCH_CHARGE &&
        player->punchChargeLevel >= 2
    )
    {
        hitReactionTime += 0.08f;
    }

    return hitReactionTime;
}