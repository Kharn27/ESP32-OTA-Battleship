# D14 — Diagnostic comparatif LCD RGB565 / inversion / gamma — TEST ONLY

**Contrat :** [Issue #69, ordre du 10/11/2026](https://github.com/Kharn27/ESP32-OTA-Battleship/issues/69#issuecomment-6762545815) et [complément PR #91](https://github.com/Kharn27/ESP32-OTA-Battleship/pull/91#issuecomment-6762552187). Autorisation sur **branche D14 existante** ; PR Draft et fusion réservée au Pilotage. Ne remplace jamais la configuration produit D14 sans décision PO.

## Pourquoi cette expérience ?

Sur une **même ESP32-2432S028R**, le PO observe les couleurs normales avec les firmwares **DoomRPG-RE** et **ESP32-Locomotive-Basic**, mais une image G03 D14 encore délavée avec un rétroéclairage PWM expérimenté à 40 %. Le seul niveau lumineux ne résout donc pas le défaut chromatique. Ce constat est **PO rapporté**, pas une mesure de saturation. Les deux firmwares témoins servent d'**oracles de comportement matériel** ; il n'est pas démontré que la cause soit l'inversion, BGR, gamma ou la source d'images.

Sources comparées, consultées directement avant implémentation :

- [DoomRPG-RE `ESP32/src/platform_video.cpp`](https://github.com/Kharn27/DoomRPG-RE/blob/main/ESP32/src/platform_video.cpp) : RGB565 framebuffer sans modification, `applyCydInvertedGammaProfile()` commande **F2=00, 26=01, E0 et E1 avec la même table 15 octets**, puis `invertDisplay(true)`. [`platformio.ini`](https://github.com/Kharn27/DoomRPG-RE/blob/main/ESP32/platformio.ini) configure **`ILI9341_2_DRIVER`**, `TFT_INVERSION_ON`, `TFT_RGB_ORDER=TFT_BGR`. C'est un **profil calibré sur ce firmware**, pas une preuve de compatibilité de sa pile complète avec `esp_lcd`.
- [ESP32-Locomotive-Basic `firmware/components/cyd_ili9341/cyd_ili9341.c`](https://github.com/Kharn27/ESP32-Locomotive-Basic/blob/main/firmware/components/cyd_ili9341/cyd_ili9341.c) : **esp_lcd ILI9341** SPI2 20 MHz, `rgb_ele_order=RGB` par défaut (`LCD_BGR=0`), `LCD_RGB_DATA_ENDIAN_BIG`, DMA 5120. Les cas d'orientation indexés **9/10** sont distincts : pour l'index 9, `swap_xy=0, mirror_x=1, mirror_y=0` ; pour l'index 10, `swap_xy=0, mirror_x=0, mirror_y=1`. Les logs de Locomotive exécuté sur la CYD seraient nécessaires pour identifier le cas **effectivement** utilisé lors de sa bonne restitution : ne pas déduire un cas de son seul code.
- [Battleship `warship_lcd.c`](https://github.com/Kharn27/ESP32-OTA-Battleship/blob/31097c892d87e1b3370e236f18d6bf7150178051/components/warship_lcd/warship_lcd.c) : esp_lcd RGB/BIG, 20 MHz, `prepare_console` **case10** `MV=0 MX=0 MY=1`, SPI2 DMA 5120. Aucune inversion/gamma/BGR personnalisée appliquée dans le produit avant le test. **Ne pas changer ce profil automatiquement sur le firmware normal**.

## Schéma A/B/C/D/E — une hypothèse comparée à la fois

Un **unique BIN TEST-ONLY** produit **dix pages successives** en ~35 s (durée 3500 ms * 2 pages * 5 profils). L'ordre est **déterministe et non bouclé** : la dernière page reste visible jusqu'à l'arrêt. À chaque étape, le firmware affiche les mêmes constantes RGB565 (DIRECT), puis les **mêmes** fond G03 ROW-RLE et cinq sprites G02 (G03_RLE_ALPHA), avec positions humaines constantes et **aucune** donnée secrète Capitaine. Les profils ne sont pas nommés à l'écran : associer l'image au log UART `D14_COLOR_STEP case=...` et `D14_COLOR_DRAW page=...`.

| Profil | Inversion panneau | MADCTL BGR | Gamma vendor | Comparaison causale |
|---|---|---|---|---|
| **A** | OFF via `esp_lcd_panel_invert_color(false)` | OFF, RGB initial | inchangée (après init esp_lcd) | référence actuelle |
| **B** | ON via `esp_lcd_panel_invert_color(true)` | OFF, RGB | inchangée | **B contre A** : inversion seule |
| **C** | OFF | ON, MADCTL=0x88 | inchangée | **C contre A** : BGR seul ; inversion remise OFF |
| **D** | ON | OFF, MADCTL=0x80 | **DoomRPG exacte** | **D contre B** : apport du gamma *sous la même inversion* |
| **E** | ON | ON, MADCTL=0x88 | DoomRPG inchangée | **E contre D** : ajout du BGR avec inversion+gamma |

**Aucun rétablissement artificiel d'une gamma « par défaut » n'est revendiqué.** Le profil D écrit la gamma **une seule fois**, après A/B/C, puis E conserve la même gamma. Le profil B teste inversion sans gamma Doom. À C la référence d'orientation est encore le **case10 (MY=1, MX=MV=0)** ; seuls les bits BGR changent avec un écrit explicite **0x36 / 0x88**. À D retour à **0x36 / 0x80**. Ces écritures directes sont exclusivement en TEST-ONLY, sans appel ultérieur à `swap_xy` ou `mirror` susceptible d'écraser le MADCTL du diagnostic. Les résultats réels sont à vérifier sur la dalle : les commandes ne lisent pas l'état effectif des registres.

**Référence DoomRPG (15 octets exacts, du source ci-dessus) :**

`00 15 17 07 11 06 2B 56 3C 05 10 0F 3F 3F 0F`

Pour D, le module envoie (avec `esp_lcd_panel_io_tx_param`) **0xF2 : 00** ; **0x26 : 01** ; **0xE0 : 15 octets ci-dessus** ; **0xE1 : les mêmes 15 octets** ; puis appelle `esp_lcd_panel_invert_color(true)`. Les appels et retours `ESP_OK`/échecs sont tous journalisés. Les autres étapes n'écrivent aucun vendor gamma.

Le protocole source RGB565-BE de `tools/t03_export.py` et `tools/d14_export.py` est `(((R>>3)<<11)|((G>>2)<<5)|(B>>3)` ; `t03_decode_rect()` et `d14_g03_compose()` écrivent `high-byte` puis `low-byte` dans le buffer DMA. `esp_lcd_panel_draw_bitmap` transmet ce buffer avec `LCD_RGB_DATA_ENDIAN_BIG`. La composition RGB565 n'échange pas les canaux ; les seules interpolations concernent l'alpha8 du sprite (régime original inchangé).

**Motif RGB565 direct :** écran natif 320×240, grille artificielle de 4 colonnes de 80 px × 2 rangées de 120 px :

| Zone (x,y) | Couleur RGB565 | Octets DMA |
|---|---|---|
| 0..79, 0..119 | Noir `0x0000` | `00 00` |
| 80..159, 0..119 | Blanc `0xFFFF` | `FF FF` |
| 160..239, 0..119 | Rouge `0xF800` | `F8 00` |
| 240..319, 0..119 | Vert `0x07E0` | `07 E0` |
| 0..79, 120..239 | Bleu `0x001F` | `00 1F` |
| 80..159, 120..239 | Gris moyen `0x8410` | `84 10` |
| 160..239, 120..239 | Dégradé de gris linéaire 5/6/5 `0x0000` vers `0xFFFF` | `00 00` → `FF FF` |
| 240..319, 120..239 | Jaune `0xFFE0` | `FF E0` |

**Sans pipeline RLE sur la page directe**. À la page suivante le **même asset G03 approuvé** est affiché par `d14_g03_compose` + navires issus d'une fixture D07 publique immuable ; aucune retouche PNG/SVG ou permutation de palette. Un écran de test n'est pas une mesure colorimétrique instrumentée.

## Sécurité, build et preuves

- Uniquement lorsque `-D WARSHIP_D14_COLOR_DIAGNOSTIC=ON` est passé à CMake (option **OFF par défaut** au niveau du projet). Entrée `app_main` de diagnostic **avant tout `d11_runtime_init`** : pas de NVS, de palmarès, de jeu, de tactile, de radio ni de mutation de mémoire Flash. Ne pas combiner avec les options D11/D13/T03. Avec l'option OFF, la routine de diagnostic n'est pas compilée dans le binaire normal.
- Toujours le vrai `warship_lcd_init` / `warship_lcd_prepare_console(0x000c)`, SPI2 **20 MHz**, **même tampon DMA 5120 B emprunté** ; bandes 320×8×2, chacune envoyée après reconstruction, attente du DMA. Sur erreur panel, le pilote met sa faute sticky et éteint LEDC ; la routine quitte le test sans retries et tente OFF. PWM expérimental **40%** conservé sur tous les cas A–E.
- Journaux : `D14_COLOR_DIAGNOSTIC=ON TEST_ONLY` ; `WARSHIP_BL PWM_INIT/PWM_STATE` ; `D14_COLOR_PROFILE_BEGIN` et `D14_COLOR_CMD` pour les commandes vendor, `D14_COLOR_INVERSION`, `D14_COLOR_DRAW case=... page=DIRECT_RGB565|G03_RLE_ALPHA`, `D14_COLOR_MEM` avec heap8/largest8, marge pile main et 0 LCD fault/timeout.
- `HOST_TESTED` : `tests/test_d14_color_diag.c` valide les cinq profils A/B/C/D/E et 15 octets de gamma, toutes les couleurs en mémoire et au niveau octet, les erreurs de bornes et le 5120B DMA. Les anciennes suites vérifient le décodage RLE et la composition G03. `TARGET_BUILD_VERIFIED` et `CI_PASSED` **uniquement au HEAD exact**, sans présumer du visuel.

**Build local optionnel** (ESP-IDF v5.5.6, Python hôte avec `CairoSVG==2.7.1` et `Pillow==11.3.0` pour la génération G03) :

```sh
idf.py -B build-d14-color -D WARSHIP_D14_COLOR_DIAGNOSTIC=ON set-target esp32
idf.py -B build-d14-color -D WARSHIP_D14_COLOR_DIAGNOSTIC=ON build
sha256sum build-d14-color/cyd_warship.bin
```

Dans la CI, seul l'artefact **`warship-d14-COLOR-TEST-ONLY-<HEAD>`** contient ce firmware. Le firmware **normal** est construit séparément et doit toujours continuer à lancer D07/D09 sans profils couleur.

## Protocole PO très court — une seule CYD, réversible

1. Après la contre-revue Pilotage et **CI 3/3 sur le HEAD final**, récupérer l'artefact **D14 COLOR TEST ONLY** et son SHA-256 ; flasher **bootloader + partitions + app du même artefact** via `flasher_args.json`, **sans `erase-flash`** pour préserver les scores. Débrancher/rebrancher ou redémarrer pour recommencer le cycle de 35s.
2. Brancher UART **115200**. Noter pour **A, B, C, D, E** au moins le jugement des **aplats directs** et, si possible, du **G03** : `normaux / délavés / inversion gênante / rouge-bleu permutés`. Le log indique le profil/page actif ; durée ~3,5s par image. Le PO peut dire **« A mauvais, B ... D bon »** sans photographier. Collecter `D14_COLOR_CMD`, `D14_COLOR_INVERSION`, `D14_COLOR_DRAW`, `WARSHIP_BL`, `D14_COLOR_MEM`, tout `FAIL`.
3. Si A et B sont délavés mais D redonne un bon contraste, Pilotage peut isoler l'apport gamma de D. Si C améliore A ou E améliore D, arbitrer la composante BGR. Si les aplats directs sont bons dans un cas mais **G03 est délavé dans le même cas**, investiguer ensuite palette/alpha/PNG et rendu visuel avec Graphiste ; ne pas déduire une erreur de panneau. Si **tous les aplats** sont délavés, étudier d'autres registres/profils avec preuve matérielle, sans essai aveugle.
4. **Revenir au binaire normal** de la CI après le diagnostic (même règle bootloader/partitions/app ; sans erase). Le diagnostic n'a pas ouvert NVS. **Aucune partie/FEU, ni 20 tirs**, pas de photo systématique. Le verdict d'acceptation G03 et le choix d'un profil de production restent **exclusivement au PO/Pilotage**.

**État actuel :** tant que le firmware diagnostic n'a pas été flashé par le PO, les profils A/B/C/D/E sont **NON TESTÉS matériellement**. Les essais avec DoomRPG et Locomotive constituent de bonnes références indépendantes, mais ne qualifient pas automatiquement le résultat du diagnostic Battleship.
