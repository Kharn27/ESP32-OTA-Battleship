# Bibliothèque d'assets approuvés

Ce dossier rassemble les composants réutilisables acceptés artistiquement par le PO, avec un ID et un SHA-256 dans [`manifest.json`](manifest.json).

- [**Logo**](logo/README.md) : sprite original RGBA 1075×489 et sprite CYD 145×66.
- [**Boutons JOUER / PALMARÈS**](boutons/README.md) : quatre PNG RGBA, tailles originales et CYD.

Les six exports sont issus de l'[accueil approuvé sur PC — PR #90](https://github.com/Kharn27/ESP32-OTA-Battleship/pull/90#issuecomment-6740684237) et classés dans le catalogue sur [mandat PO — Issue #103](https://github.com/Kharn27/ESP32-OTA-Battleship/issues/103), **sans changement des images**. Les JPEG sources restent sous [`draft/`](../draft/) et la maquette conserve son écran composé sans copie privée des sprites.

Tous les éléments sont **`ON_DEVICE_UNVERIFIED`** : leur acceptation artistique PC ne valide ni décodage embarqué ni budget Flash/RAM. Voir [le pipeline](../docs/GRAPHIC_PIPELINE.md) et [`AGENTS.md`](../AGENTS.md).
