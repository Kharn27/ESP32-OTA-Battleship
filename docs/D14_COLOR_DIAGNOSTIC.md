# D14 — Diagnostic colorimétrique G01 PR90 sur une CYD — TEST ONLY

**Contrat actualisé (10/11/2026) :** [Issue #69 ordre technique](https://github.com/Kharn27/ESP32-OTA-Battleship/issues/69#issuecomment-6763479511) et [PR #91 complément](https://github.com/Kharn27/ESP32-OTA-Battleship/pull/91#issuecomment-6763485631). La PR reste **Draft, non fusionnée** et D14 reste `CORRECTIONS_REQUISES` jusqu'à décision PO/Pilotage.

## 1. Faits matériels antérieurs, sans interpolation

Le PO a réellement exécuté la sonde précédente `App version=4251e31` sur l'ESP32-2432S028R classique et rapporte : **« A a les couleurs correctes mais délavées ; les autres [B, C, D, E] sont bullshit »**. Il **n'a pas séparé** son jugement entre aplats et page G03 : aucune conclusion individuelle inventée par type de page. Les commandes gamma/MADCTL et inversion sont revenues `ESP_OK` et la sonde a terminé 10 pages, avec heap8 295608, largest8 172032, marge pile main 2280, `lcd_fault=0`, `timeouts=0` et PWM40 `409/1023` à 5 kHz. Il s'agit de **la première sonde**, et **non du présent binaire**. Le PO rapporte également les couleurs normales sur la même CYD dans Locomotive BASIC et DoomRPG ; cette comparaison ne prouve pas la cause du délavage G01/D14.

Décision Pilotage : **arrêter les hypothèses B/C/D/E** dans l'essai PO. Revenir au seul profil **A = RGB, inversion OFF, gamma ILI9341 d'initialisation inchangée**, mais afficher comme référence artistique la **véritable image accueil PO approuvée**, plus une page d'aplats purement numériques. Pas de modification du profil LCD normal, pas de D15.

## 2. Source G01 PO : provenance vérifiable

| Propriété | Valeur |
|---|---|
| Dépôt source | `Kharn27/ESP32-OTA-Battleship`, `main` |
| Commit main de lecture | `8fc0a2b4947d9f74023a4d1dc44bd54d680a9847` |
| Approbation | PR graphique **#90**, pas l'ancienne PR #8 de la branche D14 |
| Chemin | `design/ecrans/accueil/accueil_cyd_warship_320x240.png` |
| Blob Git source | `7bd321c6ccbfc230802e20010bfa06e71a9099a9` |
| Taille | **144890 octets** |
| SHA-256 source PNG | `ca73f029e80c95c09b3c7defe1437bba3156280b2f2169465ee3f30014343873` |
| Géométrie | **320×240 RGB8 non entrelacé** |

Le fichier homonyme historique de la branche D14 (blob `21e67bba4120838c5eeef8a687fcb120f874ab36`) **était obsolète**. Cette mission importe par **référence au blob Git original** le PNG PR90 sans aucune retouche ni rééchantillonnage. Les autres ressources `design/` (SVG, ORA, G03/G02, etc.) ne sont pas modifiées. **Aucun vieux SVG accueil n'est utilisé**. Le changement du seul blob PNG sur la branche D14 n'est pas une nouvelle proposition artistique : les octets correspondent exactement à `main`.

Le script `tools/d14_color_g01_export.py` lit **ce PNG précis** avec le parseur Python standard `t03_export.read_png`, exige **taille + SHA-256** attendus, interprète uniquement RGB8, vérifie CRC/scanlines, puis réduit chaque pixel RGB888 en **RGB565BE** :

```text
RGB565 = ((R >> 3) << 11) | ((G >> 2) << 5) | (B >> 3)
DMA[2*i] = RGB565 >> 8
DMA[2*i+1] = RGB565 & 0xFF
```

**Pas** de correction de gamma/contraste/saturation, alpha, filtre, compression perceptuelle ou mise à l'échelle. C'est une **quantification** 8→5/6/5 bits par canal, pas une identité RGB888. Le RGB565 brut est **153600 octets**, SHA-256 `8a376f7ca4cf83ed31b146905b25c80455b71fdef194eee8c40ac00e2d5248dc`. Essai de compression ROW-RLE : **172909 octets avec l'index** (plus gros et plus lent). Pour cette image détaillée, le générateur choisit le **RGB565BE RAW en Flash** ; aucun RLE, PNG ou SVG n'est décodé par l'ESP32. `t03_decode_rect(T03_RAW565BE)` copie directement les lignes en bande DMA bornée.

Le fichier C contenant le tableau `static const uint8_t` est généré **uniquement dans le répertoire de compilation** du binaire `WARSHIP_D14_COLOR_DIAGNOSTIC=ON`, **jamais dans `design/` ou dans le firmware normal**.

## 3. Deux pages de référence, **A uniquement**

Binaire de réception isolé `WARSHIP_D14_COLOR_DIAGNOSTIC=ON`, **OFF par défaut** dans `CMakeLists.txt`. `app_main` du test ne compile pas les modules main `d09_probe.c` ni `d11_runtime.c` ; il ne lance **ni NVS, ni jeu, ni tactile, ni palmarès**. Les cinq profils historiques restent décrits dans le code de sonde, mais seul `warship_lcd_diag_apply_profile(0)` est appelé : **aucune commande BGR/vendor gamma, inversion OFF uniquement**.

| Ordre | Durée | Source affichée | Contrôle demandé |
|---|---|---|---|
| **1. A / DIRECT_RGB565** | ~3,5 secondes | 8 bandes colorées constantes | Le noir, blanc, rouge, vert, bleu, gris et dégradé ont-ils de la profondeur/contraste ? |
| **2. A / G01_PR90_PNG** | Reste visible jusqu'au redémarrage | PNG accueil PR90 entier, converti bit-à-bit en RGB565BE | Le véritable écran d'accueil PO a-t-il des couleurs fidèles ou paraît-il délavé ? |

**Aplats directs :** 4 colonnes de 80 px et 2 lignes de 120 px, donnant (de gauche à droite) :
- Haut : noir `0000/00:00`, blanc `FFFF/FF:FF`, rouge `F800/F8:00`, vert `07E0/07:E0`.
- Bas : bleu `001F/00:1F`, gris moyen `8410/84:10`, rampe noir→blanc, jaune `FFE0/FF:E0`.

Contrat physique constant : ILI9341 `esp_lcd`, **RGB/BIG endian**, case10 `MV=0 MX=0 MY=1`, SPI2 **20 MHz**, GPIO21 LEDC **40% / 5 kHz** (valeur expérimentale inchangée), 320×240, **30 bandes de 320×8×2 = 5120 octets**, tampon DMA existant, transferts synchrones. Aucun framebuffer plein écran RAM, aucune PSRAM, microSD, nouvelle tâche ou radio.

**Logs 115200 :** `D14_COLOR_DIAGNOSTIC=ON TEST_ONLY`, `D14_COLOR_SOURCE source=G01_PR90_PNG` (blob SHA, SHA256), `D14_COLOR_PROFILE_APPLIED case=A`, `WARSHIP_BL PWM_STATE` (`freq_hz_actual=5000`, `duty_configured=409/1023` attendus), `D14_COLOR_DRAW profile=A source=DIRECT_RGB565|G01_PR90_PNG`, `D14_COLOR_MEM` (heap8/largest8/stack_margin_bytes, LCD bands/timeouts/fault), puis `D14_COLOR_DONE`. Ces valeurs attendues sont **des critères de relevé**, pas des mesures déjà faites du nouveau firmware.

## 4. Tests et build (sur HEAD exact à fournir)

`tests/test_d14_color_g01.c` décode **les 30 bandes** du G01 et compare tous les **76800 pixels** RGB565 à une référence brute générée **à partir du PNG PR90 approuvé** ; vérifie un sous-rectangle non aligné, bornes, capacité et refus d'arguments invalides. Le SHA du PNG est imposé par le script. CI : seconde génération identique byte-to-byte (`cmp` du C et du RGB565 golden), `sha256sum -c`, `git rev-parse HEAD:<png>` attendu, et `git diff --exit-code -- design/`. Cette dernière commande vérifie l'absence de retouche par la génération, **pas** l'absence de l'import Git PR90.

```sh
# ESP-IDF v5.5.6 ; Python de build avec CairoSVG/Pillow pour les assets
# D14 G03 toujours présents dans les autres composants (pas pour G01).
idf.py -B build-d14-color -D WARSHIP_D14_COLOR_DIAGNOSTIC=ON set-target esp32
idf.py -B build-d14-color -D WARSHIP_D14_COLOR_DIAGNOSTIC=ON build
sha256sum build-d14-color/cyd_warship.bin
```

Le **firmware NORMAL** est construit et archivé séparément. Son boot reste D09/D10, son image déploiement reste G03 ; **aucun écran accueil D15/JOUER/PALMARÈS fonctionnel n'est développé ici**. Les preuves requises : GCC+Clang ASan/UBSan, ESP-IDF v5.5.6 Xtensa normal + diagnostic, CI 3/3 et SHA BIN distincts.

## 5. Réception PO ultra-courte sur **UNE** CYD

1. Après **CI verte du HEAD final**, télécharger **l'artefact explicitement nommé `warship-d14-G01-PR90-COLOR-TEST-ONLY-<HEAD>`**, vérifier le SHA-256 de son `build-d14-color/cyd_warship.bin`. Flasher bootloader, partition-table, application du **même** artefact avec `flasher_args.json`. **Ne pas lancer `erase-flash`**, pour préserver les scores existants. Le firmware TEST-ONLY ne démarre pas NVS.
2. Ouvrir UART **115200**, redémarrer et regarder **A aplats (~3,5 s) puis accueil G01 statique**. Les pages **B–E ne sont plus proposées** ; ne pas toucher l'écran. Comparer si possible la perception au vrai écran approuvé et aux couleurs connues de Locomotive BASIC.
3. Remettre un verdict en **deux parties distinctes** : `APLATS A : fidèles / ternes (+ détails)` et `ACCUEIL G01 : fidèle / délavé (+ détails)`. Joindre les lignes `D14_COLOR_SOURCE/PROFILE/DRAW/MEM`, `WARSHIP_BL`, et les éventuelles erreurs/timeout. La photo est facultative. **Pas de partie, AUTO, FEU ni campagne lourde.**
4. Restaurer l'artefact **normal** du même HEAD après l'essai, sans `erase-flash` (trois fichiers cohérents). Aucun choix de gamma/ordre de canaux ne sera imposé sans nouveau verdict PO/Pilotage.

**État de la nouvelle sonde G01 : `ON_DEVICE_UNVERIFIED` jusqu'à son flash sur la vraie CYD.** Si les aplats sont ternes, approfondir le profil matériel/pilote et la référence Locomotive BASIC. S'ils sont corrects mais G01 terne, comparer directement la source PNG et sa quantification, sans modifier les œuvres sans arbitrage. **Seul le PO décide de la fidélité et seul le Pilotage autorise toute correction de production et fusion de la PR #91.**
