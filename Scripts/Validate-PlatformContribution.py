#!/usr/bin/env python3
"""Validate a contributed precompiled Unreal Engine plugin archive."""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import pathlib
import re
import shutil
import subprocess
import sys
import urllib.parse
import urllib.request
import zipfile


SEMVER = re.compile(r"^[0-9]+\.[0-9]+\.[0-9]+(?:[-+][0-9A-Za-z.-]+)?$")
SHA256 = re.compile(r"^[0-9a-fA-F]{64}$")
COMMIT = re.compile(r"^[0-9a-fA-F]{40}$")
PLATFORMS = {"Win64", "Linux", "Mac"}
MAX_DOWNLOAD_BYTES = 2 * 1024 * 1024 * 1024
MAX_UNCOMPRESSED_BYTES = 4 * 1024 * 1024 * 1024
FORBIDDEN_PARTS = {"HostProject", "Saved", "DerivedDataCache", ".git"}


def fail(message: str) -> "NoReturn":
    raise SystemExit(message)


def load_json(path: pathlib.Path) -> dict:
    if path.is_symlink() or not path.is_file():
        fail(f"The contribution manifest must be a regular file: {path}")
    try:
        value = json.loads(path.read_text(encoding="utf-8-sig"))
    except (OSError, json.JSONDecodeError) as error:
        fail(f"Cannot read JSON {path}: {error}")
    if not isinstance(value, dict):
        fail(f"Expected a JSON object in {path}.")
    return value


def resolve_tag_commit(repository_root: pathlib.Path, version: str) -> str:
    result = subprocess.run(
        ["git", "-C", str(repository_root), "rev-list", "-n", "1", f"v{version}"],
        check=False,
        capture_output=True,
        text=True,
    )
    commit = result.stdout.strip()
    if result.returncode != 0 or not COMMIT.fullmatch(commit):
        fail(f"The official tag v{version} does not exist in the trusted repository checkout.")
    return commit.lower()


def download(url: str, destination: pathlib.Path) -> None:
    parsed = urllib.parse.urlparse(url)
    if parsed.scheme != "https" or parsed.hostname not in {"github.com", "api.github.com"}:
        fail("downloadUrl must be an HTTPS GitHub release asset URL.")
    if parsed.hostname == "github.com" and "/releases/download/" not in parsed.path:
        fail("downloadUrl must point to a stable GitHub Release asset.")
    if parsed.hostname == "api.github.com" and not re.fullmatch(
        r"/repos/[A-Za-z0-9_.-]+/[A-Za-z0-9_.-]+/releases/assets/[0-9]+", parsed.path
    ):
        fail("GitHub API URLs must point to one release asset by numeric ID.")

    headers = {"User-Agent": "Galactic-Shrine-platform-build-validator/1"}
    token = os.environ.get("PLATFORM_ASSET_TOKEN", "")
    if parsed.hostname == "api.github.com":
        headers["Accept"] = "application/octet-stream"
        if token:
            headers["Authorization"] = f"Bearer {token}"
    request = urllib.request.Request(url, headers=headers)
    total = 0
    try:
        with urllib.request.urlopen(request, timeout=60) as response, destination.open("wb") as stream:
            length = response.headers.get("Content-Length")
            if length and int(length) > MAX_DOWNLOAD_BYTES:
                fail("The contributed archive exceeds the 2 GiB download limit.")
            while block := response.read(1024 * 1024):
                total += len(block)
                if total > MAX_DOWNLOAD_BYTES:
                    fail("The contributed archive exceeds the 2 GiB download limit.")
                stream.write(block)
    except Exception as error:
        destination.unlink(missing_ok=True)
        fail(f"Cannot download contributed archive: {error}")


