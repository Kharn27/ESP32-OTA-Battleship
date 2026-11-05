# G07-R2 — MATCH NUL compact (Issue #53 / PR #74)

> **DESIGN_PREVIEW — ON_DEVICE_UNVERIFIED ; validation PO en attente.**
> Image 2/7, proposition sur le champ de bataille. La PR reste Draft.

## Aperçu natif 320 × 240

![G07-R2 MATCH NUL — natif](g07_r2_match_nul_320x240.png)

<details><summary>Zoom ×4 — 1280 × 960, strict NEAREST</summary>

![G07-R2 MATCH NUL — zoom](g07_r2_match_nul_x4.png)

</details>

<details><summary>Contrôle géométrique — diagnostic uniquement</summary>

![Repères natifs](g07_r2_match_nul_geometry_proof_320x240.png)

![Repères x4](g07_r2_match_nul_geometry_proof_x4.png)

</details>

## Mesure du fichier réel

- Origine **(21, 53)**, fin **(201, 233)** ; grille **10×10**, cases **18×18**, emprise **180×180**.
- Lignes X : **21, 39, 57, 75, 93, 111, 129, 147, 165, 183, 201**.
- Lignes Y : **53, 71, 89, 107, 125, 143, 161, 179, 197, 215, 233**.
- Traits 1 px **RVB(182,227,237)** vérifiés individuellement sur toute leur longueur.
- Image ×4 formée de blocs de pixels uniformes 4×4, exacte au niveau RGB ; test négatif décalage +1px rejeté.
- L'image de contrôle fournie ne modifie que les annotations rouges et jaunes : **3 925 pixels**, décor intact.
- Contrôle automatisé : `python3 design/ecrans/fin-partie/build_g07_r2_match_nul.py` (Pillow et NumPy) et empreintes `g07_r2_match_nul_provenance.json`.

## État visuel / règle métier

**FIN DE PARTIE — VS Capitaine — MATCH NUL**, verdict compact **informatif**, non assimilé à un bouton. Une seule grille ATTAQUE, historique des impacts visible ; boutons **DÉFENSE**, **REJOUER**, **ACCUEIL**, sans **FEU**, sans révélation spontanée de la flotte adverse.

Il s'agit uniquement d'une image fixe : cette preuve **ne démontre ni l'annonce initiale, ni la synchronisation des deux derniers tirs, ni les transitions, ni les hitboxes tactiles**.

## Sources et réserves

Le fichier OpenRaster `g07_r2_match_nul_source.ora` comporte **un calque raster fidèle mais aplati** et **un calque de contrôle géométrique masqué**. Les textes, éléments d'eau, impacts et boutons ne sont **pas** séparables individuellement ; la source d'auteur pleinement décomposée reste à produire si elle est requise pour le firmware.

Provenance : **deux PNG fournis par le PO le 05/11/2026**, aucun remplacement artistique. Le script régénère les sorties à partir du zoom ×4 et de la preuve native conservés dans ce dossier. **Pas de test sur CYD**, ni validation Flash, RAM, performance, décodage, toucher/stylet ou rendu TFT. Pas de `ON_DEVICE_VERIFIED`.

**Statut : IMAGE 2/7 soumise à revue PO / Pilotage, pas G07-R2 intégralement livré.** Pilotage seul peut fusionner la PR #74.
