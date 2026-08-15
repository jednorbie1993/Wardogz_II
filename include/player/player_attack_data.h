#ifndef PLAYER_ATTACK_DATA_H
#define PLAYER_ATTACK_DATA_H

#include "player.h"

// ============================================================
// 0023 - PLAYER ATTACK DATA / MOVE DEFINITION SYSTEM
// ============================================================
// One structure now describes the gameplay values of an attack.
// This keeps hitbox/timing/damage/knockback values out of the
// attack update logic and gives future combo systems one source
// of truth for move properties.

typedef struct PlayerAttackData
{
    int frameCount;
    float frameTimes[MAX_PLAYER_ATTACK_FRAMES];

    Rectangle frameHitboxes[MAX_PLAYER_ATTACK_FRAMES];

    int damage;
    float knockbackSpeed;
    float hitReactionTime;

    float slideDistanceScale;
    int slideStartFrame;
    int slideEndFrame;
    int slideFacingDirection;

} PlayerAttackData;
// Returns the move definition for an AttackType.
// ATTACK_NONE returns a harmless zero-value definition.
const PlayerAttackData *GetPlayerAttackData(AttackType attackType);

#endif