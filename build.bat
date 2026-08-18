@echo off
cls

echo ==========================================
echo          Building Wardogz II
echo ==========================================
echo.

if not exist build mkdir build

windres resource.rc -O coff -o build\resource.res

if errorlevel 1 (
    echo.
    echo ==========================================
    echo Resource Compile Failed!
    echo Check resource.rc and your .ico path.
    echo ==========================================
    echo.
    pause
    exit /b 1
)

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
src\enemy\enemy_wave.c ^
src\enemy\stage_1\punk.c ^
src\enemy\stage_1\hooligan.c ^
src\enemy\stage_1\gangster.c ^
src\enemy\stage_1\boss.c ^
build\resource.res ^
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

echo.

if %errorlevel%==0 (
    echo ==========================================
    echo Build Successful!
    echo EXE: build\Wardogz_II.exe
    echo ==========================================
) else (
    echo ==========================================
    echo Build Failed!
    echo ==========================================
)

echo.
pause