#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
seed="$root/Content/Localization/GsIdEditor/GsIdEditor.metadata.tsv"
metadata_source="$root/Source/GsIdEditor/Private/Localization/GsIdEditorLocalizedMetadata.cpp"
grep -Fq '#include "UObject/UObjectGlobals.h"' "$metadata_source" || { echo 'Missing Unreal Engine 5.8 CoreUObject delegate header.' >&2; exit 1; }
if grep -Fq '#include "UObject/CoreUObjectDelegates.h"' "$metadata_source"; then echo 'Obsolete CoreUObject delegate header detected.' >&2; exit 1; fi
grep -Fq 'TEXT("GalacticShrine")' "$metadata_source" || { echo 'Shared GalacticShrine root key is missing.' >&2; exit 1; }
grep -Fq 'FText::FromString(TEXT("# GalacticShrine"))' "$metadata_source" || { echo 'Prefixed GalacticShrine root is missing.' >&2; exit 1; }
if grep -Fq 'FText::FromString(TEXT("Galactic-Shrine"))' "$metadata_source"; then echo 'Legacy hyphenated category root remains.' >&2; exit 1; fi
if grep -Fq '\u2060' "$metadata_source"; then echo 'Obsolete U+2060 WORD JOINER remains in the category source.' >&2; exit 1; fi
if grep -Fq '\u2010' "$metadata_source"; then echo 'Obsolete U+2010 HYPHEN remains in the category source.' >&2; exit 1; fi
grep -Fq 'LOCTEXT("Category_GsId_Galactic_Shrine_GsId", "GSID")' "$metadata_source" || { echo 'GSID leaf category is missing.' >&2; exit 1; }
grep -Fq $'fr\tGSID\tGSID' "$seed" || { echo 'GSID category is missing from the translation seed.' >&2; exit 1; }
grep -Fq 'FindFunctionByName(TEXT("TryNewGsId"))' "$metadata_source" || { echo 'TryNewGsId localized metadata is missing.' >&2; exit 1; }
grep -Fq 'LOCTEXT("Function_UGsIdBlueprintLibrary_TryNewGsId_DisplayName", "Try New GsId")' "$metadata_source" || { echo 'TryNewGsId display-name metadata is missing.' >&2; exit 1; }
grep -Fq 'TEXT("OutGsId")' "$metadata_source" || { echo 'TryNewGsId output pin metadata is missing.' >&2; exit 1; }
grep -Fq $'fr\tTry New GsId\tEssayer de créer un GsId' "$seed" || { echo 'French TryNewGsId translation is missing.' >&2; exit 1; }
for c in fr de es it pt ru ja ko zh-Hans zh-Hant ar tr; do grep -q "^$c[[:space:]]" "$seed" || { echo "Missing culture: $c" >&2; exit 1; }; done
printf '[GsLocalization] validation succeeded.\n'
