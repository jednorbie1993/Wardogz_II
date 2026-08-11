@echo off
cls

echo ==========================================
echo          Building Wardogz II
echo ==========================================
echo.

if not exist build mkdir build

echo CURRENT FOLDER:
cd

echo.
echo GCC LOCATION:
where gcc

echo.
echo GCC VERSION:
gcc --version

echo.
echo ==========================================
echo COMPILING...
echo ==========================================
echo.

gcc src\main.c ^
src\player\player.c ^
src\player\player_move.c ^
src\player\player_attack.c ^
src\player\player_draw.c ^
src\player\player_attack_data.c ^
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

echo.

if %errorlevel%==0 (
    echo ==========================================
    echo Build Successful!
    echo ==========================================
) else (
    echo ==========================================
    echo Build Failed!
    echo ==========================================
)

echo.
pause