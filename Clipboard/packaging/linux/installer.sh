#!/bin/sh
# Clipboard for Linux (x86_64): installer and uninstaller.
# Installs for the current user; administrator rights are not needed.
#
# The released installer is this script followed by the application archive
# (see build-installer.sh). A copy without the archive is kept in the
# installation folder as uninstall.sh.
#
# shellcheck disable=SC2059  # the printf formats are the message constants below

set -u

APP_VERSION='@VERSION@'
PAYLOAD_LINE='@PAYLOAD_LINE@'
PAYLOAD_SHA256='@PAYLOAD_SHA256@'
INSTALLED_KB='@INSTALLED_KB@'
MIN_GLIBC='@MIN_GLIBC@'

PRODUCT='Clipboard'
APP_BINARY='bin/ClipboardApp'
DESKTOP_ID='com.kngc.Clipboard'
MANIFEST='.clipboard-manifest'
UNINSTALLER='uninstall.sh'

SELF=$0
DATA_HOME=${XDG_DATA_HOME:-$HOME/.local/share}
CONFIG_HOME=${XDG_CONFIG_HOME:-$HOME/.config}
CACHE_HOME=${XDG_CACHE_HOME:-$HOME/.cache}
MENU_FILE="$DATA_HOME/applications/$DESKTOP_ID.desktop"
# The app keeps this file in step with its "Launch at startup" setting
AUTOSTART_FILE="$CONFIG_HOME/autostart/ClipboardApp.desktop"

MODE=install
case "${0##*/}" in "$UNINSTALLER") MODE=uninstall ;; esac

PREFIX_ARG=''
OPT_AUTOSTART=''
OPT_DESKTOP=''
ASSUME_YES=0
NO_LAUNCH=0
PURGE=0
ALLOW_ROOT=0
SHOW_HELP=0
BAD_ARG=''
LANG_CODE=''
UI=none
TMP=''
PROGRESS_PID=''
PROGRESS_FIFO=''

while [ $# -gt 0 ]; do
    case "$1" in
        --prefix=*) PREFIX_ARG=${1#*=} ;;
        --prefix)
            if [ $# -gt 1 ]; then shift; PREFIX_ARG=$1; else BAD_ARG=$1; fi ;;
        --autostart=0|--autostart=1) OPT_AUTOSTART=${1#*=} ;;
        --desktop-icon=0|--desktop-icon=1) OPT_DESKTOP=${1#*=} ;;
        --lang=ru|--lang=en) LANG_CODE=${1#*=} ;;
        -y|--yes) ASSUME_YES=1 ;;
        --no-launch) NO_LAUNCH=1 ;;
        --uninstall) MODE=uninstall ;;
        --purge) PURGE=1 ;;
        --allow-root) ALLOW_ROOT=1 ;;
        -h|--help) SHOW_HELP=1 ;;
        --version) printf '%s %s\n' "$PRODUCT" "$APP_VERSION"; exit 0 ;;
        *) BAD_ARG=$1 ;;
    esac
    shift
done

# ---------------------------------------------------------------- messages

if [ -z "$LANG_CODE" ]; then
    LANG_CODE=en
    case "${LC_ALL:-${LC_MESSAGES:-${LANG:-}}}" in
        ru*UTF-8*|ru*utf8*|ru*utf-8*) LANG_CODE=ru ;;
    esac
fi

