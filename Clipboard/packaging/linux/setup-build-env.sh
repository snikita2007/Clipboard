#!/usr/bin/env bash
# Prepares Ubuntu 22.04 (a PC, a container or a WSL distro) for build-installer.sh.
# Run once as root. Installs the compiler, the system libraries that get bundled,
# Qt for Linux (through aqtinstall) and linuxdeploy.
set -euo pipefail

QT_VERSION="${QT_VERSION:-6.9.3}"
QT_ROOT="${QT_ROOT:-/opt/Qt}"
TOOLS_DIR="${TOOLS_DIR:-/opt/linuxdeploy}"
LINUXDEPLOY_URL='https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage'
LINUXDEPLOY_QT_URL='https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage'

if [[ $EUID -ne 0 ]]; then
    echo "Run as root: sudo $0" >&2
    exit 1
fi

export DEBIAN_FRONTEND=noninteractive
apt-get update
apt-get install -y --no-install-recommends \
    ca-certificates curl file xz-utils python3 python3-venv \
    build-essential cmake ninja-build pkg-config \
    imagemagick desktop-file-utils shellcheck \
    libgl1-mesa-dev libegl1-mesa-dev libvulkan-dev libxkbcommon-dev libx11-dev libxext-dev \
    libfontconfig1 libfreetype6 libglib2.0-0 libdbus-1-3 libgssapi-krb5-2 libssl3 libzstd1 \
    libx11-xcb1 libxkbcommon-x11-0 libsm6 libice6 \
    libxcb1 libxcb-cursor0 libxcb-glx0 libxcb-icccm4 libxcb-image0 libxcb-keysyms1 \
    libxcb-randr0 libxcb-render0 libxcb-render-util0 libxcb-shape0 libxcb-shm0 \
    libxcb-sync1 libxcb-util1 libxcb-xfixes0 libxcb-xinput0 libxcb-xkb1

# Qt: the official binaries work on glibc 2.28+, so the app's own baseline is this OS (glibc 2.35).
QT_DIR="$QT_ROOT/$QT_VERSION/gcc_64"
if [[ ! -x "$QT_DIR/bin/qmake" ]]; then
    python3 -m venv /opt/aqt-venv
    /opt/aqt-venv/bin/pip install --quiet --upgrade pip aqtinstall
    (cd /tmp && /opt/aqt-venv/bin/aqt install-qt linux desktop "$QT_VERSION" linux_gcc_64 \
        -m qtconnectivity -O "$QT_ROOT")
fi

mkdir -p "$TOOLS_DIR"
curl -fL --retry 3 -o "$TOOLS_DIR/linuxdeploy-x86_64.AppImage" "$LINUXDEPLOY_URL"
curl -fL --retry 3 -o "$TOOLS_DIR/linuxdeploy-plugin-qt-x86_64.AppImage" "$LINUXDEPLOY_QT_URL"
chmod +x "$TOOLS_DIR"/*.AppImage

echo
echo "Build environment is ready."
echo "Qt:          $QT_DIR"
echo "linuxdeploy: $TOOLS_DIR"
