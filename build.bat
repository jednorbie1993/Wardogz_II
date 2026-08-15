main(20260814-030100).c
File
enemy(20260814-030112).c
File
punk(1).c
File
gangster(1).c
File
hooligan(1).c
File
enemy(1).h
File
hooligan(1).h
File
punk.h
File
gangster(1).h
File
yan 
main(20260814-030334).c
File
meron tapos gangster.c naka ano nmn dba?
enemy_draw.c
File
Pasted text(7).txt
Document
ok na nagawa ko na si boss 
enemy(20260814-041136).c
File
enemy_animation(1).c
File
enemy_attack.c
File
enemy_collision.c
File
enemy_draw(1).c
File
enemy_hit.c
File
enemy_move.c
File
gangster(2).c
File
enemy(2).h
File
build(2).bat
File

Nakuha ko na lahat ng enemy files at na-check ko na rin ang structure. Tama rin ang enemy.h mo ngayon na naka-3 death frames, kaya gagawin nating 4 para kay Vargas.

Kulang na lang talaga ang latest:

src/main.c

Pag na-send mo iyon, gagawin ko ang Vargas-only test files at updated build.bat. Hindi ko isasama muna sina Punk, Hooligan, at Gangster sa game.

main(20260814-041647).c
File
pag ok na simulan mo na 

Tapos na ang 0061 Vargas-only test.

Ilagay/rename ang files sa exact locations:

main.c → src\main.c
enemy.h → include\enemy\enemy.h
enemy.c → src\enemy\enemy.c
enemy_animation.c → src\enemy\enemy_animation.c
enemy_attack.c → src\enemy\enemy_attack.c
enemy_draw.c → src\enemy\enemy_draw.c
boss.c → src\enemy\stage_1\boss.c
boss.h → include\enemy\stage_1\boss.h
build.bat → project root

Current behavior:

Vargas lang ang active; hidden muna ang Punk, Hooligan, at Gangster.
Normal boss_idle1–2 muna.
Pagkatapos niyang simulan ang unang punch/kick, permanent na ang boss_battle_idle1–4.
Kasama ang run, punch, kick, at four-frame death.
HP bar name: VARGAS.
Wala munang loading at final-meter trigger.
MAX_ENEMY_DEATH_FRAMES ay 4 na.

Kapag timestamped ang pangalan matapos i-download, siguraduhing palitan sa exact filenames sa listahan bago mag-build.

main.c
Code
enemy.h
Code
enemy.c
Code
5 more
pede send mo skin ung build bat ng txt?

Ito ang updated build file na .txt:

build.txt

Pagkatapos i-download, rename mo:

build.txt → build.bat

Ilagay sa project root ng Wardogz_II.

build.txt
Document

Library
/
build.txt
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
