#include "player_attack.h"
#include "player_attack_data.h"

// ============================================================
// 0029 - RECOVERY / CANCEL WINDOW SYSTEM
// ============================================================
//
// Purpose:
// 1. Add a short recovery period after an attack finishes.
// 2. Open a cancel window near the end of the attack.
// 3. Allow one buffered attack only during that cancel window.
//
// Existing:
// 0026 - one-slot input buffer
// 0027 - A -> W -> D combo recognition
// 0028 - Direction + Attack command detection
//
// 0029 makes chaining more controlled and fighting-game-like.

#define COMBO_WINDOW 2.00f

// Short post-attack recovery.
// During recovery, a fresh attack cannot start yet.
#define ATTACK_RECOVERY_TIME 0.00f

// Cancel window opens on the final attack animation frame.
#define CANCEL_WINDOW_FRAME 0

static bool IsForwardHeld(const Player *player)
{
    if (player->facingRight)
        return IsKeyDown(KEY_RIGHT);

    return IsKeyDown(KEY_LEFT);
}

static bool IsBackwardHeld(const Player *player)
{
    if (player->facingRight)
        return IsKeyDown(KEY_LEFT);

    return IsKeyDown(KEY_RIGHT);
}

static AttackType GetPressedAttack(void)
{
    if (IsKeyPressed(KEY_A))
        return ATTACK_LEFT_PUNCH;

    if (IsKeyPressed(KEY_W))
        return ATTACK_RIGHT_PUNCH;

    if (IsKeyPressed(KEY_S))
        return ATTACK_LEFT_KICK;

    if (IsKeyPressed(KEY_D))
        return ATTACK_RIGHT_KICK;

    return ATTACK_NONE;
}

static void ResetCombo(Player *player)
{
    player->comboStep = 0;
    player->comboTimer = 0.0f;
    player->comboFinisherActive = false;
}

static void RegisterComboInput(Player *player, AttackType attack)
{
    if (attack == ATTACK_NONE)
        return;

    player->comboTimer = COMBO_WINDOW;

    if (player->comboStep == 0)
    {
        if (attack == ATTACK_LEFT_PUNCH)
            player->comboStep = 1;
        else
            ResetCombo(player);

        return;
    }

    if (player->comboStep == 1)
    {
        if (attack == ATTACK_RIGHT_PUNCH)
        {
            player->comboStep = 2;
        }
        else if (attack == ATTACK_LEFT_PUNCH)
        {
            player->comboStep = 1;
        }
        else
        {
            ResetCombo(player);
        }

        return;
    }

    if (player->comboStep == 2)
    {
        if (attack == ATTACK_RIGHT_KICK)
        {
            player->comboStep = 3;
            player->comboFinisherActive = true;
        }
        else if (attack == ATTACK_LEFT_PUNCH)
        {
            player->comboStep = 1;
        }
        else
        {
            ResetCombo(player);
        }
    }
}

static void DetectDirectionalCommand(
    Player *player,
    AttackType attack
)
{
    player->commandAttackActive = false;
    player->commandAttack = ATTACK_NONE;

    if (attack == ATTACK_NONE)
        return;

    if (IsForwardHeld(player))
    {
        if (
            attack == ATTACK_LEFT_PUNCH ||
            attack == ATTACK_RIGHT_KICK
        )
        {
            player->commandAttackActive = true;
            player->commandAttack = attack;
            return;
        }
    }

    if (IsBackwardHeld(player))
    {
        if (
            attack == ATTACK_LEFT_PUNCH ||
            attack == ATTACK_RIGHT_KICK
        )
        {
            player->commandAttackActive = true;
            player->commandAttack = attack;
        }
    }
}

static void StartRecovery(Player *player)
{
    player->isRecovering = true;
    player->recoveryTimer = ATTACK_RECOVERY_TIME;
    player->cancelWindowOpen = false;
}

static void StartPlayerAttack(Player *player, AttackType attack)
{
    if (attack == ATTACK_NONE)
        return;

    DetectDirectionalCommand(player, attack);

    player->currentAttack = attack;
    player->isAttacking = true;
    player->isRecovering = false;
    player->recoveryTimer = 0.0f;
    player->cancelWindowOpen = false;

    player->attackFrame = 0;
    player->attackTimer = 0.0f;

    RegisterComboInput(player, attack);
}