if [ "$LANG_CODE" = ru ]; then
    T_TITLE="$PRODUCT $APP_VERSION — установка"
    T_HERO='Ваш буфер обмена. На всех устройствах.'
    T_HERO2='Текст, фото и файлы — под рукой. Синхронизация по Bluetooth и Wi-Fi.'
    T_LOCATION='Папка установки'
    T_AUTOSTART='Запускать при входе в систему'
    T_DESKTOP='Создать ярлык на рабочем столе'
    T_OPTION='Параметр'
    T_INSTALL='Установить'
    T_CANCEL='Отмена'
    T_YES='Да'
    T_NO='Нет'
    T_OPEN='Открыть'
    T_CLOSE='Закрыть'
    T_YES_DEFAULT='[Д/н]'
    T_NO_DEFAULT='[д/Н]'
    T_RUNNING="$PRODUCT сейчас запущен и будет закрыт. Продолжить?"
    T_CHECKING='Проверяем установщик…'
    T_UNPACKING='Распаковываем приложение…'
    T_SHORTCUTS='Добавляем ярлыки…'
    T_INSTALLING="Устанавливаем $PRODUCT…"
    T_DONE="$PRODUCT готов к работе"
    T_DONE2='Копируйте как обычно. Clipboard сохранит текст, фото и файлы и поможет передать их на другое устройство.'
    T_INSTALLED_IN='Установлено в: %s'
    T_HINT_OPEN="Откройте $PRODUCT из меню приложений. Alt+V открывает историю из любого приложения."
    T_HINT_WAYLAND='В сеансе Wayland Alt+V срабатывает, только когда активно окно X11. Чтобы клавиша работала везде, добавьте в настройках системы своё сочетание клавиш с командой:'
    T_HINT_UNINSTALL='Удаление: sh "%s"'
    T_LAUNCH="Открыть $PRODUCT сейчас?"
    T_CANCELLED='Установка отменена.'
    T_NO_DESKTOP_DIR='Папка рабочего стола не найдена, ярлык не создан.'
    E_USAGE='Неизвестный параметр: %s. Список параметров: --help'
    E_ROOT='Запустите установщик от обычного пользователя, без sudo: Clipboard устанавливается для текущего пользователя и не требует прав администратора.'
    E_ARCH='Этот пакет предназначен для 64-битных систем x86_64, а здесь %s.'
    E_GLIBC='Для Clipboard нужна glibc %s или новее, в этой системе %s.'
    E_TOOL='Не найдена команда «%s», она нужна установщику.'
    E_NO_PAYLOAD='Запустите файл установщика напрямую: sh Clipboard-Setup-….run'
    E_CORRUPT='Файл установщика повреждён или загружен не полностью. Скачайте его ещё раз.'
    E_DIR_RELATIVE='Укажите полный путь к папке установки.'
    E_DIR_CHARS='В пути установки нельзя использовать символы " ` $ \ % и перевод строки.'
    E_DIR_HOME='Выберите отдельную папку для установки, например %s.'
    E_DIR_BUSY='Папка %s уже существует и не является установкой Clipboard. Выберите другую, например %s.'
    E_DIR_CREATE='Не удалось создать папку %s.'
    E_SPACE='Недостаточно места в %s: нужно %s МБ.'
    E_STILL_RUNNING="Не удалось закрыть $PRODUCT. Закройте приложение и запустите установщик снова."
    E_EXTRACT='Не удалось распаковать приложение в %s.'
    E_START="$PRODUCT установлен, но не запускается в этой системе."
    E_MISSING_LIBS='Не хватает системных библиотек:'
    E_INSTALL_LIBS='Установите их командой:'
    U_TITLE="$PRODUCT — удаление"
    U_CONFIRM='Удалить Clipboard из %s?'
    U_NEED_YES='Для удаления без вопросов добавьте параметр --yes.'
    U_NOT_FOUND='Установка Clipboard не найдена в %s.'
    U_NOT_INSTALLED='Установка Clipboard не найдена. Укажите папку параметром --prefix.'
    U_DONE='Clipboard удалён. Настройки сохранены.'
    U_DONE_PURGE='Clipboard удалён вместе с настройками и кэшем истории.'
    U_CANCELLED='Удаление отменено.'
    T_HELP="Установщик $PRODUCT $APP_VERSION для Linux (x86_64)

