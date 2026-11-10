# D14 #69 — LCD de production G03 : réception CYD du NORMAL à réaliser

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


Mandat : [Issue #69 commentaire 6767686696](https://github.com/Kharn27/ESP32-OTA-Battleship/issues/69#issuecomment-6767686696) · [PR #91 commentaire 6767696784](https://github.com/Kharn27/ESP32-OTA-Battleship/pull/91#issuecomment-6767696784).

## Décision et autorité

Le PO a validé visuellement sur une vraie CYD la sonde `DOOM_NATIVE_LANDSCAPE` (G01 : « c'est nickel »). Cette acceptation **rapportée** concerne l'image et les couleurs de la **sonde**, pas un essai physique du nouveau firmware NORMAL. Le PO ne valide par cela ni les interactions D15 ni le tactile D14/G03. PR #91 reste Draft ; **fusion exclusivement Pilotage**. D15 #70 est hors périmètre et bloquée par la fusion/réception D14.

## Séquence exacte intégrée au chemin normal

1. LCD `warship_lcd_init` : LEDC GPIO21 arrêté niveau bas ; une bande DMA 5120 octets internes ; SPI2 20 MHz ; reset logiciel ILI9341 (20 ms) + 130 ms ; **23 commandes ILI9341_2_DRIVER TFT_eSPI V2.5.43** transposées C, avec paramètres/délais, incluant DISPON/INVON. Aucun `esp_lcd_panel_init` générique qui changerait les registres analogiques.
2. LCD `warship_lcd_prepare_console` : `MADCTL=0x28` **AVANT le premier clear / CASET / RASET / RAMWR**, soit MV=1, BGR=1, MX=MY=0, panneau 320×240. `esp_lcd_panel_draw_bitmap` conserve la propriété des fenêtres : `CASET 0000..013F`, `RASET 0000..0007` à `00E8..00EF`, `RAMWR 0x2C`. 30 bandes de 320×8 RGB565BE, 5120 octets par bande, sans framebuffer complet. Aucune rotation par `mirror()/swap_xy()` après écriture directe de MADCTL.
3. **Après clear, avant BL ON**, six commandes runtime : `F2`, `26`, gamma négative Doom dans E0/E1, `INVON` deux fois. RGB565BE MSB-first déjà dans l'ordre DMA, **sans permutation des octets**. PWM LEDC GPIO21 5 kHz, duty 40 % expérimental (~409/1023), extinction fail-closed en cas de défaut.
4. D09 normal démarre sur G03 technique ; D07, cinq navires/17 cases, AUTO/RETOUR/PRÊT, D10/FEU, D11/NVS et coordonnées tactiles 320×240 restent dans leur périmètre existant. **Leur concordance physique avec la nouvelle orientation n'est pas prouvée sans essai matériel normal.** Aucun HOME G01 réel ni PALMARÈS D15 fictif.

Source de production : `components/warship_panel_profile/` (C99/const data et oracle fenêtre) et `components/warship_lcd/`. Hôte : `tests/test_warship_panel_profile.c` compile **la même source C** en test (ordres INIT/runtime, paramètres gamma, MADCTL, fenêtres edges 240/320, erreurs), complété par les tests D14 G03 / LEDC conservés. CI : normal + D11 + D13 + T03, pas de sons D14 COLOR/DOOM. Source artistique G01 PR90 : SHA256 `ca73f029e80c95c09b3c7defe1437bba3156280b2f2169465ee3f30014343873`, blob Git `7bd321c6ccbfc230802e20010bfa06e71a9099a9` conservés après réconciliation des 20 commits de `main`.

## Réception finale à demander au PO via Pilotage : UNE CYD, firmware NORMAL

Sur le **HEAD final 3/3 vert**, télécharger **uniquement** l'artefact CI `warship-esp32-firmware-<HEAD>` : APP `build/cyd_warship.bin`, bootloader/partition/arguments correspondants. Consigner le HEAD et le **SHA256 du BIN effectivement utilisé**. Ne pas utiliser les anciennes sondes ni faire `erase-flash` : NVS et palmarès à préserver. Flash selon `flasher_args.json` du même artefact, UART 115200, reset.

1. Photo G03 au boot : fidélité des couleurs, géométrie, lisibilité et 5 navires/17 cases.
2. **AUTO ×3**, contrôler absence de fantômes, latence et 17 cases légales.
3. **RETOUR → START → PRÊT**, puis éventuellement un **FEU** en écran de combat technique D10. Vérifier la concordance des hitboxes 320×240, les taps au stylet et absence de doubles actions.
4. Collecter UART `PANEL_PROFILE phase=INIT/ROTATE_BEFORE_CLEAR/CLEAR_DONE/RUNTIME/READY`, `PANEL_WINDOW`, `WARSHIP_BL`, `D14_DRAW`, `MEM` et `COUNTERS`. Noter heap8, largest8, pile **≥1024 o après FEU**, `lcd_fault=0`, `timeouts=0`, `touch_spi_errors=0`, PWM et temps.
5. PO : verdict **conforme / non conforme** sur G03, couleur, orientation/tactile, AUTO/RETOUR/PRÊT/FEU ; joindre photo, logs, modèle de CYD, SHA APP effectivement flashé.

**Avant cet essai :** `ON_DEVICE_VERIFIED=NON TESTÉ` pour le nouveau firmware normal. Une CI réussie ou le test de la sonde ne le remplacent pas. Seul Pilotage arbitre et fusionne la PR. D15 pas lancé.
