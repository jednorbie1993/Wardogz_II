@echo off
cls

echo ==========================================
echo          Building Wardogz II
echo ==========================================
echo.

if not exist build mkdir build

gcc src\main.c ^
src\player\player.c ^
src\player\player_move.c ^
src\player\player_attack.c ^
src\player\player_draw.c ^
src\enemy\enemy.c ^
src\enemy\stage_1\punk.c ^
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