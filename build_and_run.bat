@echo off
cls

echo ==========================================
echo          Building Wardogz II
echo ==========================================
echo.

if not exist build mkdir build

gcc src\main.c ^
src\cinematic.c ^
src\player\player.c ^
src\player\player_move.c ^
src\player\player_attack.c ^
src\player\player_draw.c ^
src\player\player_attack_data.c ^
src\enemy\enemy.c ^
src\enemy\enemy_move.c ^
src\enemy\enemy_animation.c ^
src\enemy\enemy_attack.c ^
src\enemy\enemy_hit.c ^
src\enemy\enemy_collision.c ^
src\enemy\enemy_draw.c ^
src\enemy\stage_1\punk.c ^
src\enemy\stage_1\hooligan.c ^
src\enemy\stage_1\gangster.c ^
src\enemy\stage_1\boss.c ^
-o build\Wardogz_II.exe ^
-Iinclude ^
-Iinclude\player ^
-Iinclude\enemy ^
-Iinclude\enemy\stage_1 ^
-I"C:\raylib\raylib\src" ^
-L"C:\raylib\raylib\src" ^
-lraylib ^
-lopengl32 ^
-lgdi32 ^
-lwinmm

if %errorlevel% neq 0 (
    echo.
    echo ==========================================
    echo Build Failed!
    echo ==========================================
    echo.
    pause
    exit /b
)

cls

echo ==========================================
echo Build Successful!
echo Starting Game...
echo ==========================================
echo.

build\Wardogz_II.exe
