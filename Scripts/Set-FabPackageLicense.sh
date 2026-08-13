#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)"
PLUGIN_ROOT="$(cd "$SCRIPT_DIR/.." && pwd -P)"
TEMPLATE_ROOT="$PLUGIN_ROOT/Packaging/Fab"

if [[ $# -ne 1 ]]; then
    printf 'Usage: %s <package-directory>\n' "$0" >&2
    exit 2
fi

PACKAGE_DIRECTORY="$1"
[[ -d "$PACKAGE_DIRECTORY" ]] || {
    printf 'Fab package directory does not exist: %s\n' "$PACKAGE_DIRECTORY" >&2
    exit 1
}
PACKAGE_DIRECTORY="$(cd "$PACKAGE_DIRECTORY" && pwd -P)"

mapfile -d '' -t manifests < <(find "$PACKAGE_DIRECTORY" -type f -name GsId.uplugin -print0)
[[ "${#manifests[@]}" -eq 1 ]] || {
    printf 'Expected exactly one GsId.uplugin in %s, found %s.\n' "$PACKAGE_DIRECTORY" "${#manifests[@]}" >&2
    exit 1
}

PACKAGED_PLUGIN_ROOT="$(cd "$(dirname "${manifests[0]}")" && pwd -P)"
case "$PACKAGED_PLUGIN_ROOT" in
    "$PACKAGE_DIRECTORY"|"$PACKAGE_DIRECTORY"/*) ;;
    *) printf 'Resolved plugin root is outside the requested package directory.\n' >&2; exit 1 ;;
esac

PLUGIN_VERSION="$(python3 - "${manifests[0]}" <<'PY'
import json
import pathlib
import re
import sys

version = str(json.loads(pathlib.Path(sys.argv[1]).read_text(encoding="utf-8-sig"))["VersionName"])
if not re.fullmatch(r"\d+\.\d+\.\d+(?:[-+][0-9A-Za-z.-]+)?", version):
    raise SystemExit(f"Invalid semantic VersionName in the packaged GsId.uplugin: {version!r}.")
print(version)
PY
)"

find "$PACKAGED_PLUGIN_ROOT" -maxdepth 1 -type f \
    \( -name 'README*.md' -o -name 'THIRD_PARTY_NOTICES*.md' -o -name 'CHANGELOG*.md' \) \
    -delete
rm -f -- \
    "$PACKAGED_PLUGIN_ROOT/LICENSE" \
    "$PACKAGED_PLUGIN_ROOT/Source/ThirdParty/GalacticShrine/GsId/LICENSE.md" \
    "$PACKAGED_PLUGIN_ROOT/Source/ThirdParty/GalacticShrine/GsId/README.md"
rm -rf -- "$PACKAGED_PLUGIN_ROOT/Scripts" "$PACKAGED_PLUGIN_ROOT/Packaging"

for required_template in README.md LICENSE THIRD_PARTY_NOTICES.md; do
    [[ -f "$TEMPLATE_ROOT/$required_template" ]] || {
        printf 'Missing Fab legal template: %s\n' "$TEMPLATE_ROOT/$required_template" >&2
        exit 1
    }
    sed "s/__GSID_VERSION__/$PLUGIN_VERSION/g" \
        "$TEMPLATE_ROOT/$required_template" > "$PACKAGED_PLUGIN_ROOT/$required_template"
done

if grep -Fq '__GSID_VERSION__' "$PACKAGED_PLUGIN_ROOT/README.md"; then
    printf 'The generated Fab README still contains an unresolved version placeholder.\n' >&2
    exit 1
fi

if grep -RIlE \
    --include='*.md' --include='*.txt' --include='*.ini' --include='*.json' \
    --include='*.uplugin' --include='*.cs' --include='*.h' --include='*.hpp' --include='*.cpp' \
    'Mozilla Public License|MPL-2\.0|MIT License' "$PACKAGED_PLUGIN_ROOT" >/dev/null; then
    printf 'The Fab package still contains conflicting public-license terms.\n' >&2
    grep -RInE \
        --include='*.md' --include='*.txt' --include='*.ini' --include='*.json' \
        --include='*.uplugin' --include='*.cs' --include='*.h' --include='*.hpp' --include='*.cpp' \
        'Mozilla Public License|MPL-2\.0|MIT License' "$PACKAGED_PLUGIN_ROOT" >&2
    exit 1
fi

grep -q 'Fab Standard License' "$PACKAGED_PLUGIN_ROOT/LICENSE" || {
    printf 'The generated Fab package does not contain the Fab Standard License notice.\n' >&2
    exit 1
}

printf 'Applied the Fab Standard License documents to: %s\n' "$PACKAGED_PLUGIN_ROOT"
