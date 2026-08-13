[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)][string]$EngineRoot,
    [Parameter(Mandatory = $true)][string]$OutputDirectory,
    [string[]]$TargetPlatforms = @("Win64"),
    [string]$ProjectFile = "",
    [ValidateSet("Fab", "GitHub")][string]$DistributionChannel = "Fab"
)

$ErrorActionPreference = "Stop"
$EngineRoot = [IO.Path]::GetFullPath($EngineRoot)
$OutputDirectory = [IO.Path]::GetFullPath($OutputDirectory)
$PluginRoot = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
$PluginManifest = Join-Path $PluginRoot "GsId.uplugin"
$RunUat = Join-Path $EngineRoot "Engine/Build/BatchFiles/RunUAT.bat"
$Cultures = @("en","fr","de","es","it","pt","ru","ja","ko","zh-Hans","zh-Hant","ar","tr")
$Targets = @("GsId","GsIdEditor")

if ($ProjectFile) {
    & (Join-Path $PluginRoot "Scripts/Update-GsLocalization.ps1") -ProjectFile $ProjectFile -EngineRoot $EngineRoot
    if ($LASTEXITCODE -ne 0) { throw "Localization generation failed." }
}
foreach ($Required in @("README.md","LICENSE","THIRD_PARTY_NOTICES.md","CHANGELOG.md")) {
    if (-not (Test-Path (Join-Path $PluginRoot $Required))) { throw "Missing release file: $Required" }
}
foreach ($Target in $Targets) {
    $Root = Join-Path $PluginRoot "Content/Localization/$Target"
    if (-not (Test-Path (Join-Path $Root "$Target.locmeta"))) { throw "Missing $Target.locmeta" }
    foreach ($Culture in $Cultures) {
        $LocRes = Join-Path $Root "$Culture/$Target.locres"
        if (-not (Test-Path $LocRes) -or (Get-Item $LocRes).Length -eq 0) { throw "Missing localization resource: $LocRes" }
    }
}
foreach ($Forbidden in @("Binaries","Intermediate","Saved","DerivedDataCache",".vs")) {
    if (Test-Path (Join-Path $PluginRoot $Forbidden)) { throw "Forbidden release directory present: $Forbidden" }
}
& (Join-Path $PluginRoot "Scripts/Test-GsLocalization.ps1")
if (-not (Test-Path $RunUat)) { throw "RunUAT.bat is missing from '$EngineRoot'." }
$PlatformArgument = ($TargetPlatforms | Select-Object -Unique) -join "+"
New-Item $OutputDirectory -ItemType Directory -Force | Out-Null
& $RunUat BuildPlugin -Plugin="$PluginManifest" -Package="$OutputDirectory" -TargetPlatforms="$PlatformArgument"
if ($LASTEXITCODE -ne 0) { throw "Unreal BuildPlugin failed with exit code $LASTEXITCODE." }
if ($DistributionChannel -eq "Fab") {
    & (Join-Path $PluginRoot "Scripts/Set-FabPackageLicense.ps1") -PackageDirectory $OutputDirectory
}
Write-Host "$DistributionChannel package generated in: $OutputDirectory"
