# T02 — sonde matérielle ESP32-2432S028R (LOT 1)

> **CORRECTIONS_REQUISES (Pilotage, PR #22)** — [consigne #6536448760](https://github.com/Kharn27/ESP32-OTA-Battleship/pull/22#issuecomment-6536448760). Le PO rapporte un **VISUAL_FAIL** (écran incomplet et pixels d'ancien firmware). **Pas de log de boot/HEAD associé à cette observation** : provenance PO, pas preuve sur un SHA exact. L'ancien log `App version d76a9b8` était le firmware **T01**, et ne constitue pas un essai T02. Toutes les anciennes compilations prouvent seulement la compilation, **pas** la fonctionnalité LCD. Essais supplémentaires PO **suspendus** jusqu'au correctif/revue Pilotage.

**Mandat** : [Issue #21](https://github.com/Kharn27/ESP32-OTA-Battleship/issues/21), passation Pilotage [#6532928571](https://github.com/Kharn27/ESP32-OTA-Battleship/issues/21#issuecomment-6532928571), Bible v0.17 §2, §14.1, §14.9, TEST_POLICY. **Base** `main` T01 : `d76a9b8305dd305cc6f5c3fbd62571b975e86189`. Branche : `spike/issue-21-cyd-tft-touch-probe`.

## Contrat et provenance

Le PO confirme sa CYD identique à celle d'ESP32-Locomotive-Basic, USB-C + micro-USB, ILI9341 **et non ST7789**. Références **en lecture seule** :
- [Validation J0.25-HW1](https://github.com/Kharn27/ESP32-Locomotive-Basic/blob/main/docs/J0.25-HW1-cyd-physical-validation.md) : référence ESP32-D0WD-V3 rev 3.1, Flash 4 Mio, pas de PSRAM connue pour cet exemplaire. Ce sont des mesures **d'un autre firmware**.
- [HW4-B](https://github.com/Kharn27/ESP32-Locomotive-Basic/blob/main/docs/HW4-B-ili9341-driver.md) : LCD SPI2 MOSI13/SCLK14/CS15/DC2/BL21 haut actif, 20 MHz, DMA 5120 o ; profils natifs larges **8..11**, dont le **cas 10 effectivement utilisé au boot de la console actuelle**. Ces indices désignent un contrat logiciel spécifique : **MV=0 et fenêtre 320×240**, pas une rotation générique ; aucun RESET LCD GPIO piloté.
- [HW4-TOUCH-T1](https://github.com/Kharn27/ESP32-Locomotive-Basic/blob/main/docs/HW4-TOUCH-T1.md) : `/PENIRQ GPIO36`, actif bas, **aucune lecture X/Y confirmée par cette preuve**.
- [S01](S01_CYD_CARACTERISATION.md) : SPI XPT2046 **candidat documentaire** MOSI32/MISO39/SCLK25/CS33, bus différent du LCD, **non vérifié physiquement**.
- [Pilote C et manifeste d'origine](https://github.com/Kharn27/ESP32-Locomotive-Basic/tree/main/firmware/components/cyd_ili9341) étudiés **uniquement comme références techniques**. Ce dépôt possède une licence racine **GPLv3**, malgré la mention « MIT » dans certains commentaires de fichiers : **aucun code ni header n'a été copié** dans WARSHIP. Sonde écrite indépendamment via les API ESP-IDF publiques.
- Dépendance officielle : `espressif/esp_lcd_ili9341==2.1.0` via Component Manager, `idf==5.5.6` (candidature étayée T01), sans LVGL. Aucun autre pilote communautaire importé.

### Profil embarqué T02 — expérimental

| Élément | Valeur | Preuve/limite |
| --- | --- | --- |
| LCD | **Profil console 10**, fenêtre **320×240 sous MADCTL.MV=0, MX=0, MY=1**, précédée d'un effacement large **320×240 sous MV=0** et d'un redessin complet | Référence opérationnelle sur la même CYD dans **Locomotive BASIC** ; copie de comportement, **pas de code source**, encore NON TESTÉ dans WARSHIP |
| LCD SPI2 | MOSI 13, SCLK 14, CS 15, DC 2 ; rétroéclairage 21 HIGH | Rapport matériel PO historique ; aucun RESET GPIO (attention GPIO2/15 strapping) |
| SPI écran | 20 MHz, MSB-first RGB565, 8 rangées ×320×2 = **5120 octets** DMA | Choix technique pour la sonde, temps et coûts à **mesurer** |
| Tactile /PENIRQ | GPIO36 entrée uniquement, sans pull interne, actif bas | Historique PO validé pour détection d'appui, pas pour X/Y |
| Tactile SPI3 | MOSI 32, MISO 39 (entrée), SCLK 25, CS 33, mode 0, 1 MHz | **HYPOTHÈSE À VALIDER** par valeurs ADC, pression, relâchement, reproductibilité |
| XPT2046 ADC | commandes X=0xD0 et Y=0x90, 12-bit ; médiane de 3 échantillons ; lecture seulement avec /PENIRQ bas | Implémentation protocole, pas validation de la réponse du contrôleur réel |
| Coordonnées affichées dans logs | conversion linéaire provisoire **0..4095 → 0..319/0..239**, axes non inversés | **NON CALIBRÉ**. Ne pas confondre avec placement tactile fiable. Capturer les bruts 5 points et recalculer bornes/axes selon observations |

Les fonctions de mapping C99 `components/t02_touch_map/` prennent des bornes explicites, autorisent des axes inversés, bornent les valeurs, refusent paramètres invalides sans modifier les sorties. **Aucun mapping produit/UX n'est retenu**.

### Motifs écran et instrumentation

- Au démarrage : log puce, taille Flash exposée par ESP-IDF, mémoire avant/après allocations ; initialisation rétroéclairage éteint.
- **Séquence du composant WARSHIP dédié** : `swap_xy(false)`, `mirror(false,false)`, nettoyage **320×240 sous MV=0** en 30 bandes DMA 320×8, puis `swap_xy(false)`, `mirror(false,true)` et **redessin complet 320×240** avec le profil figé. Pas d'exploration d'orientations ; remplissage écran RGB en trois bandes verticales (rouge, vert, bleu). Repères **blanc en haut gauche, noir en haut droite, jaune en bas gauche, magenta en bas droite, croix cyan centrale**, bord blanc. Confirmer placement, orientation et absence de bande fantôme/scintillement.
- Toutes les 2 s : rafraîchir **seulement 48×32 px** sous le centre, alternance noir/blanc. Logs de temps microsecondes et nombre de transferts. Pas de framebuffer 320×240 (153600 o), une bande DMA de 5120 o. Sur échec/timeout DMA : extinction rétroéclairage, arrêt de dessin et **mémoire tampon conservée** (callback asynchrone possible).
- Traces `MEM phase=...` au boot, après LCD, image, tactile, puis toutes les 10 s : heap8 libre / plus grand bloc / minimum, DMA libre / plus grand bloc, marge de pile `app_main` en **octets pour ESP-IDF**, bande DMA ; `COUNTERS` transactions/erreurs. Temps de rendu en µs ; toute valeur chiffrée doit être **recopiée depuis le journal physique de la CYD**.

## Correction LCD — reproduction du chemin éprouvé, revue Pilotage requise

### Comparaison des commandes SPI et du contrat (référence en lecture seule)

La [console actuelle de Locomotive BASIC](https://github.com/Kharn27/ESP32-Locomotive-Basic/blob/main/firmware/esp32_display_probe/main/hw4_c2_console.c) suit `cyd_lcd_init()` → `cyd_lcd_orientation(10U,...)` → `repaint_full()` (30 bandes 320×8) → rétroéclairage. Le [composant de référence](https://github.com/Kharn27/ESP32-Locomotive-Basic/blob/main/firmware/components/cyd_ili9341/cyd_ili9341.c) gère un **adressage native-wide non standard** avec `MADCTL.MV=0`. Le futur WARSHIP reprend les **commandes et leur ordre**, pas les noms de fonction ni un numéro de rotation générique.

| Étape | Locomotive BASIC — comportement vérifié sur cette CYD | WARSHIP — implémentation indépendante `components/warship_lcd` |
| --- | --- | --- |
| Préparation | SPI2 GPIO13/14/15/2, BL21 OFF, 20 MHz, ILI9341 2.1.0, DMA 5120 o, aucun RESET GPIO | Même configuration ESP-IDF publique, allocation unique d'une bande DMA 5120 o |
| État pour nettoyage | `swap_xy(false)` (MV=0), `mirror(false,false)` (MX=0 MY=0) | Mêmes commandes dans cet ordre |
| Nettoyage intégral spécifique carte | **CASET 0..319 / RASET 0..239 / RAMWR**, 30 fenêtres **320×8 à MV=0**, sans permutation matérielle | Même fenêtre 320×240/MV0 **malgré la géométrie nominale 240 colonnes** ; retour DMA attendu pour chaque bande |
| Cas 10 console | `swap_xy(false)`, puis `mirror(false,true)` → **MV=0, MX=0, MY=1** | `warship_lcd_prepare_console()` applique cette séquence exacte |
| Image visible | 30 transferts 320×8, framebuffer non requis ; rétroéclairage ON après réussite | `draw_rect(0,0,320,240)` sur bande propriétaire, puis `warship_lcd_backlight(true)` |
| Garde/retransmission | Même profil : ne pas réécrire MADCTL ; attente callback DMA, erreur/timeout `sticky`, pas de retry | Même état figé `prepared`, contrôle bornes et DMA synchronisé, conservation du buffer si faute |

**Sources/licence :** `Kharn27/ESP32-Locomotive-Basic` dispose d'une **licence GPLv3 à la racine** ; le commentaire « MIT » de quelques fichiers ne démontre pas à lui seul une autorisation de reproduction textuelle sans autre contrôle des droits. **Aucun code ni header n'a été importé.** Le module `warship_lcd` est créé indépendamment depuis notre ancienne sonde et les API ESP-IDF publiées ; seuls le brochage et les *comportements matériels* de la console sont repris et explicitement attribués. Le dépôt BASIC reste inchangé.

La régression passée `swap_xy(true)` configurait un comportement générique correct au regard de MADCTL mais **non identique au chemin que le PO avait déjà validé**. Ne pas relancer des séries d'axes/mirrors sur le PO. Le nouveau profil 10 est fixe. Les anciens SHAs/empreintes (notamment `e0abae5`) sont **historiques et ne doivent pas être flashés** pour ce contrôle.

### Prochaine vérification PO — **une seule mire LCD**

Seulement **après** nouvelle CI et relecture du nouveau HEAD par Pilotage : flasher sur **une CYD connue** l'artefact complet (bootloader, partition, app et offsets cohérents), vérifier l'identifiant SHA de l'application et les logs `BOOT probe=T02`, `PROFILE_APPLIED case=10`, `LCD_PROFILE_PASS`, `DRAW region=320x240`. Observer le **plein écran sans ancien pixel résiduel**, quatre coins et centre, cadrage paysage. Retour PO : PASS/FAIL, log UART du boot, anomalie précise ; **aucune campagne supplémentaire de rotations, aucun test tactile 15 points** avant succès LCD. Les coordonnées X/Y et mesures runtime restent à caractériser à la seconde étape autorisée par Pilotage.

**Statut matériel** : `PO_REPORTED_VISUAL_FAIL` sur essai précédent (HEAD non attesté) ; **`ON_DEVICE_UNVERIFIED` sur le correctif**, tant que nouveau flash/SHA et constat ne sont pas recueillis.

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

### Historique — ancienne correction générique Codex, remplacée après le retour visuel PO

[Observation Codex P2](https://github.com/Kharn27/ESP32-OTA-Battleship/pull/22#discussion_r4428368983) : l'ancienne version lançait l'affichage 320×240 **sans activation de MADCTL.MV** (`esp_lcd_panel_swap_xy(panel, false)`). L'API `esp_lcd_ili9341` émet les coordonnées au contrôleur **sans transposition logicielle** ; les colonnes 240..319 dépassaient alors la largeur native 240. Erreur confirmée en consultant l'implémentation du [pilote officiel Espressif](https://github.com/espressif/esp-bsp/blob/master/components/lcd/esp_lcd_ili9341/esp_lcd_ili9341.c) : `swap_xy(true)` active `LCD_CMD_MV_BIT`, tandis que `mirror()` ne modifie que MX/MY.

**Ancienne correction (désormais remplacée) :** après l'effacement GRAM natif 240×320, `esp_lcd_panel_swap_xy(panel, true)` **avant** `esp_lcd_panel_mirror(panel, true, false)` et avant tout dessin 320×240. Le log `LCD_ORIENTATION` donne les trois paramètres sans réutiliser « cas 9 ». Le sens/position physique des coins et l'absence de bandes résiduelles restent à vérifier par le PO ; un build et une CI verte **ne constituent pas** une validation de la dalle.

Cette ancienne correction était compilable mais ne reproduisait pas le chemin LCD historique. **Remplacée** par `warship_lcd` / profil console 10 ; nouveau build/empreinte à acquérir.

## Fiche détaillée T02 DIFFÉRÉE — seulement après accord Pilotage et réussite du contrôle LCD ciblé

Cette fiche longue **n'est pas le prochain test demandé au PO**. Les quinze appuis et mesures X/Y restent suspendus jusqu'au succès de la vérification LCD ciblée ci-dessus et à une nouvelle autorisation du Pilotage.

1. **Carte unique :** CYD ESP32-2432S028R PO, double USB-C + micro-USB, ILI9341 ; ne pas demander d'autres photos pour commencer. Utiliser câble/port USB série habituel (ex. `/dev/ttyUSB0`, à confirmer sur poste).
2. **Vérification binaire :** télécharger l'artefact exact de la CI, relever SHA-256 et `git rev-parse HEAD` du commit transmis. `python -m esptool --chip esp32 -p PORT flash-id` (syntaxe dépend version), enregistrer taille Flash détectée. Ne **pas** supposer que le `--flash_size` généré indique la taille physique. Vérifier offsets **dans le `flasher_args.json` du même build** ; flasher avec `idf.py -p PORT flash` après build exact ou commandes générées `build/flash_args`/JSON appropriées. Ne pas inventer d'offsets.
3. **UART 115200 :** conserver log complet boot → 10+ secondes `MEM`, `LCD_INIT`, `LCD_ORIENTATION`, `DRAW`, `COUNTERS`; relever Flash / build ID, DMA et toute erreur SPI. Redémarrer une fois sans appui, vérifier absence de boucle/plantage.
4. **LCD :** relever trois bandes RGB, les couleurs et emplacement **des quatre coins** et du centre, présence/absence de bande résiduelle, sens paysage, absence de scintillement lors de l'alternance dans le rectangle 48×32. Capturer résultat visuel (description écrite suffisante, photo/vidéo facultatives selon anomalie/protocole Pilotage).
5. **Tactile :** au stylet, **3 appuis puis relâchements** par point : coin HG (à quelques mm du bord), HD, BG, BD, centre — soit au moins **15 cycles**. Marquer l'ordre dans les notes du journal ; récupérer pour chaque appui `TOUCH_PRESS`, `TOUCH_RAW` (3 triplets/ADC, X/Y, `logical_provisional`), `TOUCH_RELEASE`. Vérifier bruit/rebond, pixels de bord, axes permutés/inversés et répétabilité ; si /PENIRQ détecte mais coordonnées restent 0/4095 ou figées, **SPI tactile NON VALIDÉ**.
6. **Mesures :** recopier `MEM` au démarrage, après LCD, écran visible, tactile et après essais ; `DRAW time_us` full 320×240 et rectangle partiel ; `COUNTERS`, marge de pile, nombres de transferts et timeouts. Répéter les 15 appuis, attendre ≥10 s pour journal post-sollicitation, redémarrer et réévaluer.
7. **Verdict :** rapporter `PASS`/`FAIL` par scénario, anomalies, commandes, HEAD, empreinte du binaire, logs UART. Aucun `ON_DEVICE_VERIFIED` ne découle du build CI : statut `ON_DEVICE_UNVERIFIED` jusqu'au retour PO et contrôle du protocole par Pilotage.

## Frontières

**Exclus :** moteur, Capitaine, écran de jeu, UX G01, NVS métier, PSRAM/microSD, radio/ESP-NOW, deuxième carte, fonts/assets finaux. Ne pas utiliser les valeurs `heap` statiques comme budget de production. La PR ne pourra être déclarée **T02 terminée** sans essai matériel exigé par l'Issue.

## Preuves du correctif « console 10 » — premier commit source

La première version corrective du composant isolé est au commit **`44663d87a9a081980f6b64301bb7b2bed4bd0fbc`** sur `spike/issue-21-cyd-tft-touch-probe`. Ce commit a produit une **CI entière verte** : [GitHub Actions #41150448014](https://github.com/Kharn27/ESP32-OTA-Battleship/actions/runs/41150448014), **3/3 jobs PASS** sur le SHA exact, avec les commandes `idf.py set-target esp32`, `idf.py build`, `idf.py size`, `idf.py size-components` et `sha256sum` exécutées sous ESP-IDF **5.5.6**, GCC Xtensa **14.2.0**. Tests hôte : GCC 13.3.0 / Clang 18.1.3, C99 strict + ASan/UBSan, tests T01 13/13 et mapping T02 24/24, CTest 2/2 pour chaque compilateur.

| Zone statique du firmware WARSHIP T02 corrigé | Mesuré (octets) |
| --- | ---: |
| Flash Code | **85 802** |
| Flash Data | **50 156** |
| IRAM | **63 635** (48,55 % de 131 072) |
| DRAM | **13 720** (7,59 % de 180 736) |
| RTC SLOW | **56** |
| Image `idf.py size` | **211 001** |
| Fichier `cyd_warship.bin` | **210 096** (`0x338b0`) |
| Partition app du build | **1 048 576** (configuration temporaire par défaut IDF, pas décision de production) |

**SHA-256 du binaire pour le commit source ci-dessus uniquement :**

```text
9e428715a370fad6f4807426a7a546122b777f0ac3ad0c1609305a735a1459e1  build/cyd_warship.bin
```

Artefact : `warship-esp32-firmware-44663d87a9a081980f6b64301bb7b2bed4bd0fbc` dans la [CI #41150448014](https://github.com/Kharn27/ESP32-OTA-Battleship/actions/runs/41150448014), avec application, ELF, bootloader, partitions, `.map`, `sdkconfig`, `dependencies.lock` et `flasher_args.json`. **Ne pas confondre empreintes des builds d'autres commits** ; la présente modification documentaire crée un nouveau HEAD qui doit être soumis à sa propre CI. Aucune autre modification firmware n'est apportée par ce paragraphe. Avertissements Node.js dans `upload-artifact` (dépréciation `punycode`/`url.parse`) sans avertissement C applicatif établi.

**Statuts :** `HOST_TESTED`, `TARGET_BUILD_VERIFIED`, `CI_PASSED` **sur le commit source `44663d87a9a081980f6b64301bb7b2bed4bd0fbc`** ; `ON_DEVICE_UNVERIFIED` pour WARSHIP et `PO_REPORTED_VISUAL_FAIL` historique non lié à un SHA exact. Nouvelle observation physique suspendue jusqu'à revue Pilotage. Heap, blocs DMA et piles réellement libres à l'exécution ainsi que vitesse/cadrage LCD **NON TESTÉS**.

