#!/usr/bin/env bash
# Smoke test of the Linux installer in an isolated home folder: fresh installation,
# update that repairs damaged files, change of options, move to another folder,
# uninstallation. The real home folder and a real Clipboard installation are not
# touched, and the app window is never opened.
#
# Usage: verify-installer.sh INSTALLER WORK_DIR      (WORK_DIR must not exist)
set -euo pipefail

[[ $# -eq 2 ]] || { echo "Usage: $(basename "$0") INSTALLER WORK_DIR" >&2; exit 2; }
INSTALLER=$(readlink -f "$1")
[[ -f $INSTALLER ]] || { echo "Installer not found: $1" >&2; exit 2; }
[[ ! -e $2 ]] || { echo "WORK_DIR must not exist: $2" >&2; exit 2; }
mkdir -p "$2"
WORK=$(cd "$2" && pwd)

TEST_HOME="$WORK/home"
REFERENCE="$WORK/reference"
PREFIX="$TEST_HOME/.local/share/Clipboard"
MOVED="$TEST_HOME/My Apps/Clipboard"
MENU="$TEST_HOME/.local/share/applications/com.kngc.Clipboard.desktop"
SHORTCUT="$TEST_HOME/Desktop/com.kngc.Clipboard.desktop"
AUTOSTART="$TEST_HOME/.config/autostart/ClipboardApp.desktop"
SETTINGS="$TEST_HOME/.config/KNGC/ClipboardApp.conf"
mkdir -p "$TEST_HOME/Desktop" "$REFERENCE"

root_option=()
[[ $EUID -ne 0 ]] || root_option=(--allow-root)

step() { printf '\n== %s\n' "$*"; }
fail() { echo "FAILED: $*" >&2; exit 1; }
check() { "$@" || fail "$*"; }
# No display and no desktop variables: the installer asks nothing and opens nothing
setup() { env -i HOME="$TEST_HOME" PATH="$PATH" LANG=C.UTF-8 sh "$INSTALLER" "${root_option[@]}" "$@" > "$WORK/last-run.log" 2>&1; }
uninstaller() { env -i HOME="$TEST_HOME" PATH="$PATH" LANG=C.UTF-8 sh "$@" > "$WORK/last-run.log" 2>&1; }

# After removal only the user's file stays, next to the empty manifest that marks the folder
only_user_file_left() { # former install dir
    [[ $(ls -A "$1" | tr '
' ' ') == '.clipboard-manifest my-notes.txt ' && ! -s "$1/.clipboard-manifest" ]]
}

# The same files as in the archive, and nothing lost or changed
check_files() { # install dir
    if ! diff -r --no-dereference -x uninstall.sh "$REFERENCE" "$1" > "$WORK/diff.log" 2>&1; then
        # The user's own file is the only difference allowed
        if grep -vxF "Only in $1: my-notes.txt" "$WORK/diff.log" | grep -q .; then
            fail "installed files differ, see $WORK/diff.log"
        fi
    fi
    while IFS= read -r file; do
        [[ -e "$1/$file" || -L "$1/$file" ]] || fail "missing after installation: $file"
    done < "$1/.clipboard-manifest"
    check test -x "$1/bin/ClipboardApp"
    check test -x "$1/uninstall.sh"
}

step "Unpacking the archive for comparison"
payload_line=$(sed -n "s/^PAYLOAD_LINE='\([0-9]*\)'$/\1/p" "$INSTALLER" | head -n 1)
[[ -n $payload_line ]] || fail "this is not a built installer"
tail -n +"$payload_line" "$INSTALLER" | xz -dc | tar -xf - -C "$REFERENCE"

step "Fresh installation with login startup and a desktop shortcut"
check setup --yes --autostart=1 --desktop-icon=1
check_files "$PREFIX"
check grep -qxF "Exec=\"$PREFIX/bin/ClipboardApp\" --show" "$MENU"
check grep -qxF "X-Clipboard-InstallDir=$PREFIX" "$MENU"
if command -v desktop-file-validate >/dev/null; then check desktop-file-validate "$MENU"; fi
check test -x "$SHORTCUT"
check grep -qxF "Exec=\"$PREFIX/bin/ClipboardApp\"" "$AUTOSTART"
check grep -qx 'launchAtStartup=true' "$SETTINGS"

step "Update: repairs damaged files, keeps the options and the user's own file"
echo 'user data' > "$PREFIX/my-notes.txt"
echo 'damaged' > "$PREFIX/bin/qt.conf"
rm "$PREFIX/lib/libQt6Svg.so.6" "$PREFIX/uninstall.sh"
check setup --yes
check_files "$PREFIX"
check grep -qx 'user data' "$PREFIX/my-notes.txt"
check test -f "$AUTOSTART"
check test -x "$SHORTCUT"

step "Update with login startup and the desktop shortcut switched off"
check setup --yes --autostart=0 --desktop-icon=0
check test ! -e "$AUTOSTART"
check test ! -e "$SHORTCUT"
check grep -qx 'launchAtStartup=false' "$SETTINGS"

step "A folder with other files is refused"
mkdir -p "$TEST_HOME/Documents" && echo 'report' > "$TEST_HOME/Documents/report.txt"
if setup --yes --prefix="$TEST_HOME/Documents"; then fail "installed into a folder with the user's files"; fi
check test "$(ls -A "$TEST_HOME/Documents")" = report.txt
if setup --yes --prefix="$TEST_HOME"; then fail "installed directly into the home folder"; fi
if setup --yes --prefix='relative/path'; then fail "accepted a relative path"; fi
if setup --yes --prefix="$TEST_HOME/a\$b"; then fail "accepted a path with special characters"; fi
check_files "$PREFIX"

step "Move to a folder with a space in its name"
check setup --yes --autostart=1 --prefix="$MOVED"
check_files "$MOVED"
check grep -qxF "Exec=\"$MOVED/bin/ClipboardApp\" --show" "$MENU"
check grep -qxF "Exec=\"$MOVED/bin/ClipboardApp\"" "$AUTOSTART"
check only_user_file_left "$PREFIX"

step "The next update finds the moved installation"
check setup --yes
check_files "$MOVED"
check only_user_file_left "$PREFIX"

step "Uninstallation removes the app, its shortcuts and login startup, and keeps the settings"
check uninstaller "$MOVED/uninstall.sh" --yes
check test ! -e "$MOVED"
check test ! -e "$MENU"
check test ! -e "$AUTOSTART"
check test -f "$SETTINGS"
check grep -qx 'user data' "$PREFIX/my-notes.txt"

step "Installation next to the user's file, then uninstallation with --purge"
check setup --yes --prefix="$PREFIX" --desktop-icon=1
check_files "$PREFIX"
check setup --uninstall --purge --yes
check only_user_file_left "$PREFIX"
check test ! -e "$MENU"
check test ! -e "$SHORTCUT"
check test ! -e "$SETTINGS"
if setup --uninstall --yes; then fail "uninstalled twice"; fi

printf '\nVERIFICATION OK\n'
