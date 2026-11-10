# D14 #69 — Sonde DoomRPG de référence LCD (TEST-ONLY)

**Mandat :** Issue #69 commentaire #6765376676 et PR #91 commentaire #6765383053 (10/11/2026).
**Statut :** investigation matérielle, A et DOOM_REFERENCE sur deux BIN indépendants. Pas de correction produit validée. PR Draft, non fusionnée.

## Origine et traçabilité

- DoomRPG-RE `ESP32/platformio.ini` sur `main` déclare `bodmer/TFT_eSPI @ ^2.5.43`, `ILI9341_2_DRIVER`, `TFT_INVERSION_ON`, `TFT_RGB_ORDER=TFT_BGR`, `TFT_RST=-1`, et `SPI_FREQUENCY=55000000` (ce dernier n'est PAS copié : D14 garde 20 MHz).
- Source **figée pour cette expérience** : Bodmer/TFT_eSPI **tag `V2.5.43`**, commit complet `6ae4c97c5a07bd64fce8993e9cfa95d9a81e7c81`, fichiers `TFT_Drivers/ILI9341_Init.h`, `ILI9341_Rotation.h`, `TFT_eSPI.cpp`. La plage `^2.5.43` n'est **pas un lock de build** : sans le répertoire `.pio/libdeps`/sortie `pio pkg list` du DoomRPG physiquement flashé, la révision *effectivement résolue* reste **À VÉRIFIER**. Ne pas appeler cette sonde bit-identique au firmware DoomRPG réel avant cette preuve.
- Runtime DoomRPG : `ESP32/src/platform_video.cpp` `applyCydInvertedGammaProfile()` : `F2=00`, `26=01`, `E0=E1=00 15 17 07 11 06 2B 56 3C 05 10 0F 3F 3F 0F`, puis `invertDisplay(true)` (TFT_eSPI écrit `21` deux fois). `setSwapBytes(true)` agit sur les uint16 hôte pour `pushPixels`, pas sur les octets RGB565BE déjà MSB-first de D14.
- D14 normal : `components/warship_lcd/idf_component.yml` verrouille ESP-IDF **5.5.6** et `espressif/esp_lcd_ili9341==2.1.0`. Source de comparaison Espressif `esp-bsp/components/lcd/esp_lcd_ili9341/esp_lcd_ili9341.c` déclarant `version: 2.1.0` ; les différences sont celles de cette source et doivent être revérifiées contre les `managed_components` du build effectif.
- Source G01 PR90 inchangée : SHA-256 PNG `ca73f029e80c95c09b3c7defe1437bba3156280b2f2169465ee3f30014343873` ; RAW565BE 153600 octets `8a376f7ca4cf83ed31b146905b25c80455b71fdef194eee8c40ac00e2d5248dc`. L'audit indépendant Pillow→ELF/BIN est publié dans `docs/D14_PILLOW_AUDIT.md`.

## Comparaison détaillée (ordre par étape, valeurs hexadécimales)

La colonne IDF décrit l'initialisation **normale** après SWRESET; la séquence native débute par SLPOUT+100ms, MADCTL=00, COLMOD=55, puis les commandes vendor. La colonne Doom décrit le chemin **TFT_eSPI ILI9341_2_DRIVER V2.5.43**, SWRESET+150ms, vendor ci-dessous, SLPOUT+120ms, DISPON, INVON et `setRotation()`; enfin la calibration du fichier runtime.

| Cmd | IDF normal 2.1.0 | DoomRPG référence V2.5.43 | Observation |
|---|---|---|---|
| `01` reset | software reset+20ms | software reset+150ms | temporisation différente |
| `11` SLPOUT | avant vendor, pause 100ms | après vendor, pause 120ms | ordre/délai différents |
| `CF` | 00 AA E0 | 00 C1 30 | power control B |
| `ED` | 67 03 12 81 | 64 03 12 81 | séquence alimentation |
| `E8` | 8A 01 78 | 85 00 78 | timing driver |
| `CB` | 39 2C 00 34 02 | 39 2C 00 34 02 | identique |
| `F7` | 20 (2 écritures consécutives dans source) | 20 (1 écriture) | redondance IDF |
| `EA` | 00 00 | 00 00 | identique |
| `C0` | 23 | 10 | **power control** |
| `C1` | 11 | 00 | **power control** |
| `C5` | 43 4C | 30 30 | **VCOM** |
| `C7` | A0 | B7 | **VCOM** |
| `3A` | 55 (avant vendor) | 55 (après analogique) | RGB565, identique valeur |
| `36` | 00 au boot, puis 80 en case10 | 08 portrait au boot, rotation, puis **88 case10** pour la sonde | BGR bit3; géométrie PO inchangée |
| `B1` | 00 1B | 00 1A | fréquence frame |
| `B6` | 08 82 27 | 08 82 27 | identique |
| `B7` | 07 | absent | IDF seulement |
| `F2/26` | 00 / 01 | 00 / 01 | identiques |
| `E0` | 1F 36 36 3A 0C 05 4F 87 3C 08 11 35 19 13 00 | stock 0F 2A 28 08 0E 08 54 A9 43 0A 0F 00 00 00 00 → calibré **E1** | positif remplacé par négatif |
| `E1` | 00 09 09 05 13 0A 30 78 43 07 0E 0A 26 2C 1F | 00 15 17 07 11 06 2B 56 3C 05 10 0F 3F 3F 0F | négatif DoomRPG |
| `2B/2A` | généré à chaque DMA | init 00 00 01 3F / 00 00 00 EF, puis réécrit par le transport | initialisation sans persistance du cadrage |
| `29` DISPON | par `esp_lcd_panel_disp_on_off` | après 120ms SLPOUT | même état, moment différent |
| `21` INVON | absent du profil normal A | TFT_INVERSION_ON + 2 commandes au runtime | inversion ON |

Les différences `CF/ED/E8/C0/C1/C5/C7/B1` et l'ordre complet ne figuraient PAS dans l'ancienne sonde D; son rejet n'invalide pas DOOM_REFERENCE.

## Parité et limites de la sonde

- Pour DOOM_REFERENCE seulement, `warship_lcd_init()` conserve les objets panel/IO Espressif, SPI2 20 MHz, gestion DMA et backlight; il saute **uniquement** `esp_lcd_panel_init()` et envoie la liste V2.5.43 native via `esp_lcd_panel_io_tx_param`, après `esp_lcd_panel_reset` (20ms)+130ms additionnels. Le script finit par DISPON et INVON, puis le `prepare_console` fait 30 bandes de clear avant que `warship_lcd_doom_reference_finish()` programme calibration runtime et MADCTL 88. Ces bandes précèdent gamma finale **backlight OFF**. Aucun transfert en vol au moment d'un paramètre.
- `0x88` maintient **case10 MY=1, MX=0, MV=0, BGR=1**. Le DoomRPG en rotation 1 emploie `0x28` (MV=1, BGR=1) dans TFT_eSPI. **L'orientation n'est donc pas bit-identique**, mais case10 320×240 est une contrainte du test Pilotage. Les commandes analogiques / gamma / inversion / ordre couleurs sont transposées. La sortie est RGB565BE MSB-first **sans swap**.
- Le code normal ne compile pas ce chemin. Les deux diagnostics ne compilent ni la boucle de jeu ni NVS/tactile, ne formatent pas la Flash, n'utilisent pas PSRAM/microSD. Le G01 et les demi-teintes ne sont jamais corrigés par logiciel.
- Les erreurs `esp_lcd_panel_io_tx_param` et DMA sont vérifiées et verrouillent le LCD ; LEDC forcé OFF. Seule la validation CYD du PO pourra dire si les demi-teintes sont fidèles. Aucune parité réelle du signal SPI, GRAM ou gamma n'est présumée.

## Reproduction et réception proposée (une CYD, deux resets)

1. Compiler / obtenir les **deux artefacts du même HEAD et de la même CI** : `build-d14-color` (`WARSHIP_D14_COLOR_DIAGNOSTIC=ON`, profil A) et `build-d14-doom` (`WARSHIP_D14_DOOM_REFERENCE=ON`). Relever leurs SHA256 APP BIN et celui du HEAD. Vérifier la partition/bootloader/APP du même artefact, **sans `erase-flash`**. Le firmware normal `build/` est distinct.
2. Flasher **A**, reset, UART 115200 ; photo aplats 3,5s, demi-teintes 3,5s (gradients orange/rouge/bleu/gris et échantillons G01), puis G01 PR90 fixe ; photo à exposition comparable. Copier `D14_COLOR_SOURCE/STEP/DRAW/MEM/DONE`, `WARSHIP_BL`, statut LCD faults/timeouts.
3. Flasher **DOOM_REFERENCE**, reset matériel **indépendant**, photos du **même ordre** et du même G01 réel ; copier en plus `D14_DOOM_PROFILE phase=INIT/RUNTIME/FINAL/DONE`. Ne jamais passer A→Doom par écritures successives dans une même session.
4. PO rend un avis **séparé** pour demi-teintes et G01, A vs Doom (fidèle/délavé, zones en cause). Consigner heap8, largest8, pile >=1024 bytes, `lcd_fault=0`, `timeouts=0` avec les logs. L'absence de photos/logs reste `NON TESTÉ` et ne permet pas d'attribuer une cause.
5. Restaurer **le binaire normal du même HEAD**, toujours sans effacement NVS. Transmettre au Pilotage logs, photos, identification de la CYD et hashes exacts. Seul Pilotage décide d'une adaptation minimale du normal et fusionne la PR.

**Aucun flash ni verdict écran/tactile revendiqué par le développeur.**

## Référence PC des demi-teintes (générée des mêmes octets C99)

Le programme C hôte `d14_doom_ref_tests --dump-midtones <fichier>` sérialise les **30 bandes de 5120 octets**, soit **153600 octets** du même `d14_doom_midtone_band()` que les deux firmwares TEST-ONLY. Le script `tools/d14_doom_reference_preview.py` relit exclusivement ce RAW, convertit par décalage des composants 5/6/5 en RGB888 sans LUT, puis crée `d14_midtones_pc_reference.png` et `d14_midtones_pc_reference.json` (SHA raw + échantillons). L'archive de chaque job hôte GCC/Clang contient le RAW et la preview ; elle sert à confronter la couleur de la CYD à la même image numérique. Les différences de prise de vue ne valent pas mesure optique instrumentée.
