# Windows installer

`Clipboard.iss` creates a self-contained, per-user x64 Windows installer using
Inno Setup 6.7 or newer. Its single configuration page follows Clipboard's
Windows styling: Segoe UI, the existing app icon, a neutral background, and the
system's light/dark appearance. Russian and English are selected automatically
from the Windows UI language; `/LANG=russian` or `/LANG=english` overrides this.

The installer includes the deployed Qt libraries and the MSVC runtime. It does
not download dependencies or require administrator rights. Windows 10 version
1809 or later is required.

## Build

Build and deploy the Windows app first, then run from the repository root:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File packaging\windows\build-installer.ps1
```

The default payload is `../ClipboardApp`, and the output is
`../Clipboard-Setup-1.0.0.exe` plus its `.sha256` file. Paths and the package
version can be overridden:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File packaging\windows\build-installer.ps1 `
  -PayloadDir D:\clipboard\build-msvc\installer-payload `
  -OutputDir D:\clipboard -Version 1.0.0
```

Use `-CompilerPath` for a nonstandard Inno Setup installation. Use `-Appearance
light` or `-Appearance dark` for visual verification; release builds default to
`dynamic`. Signing is not configured: a release certificate can be integrated
using Inno Setup's `SignTool` facility when one is available.

## Installation behavior

- Default location: `%LOCALAPPDATA%\Programs\Clipboard`.
- One configuration screen with the destination, optional login startup, and an
  optional desktop shortcut. Existing startup choices are read from Clipboard's
  settings. Both options are off for a first installation.
- A Start menu shortcut is always created. Shortcuts and the final launch action
  use `--show` to open the Clipboard window; login startup stays in the tray.
- Updates reuse the previous installation directory. Windows Restart Manager
  detects locked app files and offers to close the application during an update.
- The progress screen shows a percentage. The final screen explains `Alt+V` and
  offers to open Clipboard.
- Windows Settings → Apps can uninstall Clipboard. Uninstall removes installed
  files, shortcuts, and this installation's startup entry. App preferences and
  user-created files are preserved.

Standard Inno Setup command-line options are supported, including `/VERYSILENT`,
`/NORESTART`, `/DIR="..."`, and `/LOG="..."`. Optional settings can be set explicitly
with `/AUTOSTART=0|1` and `/DESKTOPICON=0|1`. Silent installation never launches the
application afterward.

## Verify

Build an isolated test package. It has its own application ID, shortcut names,
startup entry, and preference key; it cannot update the real Clipboard install.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File packaging\windows\build-installer.ps1 `
  -PayloadDir D:\clipboard\build-msvc\installer-payload `
  -OutputDir D:\clipboard\build-msvc\installer-verification -VerificationBuild

powershell -NoProfile -ExecutionPolicy Bypass -File packaging\windows\verify-installer.ps1 `
  -InstallerPath D:\clipboard\build-msvc\installer-verification\Clipboard-Setup-Verification.exe `
  -PayloadDir D:\clipboard\build-msvc\installer-payload `
  -WorkDir D:\clipboard\build-msvc\installer-smoke
```

`WorkDir` must not already exist. The smoke test performs a fresh installation,
an upgrade that repairs a deliberately modified installed file, SHA-256 checks
of every packaged file, shortcut/autostart checks, and uninstallation. A sentinel
file verifies preservation of user-created data. Test shortcuts, registry
entries, and installed app files are removed; logs and the sentinel remain in
`WorkDir` for inspection. The test application is never launched.

Also open the installer in both appearances and languages to review text,
keyboard navigation, folder selection, progress, and the completion page.
