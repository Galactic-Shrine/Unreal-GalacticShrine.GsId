[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)][string]$EngineRoot,
    [Parameter(Mandatory = $true)][string]$OutputDirectory,
    [ValidateSet("Win64")][string]$TargetPlatform = "Win64",
    [string]$ExpectedVersion = "",
    [string]$ProjectFile = ""
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$PluginRoot = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
$ManifestPath = Join-Path $PluginRoot "GsId.uplugin"
$Manifest = Get-Content -Raw $ManifestPath | ConvertFrom-Json
$Version = [string]$Manifest.VersionName

if ($Version -notmatch '^\d+\.\d+\.\d+(?:[-+][0-9A-Za-z.-]+)?$') {
    throw "Invalid semantic VersionName in GsId.uplugin: '$Version'."
}
if ($ExpectedVersion -and $ExpectedVersion -ne $Version) {
    throw "Requested version '$ExpectedVersion' does not match GsId.uplugin '$Version'."
}

$EngineRoot = [IO.Path]::GetFullPath($EngineRoot)
$OutputDirectory = [IO.Path]::GetFullPath($OutputDirectory)
$EngineVersionFile = Join-Path $EngineRoot "Engine/Build/Build.version"
if (-not (Test-Path $EngineVersionFile)) {
    throw "Unreal Engine Build.version is missing from '$EngineRoot'."
}

$EngineVersionData = Get-Content -Raw $EngineVersionFile | ConvertFrom-Json
$EngineVersion = "$($EngineVersionData.MajorVersion).$($EngineVersionData.MinorVersion).$($EngineVersionData.PatchVersion)"
$Commit = $env:GITHUB_SHA
if ([string]::IsNullOrWhiteSpace($Commit)) {
    $GitRoot = (& git -C $PluginRoot rev-parse --show-toplevel 2>$null)
    if ($LASTEXITCODE -eq 0 -and $GitRoot -and
        [IO.Path]::GetFullPath(([string]$GitRoot).Trim()).TrimEnd('\') -ieq $PluginRoot.TrimEnd('\')) {
        $Commit = ((& git -C $PluginRoot rev-parse HEAD 2>$null) -join "").Trim()
    } else {
        $Commit = "unknown"
    }
}

$ArtifactName = "GsId-$Version-$TargetPlatform"
$PackageDirectory = Join-Path $OutputDirectory $ArtifactName
$ArchivePath = Join-Path $OutputDirectory "$ArtifactName.zip"
$ChecksumPath = "$ArchivePath.sha256"
$MetadataPath = Join-Path $OutputDirectory "$ArtifactName.build.json"

New-Item $OutputDirectory -ItemType Directory -Force | Out-Null
if (Test-Path $PackageDirectory) { Remove-Item $PackageDirectory -Recurse -Force }
foreach ($GeneratedFile in @($ArchivePath, $ChecksumPath, $MetadataPath)) {
    if (Test-Path $GeneratedFile) { Remove-Item $GeneratedFile -Force }
}

$BuildArguments = @{
    EngineRoot = $EngineRoot
    OutputDirectory = $PackageDirectory
    TargetPlatforms = @($TargetPlatform)
    DistributionChannel = "GitHub"
}
if ($ProjectFile) { $BuildArguments.ProjectFile = $ProjectFile }
& (Join-Path $PluginRoot "Scripts/Build-FabPackage.ps1") @BuildArguments

$BuildMetadata = [ordered]@{
    schemaVersion = 1
    plugin = "GsId"
    pluginVersion = $Version
    descriptorVersion = [int]$Manifest.Version
    unrealEngineVersion = $EngineVersion
    platform = $TargetPlatform
    commit = $Commit
    builtAtUtc = [DateTime]::UtcNow.ToString("o")
    workflowRunId = [string]$env:GITHUB_RUN_ID
    workflowRunAttempt = [string]$env:GITHUB_RUN_ATTEMPT
}
$MetadataJson = $BuildMetadata | ConvertTo-Json -Depth 4
$MetadataJson | Set-Content -Encoding utf8 (Join-Path $PackageDirectory "BUILD-INFO.json")
$MetadataJson | Set-Content -Encoding utf8 $MetadataPath

Compress-Archive -Path (Join-Path $PackageDirectory "*") -DestinationPath $ArchivePath -CompressionLevel Optimal
$Hash = (Get-FileHash -Algorithm SHA256 $ArchivePath).Hash.ToLowerInvariant()
"$Hash  $(Split-Path -Leaf $ArchivePath)" | Set-Content -Encoding ascii $ChecksumPath

if ($env:GITHUB_OUTPUT) {
    Add-Content $env:GITHUB_OUTPUT "version=$Version"
    Add-Content $env:GITHUB_OUTPUT "platform=$TargetPlatform"
    Add-Content $env:GITHUB_OUTPUT "archive=$ArchivePath"
    Add-Content $env:GITHUB_OUTPUT "checksum=$ChecksumPath"
    Add-Content $env:GITHUB_OUTPUT "metadata=$MetadataPath"
    Add-Content $env:GITHUB_OUTPUT "output_directory=$OutputDirectory"
}

Write-Host "Build artifact: $ArchivePath"
Write-Host "SHA-256: $Hash"
