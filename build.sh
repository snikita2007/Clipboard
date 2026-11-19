#!/usr/bin/env bash
set -euo pipefail

QT_DIR='D:/Qt/6.9.3/mingw_64'
MINGW_DIR='D:/Qt/Tools/mingw1310_64'
SRC='D:/clipboard/Clipboard'
BUILD='D:/clipboard/Clipboard/build'
DIST='D:/clipboard/ClipboardApp'

# Only Qt's MinGW and Qt on PATH, so the Strawberry/MSYS2 compilers are not picked up
export PATH="/d/Qt/Tools/mingw1310_64/bin:/d/Qt/6.9.3/mingw_64/bin:/c/Windows/System32:/c/Windows:/usr/bin"
CMAKE='C:/Strawberry/c/bin/cmake.exe'
NINJA='C:/Strawberry/c/bin/ninja.exe'

"$CMAKE" -S "$SRC" -B "$BUILD" -G Ninja \
    -DCMAKE_MAKE_PROGRAM="$NINJA" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH="$QT_DIR" \
    -DCMAKE_C_COMPILER="$MINGW_DIR/bin/gcc.exe" \
    -DCMAKE_CXX_COMPILER="$MINGW_DIR/bin/g++.exe"

"$CMAKE" --build "$BUILD" --parallel

rm -rf "$DIST"
mkdir -p "$DIST"
cp "$BUILD/ClipboardApp.exe" "$DIST/"
"$QT_DIR/bin/windeployqt.exe" --release --qmldir "$SRC/ui" --compiler-runtime "$DIST/ClipboardApp.exe"
echo "BUILD OK"
