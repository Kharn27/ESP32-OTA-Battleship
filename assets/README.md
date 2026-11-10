# Bibliothèque d'assets approuvés

Ce dossier contient **uniquement les composants graphiques réutilisables destinés à l’intégration du jeu par le Développeur** (ainsi que leurs versions sources haute résolution). Pas d’écran complet aplati, ni de maquette : ceux-ci appartiennent à [`maquettes/`](../maquettes/). Chaque composant est tracé dans [`manifest.json`](manifest.json).

- [**Logo**](logo/README.md) : sprite original RGBA 1075×489 et sprite CYD 145×66.
- [**Boutons JOUER / PALMARÈS**](boutons/README.md) : quatre PNG RGBA, tailles originales et CYD.
- [**Wallpaper d’accueil retenu**](wallpaper/README.md) : **fond nu** destiné à l’intégration firmware, en format original et 320×240.
- [**Flotte / Sous-marin INTACT**](flotte/sous-marin/README.md) : sprites RGBA H/V validés PO, versions ×4 réservées à l’inspection et sources SVG éditables.

Les six PNG du logo et des boutons sont issus de l'[accueil approuvé sur PC — PR #90](https://github.com/Kharn27/ESP32-OTA-Battleship/pull/90#issuecomment-6740684237) et classés dans le catalogue sur [mandat PO — Issue #103](https://github.com/Kharn27/ESP32-OTA-Battleship/issues/103), **sans changement des images**. Le **fond actuel sans interface** est conservé ici sur [mandat PO — Issue #105](https://github.com/Kharn27/ESP32-OTA-Battleship/issues/105). L’**ancien accueil aplati** a été reclassé dans [`maquettes/accueil-historique/`](../maquettes/accueil-historique/legacy/README.md) par [arbitrage PO #107](https://github.com/Kharn27/ESP32-OTA-Battleship/issues/107). Les JPEG sources du PO restent sous [`draft/`](../draft/) et la maquette conserve son écran composé sans copie privée des sprites.

Tous les éléments sont **`ON_DEVICE_UNVERIFIED`** : leur acceptation artistique PC ne valide ni décodage embarqué ni budget Flash/RAM. Voir [le pipeline](../docs/GRAPHIC_PIPELINE.md) et [`AGENTS.md`](../AGENTS.md).
