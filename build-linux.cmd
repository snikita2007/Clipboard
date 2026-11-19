@echo off
rem Builds the Linux installer (.run) and AppImage into D:\clipboard.
rem Runs inside the WSL distro "ClipboardBuild" (Ubuntu 22.04 with Qt 6.9.3 in /opt/Qt).
rem Extra arguments go to build-installer.sh, for example: build-linux.cmd --version 1.0.1
rem How the distro is prepared: Clipboard\packaging\linux\README.md
setlocal
set DISTRO=ClipboardBuild

wsl.exe -d %DISTRO% -- true >nul 2>&1
if errorlevel 1 (
    echo WSL distro "%DISTRO%" was not found. See Clipboard\packaging\linux\README.md
    exit /b 1
)
wsl.exe -d %DISTRO% -- bash /mnt/d/clipboard/Clipboard/packaging/linux/build-installer.sh --output-dir /mnt/d/clipboard %*
if errorlevel 1 exit /b 1
