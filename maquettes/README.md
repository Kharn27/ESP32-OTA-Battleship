# Maquettes d'écrans — CYD WARSHIP

[**Index des références courantes et des GO PO**](INDEX.md) · [**Contrat du pipeline**](../docs/GRAPHIC_PIPELINE.md).

Chaque dossier `maquettes/<écran>/` comporte un `screen.json` avec aperçu natif 320 × 240, statut artistique et éventuels IDs d'assets approuvés.

**Migration historique Issue #101 :** les fichiers des anciens écrans sont volontairement gelés dans `legacy/`, conservés sans retouche des graphismes. Le champ `migration_state: LEGACY_UNDECOMPOSED` et `assets: []` signalent que l'écran est encore une composition historique : **il n'est pas encore reconstruit à partir des ressources individuelles de `assets/`**. Les fichiers de `legacy/` sont figés par leurs Git blob SHA dans `screen.json`. La validation PC d'un écran ne valide ni chaque sprite, ni le firmware, ni le rendu sur CYD.

Aucune nouvelle maquette ne peut adopter ce régime legacy ; les créations futures suivent exclusivement le contrat `AGENTS.md` et `assets/manifest.json`.
