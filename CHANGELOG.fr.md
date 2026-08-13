# Journal des modifications

**Langues :** [English](./CHANGELOG.md) · **Français**

Toutes les modifications notables de `GsId` pour Unreal Engine sont documentées dans ce fichier.

Le format suit [Keep a Changelog](https://keepachangelog.com/fr/1.1.0/) et le projet utilise la [gestion sémantique de version](https://semver.org/lang/fr/).

## [1.0.0] — Version initiale

### Ajouts

- identifiants Galactic-Shrine de 256 bits avec la représentation Unreal `FGsId` de 32 octets ;
- génération aléatoire sécurisée sous Windows 64 bits, Linux et macOS, avec l’API explicite `TryNewGsId` ;
- formats texte compact `N` et lisible `D`, parsing strict et casse configurable ;
- conversion en octets bruts, comparaison, ordre et hachage FNV-1a stable de 32 bits ;
- génération déterministe d’identifiants avec SHA-256 ;
- sérialisation binaire `FArchive`, réseau `NetSerialize` et textuelle Unreal ;
- API C++20 native, C++ Unreal et Blueprint ;
- localisation native des métadonnées Blueprint dans 13 cultures ;
- catégorie Blueprint partagée `# Galactic Shrine|Gs Id` ;
- test d’automatisation aligné sur la catégorie `# Galactic Shrine` affichée par Unreal ;
- tests d’automatisation et scripts multiplateformes de localisation et de publication ;
- licence MIT pour le code source et les versions publiques sur GitHub, avec un paquet distinct sous licence Standard de Fab pour Fab.

[1.0.0]: https://github.com/Galactic-Shrine/Unreal-GalacticShrine.GsId/releases/tag/v1.0.0
