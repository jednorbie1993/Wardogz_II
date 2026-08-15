#include "enemy_wave.h"
#include "hooligan.h"
#include "punk.h"
#include "gangster.h"

#define WAVE_1_TRIGGER_PROGRESS 0.06f
#define WAVE_1_BARRIER_PROGRESS 0.18f
#define WAVE_2_TRIGGER_PROGRESS 0.26f
#define WAVE_2_BARRIER_PROGRESS 0.38f
#define WAVE_3_TRIGGER_PROGRESS 0.46f
#define WAVE_3_BARRIER_PROGRESS 0.59f
#define WAVE_4_TRIGGER_PROGRESS 0.66f
#define WAVE_4_BARRIER_PROGRESS 0.81f

#define REINFORCEMENT_THRESHOLD 2
#define ENEMY_OFFSCREEN_MARGIN 180.0f

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

static int CountLivingWaveEnemies(const Enemy *enemies, int startIndex, int count)
{
    if (enemies == 0 || count <= 0) return 0;

    int living = 0;
    for (int i = 0; i < count; i++)
    {
        if (enemies[startIndex + i].isAlive)
            living++;
    }
    return living;
}

static void ClearNormalEnemySlots(Enemy *enemies, int enemyCapacity)
{
    if (enemies == 0) return;

    for (int i = 0; i < enemyCapacity; i++)
        enemies[i] = (Enemy){0};
}

static void LockPlayerForward(Player *player, float boundaryX)
{
    if (player == 0) return;

    float playerCenterX = GetPlayerCenterX(player);
    if (playerCenterX > boundaryX)
    {
        player->rectangle.x =
            boundaryX - player->rectangle.width * 0.5f;
    }
}

static float GetLaneTop(float walkAreaTop, float walkAreaBottom)
{
    return walkAreaTop + (walkAreaBottom - walkAreaTop) * 0.18f;
}

static float GetLaneMiddle(float walkAreaTop, float walkAreaBottom)
{
    return walkAreaTop + (walkAreaBottom - walkAreaTop) * 0.50f;
}

static float GetLaneBottom(float walkAreaTop, float walkAreaBottom)
{
    return walkAreaTop + (walkAreaBottom - walkAreaTop) * 0.82f;
}

static float CameraLeftWorld(const Camera2D *camera, int screenWidth)
{
    return camera->target.x -
           ((float)screenWidth * 0.5f) / camera->zoom;
}

static void StartBehindEntrance(
    Enemy *enemy,
    const Camera2D *camera,
    int screenWidth,
    float stageWorldWidth,
    float stageY,
    float speed
)
{
    float cameraLeft = CameraLeftWorld(camera, screenWidth);

    float spawnX = ClampWaveValue(
        cameraLeft - ENEMY_OFFSCREEN_MARGIN,
        20.0f,
        stageWorldWidth - 160.0f
    );

    enemy->hurtbox.x = spawnX;

    float targetX = ClampWaveValue(
        cameraLeft + 130.0f,
        20.0f,
        stageWorldWidth - 160.0f
    );

    StartEnemyEntrance(enemy, targetX, stageY, speed);
}

static void SpawnWave1(
    EnemyWaveSystem *waves,
    Enemy *enemies,
    int enemyCapacity,
    float stageWorldWidth,
    float walkAreaTop,
    float walkAreaBottom
)
{
    if (enemyCapacity < 3) return;

    ClearNormalEnemySlots(enemies, enemyCapacity);

    float topY = GetLaneTop(walkAreaTop, walkAreaBottom);
    float midY = GetLaneMiddle(walkAreaTop, walkAreaBottom);
    float bottomY = GetLaneBottom(walkAreaTop, walkAreaBottom);

    enemies[0] = InitHooligan(stageWorldWidth * 0.145f, midY);
    StartEnemyEntrance(
        &enemies[0],
        stageWorldWidth * 0.095f,
        midY,
        230.0f
    );

    enemies[1] = InitPunk(stageWorldWidth * 0.155f, topY);
    enemies[2] = InitPunk(stageWorldWidth * 0.165f, bottomY);

    waves->activeEnemyStartIndex = 0;
    waves->activeEnemyCount = 3;
    waves->reinforcementSpawned = true;
    waves->waveActive = true;
    waves->waveFinished = false;
    waves->fightBoundaryX = stageWorldWidth * WAVE_1_BARRIER_PROGRESS;
}

