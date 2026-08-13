[CmdletBinding()]
param()
$ErrorActionPreference='Stop'
$Root=Split-Path -Parent $PSScriptRoot
$Seed=Join-Path $Root 'Content\Localization\GsIdEditor\GsIdEditor.metadata.tsv'
$Rows=Get-Content -LiteralPath $Seed -Encoding UTF8 | Where-Object {$_ -and -not $_.StartsWith('#')}
$MetadataSource=Join-Path $Root 'Source\GsIdEditor\Private\Localization\GsIdEditorLocalizedMetadata.cpp'
$MetadataSourceText=Get-Content -LiteralPath $MetadataSource -Raw -Encoding UTF8
if($MetadataSourceText -notmatch [regex]::Escape('#include "UObject/UObjectGlobals.h"')){throw 'Missing Unreal Engine 5.8 CoreUObject delegate header.'}
if($MetadataSourceText -match [regex]::Escape('#include "UObject/CoreUObjectDelegates.h"')){throw 'Obsolete CoreUObject delegate header detected.'}
if($MetadataSourceText -notmatch [regex]::Escape('TEXT("GalacticShrine")')){throw 'Shared GalacticShrine root key is missing.'}
if($MetadataSourceText -notmatch [regex]::Escape('FText::FromString(TEXT("# GalacticShrine"))')){throw 'Prefixed GalacticShrine root is missing.'}
if($MetadataSourceText -match [regex]::Escape('FText::FromString(TEXT("Galactic-Shrine"))')){throw 'Legacy hyphenated category root remains.'}
if($MetadataSourceText -match [regex]::Escape('\u2060')){throw 'Obsolete U+2060 WORD JOINER remains in the category source.'}
if($MetadataSourceText -match [regex]::Escape('\u2010')){throw 'Obsolete U+2010 HYPHEN remains in the category source.'}
if($MetadataSourceText -notmatch [regex]::Escape('LOCTEXT("Category_GsId_Galactic_Shrine_GsId", "GSID")')){throw 'GSID leaf category is missing.'}
if(-not($Rows | Where-Object {($_ -split "`t")[1] -eq 'GSID'})){throw 'GSID category is missing from the translation seed.'}
if($MetadataSourceText -notmatch [regex]::Escape('FindFunctionByName(TEXT("TryNewGsId"))')){throw 'TryNewGsId localized metadata is missing.'}
if($MetadataSourceText -notmatch [regex]::Escape('LOCTEXT("Function_UGsIdBlueprintLibrary_TryNewGsId_DisplayName", "Try New GsId")')){throw 'TryNewGsId display-name metadata is missing.'}
if($MetadataSourceText -notmatch [regex]::Escape('TEXT("OutGsId")')){throw 'TryNewGsId output pin metadata is missing.'}
if(-not($Rows | Where-Object {$_ -eq "fr`tTry New GsId`tEssayer de créer un GsId"})){throw 'French TryNewGsId translation is missing.'}
$Cultures=@('fr','de','es','it','pt','ru','ja','ko','zh-Hans','zh-Hant','ar','tr')
foreach($Culture in $Cultures){if(-not($Rows | Where-Object {$_.StartsWith("$Culture`t")})){throw "Missing culture: $Culture"}}
Write-Host "[GsLocalization] seed rows: $($Rows.Count)"
Write-Host '[GsLocalization] validation succeeded.'
