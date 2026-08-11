@echo off
cls

echo ==========================================
echo          Running Wardogz II
echo ==========================================
echo.

if not exist build\Wardogz_II.exe (
    echo ERROR: build\Wardogz_II.exe not found.
    echo Run build.bat or build_and_run.bat first.
    echo.
    pause
    exit /b
)

build\Wardogz_II.exe

echo.
pause