Использование: sh ${0##*/} [параметры]

  --prefix=ПАПКА       папка установки (по умолчанию $DATA_HOME/$PRODUCT)
  --autostart=0|1      запускать при входе в систему
  --desktop-icon=0|1   ярлык на рабочем столе
  -y, --yes            не задавать вопросов и не запускать приложение
  --no-launch          не предлагать запуск после установки
  --lang=ru|en         язык установщика
  --uninstall          удалить установленное приложение
  --purge              вместе с --uninstall: удалить также настройки и кэш
  --allow-root         разрешить запуск от root
  --version            показать версию
  -h, --help           показать эту справку

Установка выполняется для текущего пользователя, права администратора не нужны."
else
    T_TITLE="$PRODUCT $APP_VERSION — Setup"
    T_HERO='Your clipboard. Across your devices.'
    T_HERO2='Text, photos and files, always close. Sync over Bluetooth and Wi-Fi.'
    T_LOCATION='Install location'
    T_AUTOSTART='Start when I sign in'
    T_DESKTOP='Create a desktop shortcut'
    T_OPTION='Option'
    T_INSTALL='Install'
    T_CANCEL='Cancel'
    T_YES='Yes'
    T_NO='No'
    T_OPEN='Open'
    T_CLOSE='Close'
    T_YES_DEFAULT='[Y/n]'
    T_NO_DEFAULT='[y/N]'
    T_RUNNING="$PRODUCT is running and will be closed. Continue?"
    T_CHECKING='Checking the installer…'
    T_UNPACKING='Unpacking the application…'
    T_SHORTCUTS='Adding your shortcuts…'
    T_INSTALLING="Installing $PRODUCT…"
    T_DONE="$PRODUCT is ready"
    T_DONE2='Copy as usual. Clipboard keeps your text, photos and files close, and helps send them to your other devices.'
    T_INSTALLED_IN='Installed in: %s'
    T_HINT_OPEN="Open $PRODUCT from the application menu. Alt+V opens your history from any app."
    T_HINT_WAYLAND='In a Wayland session Alt+V works only while an X11 window is active. To make it work everywhere, add a custom keyboard shortcut in system settings with this command:'
    T_HINT_UNINSTALL='To uninstall: sh "%s"'
    T_LAUNCH="Open $PRODUCT now?"
    T_CANCELLED='Installation cancelled.'
    T_NO_DESKTOP_DIR='The desktop folder was not found; the shortcut was not created.'
    E_USAGE='Unknown option: %s. See --help for the list of options.'
    E_ROOT='Run the installer as a regular user, without sudo: Clipboard is installed for the current user and does not need administrator rights.'
    E_ARCH='This package is for 64-bit x86_64 systems; this computer is %s.'
    E_GLIBC='Clipboard needs glibc %s or newer; this system has %s.'
    E_TOOL='The "%s" command was not found; the installer needs it.'
    E_NO_PAYLOAD='Run the installer file directly: sh Clipboard-Setup-….run'
    E_CORRUPT='The installer file is damaged or incomplete. Download it again.'
    E_DIR_RELATIVE='Enter the full path of the install location.'
    E_DIR_CHARS='The install location cannot contain " ` $ \ % or line breaks.'
    E_DIR_HOME='Choose a separate folder for the installation, for example %s.'
    E_DIR_BUSY='%s already exists and is not a Clipboard installation. Choose another folder, for example %s.'
    E_DIR_CREATE='Cannot create the folder %s.'
    E_SPACE='Not enough free space in %s: %s MB needed.'
    E_STILL_RUNNING="Could not close $PRODUCT. Close the application and run the installer again."
    E_EXTRACT='Could not unpack the application to %s.'
    E_START="$PRODUCT is installed, but it cannot start on this system."
    E_MISSING_LIBS='These system libraries are missing:'
    E_INSTALL_LIBS='Install them with:'
    U_TITLE="$PRODUCT — Uninstall"
    U_CONFIRM='Remove Clipboard from %s?'
    U_NEED_YES='To uninstall without questions, add the --yes option.'
    U_NOT_FOUND='No Clipboard installation was found in %s.'
    U_NOT_INSTALLED='No Clipboard installation was found. Pass its folder with --prefix.'
    U_DONE='Clipboard has been removed. Your settings were kept.'
    U_DONE_PURGE='Clipboard has been removed, together with its settings and history cache.'
    U_CANCELLED='Uninstall cancelled.'
    T_HELP="$PRODUCT $APP_VERSION installer for Linux (x86_64)

Usage: sh ${0##*/} [options]

  --prefix=DIR         install location (default: $DATA_HOME/$PRODUCT)
  --autostart=0|1      start when you sign in
  --desktop-icon=0|1   desktop shortcut
  -y, --yes            ask nothing and do not open the app afterwards
  --no-launch          do not offer to open the app afterwards
  --lang=ru|en         installer language
  --uninstall          remove the installed app
  --purge              with --uninstall: also remove settings and cache
  --allow-root         allow running as root
  --version            print the version
  -h, --help           show this help

Clipboard is installed for the current user; administrator rights are not needed."
fi

[ "$MODE" = uninstall ] && T_TITLE=$U_TITLE

# ---------------------------------------------------------------- interface

have() { command -v "$1" >/dev/null 2>&1; }

if [ "$ASSUME_YES" = 1 ]; then
    UI=none
elif [ -t 0 ] && [ -t 1 ]; then
    UI='tty'
elif [ -n "${DISPLAY:-}${WAYLAND_DISPLAY:-}" ]; then
    # Started from a file manager: use the dialogs of the desktop
    case "${XDG_CURRENT_DESKTOP:-}" in
        *KDE*) if have kdialog; then UI=kdialog; elif have zenity; then UI=zenity; fi ;;
        *) if have zenity; then UI=zenity; elif have kdialog; then UI=kdialog; fi ;;
    esac
fi

say() { [ "$UI" = tty ] || [ "$UI" = none ] || return 0; printf '%s\n' "$1"; }

