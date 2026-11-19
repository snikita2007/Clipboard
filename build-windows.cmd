@echo off
rem Builds ClipboardApp.exe with MSVC (needed for Bluetooth on Windows) and deploys it to D:\clipboard\ClipboardApp
setlocal
set QT_DIR=D:\Qt\6.9.3\msvc2022_64
set SRC=D:\clipboard\Clipboard
set BUILD=D:\clipboard\build-msvc
set DIST=D:\clipboard\ClipboardApp
set CMAKE=C:\Strawberry\c\bin\cmake.exe
set NINJA=C:\Strawberry\c\bin\ninja.exe

set VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe
for /f "usebackq delims=" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set VSDIR=%%i
if not defined VSDIR (
    echo Visual Studio C++ build tools not found
    exit /b 1
)
call "%VSDIR%\VC\Auxiliary\Build\vcvars64.bat" >nul || exit /b 1

"%CMAKE%" -S "%SRC%" -B "%BUILD%" -G Ninja -DCMAKE_MAKE_PROGRAM="%NINJA%" -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="%QT_DIR%" -DCMAKE_C_COMPILER=cl -DCMAKE_CXX_COMPILER=cl || exit /b 1
"%CMAKE%" --build "%BUILD%" --parallel || exit /b 1

if exist "%DIST%" rmdir /s /q "%DIST%"
if exist "%DIST%" (
    echo Cannot replace %DIST%: close ClipboardApp first
    exit /b 1
)
mkdir "%DIST%"
copy /y "%BUILD%\ClipboardApp.exe" "%DIST%\" >nul || exit /b 1
"%QT_DIR%\bin\windeployqt.exe" --release --no-compiler-runtime --qmldir "%SRC%\ui" "%DIST%\ClipboardApp.exe" || exit /b 1

rem Put the MSVC runtime DLLs next to the exe, so it starts on PCs without the VC++ redistributable
for /d %%d in ("%VCToolsRedistDir%x64\Microsoft.VC*.CRT") do copy /y "%%d\*.dll" "%DIST%\" >nul

echo BUILD OK
