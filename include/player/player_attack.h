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
    // Animation timing. The values are seconds per visible image frame.
    int frameCount;
    float frameTimes[MAX_PLAYER_ATTACK_FRAMES];

    // One normalized hitbox per image frame. Coordinates are measured
    // directly from the full 1024 x 1024 source image. A zero-size box
    // means that frame cannot damage an enemy.
    Rectangle frameHitboxes[MAX_PLAYER_ATTACK_FRAMES];

    // Combat values. Multi-frame hitboxes still deal damage only once
    // to each enemy for the entire attack instance.
    int damage;
    float knockbackSpeed;
    float hitReactionTime;

    // Optional lunge. Distance is a ratio of the fully scaled sprite
    // canvas; direction is 1 = forward, -1 = backward, 0 = none.
    float slideDistanceScale;
    int slideStartFrame;
    int slideEndFrame;
    int slideFacingDirection;

} PlayerAttackData;

// Returns the move definition for an AttackType.
// ATTACK_NONE returns a harmless zero-value definition.
const PlayerAttackData *GetPlayerAttackData(AttackType attackType);

#endif