progress_stop() {
    [ -n "$PROGRESS_PID" ] || return 0
    echo 100 >&3 2>/dev/null
    exec 3>&-
    wait "$PROGRESS_PID" 2>/dev/null
    rm -f "$PROGRESS_FIFO"
    PROGRESS_PID=''
}

progress_start() {
    [ "$UI" = zenity ] || return 0
    PROGRESS_FIFO="${TMPDIR:-/tmp}/clipboard-setup-$$.fifo"
    mkfifo "$PROGRESS_FIFO" 2>/dev/null || return 0
    # The window can be closed early; writing to it must not stop the installation
    trap '' PIPE
    zenity --progress --pulsate --no-cancel --auto-close --title="$T_TITLE" --text="$1" \
        --width=420 < "$PROGRESS_FIFO" 2>/dev/null &
    PROGRESS_PID=$!
    exec 3> "$PROGRESS_FIFO"
}

cleanup() {
    progress_stop
    [ -n "$TMP" ] && [ -d "$TMP" ] && rm -rf "$TMP"
}
trap cleanup EXIT
trap 'exit 130' INT TERM HUP

show_message() { # kind (info|error), text
    progress_stop
    case "$UI" in
        zenity) zenity "--$1" --no-markup --title="$T_TITLE" --text="$2" --width=460 2>/dev/null ;;
        kdialog)
            if [ "$1" = error ]; then kdialog --title "$T_TITLE" --error "$2" 2>/dev/null
            else kdialog --title "$T_TITLE" --msgbox "$2" 2>/dev/null; fi ;;
    esac
}

die() { # text [exit code]
    show_message error "$1"
    printf '%s\n' "$1" >&2
    exit "${2:-1}"
}

ask_yes_no() { # question, default (1 = yes); terminal only
    if [ "$2" = 1 ]; then hint=$T_YES_DEFAULT; else hint=$T_NO_DEFAULT; fi
    while :; do
        printf '%s %s ' "$1" "$hint"
        IFS= read -r answer || answer=''
        case "$answer" in
            '') [ "$2" = 1 ]; return ;;
            y|Y|yes|Yes|YES|д|Д|да|Да|ДА) return 0 ;;
            n|N|no|No|NO|н|Н|нет|Нет|НЕТ) return 1 ;;
        esac
    done
}

confirm() { # question, default (1 = yes), [labels of the two buttons]
    yes_label=${3:-$T_YES}
    no_label=${4:-$T_NO}
    case "$UI" in
        tty) ask_yes_no "$1" "$2" ;;
        zenity) zenity --question --no-markup --title="$T_TITLE" --text="$1" --width=420 \
            --ok-label="$yes_label" --cancel-label="$no_label" 2>/dev/null ;;
        kdialog) kdialog --title "$T_TITLE" --yes-label "$yes_label" --no-label "$no_label" --yesno "$1" 2>/dev/null ;;
        *) return 0 ;;
    esac
}

# ---------------------------------------------------------------- helpers

desktop_dir() {
    dir=''
    have xdg-user-dir && dir=$(xdg-user-dir DESKTOP 2>/dev/null)
    # xdg-user-dir answers with the home folder when no desktop folder is set
    case "$dir" in ''|"$HOME"|"$HOME/") dir="$HOME/Desktop" ;; esac
    printf '%s\n' "$dir"
}

# True when a desktop entry starts the app from the given installation
belongs_to() { # file, install dir
    [ -f "$1" ] && grep -qF -- "$2/" "$1"
}

previous_install_dir() {
    [ -f "$MENU_FILE" ] || return 0
    dir=$(sed -n 's/^X-Clipboard-InstallDir=//p' "$MENU_FILE" | head -n 1)
    [ -n "$dir" ] && [ -s "$dir/$MANIFEST" ] && printf '%s\n' "$dir"
}

running_pids() { # path of the executable
    for proc in /proc/[0-9]*; do
        exe=$(readlink "$proc/exe" 2>/dev/null) || continue
        case "$exe" in "$1"|"$1 (deleted)") printf '%s\n' "${proc#/proc/}" ;; esac
    done
}

stop_app() { # path of the executable; fails when the app is still running
    pids=$(running_pids "$1")
    [ -n "$pids" ] || return 0
    # shellcheck disable=SC2086
    kill $pids 2>/dev/null
    tries=0
    while [ "$tries" -lt 25 ]; do
        [ -z "$(running_pids "$1")" ] && return 0
        sleep 0.2 2>/dev/null || sleep 1
        tries=$((tries + 1))
    done
    pids=$(running_pids "$1")
    # shellcheck disable=SC2086
    [ -n "$pids" ] && kill -9 $pids 2>/dev/null
    sleep 1
    [ -z "$(running_pids "$1")" ]
}

