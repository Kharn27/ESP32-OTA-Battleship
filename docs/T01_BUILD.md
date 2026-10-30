# T01 — Socle C natif et preuves reproductibles

**Contrat :** [Issue #18](https://github.com/Kharn27/ESP32-OTA-Battleship/issues/18). Source : Bible v0.17 §2/14.1/14.9, S01, politique de tests. Pas de pilotes, gameplay, NVS, PSRAM supposée, radio ni test physique.

## Contrat du code

- `components/warship_core`: C99 portable, sans dépendance ESP-IDF, fonction `warship_core_banner(out, capacity)` ; copie constante NUL incluse, taille 16 octets, aucune écriture si pointeur nul ou capacité insuffisante. Aucune allocation ni état mutable. L'API ne définit **aucune règle du jeu**.
- `main/app_main.c`: appelle le module depuis un `app_main` C, affiche une trace console. **Ne configure aucun GPIO, écran, tactile ou radio.**
- `tests`: même source C99 compilée indépendamment par CMake et testée par CTest, notamment NULL, tampon court, zéro, tampon exact et sentinelles.

## Choix d'environnement à faire confirmer par Pilotage

**Candidate explicite à validation par compilation : ESP-IDF `v5.5.6`.** C'est une release corrective officielle de la branche 5.5, publiée en octobre 2026 et disponible comme image versionnée `espressif/idf:v5.5.6`. Sa capacité à produire le binaire `esp32` est à établir **par l'exécution CI sur le HEAD final** ; elle ne constitue pas un verrou irrévocable pour les pilotes T02. Aucun `sdkconfig` matériel / partition flash spécifique n'est versionné. La configuration de build est celle des defaults ESP-IDF (capturer le `sdkconfig` généré avant une comparaison de tailles).

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

**Notes de preuve :** `idf.py size` mesure les sections statiques code Flash, données Flash, DRAM/IRAM ; **pas** le heap libre réel, ni la taille Flash câblée. Les valeurs rapportées ne sont valables que pour le SHA et le `sdkconfig` générés à ce build. L'artefact CI contient binaire/ELF/bootloader/partition table/flasher_args, pas une validation de flash ou tactile. Aucun budget produit décidé ici.

## Bilan T01 — journal de qualification

- Base GitHub `main` après S01 : `2b337ab08bea4f7ca095f9021128d4db92625d06`.
- Hôte Linux x86_64 (environnement de développement distant) : **HOST_TESTED**, GCC 14.2.0 / Clang 17.0.0, CMake 3.31.6 / Ninja 1.12.1, options `-std=c99 -Wall -Wextra -Werror -pedantic -fsanitize=address,undefined`. Chaque compilateur : **13/13 assertions PASS**, CTest 1/1 PASS. Commandes ci-dessus. Pointeurs NULL et buffers courts sont des cas négatifs dont le rejet est attendu, pas des tests échoués.
- `TARGET_BUILD_VERIFIED` : **NON TESTÉ tant que la CI cible n'a pas exécuté le build**. Toolchain ESP32 absente du PATH local ; accès réseau GitHub indisponible depuis cette machine d'exécution.
- `CI_PASSED`, SHA-256 `.bin`, mesures statiques : **À COMPLÉTER depuis GitHub Actions au HEAD soumis** ; aucune valeur inventée.
- Physique `ON_DEVICE_UNVERIFIED` : **NON APPLICABLE pour T01**, prévu T02.
