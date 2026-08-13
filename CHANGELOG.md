# Changelog

**Languages:** **English** · [Français](./CHANGELOG.fr.md)

All notable changes to `GsId` for Unreal Engine are documented in this file.

The format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and this project uses [Semantic Versioning](https://semver.org/).

## [1.0.0] — Initial release

### Added

- 256-bit Galactic-Shrine identifiers with the 32-byte Unreal `FGsId` representation;
- secure random generation on Windows 64-bit, Linux, and macOS, including the explicit `TryNewGsId` API;
- compact `N` and readable `D` text formats, strict parsing, and configurable letter case;
- raw-byte conversion, comparison, ordering, and stable 32-bit FNV-1a hashing;
- deterministic SHA-256 identifier generation;
- binary `FArchive`, network `NetSerialize`, and Unreal text serialization;
- native C++20, Unreal C++, and Blueprint APIs;
- native localization of Blueprint metadata in 13 cultures;
- shared `# Galactic Shrine|Gs Id` Blueprint category;
- automation test aligned with Unreal’s displayed `# Galactic Shrine` category;
- automation tests and cross-platform localization and release scripts;
- MIT licensing for the public GitHub source and releases, with a separate Fab Standard licensed package for Fab.

[1.0.0]: https://github.com/Galactic-Shrine/Unreal-GalacticShrine.GsId/releases/tag/v1.0.0
