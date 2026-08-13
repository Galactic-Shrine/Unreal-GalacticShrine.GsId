[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)][string]$ProjectFile,
    [Parameter(Mandatory = $true)][string]$EngineRoot,
    [ValidateSet('All', 'Gather', 'Import', 'Compile', 'Export')][string]$Action = 'All',
    [ValidateSet('All', 'GsId', 'GsIdEditor')][string]$Target = 'All'
)

$ErrorActionPreference = 'Stop'
$PluginRoot = Split-Path -Parent $PSScriptRoot
$ProjectFile = (Resolve-Path -LiteralPath $ProjectFile).Path
$EngineRoot = (Resolve-Path -LiteralPath $EngineRoot).Path
$EditorCmd = Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
if (-not (Test-Path -LiteralPath $EditorCmd)) { throw "UnrealEditor-Cmd.exe introuvable : $EditorCmd" }
$ProjectDirectory = Split-Path -Parent $ProjectFile
$TemporaryDirectory = Join-Path $ProjectDirectory 'Saved\GsId\Localization'
New-Item -ItemType Directory -Force -Path $TemporaryDirectory | Out-Null
$Targets = if ($Target -eq 'All') { @('GsId', 'GsIdEditor') } else { @($Target) }
$Actions = if ($Action -eq 'All') { @('Gather', 'Export', 'ApplyMetadataTranslations', 'Import', 'Compile', 'Export') } else { @($Action) }
$NormalizedPluginRoot = $PluginRoot.Replace('\', '/')
foreach ($CurrentTarget in $Targets) {
    foreach ($CurrentAction in $Actions) {
        if ($CurrentAction -eq 'ApplyMetadataTranslations') {
            if ($CurrentTarget -eq 'GsIdEditor') { & (Join-Path $PSScriptRoot 'Apply-GsMetadataTranslations.ps1') -TargetRoot (Join-Path $PluginRoot 'Content\Localization\GsIdEditor') -TargetName 'GsIdEditor' }
            continue
        }
        $Template = Join-Path $PluginRoot "Config\Localization\${CurrentTarget}_${CurrentAction}.ini"
        if (-not (Test-Path -LiteralPath $Template)) { throw "Configuration de localisation introuvable : $Template" }
        $TemporaryConfig = Join-Path $TemporaryDirectory "${CurrentTarget}_${CurrentAction}.ini"
        $Content = Get-Content -LiteralPath $Template -Raw -Encoding UTF8
        $Content = $Content.Replace('__GS_PLUGIN_ROOT__', $NormalizedPluginRoot)
        [System.IO.File]::WriteAllText($TemporaryConfig, $Content, [System.Text.UTF8Encoding]::new($false))
        Write-Host "[GsId] Localization: $CurrentTarget / $CurrentAction"
        & $EditorCmd $ProjectFile '-run=GatherText' "-config=$TemporaryConfig" '-unattended' '-nop4'
        if ($LASTEXITCODE -ne 0) { throw "Le commandlet de localisation a échoué avec le code $LASTEXITCODE." }
    }
}
