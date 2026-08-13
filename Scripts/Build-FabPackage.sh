#!/usr/bin/env bash
set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)"
PLUGIN_ROOT="$(cd "$SCRIPT_DIR/.." && pwd -P)"

usage() {
    cat <<'EOF'
Usage:
  ./Build-FabPackage.sh \
      --engine-root <UERoot> \
      --output <directory> \
      [--platform Linux|Mac] \
      [--project <Project.uproject>] \
      [--distribution Fab|GitHub]
EOF
}

ENGINE_ROOT=""
OUTPUT_DIRECTORY=""
PLATFORM=""
PROJECT_FILE=""
DISTRIBUTION_CHANNEL="Fab"
while (($#)); do
    case "$1" in
        --engine-root) ENGINE_ROOT="${2:?}"; shift 2 ;;
        --output) OUTPUT_DIRECTORY="${2:?}"; shift 2 ;;
        --platform) PLATFORM="${2:?}"; shift 2 ;;
        --project) PROJECT_FILE="${2:?}"; shift 2 ;;
        --distribution) DISTRIBUTION_CHANNEL="${2:?}"; shift 2 ;;
        -h|--help) usage; exit 0 ;;
        *) printf 'Unknown argument: %s\n' "$1" >&2; usage >&2; exit 2 ;;
    esac
done
[[ -n "$ENGINE_ROOT" && -n "$OUTPUT_DIRECTORY" ]] || { usage >&2; exit 2; }
[[ "$DISTRIBUTION_CHANNEL" == "Fab" || "$DISTRIBUTION_CHANNEL" == "GitHub" ]] || {
    printf 'Distribution must be Fab or GitHub.\n' >&2
    exit 2
}
ENGINE_ROOT="$(cd "$ENGINE_ROOT" && pwd -P)"
case "$(uname -s)" in
    Linux*) DETECTED_PLATFORM="Linux" ;;
    Darwin*) DETECTED_PLATFORM="Mac" ;;
    *) printf 'This script supports Linux and macOS.\n' >&2; exit 1 ;;
esac
[[ -n "$PLATFORM" ]] || PLATFORM="$DETECTED_PLATFORM"
[[ "$PLATFORM" == "$DETECTED_PLATFORM" ]] || {
    printf 'Requested platform %s does not match host %s.\n' "$PLATFORM" "$DETECTED_PLATFORM" >&2
    exit 2
}
RUN_UAT="$ENGINE_ROOT/Engine/Build/BatchFiles/RunUAT.sh"
[[ -x "$RUN_UAT" ]] || { printf 'RunUAT.sh is missing or not executable: %s\n' "$RUN_UAT" >&2; exit 1; }
if [[ -n "$PROJECT_FILE" ]]; then
    "$SCRIPT_DIR/Update-GsLocalization.sh" --project "$PROJECT_FILE" --engine-root "$ENGINE_ROOT"
fi
for required in README.md LICENSE THIRD_PARTY_NOTICES.md CHANGELOG.md; do
    [[ -f "$PLUGIN_ROOT/$required" ]] || { printf 'Missing release file: %s\n' "$required" >&2; exit 1; }
done
cultures=(en fr de es it pt ru ja ko zh-Hans zh-Hant ar tr)
targets=(GsId GsIdEditor)
for target in "${targets[@]}"; do
    root="$PLUGIN_ROOT/Content/Localization/$target"
    [[ -s "$root/$target.locmeta" ]] || { printf 'Missing %s.locmeta\n' "$target" >&2; exit 1; }
    for culture in "${cultures[@]}"; do
        [[ -s "$root/$culture/$target.locres" ]] || {
            printf 'Missing localization resource: %s/%s/%s.locres\n' "$root" "$culture" "$target" >&2
            exit 1
        }
    done
done
for forbidden in Binaries Intermediate Saved DerivedDataCache .vs; do
    [[ ! -e "$PLUGIN_ROOT/$forbidden" ]] || { printf 'Forbidden release directory present: %s\n' "$forbidden" >&2; exit 1; }
done
"$SCRIPT_DIR/Test-GsLocalization.sh"
mkdir -p "$OUTPUT_DIRECTORY"
"$RUN_UAT" BuildPlugin \
    -Plugin="$PLUGIN_ROOT/GsId.uplugin" \
    -Package="$OUTPUT_DIRECTORY" \
    -TargetPlatforms="$PLATFORM"
if [[ "$DISTRIBUTION_CHANNEL" == "Fab" ]]; then
    "$SCRIPT_DIR/Set-FabPackageLicense.sh" "$OUTPUT_DIRECTORY"
fi
printf '%s package generated in: %s\n' "$DISTRIBUTION_CHANNEL" "$OUTPUT_DIRECTORY"
