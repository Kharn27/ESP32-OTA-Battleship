# G07-R2 — VICTOIRE compact, correction de l'historique (Issue #53 / PR #74)

> **CANDIDAT POUR CONTRE-REVUE PILOTAGE — `DESIGN_PREVIEW / ON_DEVICE_UNVERIFIED`.** Image 1/7 uniquement ; pas de GO PO global ni de fusion. La composition maritime et le verdict doré sont préservés ; correction **limitée aux TOUCHÉ de la grille**.

## Vue 320 × 240 (image de jeu, sans annotation)

![G07-R2 — VICTOIRE, 17 TOUCHÉ](g07_r2_victoire_320x240.png)

<details><summary>Zoom ×4 NEAREST — 1280 × 960</summary>

![VICTOIRE ×4](g07_r2_victoire_x4.png)

</details>

<details><summary>Diagnostic séparé : touches cerclées de jaune, ratés cyan — NON destiné au jeu</summary>

![Preuve native](g07_r2_victoire_geometry_proof_320x240.png)

![Preuve ×4 NEAREST](g07_r2_victoire_geometry_proof_x4.png)

</details>

## Scénario de démonstration contrôlable

Les **17 TOUCHÉ distincts** correspondent aux 17 cases d'une flotte classique de test : porte-avions 5, croiseur 4, contre-torpilleur 3, sous-marin 3 et patrouilleur 2. Ces positions sont une **donnée de test** dans [`g07_r2_victoire_history.json`](g07_r2_victoire_history.json), et **pas** des coques, silhouettes ou groupes identifiés dans le PNG. Les navires adjacents sont autorisés par la Bible ; aucun des cinq naufrages n'est matérialisé par une épave dessinée.

**TOUCHÉ visibles :** `B1 C1 D1 E1 F1 / E4 F4 G4 H4 / J4 J5 J6 / B7 C7 D7 / G9 H9` = 17 cellules uniques. Les cinq TOUCHÉ déjà peints `F1 E4 J5 D7 G9` sont conservés sur leurs pixels d'origine ; **12 nouvelles marques** sont construites sur des calques distincts. **RATÉ conservés à l'identique :** `C2 H3 B5 G7 J8`. Le scénario final place le dernier tir humain sur `H9` (TOUCHÉ terminal), la riposte de Capitaine étant RATÉ : la révélation simultanée n'est pas illustrée par ce seul état compact.

**Aucune révélation de flotte adverse** sur VICTOIRE ; verdict « VICTOIRE » informatif, commandes tactiles DÉFENSE / REJOUER / ACCUEIL inchangées ; aucun FEU.

## Contrôles PC et comparaison

- Grille réellement mesurée : **11 verticales** `X=21,39,…,201` et **11 horizontales** `Y=53,71,…,233`, tous les 18 px ; **10 × 10**, cellules 18 × 18, emprise 180 × 180. Teinte des lignes d'origine `(182,227,237)` conservée.
- Exports natifs **320 × 240** et zoom **1280 × 960** par **NEAREST exact pixel pour pixel**.
- Les cinq cases RATÉ et les cinq cases TOUCHÉ préexistantes sont **identiques pixel pour pixel** entre la base et la correction. Aucune nouvelle modification hors des **12 cellules ajoutées** ; panneau décoratif et doré de droite **x ≥ 202 = zéro pixel modifié**.
- Chaque ajout se trouve dans sa cellule propre, sans effacer de trait de grille. Contrôles négatifs sur doublon de tir, raté confondu avec touche et translation d'1 px. Contrôle de reconstruction OpenRaster (calques composés = PNG final) exécuté sur PC.
- Surfaces extérieures (ciel, phare, lanternes, mer hors impacts, panneaux, boutons) strictement préservées. **Inspection visuelle 320×240 et ×4 réalisée sur PC** ; lisibilité TFT/stylet non prouvée.

### Source et reproduction

La source originale `g07_r2_victoire_source_hd.png` (1448 × 1086) reste celle déjà versionnée. La génération demande l'exécution **dans cet ordre** :

```bash
python3 -m pip install pillow numpy opencv-python-headless
python3 design/ecrans/fin-partie/build_g07_r2_victoire.py
python3 design/ecrans/fin-partie/build_g07_r2_victoire_history.py
```

Le premier script restaure la composition originale et les contrôles de géométrie ; le second (piloté par `g07_r2_victoire_history.json`) ajoute les douze marques, reconstruit `g07_r2_victoire_source.ora`, actualise les deux preuves/zooms et `g07_r2_victoire_provenance.json`.

`g07_r2_victoire_source.ora` : **19 calques** au total : les 7 calques hérités de la composition initiale dont un diagnostic masqué, plus **12 calques indépendants** `TOUCHÉ <case>` (éditables). **Limite honnête :** les cinq impacts d'origine demeurent dans la couche mer héritée du master ; fond, typographie et commandes du panneau droit restent peints et groupés. Ce document n'est **ni un PSD décomposé ni une planche de sprites Flash**.

**Provenance** : peinture source reçue dans la conversation du PO, impacts supplémentaires dérivés de ses propres pixels de feu/smoke ; aucune image Stock ni police externe ajoutée. Les questions de droits sur l'artwork original n'ont pas fait l'objet d'un audit autonome. Hashes SHA-256 et données de scénario dans le manifeste JSON.

## Réserves et prochaine porte

Cette correction **ne traite pas** MATCH NUL, DÉFAITE ni les images 4–7 ; elle ne prouve pas les transitions annonce→compact, la révélation synchronisée des derniers tirs, les hitboxes tactiles ni le raccord G08. **Aucun test physique CYD, Flash/RAM, contrat de compression ou intégration firmware effectué.** Revue du Pilotage et appréciation du PO requises sur le **nouveau HEAD exact** de la PR Draft #74. Seul Pilotage peut fusionner.
