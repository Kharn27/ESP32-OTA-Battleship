# PORTRAITS-01-TFT — Issue #128 : sonde CYD native TEST-ONLY

**Mandat** : [Issue #128](https://github.com/Kharn27/ESP32-OTA-Battleship/issues/128), Développeur C/ESP32 ; **pas de GO artistique**, **pas de flash physique avant revue et GO distinct Pilotage**. Les règles du jeu, G01, D15 et le firmware NORMAL restent hors mission. Le portrait A-L1-C1 est **CANDIDATE / ON_DEVICE_UNVERIFIED** et n'est pas assigné au CPU Capitaine.

## Source graphique figée

**Source graphique disponible dans main :** PR #127 **FUSIONNÉE par Pilotage le 13/11/2026**, HEAD graphique `dab0a35a1a891ab4d215a55cd16f2fe095a558df`, commit de fusion squash sur `main` **`5c59d6f4ed038981bf5d78a56d6abd55ab5b086d`**. La branche technique `feat/issue-128-portraits01-tft-test` est rebasée **directement sur ce commit de main** ; la PR Draft #129 cible `main` et porte exclusivement 13 fichiers techniques. Aucun fichier `draft/`, `maquettes/`, `assets/`, aucune image PO, aucun manifeste ne sont modifiés. **Fusion des sources candidates ≠ validation artistique PO ou matérielle.**

| Entrée PNG `draft/portraits/pilote_A_L1_C1/` | SHA-256 source exact | Conversion Flash |
|---|---|---:|
| `pilote_32.png` | `2d17cc05d8bfb64a0c01b5f4ad125979dc7c5de64d304ec52fafa992217f86e7` | 32×32×3 = **3 072 o** |
| `pilote_48.png` | `5abbcacd3981e6b426c93542dcb6289712afc4ca49b7f38dc5c35c9345360dee` | 48×48×3 = **6 912 o** |
| `pilote_64.png` | `2241bf1306772bbca3164356529a4fb8290f9ef33ae89aaa9b29d083572967da` | 64×64×3 = **12 288 o** |

**Total pixels Flash : 22 272 octets** de constantes C, hors descripteurs/alignement/code. Les SHA-256 PNG sont vérifiés **au build hôte et cible** avant compilation : tout changement de pixel, de métadonnées ou de compression qui modifie les octets fait **échouer** la génération. Tout changement des exports source après ce commit de fusion exige un **repin explicite** et une nouvelle revue. Le comparatif PC `comparatif_320x240.png` est vérifié sur SHA-256 **`10b30aff6e089c1365dc0a6c0271ab8598b4fa8abb5ec5be085da7867998fc4e`**, sans l'incorporer dans le firmware.

**Qualité graphique et statut :** la PR #127 fusionnée contient `qa_repro_report.json` : les 7 PNG ont été régénérés sans différence en octets ou pixels depuis l'original PO (CI Graphiste #43259993102, vérification non destructive #43260612438). Le diagnostic des dix pixels de frange quasi transparents est documenté. Le firmware n'invente aucune retouche : il rend les pixels candidats (avec quantification RGB565). Les PNG demeurent **CANDIDATE / ON_DEVICE_UNVERIFIED** sans choix de taille du PO.

## Architecture C et comportement à l'écran

- **Sélection explicite** : `-D WARSHIP_PORTRAITS01_TEST_ONLY=ON`, option CMake OFF par défaut, incompatible GUI-01, D15-A, D11 bench, D13, T03. Cette configuration utilise **uniquement** `main/portraits01_probe.c`, `components/portraits01/`, `warship_lcd_d15a_preview.c` (profil D14 ILI9341/TFT_eSPI native 320×240, LEDC GPIO21), et **aucun** `d11_runtime_init`, NVS, touche, HOME, ESP-NOW ou jeu.
- **Génération hôte uniquement** : `tools/portraits01_generate.py` lit trois PNG en `RGBA` 32/48/64, vérifie chaque empreinte/dimension, génère des tableaux `static const uint8_t` avec pas **RGB565BE, alpha8 droit**, et trois métadonnées SHA. Le firmware n'embarque ni PNG/JPEG ni bibliothèque de décodage, ni copies d'exports `draft/` ailleurs dans Git.
- **Rendu** : `portraits01_render.c` recompose **la même disposition** que [`export.py`](../draft/portraits/pilote_A_L1_C1/export.py) à partir de couleurs/rectangles et de l'alphabet 3×5 du script, puis compose les 3 sprites individuellement (pas de source plein écran). Transparent alpha0 = fond strictement inchangé, alpha255 = couleur source quantifiée, alpha 1..254 = interpolation entière dans les canaux RGB565 avec arrondi. Aucun « halo correctif » ni multiplication de l'alpha deux fois.
- **Bande unique** : `portrait01_compose_band(y,rows,dma,5120)` écrase entièrement **320×8×2** octets, et rejette les bornes invalides **avant écriture**. Le seul tampon LCD DMA est emprunté de `warship_lcd_band()`, alloué par le driver et réutilisé après `warship_lcd_send()` **synchrone**. Pas de framebuffer 153 600 o en RAM, ni buffer portrait persistant. Pile locale de quelques variables, 3 descripteurs const, tables de glyphes const.
- **Affichage** : la planche PC native **320×240**, avec **fond sombre et fond clair simultanés**, sans interaction. Le rendu des contours n'est pas déclaré identique bit-à-bit au PNG PC en RGB888 : la quantification RGB565 introduit une erreur de couleur bornée mesurée par test hôte.

### Coordonnées (x,y,largeur,hauteur), écran paysage 320×240

| Zone | Coordonnées |
|---|---|
| Fond sombre | (4,16,312,100), intérieur cellule (9+103i,26,94,87) |
| Fond clair | (4,128,312,100), intérieur cellule (9+103i,138,94,85) |
| Légendes principales | (8,6) `A1 / FOND SOMBRE`, (8,119) `A1 / FOND CLAIR` |
| Portrait 32 | (40,78,32,32) sombre ; (40,188,32,32) clair |
| Portrait 48 | (135,62,48,48) sombre ; (135,172,48,48) clair |
| Portrait 64 | (230,46,64,64) sombre ; (230,156,64,64) clair |
| Légendes des 3 cellules | `32 PX` / `48 PX` / `64 PX`, x = 43 / 146 / 249 ; y = 30 sombre, 142 clair |

Ces zones sont des **positions de dessin**, pas des hitboxes. Pas de navigation, d'action tactile ou de sélection du portrait « gagnant ».

## Reproduction, tests et preuves

### Hôte (GCC/Clang, ASan/UBSan)

```sh
python3 -m pip install 'Pillow==12.3.0'
cmake -S tests -B build-host -G Ninja -DWARSHIP_SANITIZERS=ON
cmake --build build-host
ctest --test-dir build-host --output-on-failure -V
```

Tests : entrée null/tailles hors bande/sentinelles, 30 bandes exactes, positions et couleurs solides, pixel de chaque portrait sur **deux fonds** et alpha 0/partiel/255, comparaison C RGB565 reconstruite contre le **PNG PC figé** avec mesure du maximum/moyenne d'écarts RGB888. `tools/portraits01_compare_pc.py` compare **tous les 76 800 pixels** en référence **indépendante** du code C et échoue si déviation de canal supérieure à 11 (seuil quantification proposé, sans interprétation artistique). Générateur et tests ne créent aucun fichier dans `draft/`. Le validateur graphique commun `python3 scripts/check_graphic_pipeline.py` reste applicable.

### Firmware ESP32 Xtensa (ESP-IDF v5.5.6)

```sh
idf.py -B build set-target esp32
idf.py -B build build
idf.py -B build-portraits01 -D WARSHIP_PORTRAITS01_TEST_ONLY=ON set-target esp32
idf.py -B build-portraits01 -D WARSHIP_PORTRAITS01_TEST_ONLY=ON build
idf.py -B build-portraits01 size
idf.py -B build-portraits01 size-components
sha256sum build-portraits01/cyd_warship.bin
```

La CI vérifie le build NORMAL non modifié, D15-A et GUI-01, la source candidat exacte, compile l'image isolée, publie l'ELF/MAP/BIN/bootloader/partition/`flasher_args.json` et `portraits01_sources.json` sous **`portraits01-TEST-ONLY-<HEAD>`**. Les chiffres `idf.py size` (Flash Code/Data, IRAM/DRAM, taille image) sont des **mesures de compilation** attribuées au HEAD, pas une métrique RAM en fonctionnement.

Au boot, log UART à **115200** : `PORTRAITS01_TFT_TEST_ONLY=ON`, `PORTRAITS01_SOURCE` tailles et SHA, `PANEL_PROFILE`, `PORTRAITS01_MEM` (`free8/min8/largest8` et high-water-mark pile de la tâche), `PORTRAITS01_RENDER` (temps microsecondes de composition seule, transferts, total), `PORTRAITS01_LCD` (écart transferts, timeouts, DMA fault), `PORTRAITS01_READY` ou `PORTRAITS01_FAIL`. **Le temps de composition n'est pas confondu avec le temps SPI**.

## Recette CYD non destructive — **APRÈS GO DISTINCT PILOTAGE UNIQUEMENT**

**Interdit avant GO : toute écriture Flash.** Cette procédure est destinée à une **seule ESP32-2432S028R classique**, explicitement identifiée (variante, port réel, taille Flash, état du bootloader/partitionnement), utilisant la CI **du HEAD technique corrigé** de la PR #129. Elle ne vaut ni autorisation de test matériel ni validation artistique. Ne pas confondre le BIN TEST-ONLY avec le firmware NORMAL.

### A. Identification et vérification SANS écriture

1. Sur PC, extraire l'artefact CI `portraits01-TEST-ONLY-<HEAD>` du **nouveau run** (BIN, ELF, MAP, `flasher_args.json`, `partition_table/partition-table.bin`, etc.). Contrôler **HEAD Git complet, nom et SHA-256 de `cyd_warship.bin`** contre le nouveau run CI et vérifier que la cible est ESP32 classique, ESP-IDF 5.5.6. **Ne pas utiliser l'ancien BIN de `adc56dff...` pour valider la recette du HEAD rebasé.**
2. Identifier le port série et la taille Flash réelle de la CYD sans les présumer. Depuis l'environnement ESP-IDF activé (esptool disponible), remplacer le port d'exemple par le port mesuré :

```sh
PORT=/dev/ttyUSB0 # EXEMPLE : remplacer par le port réellement identifié
python -m esptool --chip esp32 --port "$PORT" flash_id
python -m esptool --chip esp32 --port "$PORT" read_flash 0x8000 0xC00 cyd_device_partitions.bin
```

3. **Décoder puis comparer la table réellement lue à la table livrée par le build :** utiliser `gen_esp32part.py` de **ce même ESP-IDF** et `cmp` (comparaison d'octets). Après extraction de l'artefact, placer les fichiers sous le répertoire `build-portraits01/` ou corriger seulement les chemins de fichiers, **jamais les offsets sans revue**.

```sh
python "$IDF_PATH/components/partition_table/gen_esp32part.py" cyd_device_partitions.bin
python "$IDF_PATH/components/partition_table/gen_esp32part.py" build-portraits01/partition_table/partition-table.bin
cmp cyd_device_partitions.bin build-portraits01/partition_table/partition-table.bin
```

Le partitionnement **du build vérifié par le Pilotage** contient NVS **0x9000, taille 0x6000**, et application **factory 0x10000, taille 0x100000** ; table à **0x8000, taille de secteur 0x1000**. Vérifier ce schéma **sur la CYD physique**, le fichier `flasher_args.json`, la taille et le type des partitions, le mode de démarrage et la place suffisante pour le BIN. **La condition d'autorisation est stricte : tables binaires identiques ET offsets/types/tailles décodés conformes ET carte et mode de flash compatibles.** En cas de table différente, partition OTA, protection/chiffrement Secure Boot ou Flash Encryption inattendus, erreur de lecture, taille Flash incohérente ou doute sur le bootloader : **STOP, demander arbitrage Pilotage. NE PAS réécrire la table pour la rendre compatible.**

4. Après **validation des offsets lus sur la carte**, sauvegarder localement la partition NVS si elle contient un palmarès ou d'autres données à préserver, sans exposer la copie brute sur GitHub (elle pourrait contenir des secrets). Lecture sans modification :

```sh
umask 077
python -m esptool --chip esp32 --port "$PORT" read_flash 0x9000 0x6000 cyd_nvs_backup.bin
```

### B. Flash **application seule**, après GO explicite

5. **Option privilégiée si BIN téléchargé depuis la CI :** contrôler d'abord le SHA-256 attendu **sur le nouveau HEAD**, puis écrire **exclusivement l'application** dans la partition factory **après confirmation effective de l'offset 0x10000**. Aucun autre segment ne doit être écrit.

```sh
sha256sum build-portraits01/cyd_warship.bin
python -m esptool --chip esp32 --port "$PORT" write_flash 0x10000 build-portraits01/cyd_warship.bin
```

6. **Alternative si projet cloné et build strictement identique :** `idf.py -B build-portraits01 -D WARSHIP_PORTRAITS01_TEST_ONLY=ON -p "$PORT" app-flash`. **Attention : `app-flash` peut reconstruire l'application.** Ne l'employer qu'avec checkout exact, sdkconfig/partitionnement concordants et **SHA du BIN vérifié avant écriture** ; contrôler l'empreinte après exécution, et préférer le BIN CI pour éliminer les divergences de rebuild. La vérification de la table physique reste **obligatoire dans les deux cas**.

**INTERDICTIONS : `idf.py flash` (réécrit bootloader et table), `erase-flash`, effacement NVS, `nvs_flash_erase`, restauration/formatage non mandatés, modification de bootloader/offsets/partitions.** Le test n'a **aucun appel NVS** dans son point d'entrée. Aucune sauvegarde ne rend un flash divergent acceptable.

### C. Constat matériel à transmettre au Pilotage

7. Sur une seule CYD, observer simultanément les **32/48/64 px** sur les fonds sombre et clair : photographier/filmer l'écran à **taille réelle**, puis éventuel gros plan de la coiffe/visage/barbe, contrastes et franges vertes/rouges/jaunes. Noter scintillement, erreur de couleurs, plantage ou divergence de la planche PC.
8. Capturer **l'intégralité du log série à 115200**, dont `PORTRAITS01_TFT_TEST_ONLY=ON`, `PORTRAITS01_SOURCE`, `PORTRAITS01_RENDER`, `PORTRAITS01_LCD`, `PORTRAITS01_MEM`, `PORTRAITS01_READY` ou `PORTRAITS01_FAIL`. Relever `free8/min8/largest8/stack_hwm_bytes/compose_us/lcd_send_us/total_us/transfers/timeouts`. Transmettre le **SHA Git complet et SHA-256 du BIN**, modèle de carte, table de partitions constatée (sans secrets), procédure réelle, photos, résultats et anomalies.

Statuts à ce stade : `HOST_TESTED`, `TARGET_BUILD_VERIFIED`, `CI_PASSED` à refaire/rapporter **sur le HEAD rebasé** ; `ON_DEVICE_PO_REPORTED_PASS` et `ON_DEVICE_VERIFIED` : **NON TESTÉ**. Même un test matériel réussi **n'accorde pas de GO artistique PO** ni ne choisit la taille à sa place.

**Exclusions permanentes :** aucun sprite dans `assets/`, aucune image aplatie embarquée, aucune retouche des PNG PO, aucun personnage assigné, pas de série, pas de HOME/D15, IA, NVS ou ESP-NOW. PR Draft uniquement ; fusion réservée au Pilotage.
