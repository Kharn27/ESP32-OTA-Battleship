# D14 #69 — BLOCAGE_ASSETS_G03 : inventaire et passation Graphiste/DA

**Contrat en vigueur :** `AGENTS.md`, `docs/GRAPHIC_PIPELINE.md` (PO #107). **Mandat :** [Issue #69, #6772932082](https://github.com/Kharn27/ESP32-OTA-Battleship/issues/69#issuecomment-6772932082). **Signalement DEV :** [#6773142382](https://github.com/Kharn27/ESP32-OTA-Battleship/issues/69#issuecomment-6773142382).

## Analyse vérifiée sur main ec09a88f6947abe8a98714d571073d0fdcb90d95

- `assets/manifest.json` : **8 entrées exclusivement G01** (fond nu d'accueil, logo et boutons, original/CYD), aucun ID, SHA256 ou accord PO d'un fond, des commandes ni des silhouettes de **G03/G02**.
- `maquettes/deploiement/screen.json` : `migration_state=LEGACY_UNDECOMPOSED`, `assets=[]`, `status=VALIDATED_PO_PC`. Cette acceptation concerne **la composition**, pas ses composants individuels. Les sources `maquettes/deploiement/legacy/g03_deploiement_320x240.png` (Git blob `83d82b9b2baccbbd553e48f82cd452bd2d739f55`) et `.svg` (`114f1295997eb2a4a65c006ca0c40a00aa8c41d7`) ont conservé exactement les blobs de l'ancien `design/`.
- Avant la migration, `tools/d14_export.py` construisait **à partir de la maquette composée** (non à partir de `assets/`) : un bitmap « sans 5 navires » et 10 silhouettes `5 types × H/V`. **Attention : le bitmap historique « sans navires » contient ENCORE la grille, les repères, l'interface et les boutons AUTO/PRÊT/RETOUR ; il n'est pas un fond artistique nu admissible comme wallpaper !** Il ne peut pas être simplement déplacé/renommé en `assets/`.
- Contrat historique firmware : fond 320×240, compression **RLE par ligne RGB565BE** (`T03_ROW_RLE565BE`, table de **241 offsets**, taille ancienne **34 855 o**), masques H/V `RGB565BE + alpha8` droits (**33 048 o cumulés**), tableaux `const` en Flash ; lecture par `d14_g03_compose` dans une bande DMA **5 120 o**. Le rendu efface la grille 180×180 après AUTO. Ces chiffres sont ceux de l'ancien générateur (CI [#42821571172](https://github.com/Kharn27/ESP32-OTA-Battleship/actions/runs/42821571172)), **pas une taille contractuelle pour les nouveaux exports**.
- Empreintes de l'ancienne chaîne : PNG `44935905aebd910b57d93e8e5221c81eee0665a4bde70b1eda6d575974d0b7eb`, SVG `e026eadf3a52068ccf32a7ac57281cbab32e3dac37bdb37c391e68b09ad48f3f` ; fond RGB565BE décompressé `4097b46b2b5e42f904924e36d3ab8683563793ca4b54e337bb2c5cc217926954` ; C test généré `f80e1aa85d03ca0dfd35c9dbb5988faf137a23fb3eca158d1e4c8e8dc8fd6138`. **Ces empreintes ne prouvent PAS la validation d'un export individuel.**

## Demande minimale au Graphiste/DA à transmettre par Pilotage

Préparer pour le **déploiement G03 existant sans réinterprétation artistique**, sous validation des composants par le PO :

1. **Un fond nu** adapté 320×240, sans navires **ni boutons, légendes, grille ou titre incrustés**. Les détails de fond/ambiance restent fidèles à la maquette approuvée.
2. **Les éléments visuels non primitifs de l'interface G03** : commandes `AUTO`, `PRÊT`, `RETOUR` (et leurs états si validés) sous forme d'éléments individuels réutilisables, au lieu de pixels peints dans un écran ; une grille, des repères et libellés peuvent être des **primitives UI** si Pilotage contractualise leur rendu et fidélité, sinon prévoir les éléments séparés nécessaires.
3. **Les cinq silhouettes G02** correspondant à `patrouilleur (2)`, `sous-marin (3)`, `contre-torpilleur (3)`, `croiseur (4)`, `porte-avions (5)`, chacune en **H/V**, avec **transparence alpha native** : 10 variantes exportables de taille en cases ×18 px pour coller à la géométrie du prototype. Leur provenance est la maquette SVG validée, mais ce SVG n'est pas un catalogue.
4. Déclarer **chaque élément individuel réellement approuvé** sous `assets/` et `assets/manifest.json` avec `id` stable, `path`, `sha256`, `source_ref`, décision PO `po_validation_ref`, dimensions, `device_status` distinct ; refondre `maquettes/deploiement/screen.json` en citant ces IDs et sortir de `LEGACY_UNDECOMPOSED` suivant la revue.

**Contrat DEV à valider avec Pilotage/Graphiste avant intégration :** image de fond RGB/RGB565, masque alpha des navires, dimensions, positionnement pixel/hitboxes, ordre de calques et stratégie de conversion `assets/` → RLE/Flash ; cible RAM DMA 5120 o, SPI2 20 MHz et pas de framebuffer 153 600 o en RAM. La composition initiale devra être comparée à la maquette 320×240 sous une tolérance **décidée** (ne pas inventer une acceptation pixel-perfect automatique).

## État compilation, CI et suite

Le composant de production `components/d14_g03/CMakeLists.txt` **échoue volontairement** par `BLOCAGE_ASSETS_G03` : tant qu'il n'y a pas de catalogue approuvé et d'interface d'intégration validée, le normal et les builds intégrant D14 ne doivent pas être déclarés `TARGET_BUILD_VERIFIED`. Le test PC `tests/CMakeLists.txt` peut continuer à utiliser les sources **immutable `maquettes/deploiement/legacy/` comme oracles historiques uniquement**, jamais comme source du BIN. Le test isolé T03 sur la maquette reste un **oracle de format/Flash**, sans autorisation implicite de consommer cette maquette dans la production.

La séquence LCD de production `warship_panel_profile.c` (23 INIT, 6 RUNTIME), orientation `MADCTL=0x28`, inversion/gamma, SPI2 20 MHz, PWM GPIO21 40%/5kHz, RGB565BE et DMA 5120 o **n'est pas modifiée**.

**Décision Pilotage attendue :** prioriser la livraison conforme des assets par le Graphiste/DA ; **ou** instruire une exception transitoire pour D14, explicitement approuvée PO/Pilotage, avec son périmètre/terme, avant toute compilation permettant le chemin ancien. **Aucune exception n'est présumée par le DEV.** PR #91 reste Draft, D15 non engagée, aucun nouveau flash.
