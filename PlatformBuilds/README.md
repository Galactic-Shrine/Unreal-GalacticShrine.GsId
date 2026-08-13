# Contributing a missing platform build

Do not commit compiled ZIP archives to Git. Build the exact official tag on your own machine, publish the resulting ZIP as a stable GitHub Release asset in your fork, then open a pull request containing one JSON manifest.

Example: `PlatformBuilds/v1.0.0/Mac.json`

```json
{
  "schemaVersion": 1,
  "plugin": "GsId",
  "pluginVersion": "1.0.0",
  "unrealEngineVersion": "5.8.1",
  "platform": "Mac",
  "commit": "0123456789abcdef0123456789abcdef01234567",
  "archiveName": "GsId-1.0.0-Mac.zip",
  "downloadUrl": "https://github.com/USER/REPOSITORY/releases/download/v1.0.0-mac/GsId-1.0.0-Mac.zip",
  "sha256": "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef",
  "contributor": "USER"
}
```

Requirements:

- use `Scripts/Build-Plugin.ps1` or `Scripts/Build-Plugin.sh` from the official tag;
- use the Unreal platform name `Win64`, `Linux`, or `Mac` in filenames and metadata;
- build with Unreal Engine 5.8.x;
- publish from the exact commit referenced by the existing official `v<version>` tag;
- provide a stable HTTPS GitHub Release asset URL and its SHA-256;
- never include `HostProject`, `Saved`, `DerivedDataCache`, or `.git` data. The `Intermediate` metadata produced by `RunUAT BuildPlugin` is allowed in a precompiled package but remains excluded from the source repository.

For a private source asset, `downloadUrl` may use
`https://api.github.com/repos/OWNER/REPOSITORY/releases/assets/ASSET_ID`.
The repository secret `PLATFORM_ASSET_TOKEN` must then have read access to that asset.

The pull request workflow downloads and inspects the archive using the validator from the trusted base branch. Merging the pull request does not move or recreate the tag. After approval through the protected `official-release` environment, the ZIP, checksum, and build manifest are added to the existing GitHub Release.

Compiled code from external machines must receive maintainer review. Passing automated checks is necessary but is not proof that an arbitrary binary is trustworthy.
