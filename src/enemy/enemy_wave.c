#include "enemy_wave.h"
#include "hooligan.h"
#include "punk.h"
#include "gangster.h"

// ============================================================
// 0074 - STAGE 1 ENEMY WAVE MANAGER
// ============================================================

// First test encounter.
#define WAVE_1_TRIGGER_PROGRESS 0.09f

// Keep the spawn fully outside the visible camera.
// Positive value = farther beyond the screen edge.
#define ENEMY_OFFSCREEN_MARGIN 180.0f

// Do not let Jamber walk beyond the fight trigger while the wave is active.
#define FIGHT_LOCK_FORWARD_PADDING 35.0f


static float ClampWaveValue(float value, float minimum, float maximum)
{
    if (value < minimum) return minimum;
    if (value > maximum) return maximum;
    return value;
}


static float GetPlayerCenterX(const Player *player)
{
    return player->rectangle.x + player->rectangle.width * 0.5f;
}


static bool AreWaveEnemiesDefeated(
    const Enemy *enemies,
    int startIndex,
    int count
)
{
    if (enemies == 0 || count <= 0)
    {
        return true;
    }

    for (int i = 0; i < count; i++)
    {
        const Enemy *enemy = &enemies[startIndex + i];

        if (enemy->isAlive)
        {
            return false;
        }
    }

    return true;
}


static void LockPlayerForward(
    Player *player,
    float boundaryX
)
{
    if (player == 0)
    {
        return;
    }

    float playerCenterX = GetPlayerCenterX(player);

    if (playerCenterX <= boundaryX)
    {
        return;
    }

    // Move the player's rectangle back just enough so its CENTER cannot
    // pass the encounter boundary. Vertical movement remains untouched.
    player->rectangle.x =
        boundaryX - player->rectangle.width * 0.5f;
}


static void SpawnWave1(
    EnemyWaveSystem *waves,
    Enemy *enemies,
    int enemyCapacity,
    float stageWorldWidth,
    const Camera2D *camera,
    int screenWidth,
    float walkAreaTop,
    float walkAreaBottom
)
{
    if (
        waves == 0 ||
        enemies == 0 ||
        enemyCapacity < 1 ||
        camera == 0
    )
    {
        return;
    }

    // Visible right edge in world coordinates.
    float cameraRight =
        camera->target.x +
        ((float)screenWidth * 0.5f) / camera->zoom;

    float spawnX =
        cameraRight + ENEMY_OFFSCREEN_MARGIN;

    // Never create the enemy outside the real Stage 1 world.
    spawnX = ClampWaveValue(
        spawnX,
        0.0f,
        stageWorldWidth - 160.0f
    );

    // Middle lane for the first tutorial encounter.
    float spawnStageY =
        (walkAreaTop + walkAreaBottom) * 0.5f;

    enemies[0] =
        InitHooligan(
            spawnX,
            spawnStageY
        );

    // The Hooligan exists off-screen first, then runs into the visible area.
    float entranceTargetX =
        cameraRight - 180.0f;

    if (entranceTargetX < 0.0f)
    {
        entranceTargetX = 0.0f;
    }

    StartEnemyEntrance(
        &enemies[0],
        entranceTargetX,
        spawnStageY,
        230.0f
    );

    waves->activeEnemyStartIndex = 0;
    waves->activeEnemyCount = 1;
    waves->waveActive = true;
    waves->waveFinished = false;
}


EnemyWaveSystem InitEnemyWaveSystem(void)
{
    EnemyWaveSystem waves = {0};

    waves.currentWave = 0;
    waves.waveActive = false;
    waves.waveFinished = false;
    waves.fightBoundaryX = 0.0f;
    waves.activeEnemyCount = 0;
    waves.activeEnemyStartIndex = 0;

    return waves;
}


void UpdateEnemyWaveSystem(
    EnemyWaveSystem *waves,
    Player *player,
    Enemy *enemies,
    int enemyCapacity,
    float stageWorldWidth,
    const Camera2D *camera,
    int screenWidth,
    float walkAreaTop,
    float walkAreaBottom
)
{
    if (
        waves == 0 ||
        player == 0 ||
        enemies == 0 ||
        enemyCapacity <= 0 ||
        stageWorldWidth <= 0.0f ||
        camera == 0
    )
    {
        return;
    }

    float playerCenterX =
        GetPlayerCenterX(player);

    // ========================================================
    // WAVE 1 - 9% - ONE HOOLIGAN
    // ========================================================
    if (
        waves->currentWave == 0 &&
        !waves->waveActive &&
        playerCenterX >= stageWorldWidth * WAVE_1_TRIGGER_PROGRESS
    )
    {
        waves->fightBoundaryX =
            playerCenterX + FIGHT_LOCK_FORWARD_PADDING;

        SpawnWave1(
            waves,
            enemies,
            enemyCapacity,
            stageWorldWidth,
            camera,
            screenWidth,
            walkAreaTop,
            walkAreaBottom
        );

        waves->currentWave = 1;
    }

    // ========================================================
    // ACTIVE FIGHT LOCK
    // ========================================================
    if (waves->waveActive)
    {
        LockPlayerForward(
            player,
            waves->fightBoundaryX
        );

        if (
            AreWaveEnemiesDefeated(
                enemies,
                waves->activeEnemyStartIndex,
                waves->activeEnemyCount
            )
        )
        {
            waves->waveActive = false;
            waves->waveFinished = true;
            waves->activeEnemyCount = 0;
        }
    }
}


int GetEnemyWaveActiveCount(const EnemyWaveSystem *waves)
{
    if (waves == 0)
    {
        return 0;
    }

    return waves->activeEnemyCount;
}


int GetEnemyWaveStartIndex(const EnemyWaveSystem *waves)
{
    if (waves == 0)
    {
        return 0;
    }

    return waves->activeEnemyStartIndex;
}


bool IsEnemyWaveActive(const EnemyWaveSystem *waves)
{
    return waves != 0 && waves->waveActive;
}