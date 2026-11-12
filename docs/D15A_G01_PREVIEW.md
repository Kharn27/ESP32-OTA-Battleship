# D15-A #112 — Accueil G01, firmware de prévisualisation isolé

**Mandat :** [Issue #112](https://github.com/Kharn27/ESP32-OTA-Battleship/issues/112).
**Ce jalon n'est PAS le parcours produit D15 #70.** Pas de jeu, pas de navigation,
pas de lecture/écriture NVS, pas de toucher SPI3 et pas de changement D14 G03.
La PR D14 #91 et le blocage d'assets G03 restent indépendants.

## Entrées artistiques autorisées

Le générateur `tools/d15a_generate_g01.py` prend **seulement**
les quatre exports CYD listés ici, avec vérification SHA-256 et dimensions
à chaque génération par `assets/manifest.json`. Les originaux HD, `draft/`
et `maquettes/` ne sont jamais dans le binaire :

| ID approuvé | PNG utilisé à la compilation | Taille | Position |
| --- | --- | --- | --- |
| `wallpaper.accueil.retenu.cyd` | `assets/wallpaper/accueil_retenu_CYD_320x240.png` | 320×240 | 0,0 |
| `logo.titre.cyd` | `assets/logo/logo_titre_CYD_145x66_RGBA.png` | 145×66 | 88,8 |
| `boutons.jouer.cyd` | `assets/boutons/bouton_jouer_CYD_140x32_RGBA.png` | 140×32 | 14,198 |
| `boutons.palmares.cyd` | `assets/boutons/bouton_palmares_CYD_140x30_RGBA.png` | 140×30 | 166,199 |

L'ordre de composition est **fond → logo → JOUER → PALMARÈS**.
Le PNG de composition actuelle validée sur PC `maquettes/accueil/accueil_cyd_warship_320x240.png`
sert **uniquement** d'oracle de comparaison PC ; il n'est jamais
utilisé pour générer le C embarqué.

## Format ROM, RAM et comportement C

- Fond 320×240 en **RAW RGB565BE** : **153 600 octets en Flash**, pas en RAM.
  Le fond est une photographie/illustration détaillée ; RAW supprime
  la surcharge RLE et le coût d'un décodeur face à des pixels peu répétitifs.
  Un encodage RLE/T03 n'est pas justifié sans mesure de compression.
- Trois images séparées en **RGB565BE + alpha8 droit** :
  145×66×3 + 140×32×3 + 140×30×3 = **54 750 octets**.
  Total **208 350 octets de pixels en `const`** hors tables et code.
- `components/d15a_g01/g01_compose.c` : C99, décodage direct et
  alpha compositing entier en canaux 5/6/5 dans **un seul buffer
  DMA interne de 5 120 octets** fourni par le LCD. Dessin en 30 bandes
  320×8 ; pas de framebuffer 153 600 o, allocation ou pile graphique.
  Couche opaque et alpha zéro sont des cas exacts ; autres alpha
  mélangés puis quantifiés sur la grille native RGB565.
- Entrée indépendante `main/d15a_g01_preview.c`, sélectionnée UNIQUEMENT
  avec `-D WARSHIP_D15A_G01_TEST_ONLY=ON`. Log clair
  `D15A_G01_TEST_ONLY=ON`. Aucun bouton actif. Le firmware normal
  continue d'exécuter `app_main.c` + D09/D11 comme avant.
- Driver de la démo `components/warship_lcd/warship_lcd_d15a_preview.c`
  = **copie byte-identique** de la source D14 PR #91 ;
  `warship_lcd_light.c/.h` et `warship_panel_profile/` également
  repris sans altérer les registres/DMA ou l'algorithme PWM.
  Sélection CMake exclusivement pour le mode TEST-ONLY ;
  l'ancien `warship_lcd.c` reste propriétaire du firmware normal de `main`.
  Profil : **23 INIT, 6 RUNTIME, MADCTL 0x28 avant clear**, gamma Doom,
  inversion+BGR, SPI2 20MHz, LEDC GPIO21 5kHz/40%, fail-closed.

## Reproductibilité et comparaison PC

En environnement Python avec Pillow **12.3.0** :

```bash
python3 tools/d15a_generate_g01.py --repo . \
  --output /tmp/d15a_g01.c --report /tmp/d15a_g01.json \
  --preview /tmp/d15a_g01_firmware.png \
  --reference maquettes/accueil/accueil_cyd_warship_320x240.png
python3 scripts/check_graphic_pipeline.py
cmake -S tests -B build-host -G Ninja -DWARSHIP_SANITIZERS=ON
cmake --build build-host
ctest --test-dir build-host --output-on-failure
```

Le rapport JSON donne empreintes SHA256 des **4 fichiers PNG**,
données RGB565BE/alpha et C généré, métriques `different_pixels`,
`mean_abs_rgb_channel`, `max_abs_channel` pour :
1. maquette vs recomposition quatre calques RGB888 (écart de source),
2. recomposition RGB888 vs rendu exact RGB565 (coût de quantification),
3. maquette vs rendu exact RGB565 (écart total).

L'image PC `d15a_g01_firmware.png` est **un aperçu simulé après
quantification**, PAS une preuve CYD. La CI régénère le même C
deux fois et compare bit à bit. Les tests C99 vérifient quatre
descripteurs distincts, géométrie, transparences, bornes, 30 bandes,
sentries DMA et l'oracle LCD + PWM réutilisés.

## Firmware et réception physique — pilotage uniquement

Pour compiler la variante isolée :

```bash
idf.py -B build-d15a-g01 -D WARSHIP_D15A_G01_TEST_ONLY=ON set-target esp32
idf.py -B build-d15a-g01 -D WARSHIP_D15A_G01_TEST_ONLY=ON build
idf.py -B build-d15a-g01 size
sha256sum build-d15a-g01/cyd_warship.bin
```

L'artefact de CI **`d15a-G01-TEST-ONLY-<HEAD>`** inclura APP,
bootloader, partition, `flasher_args.json`, ELF et map. Il ne doit
**jamais** être confondu avec le normal `warship-esp32-firmware-<HEAD>`.

**Après revue du Pilotage seulement**, sur **une CYD classique**
ESP32-2432S028R : consigner HEAD et SHA256 du BIN APP de l'artefact,
flasher selon les offsets de `flasher_args.json` sans **`erase-flash`**,
surveiller l'UART **115200**, photographier 320×240, comparer contraste,
logo, halos alpha et deux boutons avec la maquette validée. Observer
`PANEL_PROFILE` (`INIT/ROTATE_BEFORE_CLEAR/CLEAR_DONE/RUNTIME/READY`),
`PANEL_WINDOW`, `WARSHIP_BL`, `G01_DRAW_PASS`, `G01_MEM`
(`heap8`, `largest8`, `stack_min_free_bytes`) et `G01_COUNTERS`
(`lcd_fault=0`, `timeouts=0`). Confirmer NVS non effacée, aucune
navigation au toucher ; absence de tactile initialisé = intentionnelle.

**Statuts séparés :** `HOST_TESTED`, `TARGET_BUILD_VERIFIED`,
`CI_PASSED` ne sont à déclarer PASS que sur preuves SHA/CI précises.
`ON_DEVICE_VERIFIED` du mode isolé reste **NON TESTÉ** sans la photo,
la carte et les logs correspondants. Aucun GO implicite pour D15 #70.