static void SpawnWave2(
    EnemyWaveSystem *waves,
    Enemy *enemies,
    int enemyCapacity,
    float stageWorldWidth,
    float walkAreaTop,
    float walkAreaBottom
)
{
    if (enemyCapacity < 4) return;

    ClearNormalEnemySlots(enemies, enemyCapacity);

    float topY = GetLaneTop(walkAreaTop, walkAreaBottom);
    float midY = GetLaneMiddle(walkAreaTop, walkAreaBottom);
    float bottomY = GetLaneBottom(walkAreaTop, walkAreaBottom);

    enemies[0] = InitPunk(stageWorldWidth * 0.335f, topY);
    enemies[1] = InitGangster(stageWorldWidth * 0.350f, midY);
    enemies[2] = InitPunk(stageWorldWidth * 0.365f, bottomY);

    waves->activeEnemyStartIndex = 0;
    waves->activeEnemyCount = 3;
    waves->reinforcementSpawned = false;
    waves->waveActive = true;
    waves->waveFinished = false;
    waves->fightBoundaryX = stageWorldWidth * WAVE_2_BARRIER_PROGRESS;
}

static void SpawnWave3(
    EnemyWaveSystem *waves,
    Enemy *enemies,
    int enemyCapacity,
    float stageWorldWidth,
    float walkAreaTop,
    float walkAreaBottom
)
{
    if (enemyCapacity < 5) return;

    ClearNormalEnemySlots(enemies, enemyCapacity);

    float topY = GetLaneTop(walkAreaTop, walkAreaBottom);
    float midY = GetLaneMiddle(walkAreaTop, walkAreaBottom);
    float bottomY = GetLaneBottom(walkAreaTop, walkAreaBottom);

    enemies[0] = InitHooligan(stageWorldWidth * 0.525f, topY);
    enemies[1] = InitGangster(stageWorldWidth * 0.545f, midY);
    enemies[2] = InitPunk(stageWorldWidth * 0.560f, bottomY);

    waves->activeEnemyStartIndex = 0;
    waves->activeEnemyCount = 3;
    waves->reinforcementSpawned = false;
    waves->waveActive = true;
    waves->waveFinished = false;
    waves->fightBoundaryX = stageWorldWidth * WAVE_3_BARRIER_PROGRESS;
}

static void SpawnWave4(
    EnemyWaveSystem *waves,
    Enemy *enemies,
    int enemyCapacity,
    float stageWorldWidth,
    float walkAreaTop,
    float walkAreaBottom
)
{
    if (enemyCapacity < 5) return;

    ClearNormalEnemySlots(enemies, enemyCapacity);

    float topY = GetLaneTop(walkAreaTop, walkAreaBottom);
    float midY = GetLaneMiddle(walkAreaTop, walkAreaBottom);
    float bottomY = GetLaneBottom(walkAreaTop, walkAreaBottom);

    enemies[0] = InitHooligan(stageWorldWidth * 0.745f, topY);
    enemies[1] = InitGangster(stageWorldWidth * 0.765f, midY);
    enemies[2] = InitHooligan(stageWorldWidth * 0.785f, bottomY);

    waves->activeEnemyStartIndex = 0;
    waves->activeEnemyCount = 3;
    waves->reinforcementSpawned = false;
    waves->waveActive = true;
    waves->waveFinished = false;
    waves->fightBoundaryX = stageWorldWidth * WAVE_4_BARRIER_PROGRESS;
}

