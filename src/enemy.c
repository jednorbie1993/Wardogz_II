#include "enemy.h"

Enemy InitEnemy(float x, float y)
{
    Enemy enemy = {0};

    enemy.hurtbox = (Rectangle)
    {
        x,
        y,
        90.0f,
        180.0f
    };

    enemy.maxHp = 100;
    enemy.hp = enemy.maxHp;
    enemy.isAlive = true;
    enemy.hitByCurrentAttack = false;

    return enemy;
}


void UpdateEnemyHit(Enemy *enemy, const Player *player)
{
    if (!enemy->isAlive)
    {
        return;
    }

    // Reset once the player's attack is completely finished.
    if (!player->isAttacking)
    {
        enemy->hitByCurrentAttack = false;
    }

    if (
        IsPlayerAttackHitboxActive(player) &&
        !enemy->hitByCurrentAttack
    )
    {
        Rectangle attackHitbox =
            GetPlayerAttackHitbox(player);

        if (CheckCollisionRecs(attackHitbox, enemy->hurtbox))
        {
            enemy->hp -= 10;
            enemy->hitByCurrentAttack = true;

            if (enemy->hp <= 0)
            {
                enemy->hp = 0;
                enemy->isAlive = false;
            }
        }
    }
}


void DrawEnemy(const Enemy *enemy)
{
    // Enemy body/dummy.
    Color bodyColor =
        enemy->isAlive ? DARKGRAY : GRAY;

    DrawRectangleRec(
        enemy->hurtbox,
        Fade(bodyColor, 0.75f)
    );

    // BLUE BOX = enemy hurtbox debug.
    DrawRectangleLinesEx(
        enemy->hurtbox,
        4.0f,
        BLUE
    );

    // HP bar.
    float hpPercent =
        (float)enemy->hp /
        (float)enemy->maxHp;

    Rectangle hpBack =
    {
        enemy->hurtbox.x,
        enemy->hurtbox.y - 24.0f,
        enemy->hurtbox.width,
        12.0f
    };

    Rectangle hpFill =
    {
        hpBack.x,
        hpBack.y,
        hpBack.width * hpPercent,
        hpBack.height
    };

    DrawRectangleRec(hpBack, BLACK);
    DrawRectangleRec(hpFill, RED);
    DrawRectangleLinesEx(hpBack, 2.0f, WHITE);

    DrawText(
        TextFormat("HP: %d", enemy->hp),
        (int)enemy->hurtbox.x,
        (int)enemy->hurtbox.y - 50,
        20,
        WHITE
    );
}