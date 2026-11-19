# Linux installer

`build-installer.sh` builds Clipboard for Linux (x86_64) and produces two packages:

- `Clipboard-Setup-<version>-linux-x86_64.run`: a self-extracting installer.
  It installs Clipboard for the current user and does not need administrator rights.
- `Clipboard-<version>-linux-x86_64.AppImage`: a portable build that runs without
  installation.

Both include Qt and the other libraries the app needs, and download nothing.
They run on distributions with glibc 2.34 or newer (Ubuntu 22.04, Debian 12,
Fedora 35, RHEL 9 and later) in an X11 session, or a Wayland session with XWayland.
The basic graphics libraries of a desktop system (OpenGL, X11, fontconfig, freetype)
come from the system.

## Build

The packages are built on Ubuntu 22.04, so that they also run on newer systems.
Prepare the build machine once, as root:

```sh
sudo packaging/linux/setup-build-env.sh
```

It installs the compiler, Qt 6.9.3 in `/opt/Qt` and linuxdeploy in `/opt/linuxdeploy`.
Then build from the repository root:

```sh
packaging/linux/build-installer.sh
```

The script compiles the app, runs the regression tests, collects the libraries
and writes the packages with their `.sha256` files next to the repository
folder. Options:

| Option | Meaning |
| --- | --- |
| `--version X.Y.Z` | Package version, `1.0.0` by default |
| `--output-dir DIR` | Where the packages are written |
| `--build-dir DIR` | Build folder; it must be on a Linux file system |
| `--qt-dir DIR`, `--tools-dir DIR` | Qt and linuxdeploy in other locations |
| `--skip-tests` | Do not build and run the regression tests |
| `--no-appimage` | Build only the installer |

### Building on Windows

Use a WSL distro with Ubuntu 22.04. The sources stay on the Windows drive; the
build folder is inside the distro, because a Windows drive does not keep Linux
file modes. A small distro only for building can be made from the
[Ubuntu Base image](https://cdimage.ubuntu.com/ubuntu-base/releases/22.04/release/)
(`ubuntu-base-22.04.5-base-amd64.tar.gz`):

```bat
wsl --import ClipboardBuild D:\wsl\ClipboardBuild ubuntu-base-22.04.5-base-amd64.tar.gz
wsl -d ClipboardBuild -- bash /mnt/d/clipboard/Clipboard/packaging/linux/setup-build-env.sh
```

After that, `build-linux.cmd` next to the repository folder builds both packages:

```bat
D:\clipboard\build-linux.cmd
D:\clipboard\build-linux.cmd --version 1.0.1 --skip-tests
```

## Installation behavior

Run the installer in a terminal, or from a file manager ("Run as a program"):

```sh
sh Clipboard-Setup-1.0.0-linux-x86_64.run
```

In a terminal it asks its questions there. Started without a terminal, it shows
dialogs through `zenity` or `kdialog`; if neither is installed, it installs with
the default options. Russian or English is chosen from the system language.

- Default location: `~/.local/share/Clipboard`. A folder that already holds
  other files is refused.
- Options: login startup and a desktop shortcut. Both are off for a first
  installation; an update keeps the earlier choices.
- An application menu entry is always created. It opens the Clipboard window;
  login startup stays in the tray.
- An update reuses the previous location, closes the running app and starts it
  again afterwards. Files that the user put into the installation folder are kept.
  Choosing another location moves the installation.
- After copying the files the installer starts the app once without a window to
  set login startup. If the app cannot start, it lists the missing system
  libraries and the command that installs them.
- `~/.local/share/Clipboard/uninstall.sh` removes the app, its menu entry,
  desktop shortcut and login startup. Settings are kept unless `--purge` is given.

Command-line options:

| Option | Meaning |
| --- | --- |
| `--prefix=DIR` | Install location |
| `--autostart=0\|1` | Login startup |
| `--desktop-icon=0\|1` | Desktop shortcut |
| `-y`, `--yes` | Ask nothing; the app is not opened afterwards |
| `--no-launch` | Do not offer to open the app afterwards |
| `--lang=ru\|en` | Installer language |
| `--uninstall` | Remove the installed app; add `--purge` to remove settings and cache too |
| `--allow-root` | Allow running as root (containers); normally the installer refuses |

## The app on Linux

- Only one copy runs per user. Starting the app again controls the running copy:
  `ClipboardApp --show` opens the window and `ClipboardApp --toggle` opens or
  hides it.
- The app uses X11, because the global hotkey, opening at the mouse cursor and
  clipboard tracking in the background need it. In a Wayland session it runs
  through XWayland, where `Alt+V` works only while an X11 window is active. For
  a hotkey that works everywhere, add a custom keyboard shortcut in system
  settings with the command
  `"<install location>/bin/ClipboardApp" --toggle`.
- The tray icon needs a desktop that shows tray icons. GNOME needs the
  AppIndicator extension (Ubuntu has it by default).
- `ClipboardApp --autostart=on|off` sets login startup without opening the app;
  the installer uses it.

## Verify

`verify-installer.sh` tests a built installer in an isolated home folder. It
does not touch the real home folder or a real installation, and never opens the
app window.

```sh
packaging/linux/verify-installer.sh Clipboard-Setup-1.0.0-linux-x86_64.run /tmp/clipboard-verify
```

The work folder must not exist. The test performs a fresh installation, an
update that repairs deliberately damaged files, a change of options, a move to a
folder with a space in its name, and uninstallation. It compares every installed
file with the archive, checks the menu entry, desktop shortcut and login startup,
and checks that a file created by the user in the installation folder survives.
Logs stay in the work folder.

Also start the installer from a file manager on GNOME and KDE to review the
dialogs, and check the tray icon and `Alt+V` in X11 and Wayland sessions.
