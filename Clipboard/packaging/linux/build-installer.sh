#!/usr/bin/env bash
# Builds Clipboard for Linux (x86_64) and packages it:
#   Clipboard-Setup-<version>-linux-x86_64.run   installer (self-extracting, per user)
#   Clipboard-<version>-linux-x86_64.AppImage    portable build that runs without installation
#
# Needs the environment prepared by setup-build-env.sh (Ubuntu 22.04).
set -euo pipefail

SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
REPO_ROOT=$(cd "$SCRIPT_DIR/../.." && pwd)

VERSION='1.0.0'
OUTPUT_DIR=$(dirname "$REPO_ROOT")
# Not next to the sources: they may be on a Windows drive (WSL), where Linux file modes are lost
BUILD_DIR="${XDG_CACHE_HOME:-$HOME/.cache}/clipboard-build/linux-x86_64"
QT_DIR="${QT_DIR:-${QT_ROOT_DIR:-/opt/Qt/6.9.3/gcc_64}}"
TOOLS_DIR="${TOOLS_DIR:-/opt/linuxdeploy}"
RUN_TESTS=1
BUILD_APPIMAGE=1

usage() {
    cat <<EOF
Usage: $(basename "$0") [options]

  --version X.Y.Z     package version (default: $VERSION)
  --output-dir DIR    where the packages are written (default: $OUTPUT_DIR)
  --build-dir DIR     build folder on a Linux file system (default: $BUILD_DIR)
  --qt-dir DIR        Qt for Linux (default: $QT_DIR)
  --tools-dir DIR     folder with the linuxdeploy AppImages (default: $TOOLS_DIR)
  --skip-tests        do not build and run the regression tests
  --no-appimage       build only the installer
EOF
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --version) VERSION=$2; shift ;;
        --output-dir) OUTPUT_DIR=$2; shift ;;
        --build-dir) BUILD_DIR=$2; shift ;;
        --qt-dir) QT_DIR=$2; shift ;;
        --tools-dir) TOOLS_DIR=$2; shift ;;
        --skip-tests) RUN_TESTS=0 ;;
        --no-appimage) BUILD_APPIMAGE=0 ;;
        -h|--help) usage; exit 0 ;;
        *) echo "Unknown option: $1" >&2; usage >&2; exit 2 ;;
    esac
    shift
done

fail() { echo "error: $*" >&2; exit 1; }

[[ $VERSION =~ ^[0-9]+\.[0-9]+\.[0-9]+(\.[0-9]+)?$ ]] || fail "version must look like 1.0.0"
[[ $(uname -m) == x86_64 ]] || fail "the package is built on x86_64"
for tool in cmake ninja convert xz tar sha256sum objdump file desktop-file-validate; do
    command -v "$tool" >/dev/null || fail "'$tool' is not installed; run setup-build-env.sh"
done
[[ -x "$QT_DIR/bin/qmake" ]] || fail "Qt was not found in $QT_DIR; run setup-build-env.sh or pass --qt-dir"
LINUXDEPLOY="$TOOLS_DIR/linuxdeploy-x86_64.AppImage"
[[ -x "$LINUXDEPLOY" && -x "$TOOLS_DIR/linuxdeploy-plugin-qt-x86_64.AppImage" ]] \
    || fail "linuxdeploy was not found in $TOOLS_DIR; run setup-build-env.sh or pass --tools-dir"

export LANG=C.UTF-8
mkdir -p "$BUILD_DIR" "$OUTPUT_DIR"
BUILD_DIR=$(cd "$BUILD_DIR" && pwd)
OUTPUT_DIR=$(cd "$OUTPUT_DIR" && pwd)

# File modes go into the package as they are in the build folder
probe="$BUILD_DIR/.mode-probe"
touch "$probe" && chmod 640 "$probe"
[[ $(stat -c %a "$probe") == 640 ]] || fail "$BUILD_DIR does not keep Linux file modes; pass --build-dir on a Linux file system"
rm -f "$probe"

