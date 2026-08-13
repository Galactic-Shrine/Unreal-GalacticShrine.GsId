# GalacticShrine GsId for Unreal Engine

**Languages:** [Français](./README.md) · **English** · [Español](./README.es.md) · [Italiano](./README.it.md) · [日本語](./README.jp.md)

[![Version](https://img.shields.io/badge/version-1.0.0-4c9aff)](https://github.com/Galactic-Shrine/Unreal-GalacticShrine.GsId/releases)
[![Unreal Engine](https://img.shields.io/badge/Unreal%20Engine-5.8-0e1128?logo=unrealengine)](https://www.unrealengine.com/)
[![License](https://img.shields.io/badge/license-MIT-blue)](./LICENSE)
[![Platforms](https://img.shields.io/badge/platforms-Windows%20%7C%20Linux%20%7C%20macOS-6c757d)](#compatibility)

`GsId` is a standalone Unreal Engine plugin that brings **256-bit** Galactic-Shrine identifiers to C++, Blueprints, serialization, and network replication.

## Overview

| Item | Value |
| --- | --- |
| Plugin | `GsId` |
| Version | `1.0.0` |
| Target engine | Unreal Engine `5.8` |
| Runtime module | `GsId` |
| Editor module | `GsIdEditor` |
| Identifier size | 256 bits / 32 bytes |
| Text formats | `N` (64 characters) and `D` (69 characters) |
| GitHub license | `MIT` |
| Fab license | `Fab Standard License` |

## Features

- Unreal `FGsId` type available in C++ and Blueprints;
- embedded native C++20 `GalacticShrine::GsId` API;
- cryptographically secure random generation and deterministic SHA-256 generation;
- strict parsing, validation, and upper- or lowercase formatting;
- conversion to and from 32 raw bytes;
- binary `FArchive`, text, and `NetSerialize` serialization;
- replication, comparison, ordering, and `GetTypeHash` support;
- Blueprint metadata localized in 13 cultures;
- 14 automation tests covering the core and Unreal integration.

## Installation

Copy the `GsId` directory into your project plugins directory:

```text
<Project>/Plugins/GsId/
```

It can also be installed at engine level:

```text
<UnrealEngine>/Engine/Plugins/Marketplace/GsId/
```

Enable **GsId** in the Plugin Browser, then rebuild the project.

## C++ integration

Add the module to your `.Build.cs` file:

```csharp
PublicDependencyModuleNames.Add("GsId");
```

Create, format, and parse an identifier:

```cpp
#include "GsId.h"

const FGsId Id = FGsId::NewGsId();
const FString Text = Id.ToString(EGsIdFormat::D);

FGsId Parsed;
if (FGsId::Parse(Text, Parsed))
{
    check(Id == Parsed);
}
```

The native C++20 API is also available:

```cpp
#include <galactic_shrine/gsid/gsid.hpp>

const GalacticShrine::GsId Id = GalacticShrine::GsId::NewGsId();
```

## Blueprint integration

The Blueprint library includes `New GsId`, `Try New GsId`, `Parse`, `Parse Exact`, `To String`, `To Bytes`, `From Bytes`, `Create Deterministic GsId`, and `Get Stable Hash`.

Use `Try New GsId` when the application needs to explicitly handle a secure random generator failure.

## Supported formats

### `N` format

Compact form with 64 hexadecimal characters:

```text
4ee77ca611ab7ebc9fabdc58c9689676f0763af9eff3c0df575f94ab230c0c66
```

### `D` format

Readable 69-character form grouped as `16-8-8-8-8-16`:

```text
4EE77CA611AB7EBC-9FABDC58-C9689676-F0763AF9-EFF3C0DF-575F94AB230C0C66
```

## Compatibility

| Platform | Secure random source |
| --- | --- |
| Windows 64-bit | `BCryptGenRandom` |
| Linux | `getrandom` |
| macOS | `SecRandomCopyBytes` |

Unsupported platforms or missing embedded C++ sources produce an explicit build error.

## Documentation

Complete documentation, API references, and integration guides are available at [docs.galactic-shrine.com](https://docs.galactic-shrine.com/).

## Distributed builds

Contributors can build the plugin in their own fork on their own Unreal runner without using a Galactic-Shrine machine. See the [personal runner guide](./.github/SELF_HOSTED_RUNNERS.md). A missing platform can then be [submitted through a pull request](./PlatformBuilds/README.md) without moving the existing tag.

## Tests

Run the suite from Session Frontend or AutomationTool with this filter:

```text
GsId.*
```

## License

The source code and releases published on GitHub, including the embedded copy of `GalacticShrine.GsId.Cpp`, are distributed under the [MIT License](./LICENSE).

The free package obtained through Fab is a separate distribution under the [Fab Standard License](https://www.fab.com/eula). The license file included in each package identifies the terms that apply to that copy. See the [third-party notices](./THIRD_PARTY_NOTICES.en.md) as well.