void UpdatePlayerAttack(Player *player, float deltaTime)
{
    AttackType pressedAttack = GetPressedAttack();

    // 0037 - Any punch switches the post-attack idle to battle mode.
    // Each new punch refreshes the 15-second timer.
    if (pressedAttack == ATTACK_LEFT_PUNCH ||
        pressedAttack == ATTACK_RIGHT_PUNCH)
    {
        player->battleIdleActive = true;
        player->battleIdleTimer = player->battleIdleDuration;
    }

    // ============================================================
    // 0027 - COMBO WINDOW TIMER
    // ============================================================
    if (player->comboTimer > 0.0f)
    {
        player->comboTimer -= deltaTime;

        if (player->comboTimer <= 0.0f)
            ResetCombo(player);
    }

    // ============================================================
    // 0029 - RECOVERY TIMER
    // ============================================================
    if (player->isRecovering)
    {
        player->recoveryTimer -= deltaTime;

        if (player->recoveryTimer <= 0.0f)
        {
            player->isRecovering = false;
            player->recoveryTimer = 0.0f;

            // If a valid next attack was buffered during the
            // cancel window, start it after recovery.
            if (player->bufferedAttack != ATTACK_NONE)
            {
                AttackType nextAttack = player->bufferedAttack;
                player->bufferedAttack = ATTACK_NONE;
                StartPlayerAttack(player, nextAttack);
            }
        }
    }

    // ============================================================
    // ATTACK INPUT
    // ============================================================
    if (!player->isAttacking && !player->isRecovering)
    {
        StartPlayerAttack(player, pressedAttack);
    }
    else if (
        player->isAttacking &&
        player->cancelWindowOpen &&
        pressedAttack != ATTACK_NONE &&
        player->bufferedAttack == ATTACK_NONE
    )
    {
        // 0029:
        // Only accept the next attack during the cancel window.
        player->bufferedAttack = pressedAttack;
    }

    // ============================================================
    // ATTACK ANIMATION
    // ============================================================
    if (player->isAttacking)
    {
        const PlayerAttackData *attackData =
            GetPlayerAttackData(player->currentAttack);

        player->attackTimer += deltaTime;

        if (player->attackTimer >= attackData->frameTime)
        {
            player->attackTimer -= attackData->frameTime;
            player->attackFrame++;

            // Open the cancel window near the end of the attack.
            if (player->attackFrame >= CANCEL_WINDOW_FRAME)
                player->cancelWindowOpen = true;

            if (player->attackFrame >= ATTACK_FRAME_COUNT)
            {
                if (player->comboFinisherActive)
                {
                    player->comboFinisherActive = false;
                    player->comboStep = 0;
                    player->comboTimer = 0.0f;
                }

                player->commandAttackActive = false;
                player->commandAttack = ATTACK_NONE;

                player->attackFrame = 0;
                player->attackTimer = 0.0f;
                player->isAttacking = false;

                StartRecovery(player);
            }
        }
    }
}

// ============================================================
// 0017 + 0023 - ATTACK HITBOX SYSTEM USING MOVE DATA
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

    return
        player->attackFrame >= attackData->activeStartFrame &&
        player->attackFrame <= attackData->activeEndFrame;
}

Rectangle GetPlayerAttackHitbox(const Player *player)
{
    if (!IsPlayerAttackHitboxActive(player))
    {
        return (Rectangle){0.0f, 0.0f, 0.0f, 0.0f};
    }

    const PlayerAttackData *attackData =
        GetPlayerAttackData(player->currentAttack);

    // Same depth scaling as DrawPlayer().
    float depth =
        (player->rectangle.y - 345.0f) /
        (700.0f - 270.0f);

    if (depth < 0.0f)
    {
        depth = 0.0f;
    }

    if (depth > 1.0f)
    {
        depth = 1.0f;
    }

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
        (player->rectangle.width / 2.0f);

    float bottomY =
        player->rectangle.y +
        player->rectangle.height;

    float topY =
        bottomY - scaledHeight;

    Rectangle hitbox =
    {
        0.0f,
        0.0f,
        scaledWidth * attackData->hitboxWidthScale,
        scaledHeight * attackData->hitboxHeightScale
    };

    hitbox.y =
        topY +
        (scaledHeight * attackData->hitboxOffsetYScale);

    if (player->facingRight)
    {
        hitbox.x =
            centerX +
            (scaledWidth * attackData->hitboxOffsetXScale);
    }
    else
    {
        hitbox.x =
            centerX -
            (scaledWidth * attackData->hitboxOffsetXScale) -
            hitbox.width;
    }

    return hitbox;
}

int GetPlayerAttackDamage(const Player *player)
{
    if (player->currentAttack == ATTACK_NONE)
    {
        return 0;
    }

    return GetPlayerAttackData(player->currentAttack)->damage;
}

float GetPlayerAttackKnockbackSpeed(const Player *player)
{
    if (player->currentAttack == ATTACK_NONE)
    {
        return 0.0f;
    }

    return GetPlayerAttackData(player->currentAttack)->knockbackSpeed;
}

float GetPlayerAttackHitReactionTime(const Player *player)
{
    if (player->currentAttack == ATTACK_NONE)
    {
        return 0.0f;
    }

    return GetPlayerAttackData(player->currentAttack)->hitReactionTime;
}