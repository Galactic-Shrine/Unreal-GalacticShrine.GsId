# GalacticShrine GsId per Unreal Engine

**Lingue:** [Français](./README.md) · [English](./README.en.md) · [Español](./README.es.md) · **Italiano** · [日本語](./README.jp.md)

[![Versione](https://img.shields.io/badge/versione-1.0.1-4c9aff)](https://github.com/Galactic-Shrine/Unreal-GalacticShrine.GsId/releases)
[![Unreal Engine](https://img.shields.io/badge/Unreal%20Engine-5.8-0e1128?logo=unrealengine)](https://www.unrealengine.com/)
[![Licenza](https://img.shields.io/badge/licenza-MIT-blue)](./LICENSE)
[![Piattaforme](https://img.shields.io/badge/piattaforme-Windows%20%7C%20Linux%20%7C%20macOS-6c757d)](#compatibilità)

`GsId` è un plugin autonomo per Unreal Engine che rende disponibili identificatori Galactic-Shrine a **256 bit** in C++, Blueprint, serializzazione e replica di rete.

## Panoramica

| Elemento | Valore |
| --- | --- |
| Plugin | `GsId` |
| Versione | `1.0.1` |
| Motore di destinazione | Unreal Engine `5.8` |
| Modulo Runtime | `GsId` |
| Modulo Editor | `GsIdEditor` |
| Dimensione identificatore | 256 bit / 32 byte |
| Formati testuali | `N` (64 caratteri) e `D` (69 caratteri) |
| Licenza GitHub | `MIT` |
| Licenza Fab | `Fab Standard License` |

## Funzionalità

- tipo Unreal `FGsId` disponibile in C++ e Blueprint;
- API nativa C++20 `GalacticShrine::GsId` integrata;
- generazione casuale sicura e generazione deterministica SHA-256;
- parsing rigoroso, validazione e formattazione maiuscola o minuscola;
- conversione da e verso 32 byte grezzi;
- serializzazione binaria `FArchive`, testuale e `NetSerialize`;
- replica, confronto, ordinamento e supporto `GetTypeHash`;
- metadati Blueprint localizzati in 13 lingue;
- 14 test automatici per il core e l’integrazione Unreal.

## Installazione

Copiare la directory `GsId` nella cartella dei plugin del progetto:

```text
<Project>/Plugins/GsId/
```

È possibile installarla anche a livello del motore:

```text
<UnrealEngine>/Engine/Plugins/Marketplace/GsId/
```

Attivare **GsId** nel Plugin Browser e ricompilare il progetto.

## Integrazione C++

Aggiungere il modulo al file `.Build.cs`:

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

È disponibile anche l’API nativa C++20:

```cpp
#include <galactic_shrine/gsid/gsid.hpp>

const GalacticShrine::GsId Id = GalacticShrine::GsId::NewGsId();
```

## Integrazione Blueprint

La libreria Blueprint include `New GsId`, `Try New GsId`, `Parse`, `Parse Exact`, `To String`, `To Bytes`, `From Bytes`, `Create Deterministic GsId` e `Get Stable Hash`.

Usare `Try New GsId` quando l’applicazione deve gestire esplicitamente un errore del generatore casuale sicuro.

## Formati supportati

### Formato `N`

Forma compatta di 64 caratteri esadecimali:

```text
4ee77ca611ab7ebc9fabdc58c9689676f0763af9eff3c0df575f94ab230c0c66
```

### Formato `D`

Forma leggibile di 69 caratteri raggruppati come `16-8-8-8-8-16`:

```text
4EE77CA611AB7EBC-9FABDC58-C9689676-F0763AF9-EFF3C0DF-575F94AB230C0C66
```

## Compatibilità

| Piattaforma | Sorgente casuale sicura |
| --- | --- |
| Windows 64 bit | `BCryptGenRandom` |
| Linux | `getrandom` |
| macOS | `SecRandomCopyBytes` |

Una piattaforma non supportata o l’assenza dei sorgenti C++ integrati produce un errore di compilazione esplicito.

## Documentazione

La documentazione completa, i riferimenti API e le guide di integrazione sono disponibili su [docs.galactic-shrine.com](https://docs.galactic-shrine.com/).

## Build distribuite

Ogni collaboratore può compilare il plugin nel proprio fork e sul proprio runner Unreal. Consultare la [guida ai runner personali](./.github/SELF_HOSTED_RUNNERS.md). Una piattaforma mancante può essere [proposta tramite pull request](./PlatformBuilds/README.md) senza spostare il tag esistente.

## Test

Eseguire la suite da Session Frontend o AutomationTool con il filtro:

```text
GsId.*
```

## Licenza

Il codice sorgente e le versioni pubblicate su GitHub, inclusa la copia integrata di `GalacticShrine.GsId.Cpp`, sono distribuiti sotto [licenza MIT](./LICENSE).

Il pacchetto gratuito ottenuto tramite Fab è una distribuzione separata sotto la [licenza Standard di Fab](https://www.fab.com/eula). Il file di licenza incluso in ogni pacchetto identifica le condizioni applicabili a quella copia. Consultare anche gli [avvisi relativi a componenti di terze parti](./THIRD_PARTY_NOTICES.it.md).