static void SpawnReinforcement(
    EnemyWaveSystem *waves,
    Enemy *enemies,
    int enemyCapacity,
    int waveNumber,
    float stageWorldWidth,
    const Camera2D *camera,
    int screenWidth,
    float walkAreaTop,
    float walkAreaBottom
)
{
    if (waves == 0 || enemies == 0 || camera == 0) return;

    float topY = GetLaneTop(walkAreaTop, walkAreaBottom);
    float midY = GetLaneMiddle(walkAreaTop, walkAreaBottom);
    float bottomY = GetLaneBottom(walkAreaTop, walkAreaBottom);

    if (waveNumber == 2 && enemyCapacity >= 4)
    {
        enemies[3] = InitHooligan(0.0f, midY);
        StartBehindEntrance(
            &enemies[3], camera, screenWidth,
            stageWorldWidth, midY, 245.0f
        );
        waves->activeEnemyCount = 4;
    }
    else if (waveNumber == 3 && enemyCapacity >= 5)
    {
        enemies[3] = InitHooligan(0.0f, topY);
        enemies[4] = InitGangster(0.0f, bottomY);

        StartBehindEntrance(
            &enemies[3], camera, screenWidth,
            stageWorldWidth, topY, 250.0f
        );
        StartBehindEntrance(
            &enemies[4], camera, screenWidth,
            stageWorldWidth, bottomY, 235.0f
        );

        waves->activeEnemyCount = 5;
    }
    else if (waveNumber == 4 && enemyCapacity >= 5)
    {
        enemies[3] = InitGangster(0.0f, topY);
        enemies[4] = InitPunk(0.0f, bottomY);

        StartBehindEntrance(
            &enemies[3], camera, screenWidth,
            stageWorldWidth, topY, 240.0f
        );
        StartBehindEntrance(
            &enemies[4], camera, screenWidth,
            stageWorldWidth, bottomY, 255.0f
        );

        waves->activeEnemyCount = 5;
    }

    waves->reinforcementSpawned = true;
}

EnemyWaveSystem InitEnemyWaveSystem(void)
{
    EnemyWaveSystem waves = {0};
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

    float playerCenterX = GetPlayerCenterX(player);
    float progress = playerCenterX / stageWorldWidth;

    if (!waves->waveActive)
    {
        if (waves->currentWave == 0 &&
            progress >= WAVE_1_TRIGGER_PROGRESS)
        {
            SpawnWave1(
                waves, enemies, enemyCapacity,
                stageWorldWidth, walkAreaTop, walkAreaBottom
            );
            waves->currentWave = 1;
        }
        else if (waves->currentWave == 1 &&
                 progress >= WAVE_2_TRIGGER_PROGRESS)
        {
            SpawnWave2(
                waves, enemies, enemyCapacity,
                stageWorldWidth, walkAreaTop, walkAreaBottom
            );
            waves->currentWave = 2;
        }
        else if (waves->currentWave == 2 &&
                 progress >= WAVE_3_TRIGGER_PROGRESS)
        {
            SpawnWave3(
                waves, enemies, enemyCapacity,
                stageWorldWidth, walkAreaTop, walkAreaBottom
            );
            waves->currentWave = 3;
        }
        else if (waves->currentWave == 3 &&
                 progress >= WAVE_4_TRIGGER_PROGRESS)
        {
            SpawnWave4(
                waves, enemies, enemyCapacity,
                stageWorldWidth, walkAreaTop, walkAreaBottom
            );
            waves->currentWave = 4;
        }
    }

    if (!waves->waveActive) return;

    LockPlayerForward(player, waves->fightBoundaryX);

    int livingEnemies =
        CountLivingWaveEnemies(
            enemies,
            waves->activeEnemyStartIndex,
            waves->activeEnemyCount
        );

    if (
        waves->currentWave >= 2 &&
        waves->currentWave <= 4 &&
        !waves->reinforcementSpawned &&
        livingEnemies <= REINFORCEMENT_THRESHOLD
    )
    {
        SpawnReinforcement(
            waves,
            enemies,
            enemyCapacity,
            waves->currentWave,
            stageWorldWidth,
            camera,
            screenWidth,
            walkAreaTop,
            walkAreaBottom
        );

        livingEnemies =
            CountLivingWaveEnemies(
                enemies,
                waves->activeEnemyStartIndex,
                waves->activeEnemyCount
            );
    }

    if (waves->reinforcementSpawned && livingEnemies == 0)
    {
        waves->waveActive = false;
        waves->waveFinished = true;
        waves->activeEnemyCount = 0;
    }
}

int GetEnemyWaveActiveCount(const EnemyWaveSystem *waves)
{
    if (waves == 0) return 0;
    return waves->activeEnemyCount;
}

int GetEnemyWaveStartIndex(const EnemyWaveSystem *waves)
{
    if (waves == 0) return 0;
    return waves->activeEnemyStartIndex;
}

bool IsEnemyWaveActive(const EnemyWaveSystem *waves)
{
    return waves != 0 && waves->waveActive;
}

void LoadEnemyWaveSharedTextures(void)
{
    LoadPunkSharedTextures();
    LoadHooliganSharedTextures();
    LoadGangsterSharedTextures();
}

void UnloadEnemyWaveSharedTextures(void)
{
    UnloadPunkSharedTextures();
    UnloadHooliganSharedTextures();
    UnloadGangsterSharedTextures();
}