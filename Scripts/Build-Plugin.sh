#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)"
PLUGIN_ROOT="$(cd "$SCRIPT_DIR/.." && pwd -P)"

usage() {
    cat <<'EOF'
Usage:
  ./Scripts/Build-Plugin.sh \
      --engine-root <UERoot> \
      --output <directory> \
      [--platform Linux|Mac] \
      [--expected-version <x.y.z>] \
      [--project <Project.uproject>]
EOF
}

ENGINE_ROOT=""
OUTPUT_DIRECTORY=""
PLATFORM=""
EXPECTED_VERSION=""
PROJECT_FILE=""
while (($#)); do
    case "$1" in
        --engine-root) ENGINE_ROOT="${2:?}"; shift 2 ;;
        --output) OUTPUT_DIRECTORY="${2:?}"; shift 2 ;;
        --platform) PLATFORM="${2:?}"; shift 2 ;;
        --expected-version) EXPECTED_VERSION="${2:?}"; shift 2 ;;
        --project) PROJECT_FILE="${2:?}"; shift 2 ;;
        -h|--help) usage; exit 0 ;;
        *) printf 'Unknown argument: %s\n' "$1" >&2; usage >&2; exit 2 ;;
    esac
done
[[ -n "$ENGINE_ROOT" && -n "$OUTPUT_DIRECTORY" ]] || { usage >&2; exit 2; }

ENGINE_ROOT="$(cd "$ENGINE_ROOT" && pwd -P)"
mkdir -p "$OUTPUT_DIRECTORY"
OUTPUT_DIRECTORY="$(cd "$OUTPUT_DIRECTORY" && pwd -P)"
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

readarray -t VERSION_DATA < <(python3 - "$PLUGIN_ROOT/GsId.uplugin" "$ENGINE_ROOT/Engine/Build/Build.version" <<'PY'
import json
import pathlib
import sys

manifest_path = pathlib.Path(sys.argv[1])
engine_path = pathlib.Path(sys.argv[2])
if not engine_path.is_file():
    raise SystemExit(f"Unreal Engine Build.version is missing: {engine_path}")
manifest = json.loads(manifest_path.read_text(encoding="utf-8-sig"))
engine = json.loads(engine_path.read_text(encoding="utf-8-sig"))
print(manifest["VersionName"])
print(manifest["Version"])
print(f'{engine["MajorVersion"]}.{engine["MinorVersion"]}.{engine["PatchVersion"]}')
PY
)
VERSION="${VERSION_DATA[0]}"
DESCRIPTOR_VERSION="${VERSION_DATA[1]}"
ENGINE_VERSION="${VERSION_DATA[2]}"
[[ "$VERSION" =~ ^[0-9]+\.[0-9]+\.[0-9]+([-+][0-9A-Za-z.-]+)?$ ]] || {
    printf 'Invalid semantic VersionName in GsId.uplugin: %s\n' "$VERSION" >&2
    exit 1
}
[[ -z "$EXPECTED_VERSION" || "$EXPECTED_VERSION" == "$VERSION" ]] || {
    printf 'Requested version %s does not match GsId.uplugin %s.\n' "$EXPECTED_VERSION" "$VERSION" >&2
    exit 1
}

COMMIT="${GITHUB_SHA:-}"
if [[ -z "$COMMIT" ]]; then
    GIT_ROOT="$(git -C "$PLUGIN_ROOT" rev-parse --show-toplevel 2>/dev/null || true)"
    if [[ -n "$GIT_ROOT" && "$(cd "$GIT_ROOT" && pwd -P)" == "$PLUGIN_ROOT" ]]; then
        COMMIT="$(git -C "$PLUGIN_ROOT" rev-parse HEAD)"
    else
        COMMIT="unknown"
    fi
fi
ARTIFACT_NAME="GsId-$VERSION-$PLATFORM"
PACKAGE_DIRECTORY="$OUTPUT_DIRECTORY/$ARTIFACT_NAME"
ARCHIVE_PATH="$OUTPUT_DIRECTORY/$ARTIFACT_NAME.zip"
CHECKSUM_PATH="$ARCHIVE_PATH.sha256"
METADATA_PATH="$OUTPUT_DIRECTORY/$ARTIFACT_NAME.build.json"
rm -rf -- "$PACKAGE_DIRECTORY"
rm -f -- "$ARCHIVE_PATH" "$CHECKSUM_PATH" "$METADATA_PATH"

BUILD_ARGS=(--engine-root "$ENGINE_ROOT" --output "$PACKAGE_DIRECTORY" --platform "$PLATFORM" --distribution GitHub)
[[ -z "$PROJECT_FILE" ]] || BUILD_ARGS+=(--project "$PROJECT_FILE")
bash "$SCRIPT_DIR/Build-FabPackage.sh" "${BUILD_ARGS[@]}"

python3 - "$PACKAGE_DIRECTORY/BUILD-INFO.json" "$METADATA_PATH" <<PY
import json
import os
import pathlib
import sys
from datetime import datetime, timezone

metadata = {
    "schemaVersion": 1,
    "plugin": "GsId",
    "pluginVersion": "${VERSION}",
    "descriptorVersion": int("${DESCRIPTOR_VERSION}"),
    "unrealEngineVersion": "${ENGINE_VERSION}",
    "platform": "${PLATFORM}",
    "commit": "${COMMIT}",
    "builtAtUtc": datetime.now(timezone.utc).isoformat(),
    "workflowRunId": os.environ.get("GITHUB_RUN_ID", ""),
    "workflowRunAttempt": os.environ.get("GITHUB_RUN_ATTEMPT", ""),
}
payload = json.dumps(metadata, indent=2, ensure_ascii=False) + "\n"
for value in sys.argv[1:]:
    pathlib.Path(value).write_text(payload, encoding="utf-8")
PY

python3 - "$PACKAGE_DIRECTORY" "$ARCHIVE_PATH" <<'PY'
import pathlib
import sys
import zipfile

source = pathlib.Path(sys.argv[1])
archive = pathlib.Path(sys.argv[2])
with zipfile.ZipFile(archive, "w", compression=zipfile.ZIP_DEFLATED, compresslevel=9) as output:
    for path in sorted(source.rglob("*")):
        if path.is_file():
            output.write(path, path.relative_to(source))
PY

(
    cd "$OUTPUT_DIRECTORY"
    sha256sum "$(basename "$ARCHIVE_PATH")" > "$(basename "$CHECKSUM_PATH")"
)

if [[ -n "${GITHUB_OUTPUT:-}" ]]; then
    {
        printf 'version=%s\n' "$VERSION"
        printf 'platform=%s\n' "$PLATFORM"
        printf 'archive=%s\n' "$ARCHIVE_PATH"
        printf 'checksum=%s\n' "$CHECKSUM_PATH"
        printf 'metadata=%s\n' "$METADATA_PATH"
        printf 'output_directory=%s\n' "$OUTPUT_DIRECTORY"
    } >> "$GITHUB_OUTPUT"
fi

printf 'Build artifact: %s\n' "$ARCHIVE_PATH"
printf 'SHA-256: %s\n' "$(cut -d' ' -f1 "$CHECKSUM_PATH")"
