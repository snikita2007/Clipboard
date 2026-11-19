[CmdletBinding()]
param(
    [string] $PayloadDir,
    [string] $OutputDir,
    [ValidatePattern('^\d+\.\d+\.\d+(\.\d+)?$')]
    [string] $Version = '1.0.0',
    [string] $CompilerPath,
    [ValidateSet('dynamic', 'light', 'dark')]
    [string] $Appearance = 'dynamic',
    [switch] $VerificationBuild
)

$ErrorActionPreference = 'Stop'
$repositoryRoot = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..\..')).Path
$workspaceRoot = Split-Path -Parent $repositoryRoot
if (-not $PayloadDir) { $PayloadDir = Join-Path $workspaceRoot 'ClipboardApp' }
if (-not $OutputDir) { $OutputDir = $workspaceRoot }
$payloadPath = (Resolve-Path -LiteralPath $PayloadDir).Path

foreach ($requiredFile in @('ClipboardApp.exe', 'Qt6Core.dll', 'Qt6Gui.dll', 'Qt6Quick.dll',
    'Qt6Bluetooth.dll', 'Qt6Network.dll', 'platforms\qwindows.dll', 'vcruntime140.dll')) {
    if (-not (Test-Path -LiteralPath (Join-Path $payloadPath $requiredFile) -PathType Leaf)) {
        throw "Incomplete Windows deployment: missing $requiredFile. Build and deploy Clipboard first."
    }
}
if (-not $CompilerPath) {
    $compilerCandidates = @(
        (Join-Path ${env:ProgramFiles(x86)} 'Inno Setup 6\ISCC.exe'),
        (Join-Path $env:ProgramFiles 'Inno Setup 6\ISCC.exe'),
        (Join-Path $env:ProgramFiles 'Inno Setup 7\ISCC.exe')
    )
    $CompilerPath = $compilerCandidates | Where-Object { Test-Path -LiteralPath $_ -PathType Leaf } | Select-Object -First 1
}
if (-not $CompilerPath) { throw 'Inno Setup 6.7 or newer is required. Supply -CompilerPath if it is installed elsewhere.' }
$compiler = (Resolve-Path -LiteralPath $CompilerPath).Path
New-Item -ItemType Directory -Force -Path $OutputDir | Out-Null
$outputPath = (Resolve-Path -LiteralPath $OutputDir).Path

$arguments = @('/Qp', "/DAppVersion=$Version", "/DPayloadDir=$payloadPath", "/DOutputDir=$outputPath",
    "/DAppearance=$Appearance")
if ($VerificationBuild) { $arguments += '/DVerificationBuild' }
$arguments += Join-Path $PSScriptRoot 'Clipboard.iss'
& $compiler @arguments
if ($LASTEXITCODE -ne 0) { throw "Inno Setup failed with exit code $LASTEXITCODE." }

$fileName = if ($VerificationBuild) { 'Clipboard-Setup-Verification.exe' } else { "Clipboard-Setup-$Version.exe" }
$installerPath = Join-Path $outputPath $fileName
$hash = (Get-FileHash -LiteralPath $installerPath -Algorithm SHA256).Hash.ToLowerInvariant()
[IO.File]::WriteAllText("$installerPath.sha256", "$hash  $fileName`r`n", [Text.UTF8Encoding]::new($false))
Write-Output "Installer: $installerPath"
Write-Output "SHA256: $hash"
