[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string] $InstallerPath,
    [Parameter(Mandatory = $true)]
    [string] $PayloadDir,
    [Parameter(Mandatory = $true)]
    [string] $WorkDir
)

$ErrorActionPreference = 'Stop'
$installer = (Resolve-Path -LiteralPath $InstallerPath).Path
if ([IO.Path]::GetFileName($installer) -ne 'Clipboard-Setup-Verification.exe') {
    throw 'Use a -VerificationBuild installer; the production installer must not be smoke-tested this way.'
}
$payload = (Resolve-Path -LiteralPath $PayloadDir).Path
$testSettings = 'HKCU:\Software\KNGC\ClipboardInstallerTest'
$testUninstall = 'HKCU:\Software\Microsoft\Windows\CurrentVersion\Uninstall\{805F60F9-DF05-4875-A196-072C0B2B404B}_is1'
$runKey = 'HKCU:\Software\Microsoft\Windows\CurrentVersion\Run'
$desktopShortcut = Join-Path ([Environment]::GetFolderPath('DesktopDirectory')) 'Clipboard Installer Test.lnk'
$startShortcut = Join-Path ([Environment]::GetFolderPath('Programs')) 'Clipboard Installer Test.lnk'
foreach ($path in @($testSettings, $testUninstall, $desktopShortcut, $startShortcut)) {
    if (Test-Path -LiteralPath $path) { throw "A previous test artifact exists: $path. Review it before running this test." }
}
if (Get-ItemProperty -LiteralPath $runKey -Name ClipboardInstallerTest -ErrorAction SilentlyContinue) {
    throw 'A previous test Run entry exists. Review it before running this test.'
}
if (Test-Path -LiteralPath $WorkDir) { throw 'WorkDir must be a new, dedicated test directory.' }
New-Item -ItemType Directory -Path $WorkDir | Out-Null
$testRoot = (Resolve-Path -LiteralPath $WorkDir).Path
$installDir = Join-Path $testRoot 'Installed Clipboard'
$uninstaller = Join-Path $installDir 'unins000.exe'

function Invoke-InstallerProcess([string] $FilePath, [string[]] $Arguments) {
    $process = Start-Process -FilePath $FilePath -ArgumentList $Arguments -PassThru -Wait -WindowStyle Hidden
    if ($process.ExitCode -ne 0) { throw "Installer process failed with exit code $($process.ExitCode). See logs in $testRoot." }
}

$commonArguments = @('/VERYSILENT', '/SUPPRESSMSGBOXES', '/NORESTART', '/NOCLOSEAPPLICATIONS',
    '/NORESTARTAPPLICATIONS', '/LANG=english', ('/DIR="' + $installDir + '"'))
$success = $false
try {
    Write-Output 'Checking a fresh per-user installation...'
    Invoke-InstallerProcess $installer ($commonArguments + @('/AUTOSTART=0', '/DESKTOPICON=0', ('/LOG="' + (Join-Path $testRoot 'install.log') + '"')))
    if (-not (Test-Path -LiteralPath $uninstaller)) { throw 'Uninstaller was not created.' }
    if (-not (Test-Path -LiteralPath $startShortcut)) { throw 'Start menu shortcut was not created.' }
    if (Test-Path -LiteralPath $desktopShortcut) { throw 'Desktop shortcut was created despite being disabled.' }
    if (Get-ItemProperty -LiteralPath $runKey -Name ClipboardInstallerTest -ErrorAction SilentlyContinue) { throw 'Autostart was enabled despite being disabled.' }
    if (-not (Test-Path -LiteralPath $testUninstall)) { throw 'Windows uninstall registration is missing.' }

    $shell = New-Object -ComObject WScript.Shell
    $shortcut = $shell.CreateShortcut($startShortcut)
    if ($shortcut.TargetPath -ne (Join-Path $installDir 'ClipboardApp.exe') -or $shortcut.Arguments -ne '--show') {
        throw 'Start menu shortcut does not open the installed Clipboard UI.'
    }

    $sentinel = Join-Path $installDir 'user-created-note.txt'
    [IO.File]::WriteAllText($sentinel, 'User-created files must survive uninstall.')
    [IO.File]::WriteAllText((Join-Path $installDir 'Qt6Network.dll'), 'stale deployment file')
    Write-Output 'Checking upgrade, shortcuts, autostart, and all packaged bytes...'
    Invoke-InstallerProcess $installer ($commonArguments + @('/AUTOSTART=1', '/DESKTOPICON=1', ('/LOG="' + (Join-Path $testRoot 'upgrade.log') + '"')))
    $runValue = (Get-ItemProperty -LiteralPath $runKey -Name ClipboardInstallerTest).ClipboardInstallerTest
    if ($runValue -ne ('"' + (Join-Path $installDir 'ClipboardApp.exe') + '"')) { throw 'Autostart command is not correctly quoted.' }
    if (-not (Test-Path -LiteralPath $desktopShortcut)) { throw 'Desktop shortcut is missing.' }
    if ((Get-ItemProperty -LiteralPath ($testSettings + '\ApplicationSettings')).launchAtStartup -ne 'true') { throw 'App startup preference was not synchronized.' }
    $count = 0
    foreach ($source in Get-ChildItem -LiteralPath $payload -Recurse -File) {
        if ($source.Extension -in @('.pdb', '.log')) { continue }
        $relative = $source.FullName.Substring($payload.Length).TrimStart('\')
        $destination = Join-Path $installDir $relative
        if (-not (Test-Path -LiteralPath $destination) -or
            (Get-FileHash -LiteralPath $source.FullName -Algorithm SHA256).Hash -ne
            (Get-FileHash -LiteralPath $destination -Algorithm SHA256).Hash) { throw "Payload mismatch: $relative" }
        $count++
    }
    Write-Output "Verified $count packaged files."

    Write-Output 'Checking uninstall and preservation of user-created files...'
    Invoke-InstallerProcess $uninstaller @('/VERYSILENT', '/SUPPRESSMSGBOXES', '/NORESTART', ('/LOG="' + (Join-Path $testRoot 'uninstall.log') + '"'))
    foreach ($path in @((Join-Path $installDir 'ClipboardApp.exe'), (Join-Path $installDir 'Qt6Core.dll'),
        $startShortcut, $desktopShortcut, $testUninstall)) {
        if (Test-Path -LiteralPath $path) { throw "Uninstall left an owned artifact: $path" }
    }
    if (Get-ItemProperty -LiteralPath $runKey -Name ClipboardInstallerTest -ErrorAction SilentlyContinue) { throw 'Uninstall left the autostart entry.' }
    if (-not (Test-Path -LiteralPath $sentinel)) { throw 'Uninstall removed a user-created file.' }
    if (-not (Test-Path -LiteralPath $testSettings)) { throw 'Uninstall unexpectedly removed preferences.' }
    $success = $true
    Write-Output "PASS: fresh install, upgrade, integrity, shortcuts, autostart, and uninstall. Logs: $testRoot"
} finally {
    # Only this script's isolated test installation is eligible for cleanup.
    if (-not $success -and (Test-Path -LiteralPath $uninstaller)) {
        Invoke-InstallerProcess $uninstaller @('/VERYSILENT', '/SUPPRESSMSGBOXES', '/NORESTART')
    }
    # The preflight checks guarantee this exact registry key was created by this test.
    if (Test-Path -LiteralPath $testSettings) { Remove-Item -LiteralPath $testSettings -Recurse -Force }
}
