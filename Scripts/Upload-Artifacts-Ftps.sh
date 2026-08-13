#!/usr/bin/env bash
set -euo pipefail

usage() {
    cat <<'EOF'
Usage: Upload-Artifacts-Ftps.sh --plugin <name> --version <x.y.z> --source <directory>

Required environment variables:
  REMOTE_FTPS_HOST
  REMOTE_FTPS_USER
  REMOTE_FTPS_PASSWORD
  REMOTE_FTPS_BASE_PATH

Optional environment variable:
  REMOTE_FTPS_PORT (default: 21)
EOF
}

PLUGIN=""
VERSION=""
SOURCE_DIRECTORY=""
while (($#)); do
    case "$1" in
        --plugin) PLUGIN="${2:?}"; shift 2 ;;
        --version) VERSION="${2:?}"; shift 2 ;;
        --source) SOURCE_DIRECTORY="${2:?}"; shift 2 ;;
        -h|--help) usage; exit 0 ;;
        *) printf 'Unknown argument: %s\n' "$1" >&2; usage >&2; exit 2 ;;
    esac
done

[[ "$PLUGIN" =~ ^[A-Za-z0-9._-]+$ ]] || { printf 'Invalid plugin name.\n' >&2; exit 2; }
[[ "$VERSION" =~ ^[0-9]+\.[0-9]+\.[0-9]+([-+][0-9A-Za-z.-]+)?$ ]] || {
    printf 'Invalid semantic version: %s\n' "$VERSION" >&2
    exit 2
}
[[ -d "$SOURCE_DIRECTORY" ]] || { printf 'Artifact directory does not exist: %s\n' "$SOURCE_DIRECTORY" >&2; exit 2; }
SOURCE_DIRECTORY="$(cd "$SOURCE_DIRECTORY" && pwd -P)"

: "${REMOTE_FTPS_HOST:?REMOTE_FTPS_HOST is required}"
: "${REMOTE_FTPS_USER:?REMOTE_FTPS_USER is required}"
: "${REMOTE_FTPS_PASSWORD:?REMOTE_FTPS_PASSWORD is required}"
: "${REMOTE_FTPS_BASE_PATH:?REMOTE_FTPS_BASE_PATH is required}"
REMOTE_FTPS_PORT="${REMOTE_FTPS_PORT:-21}"

[[ "$REMOTE_FTPS_HOST" =~ ^[A-Za-z0-9.-]+$ ]] || { printf 'Invalid FTPS host.\n' >&2; exit 2; }
[[ "$REMOTE_FTPS_USER" != *$'\n'* && "$REMOTE_FTPS_PASSWORD" != *$'\n'* ]] || {
    printf 'FTPS credentials cannot contain newlines.\n' >&2
    exit 2
}
[[ "$REMOTE_FTPS_PORT" =~ ^[0-9]{1,5}$ ]] && ((REMOTE_FTPS_PORT >= 1 && REMOTE_FTPS_PORT <= 65535)) || {
    printf 'Invalid FTPS port.\n' >&2
    exit 2
}
[[ "$REMOTE_FTPS_BASE_PATH" =~ ^/[A-Za-z0-9._/-]+$ && "$REMOTE_FTPS_BASE_PATH" != *..* ]] || {
    printf 'REMOTE_FTPS_BASE_PATH must be a safe absolute path.\n' >&2
    exit 2
}

shopt -s nullglob
artifacts=("$SOURCE_DIRECTORY"/*.zip "$SOURCE_DIRECTORY"/*.zip.sha256 "$SOURCE_DIRECTORY"/*.build.json)
[[ "${#artifacts[@]}" -gt 0 && $((${#artifacts[@]} % 3)) -eq 0 ]] || {
    printf 'Expected one ZIP, checksum, and build manifest per platform.\n' >&2
    exit 1
}
for artifact in "${artifacts[@]}"; do
    name="$(basename "$artifact")"
    valid=false
    for platform in Win64 Linux Mac; do
        for suffix in .zip .zip.sha256 .build.json; do
            [[ "$name" != "$PLUGIN-$VERSION-$platform$suffix" ]] || valid=true
        done
    done
    [[ "$valid" == true ]] || { printf 'Unexpected artifact name: %s\n' "$name" >&2; exit 1; }
done

remote_directory="${REMOTE_FTPS_BASE_PATH%/}/$PLUGIN/v$VERSION"
for artifact in "${artifacts[@]}"; do
    name="$(basename "$artifact")"
    size="$(wc -c < "$artifact" | tr -d '[:space:]')"
    trace_file="$(mktemp)"
    printf 'Uploading %s (%s bytes)...\n' "$name" "$size"

    set +e
    curl \
        --proto '=ftp' \
        --ssl-reqd \
        --ftp-pasv \
        --ftp-create-dirs \
        --fail \
        --show-error \
        --silent \
        --trace-ascii "$trace_file" \
        --retry 3 \
        --retry-all-errors \
        --user "$REMOTE_FTPS_USER:$REMOTE_FTPS_PASSWORD" \
        --upload-file "$artifact" \
        "ftp://$REMOTE_FTPS_HOST:$REMOTE_FTPS_PORT$remote_directory/$name"
    upload_status=$?
    set -e

    if ((upload_status != 0)); then
        sed -nE 's/^[0-9a-f]+: (5[0-9]{2}[- ].*)$/FTP server: \1/p' "$trace_file" >&2 || true
        rm -f "$trace_file"
        exit "$upload_status"
    fi

    rm -f "$trace_file"
done

printf 'Mirrored %s artifact files over FTPS to %s/%s/v%s.\n' \
    "${#artifacts[@]}" "${REMOTE_FTPS_BASE_PATH%/}" "$PLUGIN" "$VERSION"
