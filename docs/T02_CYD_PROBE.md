# T02 — sonde matérielle ESP32-2432S028R (LOT 1)

**Mandat** : [Issue #21](https://github.com/Kharn27/ESP32-OTA-Battleship/issues/21), passation Pilotage [#6532928571](https://github.com/Kharn27/ESP32-OTA-Battleship/issues/21#issuecomment-6532928571), Bible v0.17 §2, §14.1, §14.9, TEST_POLICY. **Base** `main` T01 : `d76a9b8305dd305cc6f5c3fbd62571b975e86189`. Branche : `spike/issue-21-cyd-tft-touch-probe`.

## Contrat et provenance

Le PO confirme sa CYD identique à celle d'ESP32-Locomotive-Basic, USB-C + micro-USB, ILI9341 **et non ST7789**. Références **en lecture seule** :
- [Validation J0.25-HW1](https://github.com/Kharn27/ESP32-Locomotive-Basic/blob/main/docs/J0.25-HW1-cyd-physical-validation.md) : référence ESP32-D0WD-V3 rev 3.1, Flash 4 Mio, pas de PSRAM connue pour cet exemplaire. Ce sont des mesures **d'un autre firmware**.
- [HW4-B](https://github.com/Kharn27/ESP32-Locomotive-Basic/blob/main/docs/HW4-B-ili9341-driver.md) : LCD SPI2 MOSI13/SCLK14/CS15/DC2/BL21 haut actif, 20 MHz, DMA 5120 o, paysage *cas 9* déclaré validé physiquement ; aucun RESET LCD GPIO piloté.
- [HW4-TOUCH-T1](https://github.com/Kharn27/ESP32-Locomotive-Basic/blob/main/docs/HW4-TOUCH-T1.md) : `/PENIRQ GPIO36`, actif bas, **aucune lecture X/Y confirmée par cette preuve**.
- [S01](S01_CYD_CARACTERISATION.md) : SPI XPT2046 **candidat documentaire** MOSI32/MISO39/SCLK25/CS33, bus différent du LCD, **non vérifié physiquement**.
- [Pilote C et manifeste d'origine](https://github.com/Kharn27/ESP32-Locomotive-Basic/tree/main/firmware/components/cyd_ili9341) étudiés **uniquement comme références techniques**. Ce dépôt possède une licence racine **GPLv3**, malgré la mention « MIT » dans certains commentaires de fichiers : **aucun code ni header n'a été copié** dans WARSHIP. Sonde écrite indépendamment via les API ESP-IDF publiques.
- Dépendance officielle : `espressif/esp_lcd_ili9341==2.1.0` via Component Manager, `idf==5.5.6` (candidature étayée T01), sans LVGL. Aucun autre pilote communautaire importé.

### Profil embarqué T02 — expérimental

| Élément | Valeur | Preuve/limite |
| --- | --- | --- |
| LCD | ILI9341, 320×240 paysage, hypothèse MADCTL *cas 9* (`swap_xy=false`, `mirror_x=true`, `mirror_y=false`) | Validé sur **autre firmware** ; à vérifier sur WARSHIP |
| LCD SPI2 | MOSI 13, SCLK 14, CS 15, DC 2 ; rétroéclairage 21 HIGH | Rapport matériel PO historique ; aucun RESET GPIO (attention GPIO2/15 strapping) |
| SPI écran | 20 MHz, MSB-first RGB565, 8 rangées ×320×2 = **5120 octets** DMA | Choix technique pour la sonde, temps et coûts à **mesurer** |
| Tactile /PENIRQ | GPIO36 entrée uniquement, sans pull interne, actif bas | Historique PO validé pour détection d'appui, pas pour X/Y |
| Tactile SPI3 | MOSI 32, MISO 39 (entrée), SCLK 25, CS 33, mode 0, 1 MHz | **HYPOTHÈSE À VALIDER** par valeurs ADC, pression, relâchement, reproductibilité |
| XPT2046 ADC | commandes X=0xD0 et Y=0x90, 12-bit ; médiane de 3 échantillons ; lecture seulement avec /PENIRQ bas | Implémentation protocole, pas validation de la réponse du contrôleur réel |
| Coordonnées affichées dans logs | conversion linéaire provisoire **0..4095 → 0..319/0..239**, axes non inversés | **NON CALIBRÉ**. Ne pas confondre avec placement tactile fiable. Capturer les bruts 5 points et recalculer bornes/axes selon observations |

Les fonctions de mapping C99 `components/t02_touch_map/` prennent des bornes explicites, autorisent des axes inversés, bornent les valeurs, refusent paramètres invalides sans modifier les sorties. **Aucun mapping produit/UX n'est retenu**.

### Motifs écran et instrumentation

- Au démarrage : log puce, taille Flash exposée par ESP-IDF, mémoire avant/après allocations ; initialisation rétroéclairage éteint.
- Nettoyage GRAM natif 240×320 avant cas 9, puis remplissage écran RGB en trois bandes verticales (rouge, vert, bleu). Repères **blanc en haut gauche, noir en haut droite, jaune en bas gauche, magenta en bas droite, croix cyan centrale**, bord blanc. Confirmer placement, orientation et absence de bande fantôme/scintillement.
- Toutes les 2 s : rafraîchir **seulement 48×32 px** sous le centre, alternance noir/blanc. Logs de temps microsecondes et nombre de transferts. Pas de framebuffer 320×240 (153600 o), une bande DMA de 5120 o. Sur échec/timeout DMA : extinction rétroéclairage, arrêt de dessin et **mémoire tampon conservée** (callback asynchrone possible).
- Traces `MEM phase=...` au boot, après LCD, image, tactile, puis toutes les 10 s : heap8 libre / plus grand bloc / minimum, DMA libre / plus grand bloc, marge de pile `app_main` en **octets pour ESP-IDF**, bande DMA ; `COUNTERS` transactions/erreurs. Temps de rendu en µs ; toute valeur chiffrée doit être **recopiée depuis le journal physique de la CYD**.

## Compilation et tests — preuves à associer au HEAD

Hôte GCC et Clang, C99 strict ASan/UBSan, tests T01 préservés et nouveau test `t02_touch_map_boundaries` :

```sh
for cc in gcc clang; do
  cmake -S tests -B "build-host-$cc" -G Ninja -DCMAKE_C_COMPILER="$cc" -DWARSHIP_SANITIZERS=ON
  cmake --build "build-host-$cc"
  ctest --test-dir "build-host-$cc" --output-on-failure -V
done
```

Cible `esp32`, ESP-IDF 5.5.6 après activation, dans **le dépôt WARSHIP** :

```sh
git rev-parse HEAD
idf.py --version
idf.py set-target esp32
idf.py build
idf.py size
idf.py size-components
sha256sum build/cyd_warship.bin
```

CI de référence : [workflow existant](../.github/workflows/t01-build.yml) exécuté sur HEAD exact, artefact `warship-esp32-firmware-<SHA>` avec binaire, ELF, `.map`, `sdkconfig`, `dependencies.lock`, partitions, `flasher_args.json`. Le `sdkconfig` de la sonde n'impose **pas** une partition applicative de production. Les mesures historiques de BASIC et T01 ne doivent jamais être substituées aux tailles **du build WARSHIP T02**.

## Premières preuves réellement acquises — commit T02 `57b860a22bb647f480fdeb6c7c0f73a941e85f22`

[GitHub Actions #41125556488](https://github.com/Kharn27/ESP32-OTA-Battleship/actions/runs/41125556488) : **3/3 jobs success**. Checkout contrôlé sur le HEAD source exact (sans commit de fusion temporaire). Sur Ubuntu 24.04 :
- **HOST_TESTED** : GCC 13.3.0 et Clang 18.1.3, C99 strict + ASan/UBSan, tests T01 **13/13** et T02 mapping tactile **24/24** pour **chaque compilateur**, CTest **2/2**. Le mapping couvre bornes, axes inversés, saturation, maxima et refus transactionnel des paramètres invalides.
- **TARGET_BUILD_VERIFIED** : image `espressif/idf:v5.5.6`, `xtensa-esp-elf-gcc` 14.2.0, `idf.py set-target esp32`, `idf.py build`, `idf.py size`, `idf.py size-components`, `sha256sum` exécutés. Pilote Espressif `esp_lcd_ili9341==2.1.0` résolu et lié. Aucune erreur de compilation applicative relevée ; avertissements `DEP0040`/`DEP0169` Node.js en upload d'artefact, sans rapport avec C.
- **CI_PASSED** : oui **sur ce commit précis**. La CI du commit documentaire ultérieur doit être réacquise avant de prononcer `CI_PASSED` sur le HEAD final.
- **ON_DEVICE_UNVERIFIED** : **aucun essai WARSHIP physique effectué** ; mesures dynamiques et calibration des coordonnées **NON TESTÉES**, matériel PO à solliciter par Pilotage après revue.

### Mesures statiques de WARSHIP T02 (pas du firmware BASIC)

| Sortie `idf.py size` | Octets | Contexte |
| --- | ---: | --- |
| Flash Code | **84 674** | code Flash |
| Flash Data | **49 868** | constantes Flash |
| IRAM | **63 635** | 48,55 % des 131 072 o rapportés |
| DRAM | **13 720** | 7,59 % des 180 736 o rapportés |
| RTC SLOW | **56** | statique |
| Total image `idf.py size` | **209 585** | taille calculée |
| `cyd_warship.bin` | **209 696** (`0x33320`) | application binaire |
| Partition app **temporaire** du build | **1 048 576** (`0x100000`) | valeurs IDF par défaut, **pas** choix de production |

**Empreinte binaire du commit `57b860a2…` uniquement :**

```text
d7b85afe589d50e0b431c45ebbf9fd9b8d77de292921fdaac9a0d726b2ebbffa  build/cyd_warship.bin
```

Artefact : `warship-esp32-firmware-57b860a22bb647f480fdeb6c7c0f73a941e85f22` dans [#41125556488](https://github.com/Kharn27/ESP32-OTA-Battleship/actions/runs/41125556488). Comprend BIN/ELF/MAP, bootloader, table de partitions, `flasher_args.json`, `sdkconfig`, `dependencies.lock`.

**Interprétation :** ce build ne mesure **ni** heap libre après initialisation, **ni** DMA réellement disponible, **ni** pile libre, **ni** latence du LCD, **ni** résultat XPT2046 sur la CYD. La mémoire dynamique et les temps sont **instrumentés dans le firmware**, mais seul le log UART matériel T02 fournira leurs valeurs. Ni la taille Flash physique ni la PSRAM réelle ne sont déduites de `idf.py size`.

## Fiche de test pour Pilotage → PO (après revue et firmware SHA exact)

Cette fiche ne constitue **pas** une demande immédiate au PO. Pilotage transmettra le HEAD, le lien de la CI et le SHA-256 final après revue.

1. **Carte unique :** CYD ESP32-2432S028R PO, double USB-C + micro-USB, ILI9341 ; ne pas demander d'autres photos pour commencer. Utiliser câble/port USB série habituel (ex. `/dev/ttyUSB0`, à confirmer sur poste).
2. **Vérification binaire :** télécharger l'artefact exact de la CI, relever SHA-256 et `git rev-parse HEAD` du commit transmis. `python -m esptool --chip esp32 -p PORT flash-id` (syntaxe dépend version), enregistrer taille Flash détectée. Ne **pas** supposer que le `--flash_size` généré indique la taille physique. Vérifier offsets **dans le `flasher_args.json` du même build** ; flasher avec `idf.py -p PORT flash` après build exact ou commandes générées `build/flash_args`/JSON appropriées. Ne pas inventer d'offsets.
3. **UART 115200 :** conserver log complet boot → 10+ secondes `MEM`, `LCD_INIT`, `LCD_ORIENTATION`, `DRAW`, `COUNTERS`; relever Flash / build ID, DMA et toute erreur SPI. Redémarrer une fois sans appui, vérifier absence de boucle/plantage.
4. **LCD :** relever trois bandes RGB, les couleurs et emplacement **des quatre coins** et du centre, présence/absence de bande résiduelle, sens paysage, absence de scintillement lors de l'alternance dans le rectangle 48×32. Capturer résultat visuel (description écrite suffisante, photo/vidéo facultatives selon anomalie/protocole Pilotage).
5. **Tactile :** au stylet, **3 appuis puis relâchements** par point : coin HG (à quelques mm du bord), HD, BG, BD, centre — soit au moins **15 cycles**. Marquer l'ordre dans les notes du journal ; récupérer pour chaque appui `TOUCH_PRESS`, `TOUCH_RAW` (3 triplets/ADC, X/Y, `logical_provisional`), `TOUCH_RELEASE`. Vérifier bruit/rebond, pixels de bord, axes permutés/inversés et répétabilité ; si /PENIRQ détecte mais coordonnées restent 0/4095 ou figées, **SPI tactile NON VALIDÉ**.
6. **Mesures :** recopier `MEM` au démarrage, après LCD, écran visible, tactile et après essais ; `DRAW time_us` full 320×240 et rectangle partiel ; `COUNTERS`, marge de pile, nombres de transferts et timeouts. Répéter les 15 appuis, attendre ≥10 s pour journal post-sollicitation, redémarrer et réévaluer.
7. **Verdict :** rapporter `PASS`/`FAIL` par scénario, anomalies, commandes, HEAD, empreinte du binaire, logs UART. Aucun `ON_DEVICE_VERIFIED` ne découle du build CI : statut `ON_DEVICE_UNVERIFIED` jusqu'au retour PO et contrôle du protocole par Pilotage.

## Frontières

**Exclus :** moteur, Capitaine, écran de jeu, UX G01, NVS métier, PSRAM/microSD, radio/ESP-NOW, deuxième carte, fonts/assets finaux. Ne pas utiliser les valeurs `heap` statiques comme budget de production. La PR ne pourra être déclarée **T02 terminée** sans essai matériel exigé par l'Issue.
