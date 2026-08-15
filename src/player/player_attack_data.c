#include "player_attack.h"

// ============================================================
// 0075 - CENTRAL JAMBER MOVE DEFINITIONS
// ============================================================
//
// Hitboxes below were measured against the real 1024 x 1024 source
// images in assets/sprites/player/battle and player/moves. They are
// stored as normalized source-image coordinates so they follow the
// same depth scale and left/right mirroring as the drawn sprite.

#define HB(x, y, width, height) \
    { \
        (x) / 1024.0f, \
        (y) / 1024.0f, \
        (width) / 1024.0f, \
        (height) / 1024.0f \
    }

static const PlayerAttackData PLAYER_ATTACK_DATA
    [PLAYER_ATTACK_TYPE_COUNT] =
{
    [ATTACK_NONE] =
    {
        .frameCount = 0,
        .damage = 0,
        .knockbackSpeed = 0.0f,
        .hitReactionTime = 0.0f,
        .slideStartFrame = -1,
        .slideEndFrame = -1
    },

    // A tap - exactly 10 game frames total at 60 FPS.
    [ATTACK_LEFT_PUNCH] =
    {
        .frameCount = 4,
        .frameTimes = {0.041667f, 0.041667f, 0.041667f, 0.041667f},
        .frameHitboxes =
        {
            [2] = HB(565.0f, 365.0f, 225.0f, 180.0f)
        },
        .damage = 8,
        .knockbackSpeed = 220.0f,
        .hitReactionTime = 0.12f,
        .slideStartFrame = -1,
        .slideEndFrame = -1
    },

    // W - exactly 18 game frames total at 60 FPS.
    [ATTACK_RIGHT_PUNCH] =
    {
        .frameCount = 4,
        .frameTimes = {0.075f, 0.075f, 0.075f, 0.075f},
        .frameHitboxes =
        {
            [2] = HB(565.0f, 340.0f, 245.0f, 205.0f)
        },
        .damage = 12,
        .knockbackSpeed = 280.0f,
        .hitReactionTime = 0.12f,
        .slideStartFrame = -1,
        .slideEndFrame = -1
    },

    [ATTACK_LEFT_KICK] =
    {
        .frameCount = 5,
        .frameTimes = {0.075f, 0.075f, 0.095f, 0.075f, 0.095f},
        .frameHitboxes =
        {
            [2] = HB(565.0f, 345.0f, 270.0f, 245.0f)
        },
        .damage = 16,
        .knockbackSpeed = 360.0f,
        .hitReactionTime = 0.12f,
        .slideStartFrame = -1,
        .slideEndFrame = -1
    },

    [ATTACK_RIGHT_KICK] =
    {
        .frameCount = 5,
        .frameTimes = {0.08f, 0.08f, 0.105f, 0.08f, 0.105f},
        .frameHitboxes =
        {
            [2] = HB(570.0f, 430.0f, 270.0f, 235.0f)
        },
        .damage = 20,
        .knockbackSpeed = 440.0f,
        .hitReactionTime = 0.12f,
        .slideStartFrame = -1,
        .slideEndFrame = -1
    },

    // A + W - both visible Frames 2 and 3 are active, one hit total.
    [ATTACK_HAMMER_PUNCH] =
    {
        .frameCount = 4,
        .frameTimes = {0.10f, 0.12f, 0.11f, 0.13f},
        .frameHitboxes =
        {
            [1] = HB(345.0f, 150.0f, 310.0f, 315.0f),
            [2] = HB(445.0f, 490.0f, 205.0f, 245.0f)
        },
        .damage = 28,
        .knockbackSpeed = 520.0f,
        .hitReactionTime = 0.16f,
        .slideStartFrame = -1,
        .slideEndFrame = -1
    },

    // Hold Back + A.
    [ATTACK_BACK_BLOW] =
    {
        .frameCount = 5,
        .frameTimes = {0.08f, 0.09f, 0.11f, 0.10f, 0.12f},
        .frameHitboxes =
        {
            [2] = HB(625.0f, 335.0f, 220.0f, 225.0f)
        },
        .damage = 18,
        .knockbackSpeed = 350.0f,
        .hitReactionTime = 0.13f,
        .slideStartFrame = -1,
        .slideEndFrame = -1
    },

    // Back, Back + W. Slide starts on visible Frame 2.
    [ATTACK_ELBOW_DASH] =
    {
        .frameCount = 4,
        .frameTimes = {0.09f, 0.10f, 0.11f, 0.13f},
        .frameHitboxes =
        {
            [2] = HB(610.0f, 350.0f, 275.0f, 245.0f)
        },
        .damage = 22,
        .knockbackSpeed = 430.0f,
        .hitReactionTime = 0.14f,
        .slideDistanceScale = 0.17f,
        .slideStartFrame = 1,
        .slideEndFrame = 2,
        .slideFacingDirection = 1
    },

    // Hold Back + W; also the A, W, A combo finisher.
    [ATTACK_DOWNWARD_FIST] =
    {
        .frameCount = 4,
        .frameTimes = {0.09f, 0.11f, 0.11f, 0.13f},
        .frameHitboxes =
        {
            [2] = HB(565.0f, 340.0f, 255.0f, 260.0f)
        },
        .damage = 24,
        .knockbackSpeed = 470.0f,
        .hitReactionTime = 0.15f,
        .slideStartFrame = -1,
        .slideEndFrame = -1
    },

    // Forward, Forward + S. Visible Frame 2 has the requested freeze.
    [ATTACK_SLIDE_KICK] =
    {
        .frameCount = 4,
        .frameTimes = {0.08f, 0.18f, 0.11f, 0.14f},
        .frameHitboxes =
        {
            [2] = HB(590.0f, 350.0f, 330.0f, 270.0f)
        },
        .damage = 26,
        .knockbackSpeed = 540.0f,
        .hitReactionTime = 0.16f,
        .slideDistanceScale = 0.20f,
        .slideStartFrame = 2,
        .slideEndFrame = 3,
        .slideFacingDirection = 1
    },

    // Forward, Forward + D; also the A, W, S combo finisher.
    [ATTACK_ROUND_KICK] =
    {
        .frameCount = 6,
        .frameTimes = {0.07f, 0.08f, 0.11f, 0.08f, 0.08f, 0.11f},
        .frameHitboxes =
        {
            [2] = HB(590.0f, 285.0f, 295.0f, 300.0f)
        },
        .damage = 30,
        .knockbackSpeed = 590.0f,
        .hitReactionTime = 0.17f,
        .slideStartFrame = -1,
        .slideEndFrame = -1
    },

    // Forward, Forward + A + W. Charge Frames 2-3 are active.
    [ATTACK_HAMMER_CHARGE] =
    {
        .frameCount = 4,
        .frameTimes = {0.10f, 0.11f, 0.11f, 0.14f},
        .frameHitboxes =
        {
            [1] = HB(580.0f, 390.0f, 190.0f, 230.0f),
            [2] = HB(640.0f, 390.0f, 190.0f, 225.0f)
        },
        .damage = 34,
        .knockbackSpeed = 650.0f,
        .hitReactionTime = 0.18f,
        .slideDistanceScale = 0.20f,
        .slideStartFrame = 1,
        .slideEndFrame = 2,
        .slideFacingDirection = 1
    },

    // Hold A. Frame 2 is held by player_attack.c until A is released.
    [ATTACK_PUNCH_CHARGE] =
    {
        .frameCount = 4,
        .frameTimes = {0.10f, 0.10f, 0.12f, 0.15f},
        .frameHitboxes =
        {
            [2] = HB(555.0f, 330.0f, 270.0f, 240.0f)
        },
        .damage = 18,
        .knockbackSpeed = 430.0f,
        .hitReactionTime = 0.15f,
        .slideDistanceScale = 0.10f,
        .slideStartFrame = 2,
        .slideEndFrame = 2,
        .slideFacingDirection = 1
    },

    // Forward, Forward + A or Ctrl + A. Frames 2-3 are active.
    [ATTACK_UPPERCUT] =
    {
        .frameCount = 4,
        .frameTimes = {0.09f, 0.11f, 0.11f, 0.14f},
        .frameHitboxes =
        {
            [1] = HB(555.0f, 245.0f, 165.0f, 230.0f),
            [2] = HB(565.0f, 90.0f, 165.0f, 285.0f)
        },
        .damage = 26,
        .knockbackSpeed = 500.0f,
        .hitReactionTime = 0.16f,
        .slideStartFrame = -1,
        .slideEndFrame = -1
    },

    // Forward, Forward + W.
    [ATTACK_CHOP] =
    {
        .frameCount = 4,
        .frameTimes = {0.09f, 0.10f, 0.11f, 0.13f},
        .frameHitboxes =
        {
            [2] = HB(555.0f, 335.0f, 285.0f, 255.0f)
        },
        .damage = 20,
        .knockbackSpeed = 390.0f,
        .hitReactionTime = 0.14f,
        .slideStartFrame = -1,
        .slideEndFrame = -1
    },

    // Forward, Forward + S + D. The active box covers both feet.
    [ATTACK_DROP_KICK] =
    {
        .frameCount = 4,
        .frameTimes = {0.10f, 0.10f, 0.13f, 0.17f},
        .frameHitboxes =
        {
            [2] = HB(610.0f, 330.0f, 325.0f, 275.0f)
        },
        .damage = 36,
        .knockbackSpeed = 700.0f,
        .hitReactionTime = 0.20f,
        .slideDistanceScale = 0.12f,
        .slideStartFrame = 2,
        .slideEndFrame = 2,
        .slideFacingDirection = 1
    },

    // Ctrl + W. This elbow stays stationary.
    [ATTACK_ELBOW_RISE] =
    {
        .frameCount = 4,
        .frameTimes = {0.10f, 0.11f, 0.11f, 0.14f},
        .frameHitboxes =
        {
            [2] = HB(485.0f, 220.0f, 245.0f, 315.0f)
        },
        .damage = 22,
        .knockbackSpeed = 440.0f,
        .hitReactionTime = 0.15f,
        .slideStartFrame = -1,
        .slideEndFrame = -1
    },

    // A + D. Jamber turns his torso and drives the hip toward the opponent.
    [ATTACK_HIP_CHECK] =
    {
        .frameCount = 6,
        .frameTimes = {0.08f, 0.08f, 0.10f, 0.12f, 0.09f, 0.12f},
        .frameHitboxes =
        {
            [3] = HB(625.0f, 405.0f, 180.0f, 280.0f)
        },
        .damage = 28,
        .knockbackSpeed = 570.0f,
        .hitReactionTime = 0.17f,
        .slideDistanceScale = 0.24f,
        .slideStartFrame = 2,
        .slideEndFrame = 3,
        .slideFacingDirection = 1
    },

    // W + S. Slide begins on visible Frame 2; Frame 3 is active.
    [ATTACK_HEADBUTT] =
    {
        .frameCount = 4,
        .frameTimes = {0.09f, 0.10f, 0.11f, 0.14f},
        .frameHitboxes =
        {
            [2] = HB(635.0f, 360.0f, 205.0f, 245.0f)
        },
        .damage = 24,
        .knockbackSpeed = 480.0f,
        .hitReactionTime = 0.15f,
        .slideDistanceScale = 0.15f,
        .slideStartFrame = 1,
        .slideEndFrame = 2,
        .slideFacingDirection = 1
    }
};

const PlayerAttackData *GetPlayerAttackData(AttackType attackType)
{
    if (
        attackType < ATTACK_NONE ||
        attackType >= PLAYER_ATTACK_TYPE_COUNT
    )
    {
        return &PLAYER_ATTACK_DATA[ATTACK_NONE];
    }

    return &PLAYER_ATTACK_DATA[attackType];
}