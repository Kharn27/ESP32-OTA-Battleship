# D14 — G03 runtime Flash/TFT (Issue #69)

**Mandat :** [Issue #69](https://github.com/Kharn27/ESP32-OTA-Battleship/issues/69), [GO Pilotage du 08/11/2026](https://github.com/Kharn27/ESP32-OTA-Battleship/issues/69#issuecomment-6714475888). Base initiale `main` `4163d79ae5009a8970ee97df7a04108a41368ff7` (T03 fusionnée). Le **PO garde la décision de fidélité graphique/ergonomique**, Pilotage seul fusionne.

## Provenance et limite artistique

Les seuls sources sont les deux fichiers **inchangés** dans `design/ecrans/deploiement/` :
- `g03_deploiement_320x240.png` — export **canonique PO DESIGN_PREVIEW** 320×240, y compris retouches des trois boutons.
- `g03_deploiement_320x240.svg` — cinq définitions G02 `<g id="nav-...">`, cinq `<use>` de placement; voir `export_previews.py` qui garantit identité textuelle avec G02.

`tools/d14_export.py` vérifie les cinq noms, longueurs, emplacements, orientation et transformations SVG. Il rasterise l'écran **sans les cinq `<use>`** et remplace **uniquement les cinq rectangles de placement initial** dans le PNG canonique par ce fond vide. **Tous les autres pixels RGB888, notamment les boutons retouchés, restent identiques octet pour octet** : un contrôle Python rejette toute divergence hors zones dynamiques. Ces zones ont la dimension des navires G03 à leur position d'origine, soit `longueur*18 × 18` horizontalement et `18 × longueur*18` verticalement.

La même source G02 définit les **dix orientations de sprites**, via CairoSVG sur fond transparent sans modification artistique. Chaque sprite opaque/antialiasé est exporté en triplets `(RGB565 high,low, alpha8)`, sans allocation runtime. Transparence v1 : alpha 0 = conserver le pixel de fond, 255 = remplacer, 1..254 = interpolation entière **dans les canaux 5/6/5**. Ce compositing peut différer visuellement de la prévisualisation 24 bits à la frontière des silhouettes ; **la validation PO sur vrai TFT reste obligatoire**. Aucune retouche de couleur/taille/glyphe n'a été effectuée par DEV. Si le rendu de ces bords n'est pas accepté, retourner au Graphiste/Pilotage plutôt que peindre des compensations.

Le convertisseur s'exécute **à la configuration CMake**, dans le build (pas dans `design/`) avec `Python3`, **CairoSVG==2.7.1 / Pillow==11.3.0** (mêmes versions que l'export source G03). Le résultat généré `d14_g03_data.c` contient un fond opaque **ROW-RLE565BE v1** et dix masques Flash. Un export répété doit produire le même SHA-256; la CI effectue un `cmp` de deux générations. La signature des PNG/SVG, les tailles et l'identifiant de flux sont imprimés par le générateur.

## Architecture runtime bornée

`components/d14_g03/` expose une fonction de composition C99 pure indépendante de FreeRTOS/ESP-IDF. Elle accepte exclusivement `WarshipDeployFrame.cells[100]` **dérivé de l'observation publique D07**, pas `WarshipLifecycle`, pas `game.captain`. Elle valide les cinq identités et les 17 cases avec ligne/colonne et alignement; aucune copie durable de flotte (les cinq coordonnées temporaires sont locales). `main/d09_probe.c` garde la possession unique du modèle D07 et du LCD, tactile et NVS.

