# GalacticShrine GsId para Unreal Engine

**Idiomas:** [Français](./README.md) · [English](./README.en.md) · **Español** · [Italiano](./README.it.md) · [日本語](./README.jp.md)

[![Versión](https://img.shields.io/badge/versión-1.0.1-4c9aff)](https://github.com/Galactic-Shrine/Unreal-GalacticShrine.GsId/releases)
[![Unreal Engine](https://img.shields.io/badge/Unreal%20Engine-5.8-0e1128?logo=unrealengine)](https://www.unrealengine.com/)
[![Licencia](https://img.shields.io/badge/licencia-MIT-blue)](./LICENSE)
[![Plataformas](https://img.shields.io/badge/plataformas-Windows%20%7C%20Linux%20%7C%20macOS-6c757d)](#compatibilidad)

`GsId` es un plugin independiente para Unreal Engine que incorpora identificadores Galactic-Shrine de **256 bits** a C++, Blueprints, serialización y replicación de red.

## Descripción general

| Elemento | Valor |
| --- | --- |
| Plugin | `GsId` |
| Versión | `1.0.1` |
| Motor objetivo | Unreal Engine `5.8` |
| Módulo Runtime | `GsId` |
| Módulo Editor | `GsIdEditor` |
| Tamaño del identificador | 256 bits / 32 bytes |
| Formatos de texto | `N` (64 caracteres) y `D` (69 caracteres) |
| Licencia de GitHub | `MIT` |
| Licencia de Fab | `Fab Standard License` |

## Características

- tipo Unreal `FGsId` disponible en C++ y Blueprints;
- API nativa C++20 `GalacticShrine::GsId` integrada;
- generación aleatoria segura y generación determinista SHA-256;
- análisis estricto, validación y formato en mayúsculas o minúsculas;
- conversión desde y hacia 32 bytes sin procesar;
- serialización binaria `FArchive`, textual y `NetSerialize`;
- replicación, comparación, orden y compatibilidad con `GetTypeHash`;
- metadatos Blueprint localizados en 13 idiomas;
- 14 pruebas automatizadas para el núcleo y la integración Unreal.

## Instalación

Copie el directorio `GsId` en la carpeta de plugins del proyecto:

```text
<Project>/Plugins/GsId/
```

También puede instalarlo a nivel del motor:

```text
<UnrealEngine>/Engine/Plugins/Marketplace/GsId/
```

Active **GsId** en el explorador de plugins y vuelva a compilar el proyecto.

## Integración C++

Añada el módulo al archivo `.Build.cs`:

```csharp
PublicDependencyModuleNames.Add("GsId");
```

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

La API nativa C++20 también está disponible:

```cpp
#include <galactic_shrine/gsid/gsid.hpp>

const GalacticShrine::GsId Id = GalacticShrine::GsId::NewGsId();
```

## Integración Blueprint

La biblioteca Blueprint incluye `New GsId`, `Try New GsId`, `Parse`, `Parse Exact`, `To String`, `To Bytes`, `From Bytes`, `Create Deterministic GsId` y `Get Stable Hash`.

Use `Try New GsId` cuando la aplicación deba gestionar explícitamente un fallo del generador aleatorio seguro.

## Formatos compatibles

### Formato `N`

Forma compacta de 64 caracteres hexadecimales:

```text
4ee77ca611ab7ebc9fabdc58c9689676f0763af9eff3c0df575f94ab230c0c66
```

### Formato `D`

Forma legible de 69 caracteres agrupados como `16-8-8-8-8-16`:

```text
4EE77CA611AB7EBC-9FABDC58-C9689676-F0763AF9-EFF3C0DF-575F94AB230C0C66
```

## Compatibilidad

| Plataforma | Fuente aleatoria segura |
| --- | --- |
| Windows 64 bits | `BCryptGenRandom` |
| Linux | `getrandom` |
| macOS | `SecRandomCopyBytes` |

Una plataforma no compatible o la ausencia del código C++ integrado genera un error de compilación explícito.

## Documentación

La documentación completa, las referencias de API y las guías de integración están disponibles en [docs.galactic-shrine.com](https://docs.galactic-shrine.com/).

## Compilaciones distribuidas

Cada colaborador puede compilar el plugin en su propio fork y runner Unreal. Consulte la [guía de runners personales](./.github/SELF_HOSTED_RUNNERS.md). Una plataforma ausente puede [proponerse mediante pull request](./PlatformBuilds/README.md) sin mover el tag existente.

## Pruebas

Ejecute la suite desde Session Frontend o AutomationTool con el filtro:

```text
GsId.*
```

## Licencia

El código fuente y las versiones publicadas en GitHub, incluida la copia integrada de `GalacticShrine.GsId.Cpp`, se distribuyen bajo la [licencia MIT](./LICENSE).

El paquete gratuito obtenido a través de Fab es una distribución separada bajo la [licencia estándar de Fab](https://www.fab.com/eula). El archivo de licencia incluido en cada paquete identifica las condiciones aplicables a esa copia. Consulte también los [avisos de componentes de terceros](./THIRD_PARTY_NOTICES.es.md).