# Removes the files listed in the manifest, then the folders that became empty.
# Anything else in the folder stays.
remove_installed_files() { # install dir
    [ -f "$1/$MANIFEST" ] || return 0
    folders=$(awk -F/ '{ path = ""; for (i = 1; i < NF; i++) { path = (i == 1 ? $i : path "/" $i); print path } }' \
        "$1/$MANIFEST" | sort -u -r)
    while IFS= read -r entry; do
        case "$entry" in ''|/*|..|../*|*/..|*/../*) continue ;; esac
        rm -f -- "$1/$entry"
    done < "$1/$MANIFEST"
    printf '%s\n' "$folders" | while IFS= read -r entry; do
        case "$entry" in ''|/*|..|../*|*/..|*/../*) continue ;; esac
        rmdir -- "$1/$entry" 2>/dev/null
    done
    return 0
}

# Removes an installation. A folder that still holds other files stays and keeps an
# empty manifest: it marks the folder as Clipboard's, so a later installation accepts it.
remove_installation() { # install dir
    remove_installed_files "$1"
    if [ -d "$1" ] && ! rmdir -- "$1" 2>/dev/null; then
        printf '' > "$1/$MANIFEST"
    fi
}

launch_app() { # app arguments
    [ -n "${DISPLAY:-}${WAYLAND_DISPLAY:-}" ] || return 0
    # The terminal may close as soon as the installer exits; the app must not go with it
    trap '' HUP
    if have setsid; then
        setsid "$APP" "$@" </dev/null >/dev/null 2>&1 &
    else
        nohup "$APP" "$@" </dev/null >/dev/null 2>&1 &
    fi
}

set_install_dir() { # install dir
    PREFIX=$1
    APP="$PREFIX/$APP_BINARY"
}

# ---------------------------------------------------------------- uninstall

uninstall() {
    if [ "${0##*/}" = "$UNINSTALLER" ] && [ -z "$PREFIX_ARG" ]; then
        PREFIX_ARG=$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd)
    fi
    [ -n "$PREFIX_ARG" ] || PREFIX_ARG=$(previous_install_dir)
    [ -n "$PREFIX_ARG" ] || die "$U_NOT_INSTALLED"
    set_install_dir "${PREFIX_ARG%/}"
    [ -s "$PREFIX/$MANIFEST" ] || die "$(printf "$U_NOT_FOUND" "$PREFIX")"

    if [ "$UI" = none ] && [ "$ASSUME_YES" != 1 ]; then
        die "$U_NEED_YES"
    fi
    if ! confirm "$(printf "$U_CONFIRM" "$PREFIX")" 0; then
        say "$U_CANCELLED"
        exit 1
    fi

    stop_app "$APP" || die "$E_STILL_RUNNING"

    # Shortcuts and login startup of this installation only
    belongs_to "$MENU_FILE" "$PREFIX" && rm -f -- "$MENU_FILE"
    shortcut="$(desktop_dir)/$DESKTOP_ID.desktop"
    belongs_to "$shortcut" "$PREFIX" && rm -f -- "$shortcut"
    belongs_to "$AUTOSTART_FILE" "$PREFIX" && rm -f -- "$AUTOSTART_FILE"

    remove_installation "$PREFIX"

    if [ "$PURGE" = 1 ]; then
        rm -f -- "$CONFIG_HOME/KNGC/ClipboardApp.conf"
        rm -rf -- "$CACHE_HOME/KNGC/ClipboardApp" "$DATA_HOME/KNGC/ClipboardApp"
        rmdir -- "$CONFIG_HOME/KNGC" "$CACHE_HOME/KNGC" "$DATA_HOME/KNGC" 2>/dev/null
        done_text=$U_DONE_PURGE
    else
        done_text=$U_DONE
    fi
    show_message info "$done_text"
    say "$done_text"
    exit 0
}

# ---------------------------------------------------------------- install

check_system() {
    arch=$(uname -m)
    [ "$arch" = x86_64 ] || die "$(printf "$E_ARCH" "$arch")"

    for tool in tar xz tail awk sed sort grep; do
        have "$tool" || die "$(printf "$E_TOOL" "$tool")"
    done

    glibc=$(getconf GNU_LIBC_VERSION 2>/dev/null | awk '{ print $2 }')
    case "$glibc" in
        [0-9]*.[0-9]*)
            awk -v have="$glibc" -v need="$MIN_GLIBC" 'BEGIN {
                split(have, h, "."); split(need, n, ".")
                exit !(h[1] + 0 > n[1] + 0 || (h[1] + 0 == n[1] + 0 && h[2] + 0 >= n[2] + 0))
            }' || die "$(printf "$E_GLIBC" "$MIN_GLIBC" "$glibc")" ;;
    esac
}

