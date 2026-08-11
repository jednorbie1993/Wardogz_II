#include "player_attack.h"
#include "player_attack_data.h"

// ============================================================
// 0028 - DIRECTION + ATTACK COMMANDS
// ============================================================
//
// Existing basic attacks:
// A = Left Punch
// W = Right Punch
// S = Left Kick
// D = Right Kick
//
// Existing combo:
// A -> W -> D
//
// New directional command detection:
// FORWARD  + A = Forward Left Punch
// FORWARD  + D = Forward Right Kick
// BACKWARD + A = Backward Left Punch
// BACKWARD + D = Backward Right Kick
//
// 0028 detects the command only.
// It still reuses the normal A/D animations.

#define COMBO_WINDOW 0.70f

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

static void StartPlayerAttack(Player *player, AttackType attack)
{
    if (attack == ATTACK_NONE)
        return;

    DetectDirectionalCommand(player, attack);

    player->currentAttack = attack;
    player->isAttacking = true;
    player->attackFrame = 0;
    player->attackTimer = 0.0f;

    RegisterComboInput(player, attack);
}

void UpdatePlayerAttack(Player *player, float deltaTime)
{
    AttackType pressedAttack = GetPressedAttack();

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
    // 0026 - ATTACK INPUT BUFFER
    // ============================================================
    if (!player->isAttacking)
    {
        StartPlayerAttack(player, pressedAttack);
    }
    else if (
        pressedAttack != ATTACK_NONE &&
        player->bufferedAttack == ATTACK_NONE
    )
    {
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

                if (player->bufferedAttack != ATTACK_NONE)
                {
                    AttackType nextAttack = player->bufferedAttack;

                    player->bufferedAttack = ATTACK_NONE;
                    StartPlayerAttack(player, nextAttack);
                }
                else
                {
                    player->attackFrame = 0;
                    player->attackTimer = 0.0f;
                    player->isAttacking = false;
                    player->currentAttack = ATTACK_NONE;
                }
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