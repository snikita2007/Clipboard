#!/usr/bin/env bash
# Builds a signed (debug keystore) arm64 APK of ClipboardApp.
set -euo pipefail

QT_ANDROID='D:/Qt/6.9.3/android_arm64_v8a'
QT_HOST='D:/Qt/6.9.3/mingw_64'
SDK='C:/Users/nikit/AppData/Local/Android/Sdk'
NDK="$SDK/ndk/26.1.10909125"
SRC='D:/clipboard/Clipboard'
BUILD='D:/clipboard/build-android'
OUT='D:/clipboard/ClipboardApp-android.apk'
CMAKE='C:/Strawberry/c/bin/cmake.exe'
NINJA='C:/Strawberry/c/bin/ninja.exe'

export JAVA_HOME='C:/Program Files/Android/Android Studio/jbr'
export ANDROID_SDK_ROOT="$SDK" ANDROID_HOME="$SDK" ANDROID_NDK_ROOT="$NDK"
export PATH="/c/Program Files/Android/Android Studio/jbr/bin:/c/Windows/System32:/c/Windows:/usr/bin"

# android/build.gradle comes from a newer Qt template and reads `legacyPackaging`,
# which androiddeployqt 6.9 does not write. Supply it as a Gradle project property
# (false = AGP default, which is what Qt 6.9's own template uses).
export ORG_GRADLE_PROJECT_legacyPackaging="${LEGACY_PACKAGING:-false}"

# Sign with the local Android debug keystore so the APK can be installed directly
export QT_ANDROID_KEYSTORE_PATH="$USERPROFILE/.android/debug.keystore"
export QT_ANDROID_KEYSTORE_ALIAS='androiddebugkey'
export QT_ANDROID_KEYSTORE_STORE_PASS='android'
export QT_ANDROID_KEYSTORE_KEY_PASS='android'

"$CMAKE" -S "$SRC" -B "$BUILD" -G Ninja \
    -DCMAKE_MAKE_PROGRAM="$NINJA" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE="$QT_ANDROID/lib/cmake/Qt6/qt.toolchain.cmake" \
    -DQT_HOST_PATH="$QT_HOST" \
    -DANDROID_SDK_ROOT="$SDK" \
    -DANDROID_NDK_ROOT="$NDK" \
    -DQT_ANDROID_COMPILE_SDK_VERSION=36 \
    -DQT_ANDROID_TARGET_SDK_VERSION=36 \
    -DQT_ANDROID_SIGN_APK=ON

"$CMAKE" --build "$BUILD" --parallel
"$CMAKE" --build "$BUILD" --target apk

APK="$BUILD/android-build/build/outputs/apk/release/android-build-release-signed.apk"
cp "$APK" "$OUT"
echo "APK: $OUT (from $APK)"
