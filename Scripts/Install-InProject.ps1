[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$ProjectRoot
)

$ErrorActionPreference = "Stop"
$ProjectRoot = [System.IO.Path]::GetFullPath($ProjectRoot)
$SourcePlugin = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
$PluginsDirectory = Join-Path $ProjectRoot "Plugins"
$Destination = Join-Path $PluginsDirectory "GsId"

if (-not (Test-Path (Join-Path $SourcePlugin "GsId.uplugin"))) {
    throw "The source plugin manifest is missing."
}

New-Item $PluginsDirectory -ItemType Directory -Force | Out-Null
if (Test-Path $Destination) {
    Remove-Item $Destination -Recurse -Force
}
Copy-Item $SourcePlugin $Destination -Recurse -Force

foreach ($BuildDirectory in @("Binaries", "Intermediate")) {
    $Path = Join-Path $Destination $BuildDirectory
    if (Test-Path $Path) { Remove-Item $Path -Recurse -Force }
}

Write-Host "GsId installed in: $Destination"
