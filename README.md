# GalacticShrine GsId pour Unreal Engine

**Langues :** **Français** · [English](./README.en.md) · [Español](./README.es.md) · [Italiano](./README.it.md) · [日本語](./README.jp.md)

[![Version](https://img.shields.io/badge/version-1.0.1-4c9aff)](https://github.com/Galactic-Shrine/Unreal-GalacticShrine.GsId/releases)
[![Unreal Engine](https://img.shields.io/badge/Unreal%20Engine-5.8-0e1128?logo=unrealengine)](https://www.unrealengine.com/)
[![Licence](https://img.shields.io/badge/licence-MIT-blue)](./LICENSE)
[![Plateformes](https://img.shields.io/badge/plateformes-Windows%20%7C%20Linux%20%7C%20macOS-6c757d)](#compatibilité)

`GsId` est un plugin Unreal Engine autonome qui fournit des identifiants Galactic-Shrine de **256 bits** aux projets C++, aux Blueprints, à la sérialisation et à la réplication réseau.

## Vue d’ensemble

| Élément | Valeur |
| --- | --- |
| Plugin | `GsId` |
| Version | `1.0.1` |
| Moteur cible | Unreal Engine `5.8` |
| Module Runtime | `GsId` |
| Module Editor | `GsIdEditor` |
| Taille d’un identifiant | 256 bits / 32 octets |
| Formats texte | `N` (64 caractères) et `D` (69 caractères) |
| Licence GitHub | `MIT` |
| Licence Fab | `Fab Standard License` |

## Fonctionnalités

- type Unreal `FGsId` exposé en C++ et en Blueprint ;
- API native C++20 `GalacticShrine::GsId` intégrée ;
- génération aléatoire sécurisée et génération déterministe SHA-256 ;
- parsing strict, validation et formatage en majuscules ou minuscules ;
- conversion vers et depuis 32 octets bruts ;
- sérialisation binaire `FArchive`, sérialisation textuelle et `NetSerialize` ;
- réplication, comparaison, ordre et prise en charge de `GetTypeHash` ;
- métadonnées Blueprint localisées dans 13 cultures ;
- 14 tests d’automatisation couvrant le cœur et l’intégration Unreal.

## Installation

Copiez le dossier `GsId` dans le répertoire des plugins du projet :

```text
<Project>/Plugins/GsId/
```

Vous pouvez aussi l’installer au niveau du moteur :

```text
<UnrealEngine>/Engine/Plugins/Marketplace/GsId/
```

Activez ensuite **GsId** dans le gestionnaire de plugins et recompilez le projet.

## Intégration C++

Ajoutez le module à votre fichier `.Build.cs` :

```csharp
PublicDependencyModuleNames.Add("GsId");
```

Créez, formatez et analysez un identifiant :

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

L’API C++20 native reste également accessible :

```cpp
#include <galactic_shrine/gsid/gsid.hpp>

const GalacticShrine::GsId Id = GalacticShrine::GsId::NewGsId();
```

## Intégration Blueprint

La bibliothèque Blueprint fournit notamment `New GsId`, `Try New GsId`, `Parse`, `Parse Exact`, `To String`, `To Bytes`, `From Bytes`, `Create Deterministic GsId` et `Get Stable Hash`.

`Try New GsId` est recommandé lorsque l’application doit gérer explicitement une éventuelle défaillance du générateur aléatoire sécurisé.

## Formats pris en charge

### Format `N`

Forme compacte de 64 caractères hexadécimaux :

```text
4ee77ca611ab7ebc9fabdc58c9689676f0763af9eff3c0df575f94ab230c0c66
```

### Format `D`

Forme lisible de 69 caractères, groupée en `16-8-8-8-8-16` :

```text
4EE77CA611AB7EBC-9FABDC58-C9689676-F0763AF9-EFF3C0DF-575F94AB230C0C66
```

## Compatibilité

| Plateforme | Source d’aléa sécurisée |
| --- | --- |
| Windows 64 bits | `BCryptGenRandom` |
| Linux | `getrandom` |
| macOS | `SecRandomCopyBytes` |

Une plateforme non prise en charge ou l’absence des sources C++ intégrées provoque une erreur explicite à la compilation.

## Documentation

La documentation complète, les références d’API et les guides d’intégration sont disponibles sur [docs.galactic-shrine.com](https://docs.galactic-shrine.com/).

## Compilation distribuée

Chaque contributeur peut compiler le plugin sur son propre fork et son propre runner Unreal, sans utiliser une machine Galactic-Shrine. Consultez le [guide des runners personnels](./.github/SELF_HOSTED_RUNNERS.md). Une plateforme manquante peut ensuite être [proposée par pull request](./PlatformBuilds/README.md) sans déplacer le tag existant.

## Tests

Exécutez la suite depuis Session Frontend ou avec AutomationTool en utilisant le filtre :

```text
GsId.*
```

## Licence

Le code source et les versions publiées sur GitHub, y compris la copie intégrée de `GalacticShrine.GsId.Cpp`, sont distribués sous [licence MIT](./LICENSE).

Le paquet gratuit obtenu via Fab constitue une distribution séparée sous la [licence Standard de Fab](https://www.fab.com/eula). Le fichier de licence inclus dans chaque paquet indique les conditions applicables à cette copie. Consultez également les [notices relatives aux composants tiers](./THIRD_PARTY_NOTICES.md).
