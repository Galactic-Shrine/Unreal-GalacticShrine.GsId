#!/usr/bin/env bash
set -euo pipefail
if [[ $# -lt 2 ]]; then echo "Usage: $0 <Project.uproject> <EngineRoot> [Action] [Target]" >&2; exit 2; fi
project_file=$(cd "$(dirname "$1")" && pwd)/$(basename "$1")
engine_root=$(cd "$2" && pwd)
action=${3:-All}
target=${4:-All}
plugin_root=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
editor_cmd="$engine_root/Engine/Binaries/Linux/UnrealEditor-Cmd"
if [[ "$(uname -s)" == "Darwin" ]]; then editor_cmd="$engine_root/Engine/Binaries/Mac/UnrealEditor-Cmd"; fi
[[ -x "$editor_cmd" ]] || { echo "UnrealEditor-Cmd not found: $editor_cmd" >&2; exit 1; }
tmp_dir="$(dirname "$project_file")/Saved/GsId/Localization"
mkdir -p "$tmp_dir"
if [[ "$target" == "All" ]]; then targets=('GsId' 'GsIdEditor'); else targets=("$target"); fi
if [[ "$action" == "All" ]]; then actions=(Gather Export ApplyMetadataTranslations Import Compile Export); else actions=("$action"); fi
for current_target in "${targets[@]}"; do
  for current_action in "${actions[@]}"; do
    if [[ "$current_action" == "ApplyMetadataTranslations" ]]; then
      if [[ "$current_target" == "GsIdEditor" ]]; then "$plugin_root/Scripts/Apply-GsMetadataTranslations.sh" "$plugin_root/Content/Localization/GsIdEditor" "GsIdEditor"; fi
      continue
    fi
    template="$plugin_root/Config/Localization/${current_target}_${current_action}.ini"
    tmp="$tmp_dir/${current_target}_${current_action}.ini"
    sed "s#__GS_PLUGIN_ROOT__#${plugin_root//\/#}#g" "$template" > "$tmp"
    printf '[GsId] Localization: %s / %s\n' "$current_target" "$current_action"
    "$editor_cmd" "$project_file" -run=GatherText -config="$tmp" -unattended -nop4
  done
done
