#ifndef PLAYER_ATTACK_H
#define PLAYER_ATTACK_H

#include "player.h"

void UpdatePlayerAttack(Player *player, float deltaTime);

bool IsPlayerAttackHitboxActive(const Player *player);
Rectangle GetPlayerAttackHitbox(const Player *player);

#endif