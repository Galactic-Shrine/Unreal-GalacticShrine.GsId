# GalacticShrine.GsId.Cpp — copie intégrée

Source amont : https://github.com/Galactic-Shrine/GalacticShrine.GsId.Cpp

Bibliothèque C++20 fournissant `GalacticShrine::GsId`, un identifiant de 256 bits conservé sous forme de 32 octets bruts. La copie intégrée à la distribution GitHub de GsId 1.0.0 est publiée sous licence MIT.

Le paquet distribué séparément par Fab est proposé sous la licence Standard de Fab. Le fichier `LICENSE` inclus dans chaque paquet indique les conditions applicables à cette copie.

Dépendances de génération sécurisée :

- Windows : `BCryptGenRandom` / `bcrypt.lib` ;
- Linux : `getrandom` ;
- macOS : `SecRandomCopyBytes` / framework `Security`.

API de génération contrôlée :

```cpp
GalacticShrine::GsId value;
if (!GalacticShrine::GsId::TryNewGsId(value))
{
    // Le fournisseur aléatoire sécurisé a échoué.
}
```
