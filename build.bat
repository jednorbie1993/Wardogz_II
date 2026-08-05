@echo off
cls

echo ==========================================
echo          Building Wardogz II
echo ==========================================
echo.

gcc src/main.c src/player.c -o build/Wardogz_II.exe ^
-Iinclude ^
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