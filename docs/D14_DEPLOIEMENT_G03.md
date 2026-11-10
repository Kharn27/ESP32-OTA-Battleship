# D14 — G03 runtime Flash/TFT (Issue #69)

> **P0 — BLOCAGE_ASSETS_G03 (10/11/2026) :** la branche est reprise
> avec la migration graphique de `main`. Le catalogue `assets/` ne
> comprend pas encore les composants G03/G02 nécessaires à la production.
> **Aucun nouveau firmware NORMAL D14 ne doit être flashé ni déclaré
> compilé sur la base actuelle** ; le CMake production échoue
> volontairement plutôt que d'embarquer la maquette `legacy/` sans
> dérogation. Le profil LCD validé est préservé dans le code et les tests.
> Voir [inventaire et passation Graphiste](D14_G03_ASSETS_BLOCKER.md).
> Les mesures/artefacts précédents sont historiques et non recevables
> pour le HEAD corrigé.


> **État courant (10/11/2026) :** profil LCD PO-accepté intégré au
> firmware normal : `MADCTL=0x28` natif paysage, gamma DoomRPG,
> inversion ON, BGR, PWM40. Les indications « case10 » et comparaisons
> anciennes qui suivent sont historiques.
> Voir [D14_LCD_PRODUCTION.md](D14_LCD_PRODUCTION.md).



## Correctif de réception 09/11/2026 — rétroéclairage LEDC, ordre Pilotage

**Verdict PO sur l'ancien HEAD `5670bc84c98f1d9c87b9f02ef6e3e4335c3e8779` : `CORRECTIONS_REQUISES`.** Le PO a réellement flashé un **build local** (identifié par `App version=5670bc8`, SHA-256 BIN local **non fourni**). Le rejet était **« luminosité extrême, image délavée »**, et **non** un défaut prouvé des assets G03. Ne pas confondre cette preuve avec l'artefact CI, ni avec la réception du nouveau correctif.

Mesures CYD reçues via [Pilotage / Issue #69](https://github.com/Kharn27/ESP32-OTA-Battleship/issues/69#issuecomment-6743417012) sur l'ancien HEAD : G03 complet **96 423 µs** (composition **28 802**, LCD **67 360**) ; AUTO ×3 **48 797 / 48 476 / 48 356 µs** ; heap8 **290 276**, largest8 **163 840**, marge pile main **2 216 o** au déploiement et **1 448 o** après FEU ; **0** défaut LCD/DMA/tactile, ACK D10 après révélation. Ces temps/perfs **ne prouvent pas** la fidélité visuelle ni la correction de luminosité.

### PWM expérimental (firmware corrigé)

Le seul changement d'affichage demandé est le **rétroéclairage LCD GPIO21** : GPIO actif-haut **à confirmer physiquement**, LEDC **LOW_SPEED / timer0 / canal0, 5 000 Hz, résolution 10 bits**, sortie initialement **OFF** jusqu'au premier écran prêt, duty de départ **40 % expérimental**, sans décision PO. La fréquence effective est lue avec `ledc_get_freq()` ; les journaux `WARSHIP_BL: PWM_INIT` / `PWM_STATE` / `PWM_TARGET` donnent la fréquence effective, le duty **appliqué par API** (`0..1023`) et la consigne en pourcentage. Cela **ne mesure pas** la lumière optique réelle.

**Réglage reproductible sans toucher aux assets ni aux sources :**

```sh
idf.py -B build-d14 -D WARSHIP_LCD_BACKLIGHT_PERCENT=40 set-target esp32
idf.py -B build-d14 -D WARSHIP_LCD_BACKLIGHT_PERCENT=40 build
# Pour un NOUVEL essai (nouveau build et SHA), remplacer 40 par 25, 60, etc.
sha256sum build-d14/cyd_warship.bin
```

Valeurs admises `0..100`, validées dès la configuration CMake. À l'exécution, API synchrone réservée à la **tâche LCD propriétaire** : `warship_lcd_backlight_set_percent(uint8_t percent)` ; `warship_lcd_backlight(false)` arrête LEDC avec `ledc_stop(...,idle_level=0)`, `true` redémarre le PWM au pourcentage choisi. Si une commande GPIO/LEDC ou le DMA échoue : extinction par **LEDC stop à niveau bas**, puis reprise de contrôle du GPIO en sortie LOW si stop échoue, état LCD `fault` bloquant les nouveaux transferts. Pas de PWM en ISR. L'ancienne instruction `gpio_set_level(21,0)` seule n'est plus utilisée après que LEDC possède la broche. Aucune adaptation de couleur/gamma/MADCTL ni du bus SPI2 à ce stade.

### Dépendances locales du générateur et alternative CI

Un build **depuis les sources** exécute `tools/d14_export.py` à la **configuration CMake**, dans l'interpréteur Python sélectionné par `find_package(Python3)`. Il requiert **CairoSVG==2.7.1** et **Pillow==11.3.0** sur le **PC** ; ni Python ni ces bibliothèques ne sont embarqués sur ESP32. Le PO a constaté `ModuleNotFoundError: No module named 'cairosvg'` avec le venv ESP-IDF local. Installation dans **le Python réellement utilisé par CMake/ESP-IDF**, par exemple :

```sh
/path/to/python3 -m pip install 'CairoSVG==2.7.1' 'Pillow==11.3.0'
# Remplacer /path/to/python3 par l'exécutable affiché par CMake/IDF,
# souvent l'environnement virtuel ESP-IDF, pas forcément python3 global.
idf.py -B build-d14 build
```

**Alternative sans génération locale :** utiliser l'artefact **firmware normal** de la CI **du HEAD corrigé** et flasher application + bootloader + partitions du **même** artefact selon `flasher_args.json`, vérifier son SHA-256. Ne pas utiliser l'artefact `T03-TEST-ONLY`, ne pas `erase-flash` (scores NVS). **Flasher le BIN précompilé ne dépend pas de CairoSVG/Pillow.** Les dépendances locales restent une dette mineure de l'outillage ; pas de refonte des assets dans D14.

### Réception ciblée du seul correctif (1 CYD)

Après CI **3/3** du **nouveau HEAD**, flasher le BIN normal, conserver son SHA/version, relever `WARSHIP_BL` + `D14_DRAW` et `MEM`. Le PO vérifie **le confort lumineux et le contraste au démarrage G03**, puis effectue **un AUTO** pour vérifier l'absence de régression visible. Relever heap8, largest8, pile et `timeouts=0`, `lcd_fault=0`; transmettre en une phrase le verdict `G03 confortable/fidèle` ou défaut visuel précis. **Aucune nouvelle campagne de partie ni 20 tirs.** En cas d'image encore délavée malgré le PWM, le Pilotage doit décider d'un diagnostic séparé RGB565 / inversion / gamma avant toute retouche artistique. Le nouveau HEAD reste `ON_DEVICE_UNVERIFIED` tant que cet essai ciblé n'a pas eu lieu.


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

**Niveaux :** `HOST_TESTED`, `TARGET_BUILD_VERIFIED`, `CI_PASSED` uniquement avec logs HEAD ; `ON_DEVICE_UNVERIFIED` pour la **correction PWM** tant qu'elle n'a pas été essayée sur une CYD. La première version D14 a reçu un essai PO **visuellement refusé**. L'objectif reste la fidélité et l'effacement AUTO sans fantômes; les temps ne doivent pas être extrapolés des 88 ms T03 statiques, qui ne mesurent pas la composition.
