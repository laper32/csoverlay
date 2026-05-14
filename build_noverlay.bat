@echo off
setlocal enabledelayedexpansion

REM Get VS installation path
for /f "usebackq delims=" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -property installationPath`) do (
    set VS_PATH=%%i
)

if not defined VS_PATH (
    echo ERROR: Visual Studio not found!
    exit /b 1
)

echo VS_PATH: %VS_PATH%

REM Initialize VS environment
call "%VS_PATH%\Common7\Tools\vsdevcmd.bat" -arch=x64 -host_arch=x64

if errorlevel 1 (
    echo ERROR: Failed to initialize VS environment!
    exit /b 1
)

echo.
echo Building n_overlay...
msbuild game-overlay\sln\gameoverlay.sln /p:Configuration=Release /p:Platform=x64 /m /v:minimal

if errorlevel 1 (
    echo ERROR: n_overlay build failed!
    exit /b 1
)

echo.
echo Build completed successfully!
endlocal
