@echo off
setlocal enabledelayedexpansion

cd /d "%~dp0"

REM Get VS installation path
for /f "usebackq delims=" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -property installationPath`) do (
    set VS_PATH=%%i
)

if not defined VS_PATH (
    echo ERROR: Visual Studio not found!
    exit /b 1
)

set MSBUILD=%VS_PATH%\Msbuild\Current\Bin\MSBuild.exe

echo Building Win32...
"%MSBUILD%" gameoverlay.sln /t:build /p:Configuration=Release /p:Platform=Win32
if errorlevel 1 exit /b 1

echo Building x64...
"%MSBUILD%" gameoverlay.sln /t:build /p:Configuration=Release /p:Platform=x64
if errorlevel 1 exit /b 1

echo Copying DLLs...
copy /y .\bin\release\n_overlay.dll ..\prebuilt
copy /y .\bin\release\n_overlay.x64.dll ..\prebuilt
copy /y .\bin\release\injector_helper.exe ..\prebuilt\injector_helper.exe
copy /y .\bin\release\injector_helper.x64.exe ..\prebuilt\injector_helper.x64.exe

echo Build completed!
endlocal
