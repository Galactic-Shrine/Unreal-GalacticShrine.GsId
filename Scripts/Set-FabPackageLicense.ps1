[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)][string]$PackageDirectory
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$PluginRoot = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
$TemplateRoot = Join-Path $PluginRoot "Packaging/Fab"
$PackageDirectory = [IO.Path]::GetFullPath($PackageDirectory)

if (-not (Test-Path -LiteralPath $PackageDirectory -PathType Container)) {
    throw "Fab package directory does not exist: '$PackageDirectory'."
}

$Manifests = @(Get-ChildItem -LiteralPath $PackageDirectory -Filter "GsId.uplugin" -File -Recurse)
if ($Manifests.Count -ne 1) {
    throw "Expected exactly one GsId.uplugin in '$PackageDirectory', found $($Manifests.Count)."
}

$PackagedPluginRoot = $Manifests[0].Directory.FullName
$Descriptor = Get-Content -Raw -LiteralPath $Manifests[0].FullName | ConvertFrom-Json
$PluginVersion = [string]$Descriptor.VersionName
if ($PluginVersion -notmatch '^\d+\.\d+\.\d+(?:[-+][0-9A-Za-z.-]+)?$') {
    throw "Invalid semantic VersionName in the packaged GsId.uplugin: '$PluginVersion'."
}
$ResolvedPackageRoot = [IO.Path]::GetFullPath($PackageDirectory).TrimEnd('\', '/')
$ResolvedPluginRoot = [IO.Path]::GetFullPath($PackagedPluginRoot).TrimEnd('\', '/')
if (-not ($ResolvedPluginRoot -eq $ResolvedPackageRoot -or $ResolvedPluginRoot.StartsWith("$ResolvedPackageRoot$([IO.Path]::DirectorySeparatorChar)", [StringComparison]::OrdinalIgnoreCase))) {
    throw "Resolved plugin root is outside the requested package directory."
}

foreach ($Pattern in @("README*.md", "THIRD_PARTY_NOTICES*.md", "CHANGELOG*.md")) {
    Get-ChildItem -LiteralPath $PackagedPluginRoot -Filter $Pattern -File -ErrorAction SilentlyContinue |
        Remove-Item -Force
}
foreach ($RelativePath in @(
    "LICENSE",
    "Source/ThirdParty/GalacticShrine/GsId/LICENSE.md",
    "Source/ThirdParty/GalacticShrine/GsId/README.md",
    "Scripts",
    "Packaging"
)) {
    $Target = Join-Path $PackagedPluginRoot $RelativePath
    if (Test-Path -LiteralPath $Target) {
        Remove-Item -LiteralPath $Target -Recurse -Force
    }
}

foreach ($RequiredTemplate in @("README.md", "LICENSE", "THIRD_PARTY_NOTICES.md")) {
    $Source = Join-Path $TemplateRoot $RequiredTemplate
    if (-not (Test-Path -LiteralPath $Source -PathType Leaf)) {
        throw "Missing Fab legal template: '$Source'."
    }
    $Destination = Join-Path $PackagedPluginRoot $RequiredTemplate
    $TemplateContent = Get-Content -Raw -LiteralPath $Source
    $TemplateContent.Replace('__GSID_VERSION__', $PluginVersion) |
        Set-Content -LiteralPath $Destination -Encoding utf8
}

if (Select-String -LiteralPath (Join-Path $PackagedPluginRoot "README.md") -SimpleMatch '__GSID_VERSION__' -Quiet) {
    throw "The generated Fab README still contains an unresolved version placeholder."
}

$TextExtensions = @(".md", ".txt", ".ini", ".json", ".uplugin", ".cs", ".h", ".hpp", ".cpp")
$ConflictingTerms = 'Mozilla Public License|MPL-2\.0|MIT License'
$Conflicts = @(
    Get-ChildItem -LiteralPath $PackagedPluginRoot -File -Recurse |
        Where-Object { $TextExtensions -contains $_.Extension.ToLowerInvariant() } |
        Select-String -Pattern $ConflictingTerms
)
if ($Conflicts.Count -gt 0) {
    $Locations = ($Conflicts | ForEach-Object { "$($_.Path):$($_.LineNumber)" }) -join ", "
    throw "The Fab package still contains conflicting public-license terms: $Locations"
}

$FabLicense = Get-Content -Raw -LiteralPath (Join-Path $PackagedPluginRoot "LICENSE")
if ($FabLicense -notmatch 'Fab Standard License') {
    throw "The generated Fab package does not contain the Fab Standard License notice."
}

Write-Host "Applied the Fab Standard License documents to: $PackagedPluginRoot"