check_payload() {
    say "$T_CHECKING"
    if [ ! -f "$SELF" ] || [ ! -r "$SELF" ]; then die "$E_NO_PAYLOAD"; fi
    case "$PAYLOAD_LINE" in ''|*[!0-9]*) die "$E_NO_PAYLOAD" ;; esac
    if have sha256sum; then
        sum=$(tail -n +"$PAYLOAD_LINE" "$SELF" | sha256sum | awk '{ print $1 }')
    elif have shasum; then
        sum=$(tail -n +"$PAYLOAD_LINE" "$SELF" | shasum -a 256 | awk '{ print $1 }')
    else
        return 0
    fi
    [ "$sum" = "$PAYLOAD_SHA256" ] || die "$E_CORRUPT"
}

# Makes the path absolute and checks that the folder can be used
check_install_dir() { # path as entered
    dir=$1
    case "$dir" in
        \~) dir=$HOME ;;
        \~/*) dir="$HOME/${dir#\~/}" ;;
    esac
    case "$dir" in /*) ;; *) die "$E_DIR_RELATIVE" ;; esac
    case "$dir" in
        *'"'*|*'`'*|*'$'*|*\\*|*'%'*|*'
'*) die "$E_DIR_CHARS" ;;
    esac
    while [ "$dir" != / ] && [ "${dir%/}" != "$dir" ]; do dir=${dir%/}; done
    case "$dir" in
        /|"${HOME%/}"|"${DATA_HOME%/}") die "$(printf "$E_DIR_HOME" "$DATA_HOME/$PRODUCT")" ;;
    esac
    if [ -e "$dir" ] && [ ! -f "$dir/$MANIFEST" ]; then
        # An empty folder is fine; anything else may be the user's own data
        if [ ! -d "$dir" ] || [ -n "$(ls -A "$dir" 2>/dev/null)" ]; then
            die "$(printf "$E_DIR_BUSY" "$dir" "$dir/$PRODUCT")"
        fi
    fi
    set_install_dir "$dir"
}

ask_options() {
    case "$UI" in
        tty)
            printf '\n%s\n%s\n%s\n\n' "$T_TITLE" "$T_HERO" "$T_HERO2"
            if [ -z "$PREFIX_ARG" ]; then
                printf '%s [%s]: ' "$T_LOCATION" "$PREFIX"
                IFS= read -r answer || answer=''
                [ -n "$answer" ] && check_install_dir "$answer"
            else
                printf '%s: %s\n' "$T_LOCATION" "$PREFIX"
            fi
            if [ -z "$OPT_AUTOSTART" ]; then
                if ask_yes_no "$T_AUTOSTART?" "$AUTOSTART"; then AUTOSTART=1; else AUTOSTART=0; fi
            fi
            if [ -z "$OPT_DESKTOP" ]; then
                if ask_yes_no "$T_DESKTOP?" "$DESKTOP_ICON"; then DESKTOP_ICON=1; else DESKTOP_ICON=0; fi
            fi
            printf '\n' ;;
        zenity)
            if [ "$AUTOSTART" = 1 ]; then autostart_state=TRUE; else autostart_state=FALSE; fi
            if [ "$DESKTOP_ICON" = 1 ]; then desktop_state=TRUE; else desktop_state=FALSE; fi
            choice=$(zenity --list --checklist --title="$T_TITLE" \
                --text="$T_HERO
$T_HERO2

$T_LOCATION: $PREFIX" \
                --column='' --column='id' --column="$T_OPTION" --hide-column=2 --print-column=2 \
                --separator=' ' --hide-header --width=560 --height=440 \
                --ok-label="$T_INSTALL" --cancel-label="$T_CANCEL" \
                "$autostart_state" autostart "$T_AUTOSTART" \
                "$desktop_state" desktop "$T_DESKTOP" 2>/dev/null) || { say "$T_CANCELLED"; exit 1; }
            case "$choice" in *autostart*) AUTOSTART=1 ;; *) AUTOSTART=0 ;; esac
            case "$choice" in *desktop*) DESKTOP_ICON=1 ;; *) DESKTOP_ICON=0 ;; esac ;;
        kdialog)
            if [ "$AUTOSTART" = 1 ]; then autostart_state=on; else autostart_state=off; fi
            if [ "$DESKTOP_ICON" = 1 ]; then desktop_state=on; else desktop_state=off; fi
            choice=$(kdialog --title "$T_TITLE" --checklist "$T_HERO
$T_HERO2

$T_LOCATION: $PREFIX" \
                autostart "$T_AUTOSTART" "$autostart_state" \
                desktop "$T_DESKTOP" "$desktop_state" 2>/dev/null) || { say "$T_CANCELLED"; exit 1; }
            case "$choice" in *autostart*) AUTOSTART=1 ;; *) AUTOSTART=0 ;; esac
            case "$choice" in *desktop*) DESKTOP_ICON=1 ;; *) DESKTOP_ICON=0 ;; esac ;;
    esac
}

unpack() {
    parent=$(dirname -- "$PREFIX")
    mkdir -p -- "$parent" 2>/dev/null || die "$(printf "$E_DIR_CREATE" "$parent")"

    free_kb=$(df -Pk -- "$parent" 2>/dev/null | awk 'NR == 2 { print $4 }')
    need_kb=$((INSTALLED_KB + 20480))
    case "$free_kb" in
        ''|*[!0-9]*) ;;
        *) [ "$free_kb" -ge "$need_kb" ] || die "$(printf "$E_SPACE" "$parent" "$((need_kb / 1024))")" ;;
    esac

    TMP="$parent/.clipboard-setup.$$"
    rm -rf -- "$TMP"
    mkdir -m 700 -- "$TMP" 2>/dev/null || die "$(printf "$E_DIR_CREATE" "$TMP")"

    say "$T_UNPACKING"
    tail -n +"$PAYLOAD_LINE" "$SELF" | xz -dc 2>/dev/null | tar -xf - -C "$TMP" 2>/dev/null
    # shellcheck disable=SC2181
    if [ $? -ne 0 ] || [ ! -x "$TMP/$APP_BINARY" ] || [ ! -f "$TMP/$MANIFEST" ]; then
        die "$(printf "$E_EXTRACT" "$parent")"
    fi

    # The uninstaller is this script without the archive
    head -n "$((PAYLOAD_LINE - 1))" "$SELF" > "$TMP/$UNINSTALLER" || die "$(printf "$E_EXTRACT" "$parent")"
    chmod 755 "$TMP" "$TMP/$UNINSTALLER"
}

place_files() {
    if [ -e "$PREFIX" ]; then
        remove_installed_files "$PREFIX"
        rmdir -- "$PREFIX" 2>/dev/null
    fi
    if [ ! -e "$PREFIX" ]; then
        mv -- "$TMP" "$PREFIX" || die "$(printf "$E_EXTRACT" "$PREFIX")"
    else
        # The user's own files are still in the folder: add the app next to them
        cp -a -- "$TMP/." "$PREFIX/" || die "$(printf "$E_EXTRACT" "$PREFIX")"
        rm -rf -- "$TMP"
    fi
    TMP=''
}

add_shortcuts() {
    say "$T_SHORTCUTS"
    icon="$PREFIX/share/icons/hicolor/256x256/apps/$DESKTOP_ID.png"
    mkdir -p -- "$(dirname -- "$MENU_FILE")"
    awk -v exec_line="Exec=\"$APP\" --show" -v try_exec="TryExec=$APP" -v icon_line="Icon=$icon" \
        -v dir_line="X-Clipboard-InstallDir=$PREFIX" '
        /^Exec=/ { print exec_line; print try_exec; next }
        /^Icon=/ { print icon_line; next }
        { print }
        END { print dir_line }' "$PREFIX/share/applications/$DESKTOP_ID.desktop" > "$MENU_FILE.new" \
        && mv -- "$MENU_FILE.new" "$MENU_FILE"

    desktop=$(desktop_dir)
    shortcut="$desktop/$DESKTOP_ID.desktop"
    if [ "$DESKTOP_ICON" = 1 ]; then
        if [ -d "$desktop" ]; then
            cp -- "$MENU_FILE" "$shortcut" && chmod 755 "$shortcut"
            # GNOME starts a desktop file only when it is marked as trusted
            have gio && gio set "$shortcut" metadata::trusted true >/dev/null 2>&1
        else
            say "$T_NO_DESKTOP_DIR"
        fi
    elif belongs_to "$shortcut" "$PREFIX"; then
        rm -f -- "$shortcut"
    fi
}

# Sets login startup through the app itself, which also proves that the app can start here
apply_autostart() {
    if [ "$AUTOSTART" = 1 ]; then state=on; else state=off; fi
    start_error=$("$APP" "--autostart=$state" 2>&1 >/dev/null) && return 0

    text="$E_START"
    [ -n "$start_error" ] && text="$text
$start_error"
    missing=$(for file in "$APP" "$PREFIX/plugins/platforms/libqxcb.so"; do ldd "$file" 2>/dev/null; done \
        | awk '/not found/ { print $1 }' | sort -u | tr '\n' ' ')
    if [ -n "$missing" ]; then
        text="$text

$E_MISSING_LIBS $missing"
        packages=''
        if have apt-get; then
            packages='sudo apt install libgl1 libegl1 libopengl0 libfontconfig1 libfreetype6 libx11-6 libx11-xcb1 libxcb1'
        elif have dnf; then
            packages='sudo dnf install mesa-libGL mesa-libEGL libglvnd-opengl fontconfig freetype libX11 libX11-xcb libxcb'
        elif have pacman; then
            packages='sudo pacman -S --needed libglvnd fontconfig freetype2 libx11 libxcb'
        elif have zypper; then
            packages='sudo zypper install Mesa-libGL1 Mesa-libEGL1 libOpenGL0 fontconfig libfreetype6 libX11-6 libX11-xcb1 libxcb1'
        fi
        [ -n "$packages" ] && text="$text
$E_INSTALL_LIBS $packages"
    fi
    die "$text" 3
}

install() {
    [ "$(id -u)" != 0 ] || [ "$ALLOW_ROOT" = 1 ] || die "$E_ROOT"
    check_system

    previous=$(previous_install_dir)
    check_install_dir "${PREFIX_ARG:-${previous:-$DATA_HOME/$PRODUCT}}"

    # Updating keeps the earlier choices; both are off for a first installation
    AUTOSTART=0
    DESKTOP_ICON=0
    [ -f "$AUTOSTART_FILE" ] && AUTOSTART=1
    [ -f "$(desktop_dir)/$DESKTOP_ID.desktop" ] && DESKTOP_ICON=1
    [ -n "$OPT_AUTOSTART" ] && AUTOSTART=$OPT_AUTOSTART
    [ -n "$OPT_DESKTOP" ] && DESKTOP_ICON=$OPT_DESKTOP

    ask_options

    # A new location moves the installation: the earlier copy is removed afterwards
    moved_from=''
    [ -n "$previous" ] && [ "$previous" != "$PREFIX" ] && moved_from=$previous

    was_running=0
    if [ -n "$(running_pids "$APP")" ] || [ -n "$(running_pids "${moved_from:-$PREFIX}/$APP_BINARY")" ]; then
        was_running=1
        confirm "$T_RUNNING" 1 || { say "$T_CANCELLED"; exit 1; }
    fi

    progress_start "$T_INSTALLING"
    check_payload
    unpack
    stop_app "$APP" || die "$E_STILL_RUNNING"
    place_files
    if [ -n "$moved_from" ]; then
        stop_app "$moved_from/$APP_BINARY" || die "$E_STILL_RUNNING"
        remove_installation "$moved_from"
    fi
    add_shortcuts
    apply_autostart
    progress_stop

    summary="$T_DONE
$T_DONE2

$(printf "$T_INSTALLED_IN" "$PREFIX")
$T_HINT_OPEN"
    if [ "${XDG_SESSION_TYPE:-}" = wayland ]; then
        summary="$summary

$T_HINT_WAYLAND
\"$APP\" --toggle"
    fi
    summary="$summary

$(printf "$T_HINT_UNINSTALL" "$PREFIX/$UNINSTALLER")"
    say ''
    say "$summary"
    say ''

    open_now=0
    if [ "$UI" != none ] && [ "$NO_LAUNCH" != 1 ] && [ -n "${DISPLAY:-}${WAYLAND_DISPLAY:-}" ]; then
        if [ "$UI" = tty ]; then
            ask_yes_no "$T_LAUNCH" 1 && open_now=1
        else
            confirm "$summary

$T_LAUNCH" 1 "$T_OPEN" "$T_CLOSE" && open_now=1
        fi
    elif [ "$UI" = zenity ] || [ "$UI" = kdialog ]; then
        show_message info "$summary"
    fi

    if [ "$open_now" = 1 ]; then
        launch_app --show
    elif [ "$was_running" = 1 ]; then
        # Put the updated app back in the tray
        launch_app
    fi
    exit 0
}

# ---------------------------------------------------------------- start

if [ "$SHOW_HELP" = 1 ]; then
    printf '%s\n' "$T_HELP"
    exit 0
fi
[ -z "$BAD_ARG" ] || die "$(printf "$E_USAGE" "$BAD_ARG")" 2

if [ "$MODE" = uninstall ]; then
    uninstall
else
    install
fi
exit 0
# The application archive (tar.xz) starts on the next line.
