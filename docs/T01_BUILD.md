# T01 — Socle C natif et preuves reproductibles

**Contrat :** [Issue #18](https://github.com/Kharn27/ESP32-OTA-Battleship/issues/18). Source : Bible v0.17 §2/14.1/14.9, S01, politique de tests. Pas de pilotes, gameplay, NVS, PSRAM supposée, radio ni test physique.

## Contrat du code

- `components/warship_core`: C99 portable, sans dépendance ESP-IDF, fonction `warship_core_banner(out, capacity)` ; copie constante NUL incluse, taille 16 octets, aucune écriture si pointeur nul ou capacité insuffisante. Aucune allocation ni état mutable. L'API ne définit **aucune règle du jeu**.
- `main/app_main.c`: appelle le module depuis un `app_main` C, affiche une trace console. **Ne configure aucun GPIO, écran, tactile ou radio.**
- `tests`: même source C99 compilée indépendamment par CMake et testée par CTest, notamment NULL, tampon court, zéro, tampon exact et sentinelles.

## Environnement retenu pour T01 et preuves de compatibilité

**CONFIRMÉ pour le socle T01 : ESP-IDF `v5.5.6`.** Le Pilotage a retenu cette release corrective officielle (16 octobre 2026) après vérification d'un vrai build Xtensa `esp32`, [CI #41100144488](https://github.com/Kharn27/ESP32-OTA-Battleship/actions/runs/41100144488) sur `dfccfdaf1fc39dcaf0e2e780a8297f5dea5e4db6`. Image versionnée `espressif/idf:v5.5.6`, compilateur cible `xtensa-esp-elf-gcc` 14.2.0 (crosstool-NG `esp-14.2.0_20260121`), Python 3.12 et CMake dans le conteneur officiel. Cela **ne valide pas** les futurs pilotes TFT/tactile T02. Aucun `sdkconfig` matériel ni partition Flash de production ne sont figés : le `sdkconfig` généré par le build CI est conservé en artefact.

Sources : https://github.com/espressif/esp-idf/releases/tag/v5.5.6 ; https://hub.docker.com/r/espressif/idf/tags ; https://github.com/espressif/esp-idf-ci-action/tree/v1 .

### Hôte Linux

```sh
cmake -S tests -B build-host-gcc -G Ninja -DCMAKE_C_COMPILER=gcc -DWARSHIP_SANITIZERS=ON
cmake --build build-host-gcc
ctest --test-dir build-host-gcc --output-on-failure -V
cmake -S tests -B build-host-clang -G Ninja -DCMAKE_C_COMPILER=clang -DWARSHIP_SANITIZERS=ON
cmake --build build-host-clang
ctest --test-dir build-host-clang --output-on-failure -V
```

### Cible ESP32 classique (aucun périphérique CYD activé)

Installation officielle par [Espressif Installation Manager](https://docs.espressif.com/projects/esp-idf/en/v5.5.6/esp32/get-started/index.html) ou clone `git clone -b v5.5.6 --recursive https://github.com/espressif/esp-idf.git` suivi de `./install.sh esp32`, activation `source export.sh`. Installer CMake/Ninja et l'outillage Xtensa gérés par cette distribution ; relever versions dans les logs.

```sh
idf.py --version
idf.py set-target esp32
idf.py build
idf.py size
idf.py size-components
sha256sum build/cyd_warship.bin
```

Alternative sans installation locale, en environnement Docker équipé :

```sh
docker run --rm -v "$PWD:/project" -w /project espressif/idf:v5.5.6 \
  bash -lc 'idf.py --version && idf.py set-target esp32 && idf.py build && idf.py size && idf.py size-components && sha256sum build/cyd_warship.bin'
```

**Notes de preuve :** `idf.py size` mesure les sections statiques code Flash, données Flash, DRAM/IRAM ; **pas** le heap libre réel, ni la taille Flash câblée. Les valeurs rapportées ne sont valables que pour le SHA et le `sdkconfig` générés à ce build. L'artefact CI contient binaire/ELF/bootloader/partition table/flasher_args, `sdkconfig` généré et fichier `.map`, mais **ne prouve aucun flash ni fonctionnement de l'écran/tactile**. Aucun budget produit décidé ici.

## Bilan T01 — preuves acquises, liées à un SHA explicite

**Socle initial :** `main` après S01, `2b337ab08bea4f7ca095f9021128d4db92625d06`. **Référence de mesure firmware :** branche `feat/issue-18-socle-c-espidf`, commit source `dfccfdaf1fc39dcaf0e2e780a8297f5dea5e4db6`. La CI de référence est [run #41100144488](https://github.com/Kharn27/ESP32-OTA-Battleship/actions/runs/41100144488) — **trois jobs terminés en `success`**, vérifiés via GitHub Actions ; checkout explicitement effectué sur ce HEAD, et non un commit de merge éphémère. Il s'agit des preuves **sur ce commit**, indépendamment de la CI du commit documentaire qui portera le présent texte.

| Contrôle | Environnement réellement exécuté | Résultat |
| --- | --- | --- |
| `HOST_TESTED` hôte local initial | Linux x86_64, GCC 14.2.0, Clang 17.0.0, CMake 3.31.6, Ninja 1.12.1, C99 strict, ASan/UBSan | 13/13 assertions et CTest 1/1 PASS pour chaque compilateur ; relevé initial T01 |
| [CI GCC](https://github.com/Kharn27/ESP32-OTA-Battleship/actions/runs/41100144488) | Ubuntu 24.04, GCC **13.3.0**, CMake 3.31.6, `-std=c99 -Wall -Wextra -Werror -pedantic`, ASan/UBSan | **PASS**, 13 checks, 0 failure ; CTest **1/1 PASS** |
| [CI Clang](https://github.com/Kharn27/ESP32-OTA-Battleship/actions/runs/41100144488) | Ubuntu 24.04, Clang **18.1.3**, CMake 3.31.6, mêmes options et sanitizers | **PASS**, 13 checks, 0 failure ; CTest **1/1 PASS** |
| `TARGET_BUILD_VERIFIED` | Conteneur officiel `espressif/idf:v5.5.6`, Xtensa GCC 14.2.0, `idf.py set-target esp32 && idf.py build && idf.py size && idf.py size-components` | **PASS**, application `cyd_warship.bin`, aucune CYD physique |
| `CI_PASSED` sur `dfccfdaf...` | [GitHub Actions #41100144488](https://github.com/Kharn27/ESP32-OTA-Battleship/actions/runs/41100144488) | **PASS, 3/3 jobs** — l'état GCC passé temporairement par `in_progress` est maintenant `completed/success` |
| `ON_DEVICE_VERIFIED` | Aucune carte utilisée | **NON TESTÉ / non applicable à la fusion T01**, réservé à T02 |

**Cas négatifs réellement testés :** refus d'un pointeur `NULL`, d'un buffer trop petit ou de capacité nulle, préservation du tampon et des sentinelles lors des refus ou d'une capacité supérieure ; aucune prétention de test du moteur futur.

### Mesures statiques réelles — `idf.py size` sur `dfccfdaf...`

| Zone | Usage [octets] | Portée |
| --- | ---: | --- |
| Flash Code | **61 790** | Code statique en Flash |
| Flash Data | **37 284** | Constantes stockées en Flash |
| IRAM | **52 943** | 40,39 % des 131 072 o répertoriés par l'outil |
| DRAM | **12 508** | 6,92 % des 180 736 o répertoriés par l'outil |
| RTC SLOW | **56** | Section statique |
| Total image (`idf.py size`) | **162 309** | Taille calculée, pas nécessairement taille binaire physique |
| `build/cyd_warship.bin` | **162 432** (`0x27a80`) | Binaire effectivement généré |
| Partition application du build | **1 048 576** (`0x100000`) | Configuration IDF par défaut **de cette compilation**, non imposée pour WARSHIP |

**SHA-256 du binaire de ce build :**

```text
f37371e5cd9a7867440c64d88d04b0ac4e879b9cdb786f7efdbb8e74c24e3db6  build/cyd_warship.bin
```

Artefact GitHub : `t01-esp32-firmware-dfccfdaf1fc39dcaf0e2e780a8297f5dea5e4db6` dans le [run #41100144488](https://github.com/Kharn27/ESP32-OTA-Battleship/actions/runs/41100144488) — inclut `.bin`, `.elf`, bootloader, table de partitions, `flasher_args.json`, fichier `.map` et `sdkconfig`.

**Limites importantes :** la chaîne de flash proposée par ce build indique `--flash_size 2MB` (paramètre généré), **sans preuve de la taille Flash de la CYD possédée**. PSRAM, brochage, RAM/heap libre à l'exécution, plus gros bloc, piles FreeRTOS, écran, tactile et comportement sur matériel restent **À VÉRIFIER / NON TESTÉS** ; aucune mesure de taille statique ne constitue un budget validé pour les modules futurs. Des avertissements de dépréciation Node.js apparaissent lors de l'upload d'artefacts ; ils ne sont pas des diagnostics du code applicatif C.

**Évolution du rapport :** cette mise à jour documentaire intervient **après** le commit de mesure `dfccfdaf...` ; le commit final de la PR est donc différent. Ne pas attribuer l'empreinte binaire ci-dessus à un autre SHA sans nouvelle vérification. Le run CI sur le nouveau HEAD doit être consulté séparément dans la PR avant de prononcer `CI_PASSED` **sur le HEAD final**. Aucune modification de code source ou des commandes de compilation n'est apportée par ce correctif documentaire.
