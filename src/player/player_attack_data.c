#include "player_attack_data.h"

// ============================================================
// 0023 - CENTRAL PLAYER MOVE DEFINITIONS
// ============================================================
// 0025 - PER-MOVE KNOCKBACK VALUES
// Each basic attack now has its own damage AND knockback value.
// Combat balance can be changed here without rewriting enemy.c
// or the attack/collision logic.

static const PlayerAttackData PLAYER_ATTACK_DATA[] =
{
    // ATTACK_NONE
    {
        0.10f,
        0,
        -1,
        0.0f,
        0.0f,
        0.0f,
        0.0f,
        0,
        0.0f,
        0.0f
    },

    // ATTACK_LEFT_PUNCH - A
    {
        0.10f,   // animationFrameTime
        0,       // activeStartFrame
        2,       // activeEndFrame

        0.25f,   // hitboxWidthScale
        0.10f,   // hitboxHeightScale
        -0.01f,   // hitboxOffsetX
        0.35f,   // hitboxOffsetY

        8,       // damage
        220.0f,  // knockbackSpeed
        0.12f    // hitReactionTime
    },

    // ATTACK_RIGHT_PUNCH - W
    {
        0.10f,
        0,
        2,
        0.24f,
        0.10f,
        0.02f,
        0.36f,
        12,
        280.0f,
        0.12f
    },

    // ATTACK_LEFT_KICK - S
    {
        0.10f,
        0,
        2,
        0.25f, //width size
        0.23f, //height size
        0.04f,  //left and right measurement
        0.34f, // height measurement
        16,
        360.0f,
        0.12f
    },

    // ATTACK_RIGHT_KICK - D
    {
        0.10f,
        0,
        2,
        0.23f,
        0.22f,
        0.06f,
        0.43f,
        20,
        440.0f,
        0.12f
    }
};

const PlayerAttackData *GetPlayerAttackData(AttackType attackType)
{
    if (
        attackType < ATTACK_NONE ||
        attackType > ATTACK_RIGHT_KICK
    )
    {
        return &PLAYER_ATTACK_DATA[ATTACK_NONE];
    }

    return &PLAYER_ATTACK_DATA[attackType];
}