def validate_archive(
    archive: pathlib.Path,
    manifest: dict,
    expected_plugin: str,
    official_commit: str,
) -> None:
    try:
        package = zipfile.ZipFile(archive)
    except (OSError, zipfile.BadZipFile) as error:
        fail(f"Invalid ZIP archive: {error}")

    with package:
        files = [entry for entry in package.infolist() if not entry.is_dir()]
        if not files:
            fail("The contributed ZIP archive is empty.")
        if sum(entry.file_size for entry in files) > MAX_UNCOMPRESSED_BYTES:
            fail("The contributed archive exceeds the 4 GiB uncompressed limit.")

        normalized: dict[str, zipfile.ZipInfo] = {}
        for entry in files:
            name = entry.filename.replace("\\", "/")
            path = pathlib.PurePosixPath(name)
            if path.is_absolute() or ".." in path.parts or any(part in FORBIDDEN_PARTS for part in path.parts):
                fail(f"Unsafe or forbidden archive path: {entry.filename}")
            if ((entry.external_attr >> 16) & 0o170000) == 0o120000:
                fail(f"Symbolic links are not allowed in contributed archives: {entry.filename}")
            normalized[name] = entry

        descriptor_name = f"{expected_plugin}.uplugin"
        descriptors = [name for name in normalized if pathlib.PurePosixPath(name).name == descriptor_name]
        if len(descriptors) != 1:
            fail(f"Expected exactly one {descriptor_name} in the archive.")
        descriptor = json.loads(package.read(normalized[descriptors[0]]).decode("utf-8-sig"))
        if str(descriptor.get("VersionName")) != manifest["pluginVersion"]:
            fail("The archive descriptor version does not match the contribution manifest.")
        supported = descriptor.get("SupportedTargetPlatforms", [])
        if manifest["platform"] not in supported:
            fail("The archive descriptor does not declare the contributed platform.")

        binary_marker = f"Binaries/{manifest['platform']}/"
        if not any(binary_marker in f"/{name}" for name in normalized):
            fail(f"The archive does not contain compiled {manifest['platform']} binaries.")

        build_infos = [name for name in normalized if pathlib.PurePosixPath(name).name == "BUILD-INFO.json"]
        if len(build_infos) != 1:
            fail("Expected exactly one BUILD-INFO.json in the archive.")
        build_info = json.loads(package.read(normalized[build_infos[0]]).decode("utf-8-sig"))
        expected = {
            "plugin": expected_plugin,
            "pluginVersion": manifest["pluginVersion"],
            "unrealEngineVersion": manifest["unrealEngineVersion"],
            "platform": manifest["platform"],
            "commit": official_commit,
        }
        for key, value in expected.items():
            if str(build_info.get(key, "")).lower() != str(value).lower():
                fail(f"BUILD-INFO.json field {key!r} does not match the official contribution metadata.")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--manifest", required=True, type=pathlib.Path)
    parser.add_argument("--plugin", required=True)
    parser.add_argument("--repository-root", required=True, type=pathlib.Path)
    parser.add_argument("--output", required=True, type=pathlib.Path)
    parser.add_argument("--github-output", type=pathlib.Path)
    args = parser.parse_args()

    manifest = load_json(args.manifest)
    required = {
        "schemaVersion", "plugin", "pluginVersion", "unrealEngineVersion", "platform",
        "commit", "archiveName", "downloadUrl", "sha256", "contributor",
    }
    missing = sorted(required - manifest.keys())
    if missing:
        fail(f"Missing contribution fields: {', '.join(missing)}")
    if manifest["schemaVersion"] != 1 or manifest["plugin"] != args.plugin:
        fail("Unsupported schemaVersion or unexpected plugin name.")
    version = str(manifest["pluginVersion"])
    platform = str(manifest["platform"])
    if not SEMVER.fullmatch(version) or platform not in PLATFORMS:
        fail("Invalid pluginVersion or platform.")
    if not re.fullmatch(r"5\.8(?:\.\d+)?", str(manifest["unrealEngineVersion"])):
        fail("The contribution must be compiled with Unreal Engine 5.8.x.")
    expected_archive = f"{args.plugin}-{version}-{platform}.zip"
    if manifest["archiveName"] != expected_archive:
        fail(f"archiveName must be {expected_archive}.")
    if not SHA256.fullmatch(str(manifest["sha256"])) or not COMMIT.fullmatch(str(manifest["commit"])):
        fail("Invalid SHA-256 or source commit.")
    if not str(manifest["contributor"]).strip():
        fail("The contributor field cannot be empty.")

    official_commit = resolve_tag_commit(args.repository_root, version)
    if str(manifest["commit"]).lower() != official_commit:
        fail(f"The contributed binary was not built from the commit tagged v{version}.")

    args.output.mkdir(parents=True, exist_ok=True)
    archive = args.output / expected_archive
    download(str(manifest["downloadUrl"]), archive)
    hasher = hashlib.sha256()
    with archive.open("rb") as stream:
        while block := stream.read(1024 * 1024):
            hasher.update(block)
    digest = hasher.hexdigest()
    if digest.lower() != str(manifest["sha256"]).lower():
        fail("The downloaded archive SHA-256 does not match the contribution manifest.")
    validate_archive(archive, manifest, args.plugin, official_commit)

    checksum = args.output / f"{expected_archive}.sha256"
    checksum.write_text(f"{digest}  {expected_archive}\n", encoding="ascii")
    metadata = args.output / f"{expected_archive[:-4]}.build.json"
    shutil.copyfile(args.manifest, metadata)

    if args.github_output:
        with args.github_output.open("a", encoding="utf-8") as stream:
            stream.write(f"version={version}\nplatform={platform}\ntag=v{version}\n")
            stream.write(f"archive={archive}\nchecksum={checksum}\nmetadata={metadata}\n")
    print(f"Validated {expected_archive} ({digest}) from official tag v{version}.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