APP_BUILD="$BUILD_DIR/app"
APPDIR="$BUILD_DIR/AppDir"
PAYLOAD="$BUILD_DIR/payload"
ICONS="$BUILD_DIR/icons"
INSTALLER_NAME="Clipboard-Setup-$VERSION-linux-x86_64.run"
APPIMAGE_NAME="Clipboard-$VERSION-linux-x86_64.AppImage"

echo "==> Building the app (Qt: $QT_DIR)"
tests_option=OFF
[[ $RUN_TESTS == 1 ]] && tests_option=ON
cmake -S "$REPO_ROOT" -B "$APP_BUILD" -G Ninja -Wno-dev \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="$QT_DIR" -DCLIPBOARD_BUILD_TESTS=$tests_option
cmake --build "$APP_BUILD" --parallel

if [[ $RUN_TESTS == 1 ]]; then
    echo "==> Running the regression tests"
    ctest --test-dir "$APP_BUILD" --output-on-failure
fi

echo "==> Collecting the app with its Qt libraries"
rm -rf "$APPDIR" "$PAYLOAD" "$ICONS"
desktop-file-validate "$SCRIPT_DIR/com.kngc.Clipboard.desktop"
icon_options=()
for size in 16 32 48 64 128 256 512; do
    mkdir -p "$ICONS/$size"
    convert "$REPO_ROOT/ui/assets/app.png" -resize "${size}x${size}" "$ICONS/$size/com.kngc.Clipboard.png"
    icon_options+=(--icon-file "$ICONS/$size/com.kngc.Clipboard.png")
done

# The linuxdeploy AppImages unpack themselves, so FUSE is not needed (containers, WSL)
export APPIMAGE_EXTRACT_AND_RUN=1
# Qt's libraries are already stripped; the bundled strip is too old for some of the system ones
export NO_STRIP=1
export QMAKE="$QT_DIR/bin/qmake"
export QML_SOURCES_PATHS="$REPO_ROOT/ui"
# SVG icons in the interface are loaded through the image format plugin
export EXTRA_QT_MODULES='svg'
export LD_LIBRARY_PATH="$QT_DIR/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"

deploy_log="$BUILD_DIR/linuxdeploy.log"
(cd "$BUILD_DIR" && "$LINUXDEPLOY" --appdir "$APPDIR" \
    --executable "$APP_BUILD/ClipboardApp" \
    --desktop-file "$SCRIPT_DIR/com.kngc.Clipboard.desktop" \
    "${icon_options[@]}" --plugin qt) > "$deploy_log" 2>&1 \
    || { tail -n 40 "$deploy_log" >&2; fail "linuxdeploy failed; the full log is in $deploy_log"; }

for required in bin/ClipboardApp bin/qt.conf lib/libQt6Core.so.6 lib/libQt6Quick.so.6 lib/libQt6Bluetooth.so.6 \
    lib/libxcb-cursor.so.0 plugins/platforms/libqxcb.so plugins/imageformats/libqsvg.so \
    qml/QtQuick/Controls/Basic/qmldir qml/Qt/labs/platform/qmldir \
    share/applications/com.kngc.Clipboard.desktop share/icons/hicolor/256x256/apps/com.kngc.Clipboard.png; do
    [[ -e "$APPDIR/usr/$required" ]] || fail "incomplete deployment: usr/$required is missing"
done

# Every library must resolve, either inside the package or among the basic system ones
unresolved=$(find "$APPDIR/usr" -type f \( -name '*.so*' -o -name ClipboardApp \) -exec ldd {} + 2>/dev/null \
    | awk '/not found/ { print $1 }' | sort -u)
[[ -z $unresolved ]] || fail "libraries without a provider: $unresolved"

# The oldest glibc the package runs on: the newest symbol version used inside it
min_glibc=$(find "$APPDIR/usr" -type f \( -name '*.so*' -o -name ClipboardApp \) -exec objdump -T {} + 2>/dev/null \
    | grep -oE 'GLIBC_[0-9]+\.[0-9]+' | sort -uV | tail -n 1 | cut -d_ -f2)
[[ -n $min_glibc ]] || fail "cannot find the required glibc version"

