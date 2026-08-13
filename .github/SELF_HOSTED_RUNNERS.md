# Personal and official Unreal Engine runners

The workflows in this repository never build on a contributor's machine unless that contributor explicitly registers and starts a self-hosted runner.

## Personal builds

1. Fork this repository.
2. In the fork, open **Settings → Actions → Runners → New self-hosted runner**.
3. Register the runner on your own fork, not on the upstream public repository.
4. Add the custom label `unreal-5.8` during registration.
5. Set `UE_ROOT_WINDOWS`, `UE_ROOT_LINUX`, or `UE_ROOT_MAC` under **Settings → Secrets and variables → Actions → Variables**.
6. Start the runner only when needed, then run **Build plugin on a personal runner** manually.

Recommended labels:

```text
Windows: self-hosted, windows, x64, unreal-5.8
Linux:   self-hosted, linux, x64, unreal-5.8
Mac:     self-hosted, macos, unreal-5.8
```

The Mac job deliberately omits an architecture label so that an Intel or Apple Silicon runner can accept it.

Use `--ephemeral` with `config.sh` or `config.cmd` when possible. An ephemeral runner accepts one job and automatically unregisters afterward. Do not configure a personal runner as an always-on service for this public repository.

The Windows, Linux, and Mac jobs use the native Unreal Engine installation for their host. A Linux runner inside WSL therefore requires a native Linux Unreal Engine installation accessible from WSL.

## Official releases

Official release runners require the additional `official-release` label and must be controlled by Galactic-Shrine maintainers. The `official-release` GitHub environment should require maintainer approval.

The release workflow exposes independent `Windows`, `Linux`, and `Mac` options. At least one platform must be selected. It verifies every selected artifact and SHA-256 checksum before creating `v<VersionName>` and the GitHub Release. When Windows and Linux are both selected, Linux waits for Windows so that Windows and WSL do not compete when both runners share one physical computer.

Any platform omitted from the initial release can complete that existing Release through the validated pull-request process documented in [`PlatformBuilds/README.md`](../PlatformBuilds/README.md).

## Optional FTPS mirror

`publish_remote` is disabled by default in the official release workflow. When it is not selected, no remote-server configuration is required and the GitHub Release is published normally.

To enable the mirror, configure these repository variables:

- `REMOTE_FTPS_HOST`
- `REMOTE_FTPS_PORT` (optional; defaults to `21`)
- `REMOTE_FTPS_BASE_PATH`

Configure these repository or `official-release` environment secrets:

- `REMOTE_FTPS_USER`
- `REMOTE_FTPS_PASSWORD`

Selected files are mirrored under `<base>/GsId/v<version>/` using explicit TLS, passive mode, and certificate verification. Set the repository variable `REMOTE_ARTIFACTS_ENABLED` to `true` only if platform builds accepted later through pull requests must also be mirrored.

The mirror job uses a dedicated Linux runner with the labels `self-hosted`, `linux`, `x64`, and `remote-ftps`; Unreal Engine is not required on this runner. This keeps the Free Pages Perso transfer on a maintainer-controlled connection in the supported region. A remote-mirror failure does not remove or invalidate the GitHub Release.

Use **Mirror an existing release to FTPS** to retry or add the mirror later without recompiling and without changing the existing tag.

Never expose Epic credentials, Unreal Engine source code, or an Unreal development image as a public workflow artifact.
