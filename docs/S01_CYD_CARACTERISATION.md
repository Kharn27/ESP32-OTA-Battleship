# S01 — Caractérisation ESP32 CYD et environnement C (LOT 1)

**Issue :** [#15](https://github.com/Kharn27/ESP32-OTA-Battleship/issues/15) — SPIKE, réalisation Développeur, 30/10/2026. **Base auditée :** `main` à `58b3ddad470fa682f090d4ae121d58f89064ec9b`. **Références normatives :** `BIBLE_PRODUIT.md` v0.17 (§2, §14.1, §14.9), `docs/WORKFLOW_AGENTS.md`, `docs/TEST_POLICY.md` et commentaire Pilotage de l'Issue #15. **Périmètre :** investigation, outillage, méthode de validation ; aucun gameplay, IA, radio ou graphisme de production.

## 1. État réel du dépôt — CONFIRMÉ par GitHub

- Arbre `main` obtenu via GitHub Git Trees API (non tronqué), commit `58b3ddad...` : `BIBLE_PRODUIT.md`, `MAQUETTES.md`, `README.md`, `docs/WORKFLOW_AGENTS.md`, `docs/TEST_POLICY.md`, modèles d'Issue/PR et dossier `design/graph-acc-001/` avec SVG/PNG de **prévisualisation**, pas assets runtime.
- **ABSENTS de cet arbre :** fichiers C/C++, `CMakeLists.txt`, `sdkconfig`, `platformio.ini`, `.github/workflows/`, partitionnement, scripts de test et pilote. Pas de firmware à compiler ni de dépendance logicielle déclarée. L'absence de sources rend toute affirmation `HOST_TESTED` ou `TARGET_BUILD_VERIFIED` **non applicable au projet actuel**.
- 13 branches renvoyées par l'API GitHub (dont `main` avant la création de cette branche), principalement documentaires / artistiques ; aucun workflow GitHub Actions dans l'arbre ; **0 run GitHub Actions renvoyé** par l'API à l'audit. **CI : ABSENTE**, pas « verte ».
- `README.md` décrit explicitement l'état pré-firmware. La maquette GRAPH-ACC-001 est `DESIGN_PREVIEW`, pas une preuve d'affichage ni de stockage Flash runtime.
- Les seules Issues de réalisation ouvertes retrouvées à l'audit sont S01 #15 et G01 #16. **T01/T02 ne sont pas encore des Issues contractualisées** : les conditions ci-dessous sont proposées au Pilotage, et ne constituent pas un lancement autonome.

## 2. Matériel — distinction stricte

| Élément | État | Base / décision de vérification |
| --- | --- | --- |
| Cible LOT 1 : une ESP32-2432S028R, 320×240 paysage, stylet, sans microSD utilisée, sans PSRAM supposée | **CONFIRMÉ (produit)** | Bible §2 et §14.1, pas mesure physique |
| SoC classique ESP32, SRAM sur puce 520 Kio (répartie entre usages) | **CONFIRMÉ (fiche famille ESP32)** | [Espressif ESP32 datasheet](https://documentation.espressif.com/esp32_datasheet_en.html), **pas** 520 Kio de heap applicatif |
| Module ESP32-WROOM-32 et Flash 4 Mio, absence PSRAM | **À VÉRIFIER sur l'exemplaire** | Profil souvent documenté : [ESP3D Sunton 2.8](https://esp3d.io/esp3d-tft/version_1x/hardware/esp32/sunton-28-2432/). Ne pas réserver de partitions ni de taille de Flash sur cette seule base |
| LCD ILI9341 SPI, natif 240×320 orienté paysage par logiciel | **À VÉRIFIER** | [Pinout CYD de référence](https://github.com/marcelrv/Cheap-Yellow-Display-ESP32-2432S028R/blob/main/pinout.txt), [ESP3D](https://esp3d.io/esp3d-tft/version_1x/hardware/esp32/sunton-28-2432/) |
| Tactile XPT2046 résistif SPI | **À VÉRIFIER** | Mêmes références ; calibrage, rotation, pression et bruit **non mesurés** |
| USB série CH340C, LEDs, audio éventuel | **À VÉRIFIER** | Documentation d'une variante, aucune fonction audio requise en S01 |

**Brochage *candidat documentaire* (aucune broche confirmée sur la CYD du PO)** :

| Groupe | Signal → GPIO candidat |
| --- | --- |
| LCD (bus SPI) | MOSI→13 ; MISO→12 ; SCLK→14 ; CS→15 ; DC→2 ; BL→21 |
| Tactile (second bus SPI documenté sur cette variante) | MOSI→32 ; MISO→39 ; SCLK→25 ; CS→33 ; IRQ→36 |
| LCD RESET | **non tranché** : certaines sources utilisent -1 (reset commun), d'autres citent 12 (également MISO LCD !) ; **ne jamais piloter 12 comme reset sans schéma concordant** |
| GPIO à risque | GPIO2/12/15 : fonctions de strapping au boot ; GPIO36/39 : entrées uniquement ; ne pas réaffecter sans analyse |

La discussion [Bodmer/TFT_eSPI #2677](https://github.com/Bodmer/TFT_eSPI/discussions/2677) confirme un piège documenté : le LCD et le XPT2046 ne partagent **pas** les mêmes fils MOSI/SCLK sur ce modèle, contrairement à certains exemples génériques. Il existe des sources communautaires contradictoires : seules photographie de PCB/révision, schéma correspondant, journaux de démarrage et sondes écran/tactile permettront de conclure.

**À relever physiquement avant choix final :** recto/verso, sérigraphie complète, référence du module ESP32 et de la Flash, variante USB, version PCB ; `esptool ... flash-id` (syntaxe à confirmer selon version installée) et logs de boot / détection PSRAM ; brochage au schéma de cette *révision*. **MESURÉ sur cible : rien à ce jour.**

## 3. Options de développement — recommandation soumise au Pilotage

| Choix | Avantage | Limite / décision |
| --- | --- | --- |
| **ESP-IDF natif C + CMake** (**recommandé pour T01**) | Support officiel ESP32 Xtensa, SPI DMA, `esp_lcd`, heap capabilities, FreeRTOS, partitions, flash/monitor, métriques de taille, NVS ; APIs C ; séparation nette du cœur C99 portable | Installation/toolchain à prouver ; **aucune version fixée** par S01. Choisir une release maintenue après matrice de compatibilité et *build réel* sur ESP32 |
| PlatformIO + `framework=espidf` | Outillage reproductible dans certains environnements et intégration VS Code | Ajoute une couche de configuration à maîtriser ; épinglage plateforme/outils à justifier si retenu |
| Arduino-ESP32 / TFT_eSPI | Exemples communautaires faciles à trouver | API essentiellement C++, risque d'interface tactile mal câblée, cœur natif C moins direct ; **non privilégié** pour ce mandat |
| LVGL | Widgets et redraw partiels possibles | Coût RAM/Flash, callbacks et intégration supplémentaires ; **à mesurer** avant introduction, non requis pour première sonde |

**Écran / entrée — options à instruire, non imposées :** `esp_lcd` (SPI DMA) + composant [Espressif esp_lcd_ili9341](https://components.espressif.com/components/espressif/esp_lcd_ili9341) ; XPT2046 via pilote C évalué, par exemple [atanisoft/esp_lcd_touch_xpt2046](https://github.com/atanisoft/esp_lcd_touch_xpt2046), ou adaptateur SPI minimal. Vérifier versions, dépendances et licences **au build**. L'[exemple ESP-IDF SPI LCD + touch](https://github.com/espressif/esp-idf/tree/master/examples/peripherals/lcd/spi_lcd_touch) est un point de comparaison technique, **pas** une configuration de broches CYD ni une obligation de LVGL.

**Proposition de frontière C :** cœur métier C99 sans include ESP-IDF ni FreeRTOS (interfaces d'horloge/aléa/persistance explicites), couche d'adaptation ESP-IDF strictement séparée. Développement et validation du moteur/IA **dans leurs futures Issues seulement**. Pas de version `sdkconfig`, fréquence SPI, pilote ou partitionnement retenus dans cette PR.

## 4. Essais réalisés — environnement de cette exécution (pas machine PO)

**MESURÉ / CONFIRMÉ sur hôte de test :** Linux x86_64 ; GCC 14.2.0, Clang 17.0.0, CMake 3.31.6, git et ninja disponibles. `idf.py`, `xtensa-esp32-elf-gcc`, `esptool` / `pio` non trouvés dans le PATH ; pas d'environnement Espressif installé aux chemins usuels. `git ls-remote` vers le dépôt GitHub échoue (résolution DNS), d'où audit et livraison via **connecteur GitHub**, pas clone local.

**MESURÉ — contrôle des compilateurs (code temporaire uniquement, hors dépôt) :**
- `gcc` et `clang` : `-std=c99 -Wall -Wextra -Werror -pedantic -O1 -g -fno-omit-frame-pointer -fsanitize=address,undefined` → compilation PASS ; exécution d'une micro-épreuve avec addition bornée 200+55, rejet 200+56 et rejet du pointeur nul → **3/3 PASS** avec chacun des compilateurs.
- Test négatif GCC/UBSan : addition signée `INT_MAX+1` avec `-fsanitize=undefined -fno-sanitize-recover=undefined` → **exit 1 attendu** ; diagnostic `runtime error: signed integer overflow`. Ce n'est pas un échec caché : le sanitizer a détecté la faute injectée.
- Ces micro-épreuves sont un **smoke test de l'outillage**, **pas HOST_TESTED du projet** (aucun code WARSHIP à tester). Résultats propres à l'environnement d'exécution, non transférables par supposition à la machine du PO.

**NON TESTÉ :** build Xtensa, exécution de tests projet, CI, Flash réelle, heaps/piles, TFT/tactile, performances, PSRAM, port série sur CYD.

## 5. Stratégie de validation (proposition pour futures Issues)

1. **Hôte PC :** cible CMake minimale autonome, `-std=c99 -Wall -Wextra -Werror -pedantic`, GCC puis Clang ; tests unitaires des règles et API sans ESP-IDF. Scénarios déterministes, invalides, bornes et état réinitialisé ; ASan/UBSan (et analyse statique si disponible). CI Linux sur commit précis et logs ; les tests du projet commencent **après** création de sources autorisées.
2. **Cible :** une fois version ESP-IDF sélectionnée et installée, journaliser `idf.py --version`, outil Xtensa, `idf.py set-target esp32` (uniquement nouveau projet), `idf.py build`, puis `idf.py size`, `idf.py size-components`. Conserver SHA, `sdkconfig` utile, versions, résumé warnings, SHA-256 de `.bin`. L'absence de firmware à date interdit de lancer ces commandes avec prétention de succès.
3. **Mémoire statique / Flash :** ELF/`idf.py size` pour sections Flash `.text` / `.rodata`, IRAM, DRAM `.data` / `.bss` ; tailles de `.bin`, bootloader, partition table et partitions réellement retenues, marge par rapport à partition app et Flash **identifiée**. Les chiffres de `idf.py size` ne mesurent pas le heap restant en fonctionnement.
4. **Mémoire dynamique :** à boot, initialisation TFT/tactile, écran affiché et après sollicitations répétées : `heap_caps_get_free_size(MALLOC_CAP_8BIT)`, `heap_caps_get_largest_free_block(MALLOC_CAP_8BIT)`, `heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT)` ; vérifier capacité DMA, mémoire PSRAM seulement si détectée. Suivre taille des buffers, nombre d'allocations, fragmentation et erreurs ; task stack high-water mark via FreeRTOS en précisant unité propre à la release choisie, tâche et scénario.
5. **Écran / tactile :** isoler le pilote ; test de couleurs distinctes, orientation paysage 320×240, lecture des quatre coins et du centre, coordonnées brutes, rotation/inversion, stabilité, erreur de sélection, pression au stylet. Tester dessins de régions et rafraîchissements répétés, chronométrer de manière instrumentée, ne pas fixer un FPS par déduction.
6. **Cas négatifs :** absence de pression, appui hors zone, bords, rebonds/bruit, perte init bus, reset/reboot, SPI invalide, allocation DMA refusée ; tester une reprise sûre sans accès mémoire hors limites. La CI n'équivaudra pas à l'essai CYD.

**Calcul de gabarit, pas budget alloué :** 320 × 240 × 2 octets (RGB565) = **153 600 octets** de framebuffer ; bande de 16 lignes = **10 240 octets** avant alignement/double buffering. Éviter un framebuffer plein écran obligatoire et mesurer la consommation réellement retenue.

## 6. Première fiche matériel (à orchestrer par Pilotage)

**État présent : NON TESTÉ / ON_DEVICE_UNVERIFIED.** Aucun `.bin`, hash de firmware ni SHA applicatif ne sont disponibles. Ne pas recycler un firmware d'un autre projet. Le futur essai se déroulera sur **une CYD seulement**, alimentée via câble USB données.

| Étape | Action / observation à enregistrer |
| --- | --- |
| 0. Fiche d'identité | Photos recto/verso lisibles, référence/revision PCB et module, contrôleurs si lisibles ; identité de la carte, alimentation, poste et OS |
| 1. Outils | Versions `idf.py`, toolchain, `esptool`, port USB et permission série ; boot log initial, `flash-id`, taille Flash reconnue ; vérifier réellement présence/absence PSRAM |
| 2. Artefact | Une **future** sonde T02 ciblée, **préalablement relue/autorisée par Pilotage** ; noter Issue/PR, SHA complet, binaire `.bin` + SHA-256, configuration et cible ESP32 |
| 3. Flash | Compiler depuis SHA identifié, relever ligne de commande et logs ; utiliser les **offsets générés** par le build/`idf.py -p PORT flash monitor`, jamais des offsets pris d'un autre firmware ; capturer erreurs et éventuel besoin BOOT |
| 4. Affichage | Démarrage puis écran de motifs techniques RGB565 : fond et zones rouge/vert/bleu/blanc/noir, repères des quatre coins, flèche orientation ; constater 320×240 paysage sans bandes, couleurs échangées ou inversion |
| 5. Tactile | Toucher centre et quatre coins au stylet (≥3 répétitions par zone) ; journal `raw_x/raw_y/z` si disponibles, position calculée, zone logique et erreur apparente ; relever appuis sans pression, répétitions et bords |
| 6. Ressources | Série avec étapes clairement identifiées : heap libre, plus grand bloc, minimum, taille buffers, pile de chaque tâche, temps d'affichage/réponse avant et après au moins 30 interactions (nombre proposé, non seuil produit) |
| 7. Reset | Reboot et redémarrage autonome ; collecter logs, anomalies et photos/vidéo de l'écran/interaction |
| 8. Remise | Rapport PO daté, carte identifiée, `SHA firmware`, commandes, logs complets, photos/vidéos et verdict par étape PASS/FAIL/NON TESTÉ. Qualification `ON_DEVICE_PO_REPORTED_PASS` uniquement après retour PO ; `ON_DEVICE_VERIFIED` seulement selon preuve conforme à la politique |

**Critère de réussite proposé, pas décision métier :** écran entier orienté correctement, couleurs vérifiables, touches répétables aux cinq points, logs et mesures présents, redémarrage sans blocage. Une erreur tactile ou couleurs inversées donne **FAIL ciblé**, sans conclure que « le CYD fonctionne ».

## 7. Risques, dépendances, sortie pour Pilotage

| Risque | Conséquence | Mesure / propriétaire |
| --- | --- | --- |
| Révisions CYD et schémas contradictoires | Affichage/tactile hors service, conflit GPIO/boot | Photos + identification physique et sonde T02 ; Pilotage coordonne PO |
| Flash et PSRAM non identifiées | Partitions/buffers invalides | `flash-id`, logs module et mesures ; aucun budget supposé |
| Discordance release ESP-IDF / composants LCD/tactile | Build non reproductible | Épingler **après** comparaison et compilation éprouvée ; Pilotage décide version/driver |
| RAM DMA et gros assets | OOM, fragmentation, temps de rendu | Render par régions, mesures heap/IRAM/DRAM et temps SPI ; contrat assets après G01 et T02 |
| Tactile non calibré et rotation fausse | Sélection dangereuse sur petites cases | Calibration réelle au stylet et erreurs mesurées avant G02/G03 |
| Outillage cible absent ici | Pas de preuve build/device S01 | T01 prépare toolchain, T02 et PO acquièrent les preuves |
| Absence de CI | Pas de `CI_PASSED` | Future CI hôte/ESP32 seulement après projet compilable |

**Pour T01 (proposition, pas US lancée) :** Pilotage choisit une release ESP-IDF **supportée par l'outil et les composants réellement retenus**, autorise un squelette C minimal, fixe les commandes d'installation reproductibles, obtient au moins un build ESP32 et une cible de test PC. Aucun pilote définitif requis pour cette seule étape.

**Pour T02 (proposition, pas US lancée) :** disposer d'une carte identifiée, du socle T01 compilable, du brochage provisoire confronté au PCB, d'une sonde ILI9341/XPT2046 bornée et d'une fiche d'essai physique transmise au PO via Pilotage. Exiger mesures mémoire et traces écran/tactile ; laisser les choix d'ergonomie/graphismes au PO, Pilotage et G01. Le lancement de T02 et l'ordre des travaux demeurent **à la discrétion du Pilotage**.

**Statuts de livraison S01 :** audit dépôt **CONFIRMÉ** ; capacités compilateurs hôte **MESURÉES** ; matériel, fréquences, heap, Flash réelle et PSRAM **À VÉRIFIER** ; `HOST_TESTED` (code jeu), `TARGET_BUILD_VERIFIED`, `CI_PASSED`, `ON_DEVICE_VERIFIED` **NON TESTÉ / ABSENT**. Document uniquement ; pas de changement métier. **Revue et fusion exclusivement Pilotage.**