if [[ $BUILD_APPIMAGE == 1 ]]; then
    echo "==> Packaging the AppImage"
    rm -f "$BUILD_DIR/$APPIMAGE_NAME"
    (cd "$BUILD_DIR" && LDAI_OUTPUT="$APPIMAGE_NAME" OUTPUT="$APPIMAGE_NAME" \
        LINUXDEPLOY_OUTPUT_VERSION="$VERSION" VERSION="$VERSION" ARCH=x86_64 \
        "$LINUXDEPLOY" --appdir "$APPDIR" --output appimage) >> "$deploy_log" 2>&1 \
        || { tail -n 40 "$deploy_log" >&2; fail "AppImage packaging failed; the full log is in $deploy_log"; }
    [[ -s "$BUILD_DIR/$APPIMAGE_NAME" ]] || fail "the AppImage was not created; see $deploy_log"
fi

echo "==> Packaging the installer"
# The installed layout is the content of usr/: bin, lib, plugins, qml, share, translations
cp -a "$APPDIR/usr" "$PAYLOAD"
# The uninstaller removes what the manifest lists: files and the folders that held them
find "$PAYLOAD" -depth -type d -empty -delete
chmod -R u=rwX,go=rX "$PAYLOAD"
(cd "$PAYLOAD" && { find . -type f -o -type l; } | sed 's|^\./||' | LC_ALL=C sort > "$BUILD_DIR/manifest"
 printf '%s\n' .clipboard-manifest uninstall.sh >> "$BUILD_DIR/manifest")
install -m 644 "$BUILD_DIR/manifest" "$PAYLOAD/.clipboard-manifest"
installed_kb=$(du -sk "$PAYLOAD" | cut -f1)

tar -C "$PAYLOAD" --owner=0 --group=0 --numeric-owner --sort=name -cf - . | xz -9 > "$BUILD_DIR/payload.tar.xz"
payload_sha256=$(sha256sum "$BUILD_DIR/payload.tar.xz" | cut -d' ' -f1)

stub="$BUILD_DIR/installer-stub.sh"
payload_line=$(($(wc -l < "$SCRIPT_DIR/installer.sh") + 1))
sed -e "s/@VERSION@/$VERSION/" -e "s/@PAYLOAD_LINE@/$payload_line/" -e "s/@PAYLOAD_SHA256@/$payload_sha256/" \
    -e "s/@INSTALLED_KB@/$installed_kb/" -e "s/@MIN_GLIBC@/$min_glibc/" "$SCRIPT_DIR/installer.sh" > "$stub"
[[ $(tail -c 1 "$stub" | wc -l) == 1 ]] || fail "installer.sh must end with a line break"
[[ $(wc -l < "$stub") == $((payload_line - 1)) ]] || fail "installer.sh changed its length during substitution"
! grep -q $'\r' "$stub" || fail "installer.sh has Windows line endings"
! grep -qE '@(VERSION|PAYLOAD_LINE|PAYLOAD_SHA256|INSTALLED_KB|MIN_GLIBC)@' "$stub" || fail "a placeholder was left in installer.sh"
if command -v shellcheck >/dev/null; then shellcheck -s sh "$stub"; fi

cat "$stub" "$BUILD_DIR/payload.tar.xz" > "$BUILD_DIR/$INSTALLER_NAME"
chmod 755 "$BUILD_DIR/$INSTALLER_NAME"

echo "==> Copying the packages to $OUTPUT_DIR"
packages=("$INSTALLER_NAME")
[[ $BUILD_APPIMAGE == 1 ]] && packages+=("$APPIMAGE_NAME")
for package in "${packages[@]}"; do
    cp "$BUILD_DIR/$package" "$OUTPUT_DIR/$package"
    chmod 755 "$OUTPUT_DIR/$package" 2>/dev/null || true
    hash=$(sha256sum "$OUTPUT_DIR/$package" | cut -d' ' -f1)
    printf '%s  %s\n' "$hash" "$package" > "$OUTPUT_DIR/$package.sha256"
    echo "$package"
    echo "  size:   $(du -h "$OUTPUT_DIR/$package" | cut -f1)"
    echo "  sha256: $hash"
done
echo "Installed size: $((installed_kb / 1024)) MB. Needs glibc $min_glibc or newer."
echo "BUILD OK"
