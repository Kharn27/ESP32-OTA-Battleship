# Sous-marin FLOT-01 — INTACT, horizontal et vertical

Deux **sprites autonomes PNG RGBA** validés artistiquement par le PO en [PR #97 (R2)](https://github.com/Kharn27/ESP32-OTA-Battleship/pull/97#issuecomment-6766809070), pour un navire occupant **3 cases de 18 px**. Les versions natives ont été déplacées depuis `draft/etudes/flotte/sous-marin/` **sans aucune retouche d’octets**.

## Sous-marin horizontal — 54 × 18 px (proue à droite)

![Sous-marin intact, orientation horizontale, 54×18](sous_marin_intact_h.png)

**Confort visuel ×4 — 216 × 72 px**, agrandissement exact `NEAREST` sans interpolation :

![Sous-marin horizontal agrandi ×4, 216×72](sous_marin_intact_h_x4.png)

## Sous-marin vertical — 18 × 54 px (proue en haut)

![Sous-marin intact, orientation verticale, 18×54](sous_marin_intact_v.png)

**Confort visuel ×4 — 72 × 216 px**, agrandissement exact `NEAREST` sans interpolation :

![Sous-marin vertical agrandi ×4, 72×216](sous_marin_intact_v_x4.png)

## Contrat et traçabilité

- **Intégration firmware :** utiliser uniquement les PNG natifs **54×18** / **18×54**, avec alpha RGBA 0/255 effectif (486 pixels opaques, 486 transparents par orientation). Aucune mer ni grille n'est incrustée. Le zoom ×4 est **INSPECTION_ONLY_NOT_RUNTIME**, à ne pas embarquer par défaut.
- **Éditabilité :** [SVG horizontal](sources/sous_marin_intact_h.svg) et [SVG vertical](sources/sous_marin_intact_v.svg) (sources, non nécessaires au runtime), conservés à l'identique ; PNG H/V natifs demeurent les exports canoniques, la rasterisation SVG pouvant varier.
- **Statut artistique :** `VALIDATED_PO_PC` pour **INTACT H/V uniquement**. Les états **TOUCHÉ / COULÉ** restent hors périmètre et non approuvés. GO : [commentaire PO #6766809070](https://github.com/Kharn27/ESP32-OTA-Battleship/pull/97#issuecomment-6766809070), [fusion technique PR #97](https://github.com/Kharn27/ESP32-OTA-Battleship/pull/97) ; migration et suppression des preuves dans [Issue #109](https://github.com/Kharn27/ESP32-OTA-Battleship/issues/109).
- **Contrôle :** [manifeste SHA-256](../../manifest.json) et [script de vérification/zoom](../../../scripts/check_sous_marin_assets.py) exécuté en CI (`--check`). Les anciens tableaux de preuve ont été retirés de l'arbre actif à la demande du PO, mais restent récupérables dans le Git de la PR #97.
- **Statut matériel :** `ON_DEVICE_UNVERIFIED`. Aucune mesure RAM/Flash/TFT ni intégration effective sur CYD n'est déclarée.

