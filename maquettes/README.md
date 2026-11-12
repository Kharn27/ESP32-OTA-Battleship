# Maquettes d'écrans — CYD WARSHIP

[**Index des références courantes et des GO PO**](INDEX.md) · [**Contrat du pipeline**](../docs/GRAPHIC_PIPELINE.md).

Chaque dossier `maquettes/<écran>/` comporte un `screen.json` avec aperçu natif 320 × 240, statut artistique et éventuels IDs d'assets approuvés.

**Migration historique Issue #101 :** les fichiers des anciens écrans sont volontairement gelés dans `legacy/`, conservés sans retouche des graphismes. Le champ `migration_state: LEGACY_UNDECOMPOSED` et `assets: []` signalent que l'écran est encore une composition historique : **il n'est pas encore reconstruit à partir des ressources individuelles de `assets/`**. Les fichiers de `legacy/` sont figés par leurs Git blob SHA dans `screen.json`. La validation PC d'un écran ne valide ni chaque sprite, ni le firmware, ni le rendu sur CYD.

**Accueil actuel PR #90 :** composition de quatre ressources approuvées désormais sous `maquettes/accueil/`, sans `legacy/`. **Ancien accueil PR #8 :** retiré de l'arbre actif sur décision du PO du 12/11/2026 et [préservé dans l'historique Git](https://github.com/Kharn27/ESP32-OTA-Battleship/tree/55ee8b72a098d3da5e28bac45bb1a2dde7129f4a/maquettes/accueil-historique/legacy). Les autres écrans encore non décomposés conservent leur contrat historique.