**G03 runtime** :
- Écran 320×240, grille `x=18,y=42`, 10×10, pas 18. **Row = A..J vertical**, **Col = 1..10 horizontal** ; `(col,row)` D07 (0,0) correspond à `A1`, `(1,5)` à `F2`.
- Cibles stylet `AUTO (208,47,108,48)`, `PRÊT (208,104,108,54)`, `RETOUR (208,167,108,54)`; le dispatcher B01 retire **2 pixels de bord** et conserve 40 ms DOWN/60 ms UP/120 ms inter-gestes. Grille non interactive : **pas de placement manuel**. Ces zones **sont proposées pour essai**, pas décidées par le PO.
- Au boot et en revanche : fond + 5 coques dérivées de `WarshipLifecyclePublic.human` via `warship_deploy_make_frame`. En revanche D07 conserve la formation précédente jusqu'à AUTO; ce comportement métier reste inchangé.
- Après AUTO : invalider `(18,42,180,180)` **en entier**, décoder le fond propre dans chaque bande et composer la nouvelle flotte. L'ancienne formation n'est jamais réutilisée. Les boutons/labels/fond hors grille ne sont pas recalculés. Un appui pendant rafraîchissement est consommé selon le dispatcher historique, sans mise en attente.
- PRÊT : moteur D07 valide avant changement de phase, puis D10 technique inchangé. RETOUR avant PRÊT : HOME technique D09/START, sans faux menu G01/PALMARÈS, aucun forfait.
- Bandes de maximum `320×8×2=5120` octets empruntées au pilote LCD SPI2 synchrone; **aucun nouveau DMA, PSRAM, microSD, framebuffer plein écran ni réinitialisation SPI**. L'appel de transport ne vient qu'après une bande entièrement reconstituée. Aucun nouvel ACK/NVS, aucune interaction avec le secret du Capitaine; D13 purge et D11 registre préservés.
- Journal `D14_DRAW reason=DEPLOY_full|AUTO_grid_clean ... compose_us/lcd_us/total_us/bands/dma_bytes` et `MEM phase=...`. Les temps `lcd_us` incluent les commandes et l'attente de terminaison DMA, pas uniquement les bits SPI.

## Validation

Tests C99 `tests/test_d14_g03.c` : étiquettes et limites de grille, boutons et HOME, cinq navires valides 17 cases, compositing sur les 30 bandes, reconstitution seconde AUTO vs rendu neuf, disparition de l'ancien navire dans sa case, données RLE et frames invalides, maintien de PRÊT (B01). La conversion rejette un SVG ne possédant pas les 5 placements G02 attendus. Tests GCC/Clang ASan+UBSan et CMake cible **à rapporter au HEAD exact** ; ne pas prétendre tester le matériel par la CI.

**Réception 1 CYD à demander par Pilotage :** récupérer l'artefact **normal** de CI sur le HEAD final (ne pas flasher la variante T03 si non explicitement demandée), vérifier le SHA BIN; avec esptool/VS Code, flasher bootloader/partitions/application d'un **même artefact**. Sur une ESP32-2432S028R 320×240 en paysage : contrôler la qualité de la mer, des cinq coques G02 et des 3 boutons; appuyer AUTO **3 fois**, avec différence de placement à chaque fois **non exigée** (coïncidences RNG légales), vérifier 5 coques/17 cases et **zéro ombre de l'ancien emplacement**; vérifier le retour au HOME technique sans forfait, START, PRÊT et passage COMBAT technique D10. Enregistrer la photo de l'écran de déploiement, le SHA flashé, `D14_DRAW`, `MEM phase=after_action/running`, `timeouts=0`, `lcd_fault=0`, pile main watermark **>=1024 o** après FEU si un tir de contrôle est possible. Aucun essai prolongé de 20 tirs requis. Tout doute de lisibilité/cadrage/hitbox relève du PO.

**Niveaux :** `HOST_TESTED`, `TARGET_BUILD_VERIFIED`, `CI_PASSED` uniquement avec logs HEAD ; `ON_DEVICE_UNVERIFIED` tant qu'aucune CYD n'a été flashée sur D14. L'objectif reste la fidélité et l'effacement AUTO sans fantômes; les temps ne doivent pas être extrapolés des 88 ms T03 statiques, qui ne mesurent pas la composition.
