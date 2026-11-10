# T03 — Pipeline G03 Flash → bandes TFT, diagnostic borné (#67)

**Mandat :** [Issue #67](https://github.com/Kharn27/ESP32-OTA-Battleship/issues/67) et [GO Pilotage du 06/11](https://github.com/Kharn27/ESP32-OTA-Battleship/issues/67#issuecomment-6682384632). Base `main` **026db0e14a0b74289ed020aba5e485e38e19c435**. Ce document est un **SPIKE de mesure**, pas une décision produit, un contrat graphique PO ni l'intégration D14/D15.

## 1. Baseline reproductible : distinguer observation et hypothèse

**Observation matérielle PO transmise par Pilotage (D13, pas test T03) :**

| Scénario D13 sur vraie CYD | Observation |
|---|---|
| Plein écran 320×240, 30 bandes | **763 à 882 ms** |
| SHOW/HIDE post-défaite | **826,5 ms** environ |
| Déploiement complet | **1 167 ms** environ |
| Sélection cellule 19×19 | **4 ms** environ |
| FEU partiel 91×38 | **38 ms** environ |
| Pile main watermark minimum | **1 464 octets** (borne exigée ≥1 024) |
| Heap8 / plus grand bloc | **290 228 / 163 840 octets** |
| Bande DMA existante | **5 120 octets**, zéro défaut LCD/timeout relevé |

Aucune de ces valeurs n'a été **remesurée** par T03. La trace D13 `DRAW us=` mélange la synthèse C des pixels et l'attente synchrone LCD. L'hypothèse « raster C99 dominant » reste **À VÉRIFIER**.

**Profilage T03 instrumenté (option ON uniquement) :** `T03_RASTER pixel_us=... lcd_us=...` pour le raster D09/D10; `T03 ASSET format=... decode_us=... lcd_us=... total_us=... bands=30` pour deux écrans G03 complets. Chaque séquence comprend exactement 30 rectangles 320×8 via **le même** `warship_lcd_send()`, avec transport SPI2 et bloc DMA partagé; pas d'initialisation additionnelle. La mesure `lcd_us` inclut commandes, file SPI, temps d'attente/ACK de DMA, ordonnancement; elle n'isole **pas** le débit physique des seules données du bus. `total_us` inclut les frais de boucle, mesure et journalisation. Comparer les trois résultats sur la **même carte / même firmware** sans présenter le contenu synthétique et G03 comme des pixels visuellement identiques.

## 2. Une source graphique, deux formats **sans perte RGB565**

Source unique et inchangée : `maquettes/deploiement/legacy/g03_deploiement_320x240.png` (G03 `VALIDÉ PO` comme `DESIGN_PREVIEW` uniquement), SVG associé dans `maquettes/deploiement/legacy/`. Convertisseur `tools/t03_export.py` : Python standard uniquement, refuse les dimensions autres que 320×240, les PNG non RGB8, entrelacés, alpha/transparence et malformés (CRC, filtres, longueurs). Quantification **déterministe et identique** dans les deux formats : canaux RGB8 réduits en RGB565 par décalage 5/6/5 bits; octets **high-byte first** compatibles `warship_lcd`. Toute éventuelle perte initiale RGB888→RGB565 est **commune** aux deux.

| Option | Flash utile (octets, **hors alignement et linker**) | Restitution | Coût CPU et RAM |
|---|---:|---|---|
| **RAW565BE v1** | **153 600** = 320×240×2 | Pixels RGB565 exacts, aucune compression | Copie `memcpy` par bande; **0 tampon supplémentaire**; destination = DMA existant |
| **ROW-RLE565BE v1** | **RLE réel + 964** = flux de triplets `(run 1..255, hi, lo)` + 241 offsets `uint32` | Pixels RGB565 exacts, **sans perte**; chaque ligne indépendante | Lecture/validation des runs et décompression bornée dans DMA existant; 0 heap et 0 full framebuffer |

Les **tailles RLE et empreintes ELF exactes** sont imprimées lors de la configuration CMake et dans les logs CI. Tant que ces sorties n'existent pas, elles sont **NON TESTÉES**, pas remplacées par une estimation. Le décodeur C99 `components/t03_decode/` valide les coordonnées et la capacité du destinataire, les offsets de rangée, la taille, les runs nuls, tronqués et débordants. Deux passes valident tout le rectangle RLE avant écriture, afin de ne pas utiliser un tampon partiellement décodé après erreur. `row_offsets` appartient aux données constantes en Flash, pas à la pile de la CYD.

**Protocole reproductible PC :**

```sh
python3 tools/t03_export.py --input maquettes/deploiement/legacy/g03_deploiement_320x240.png --output /tmp/t03-a.c
python3 tools/t03_export.py --input maquettes/deploiement/legacy/g03_deploiement_320x240.png --output /tmp/t03-b.c
cmp /tmp/t03-a.c /tmp/t03-b.c
cmake -S tests -B build-host -G Ninja -DCMAKE_C_COMPILER=gcc -DWARSHIP_SANITIZERS=ON
cmake --build build-host
ctest --test-dir build-host --output-on-failure
# Même séquence avec -DCMAKE_C_COMPILER=clang dans un AUTRE répertoire.
```

Le test `t03_g03_exact_raw_vs_rle` compare tous les pixels de la référence **RAW565 calculée depuis le PNG** au flux RLE sur 30 bandes, puis des rectangles partiels; vérifie aussi des entrées illégales et des données RLE corrompues. La CI relance l'export et compare les fichiers générés octet pour octet. La conversion laisse `design/` intact.

## 3. Flash/DRAM/IRAM et mode de diagnostic

Option `WARSHIP_T03_DIAGNOSTIC=OFF` par défaut : D09/D10, D11/D13, le boot, le moteur et les partitions conservent leur comportement de production; **aucune image G03 n'est compilée/liennée dans le firmware normal**. Un fichier `t03_g03_data.c` est généré **dans le répertoire de build** seulement lorsque l'option ON est choisie. Ce binaire **de laboratoire** embarque **les deux variantes simultanément** pour le protocole comparatif : sa taille Flash additionnelle **ne représente pas** une décision d'intégrer les deux en production.

```sh
# CI/test isolé ; pas un firmware de production
idf.py -B build-t03 -D WARSHIP_T03_DIAGNOSTIC=ON set-target esp32
idf.py -B build-t03 -D WARSHIP_T03_DIAGNOSTIC=ON build
idf.py -B build-t03 size
idf.py -B build-t03 size-components
sha256sum build-t03/cyd_warship.bin
```

Au démarrage de ce mode, **après** `warship_lcd_init()` et `warship_lcd_prepare_console()`, et **avant** le dessin normal D09 et le tactile, le diagnostic envoie G03 en RAW puis en RLE dans le tampon DMA emprunté au pilote. En cas d'erreur, il arrête le parcours sans réutiliser la bande. Il ne modifie pas l'acquittement D10 (ACK après dessin), les règles, le NVS, les données CPU, le SPI3 tactile ni les contrôles de gestes. Le LCD est initialisé **une fois**. `MEM phase=t03_complete` rapporte pile minimum, heap libre/plus grand bloc et défauts; le rapport exact est à collecter **sur CYD**.

**Statuts de preuve :** GCC/Clang C99 ASan/UBSan : en attente des runs; compilation ESP32 v5.5.6 : en attente du job; CI 3/3 HEAD exact : en attente; **ON_DEVICE_UNVERIFIED**. Une compilation/CI ne confirme ni l'orientation physique, ni les temps, ni la fluidité.

## 4. Contrat technique **proposé** pour D14 puis D15 (soumis au Pilotage)

- **Entrée/source :** exporter une version approuvée du PNG + référence à son SVG/source, SHA-256 des octets d'entrée et du binaire décodé, auteur/licence/provenance à documenter; **pas de retouche par DEV**.
- **Format v1 :** image opaque 320×240 RGB565 big-endian, **RAW565BE** ou **ROW-RLE565BE** avec offsets par ligne. Pas de canal alpha; toute transparence future exige un contrat de masquage/compositing explicite et testé. Aucun format implicite « RGB565 natif » non endianné.
- **Mémoire :** Flash pour données `const`, décodage par rectangles dans **la seule bande DMA 5 120 octets** SPI2, ne jamais allouer 153 600 octets de framebuffer. Échec décodeur ou LCD = arrêt de la séquence sans ACK/écriture DMA ultérieure. Pile main ≥1 024 octets de marge **mesurée**, heap8 et `largest8` à comparer à la baseline.
- **Écrans dynamiques :** G03 reste un **aperçu figé**. Le futur fond ne doit pas prépeindre des navires de placement, textes dépendants des états ou secrets CPU; demander au Graphiste des fonds/couches **approuvés et exploitables séparément** avant intégration D14. Les navires humains, sélections, boutons AUTO/PRÊT et messages restent des régions redessinées selon l'état public. Ne pas persister la révélation CPU dans un asset ou cache. Ne pas inventer d'assets non approuvés.
- **Dirty rectangles :** appliquer les rectangles déjà éprouvés par D09/D10, puis réduire les invalidations des seules régions réellement modifiées, clipsées à 320×240 et découpées en bandes ≤5 120 octets; reconstruire et valider un fond local avant surimpression de l'état public. Les séquences FEU conservent l'ACK seulement **après** visibilité de la paire; abandon/gestes ignorés pendant animation; retour ATT/DÉF ne doit ni montrer ni mémoriser un secret. Ne pas recadrer le visuel par simple découpage sans accord Graphiste/PO.
- **Budget performance proposé, pas décidé PO :** viser **< 250 ms** pour le premier plein écran statique G03 sur CYD réelle, et privilégier les rectangles partiels pour les interactions. La borne **purement physique** 153 600×8 / 20 000 000 ≈ **61,44 ms** ne compte ni commandes LCD, ni attente DMA, ni décodage, ni ordonnancement. **Aucun gain ×N garanti** avant mesures sur l'appareil. Si le raster synthétique demeure coûteux, proposer une US distincte de réduction/régionalisation D10, sans la réaliser dans T03.

**Choix provisoire à soumettre :** comparer les tailles issues de l'export et les temps LCD réels. Préférer ROW-RLE565BE **si** son flux est sensiblement inférieur au RAW et si sa décompression mesurée respecte la marge de pile et le temps de rendu proposé. Sinon privilégier RAW565BE exact et simple, ou **NO_GO** si l'une des limites de Flash/RAM/latence est dépassée. Ne pas généraliser avant les essais.

## 5. Protocole matériel T03 minimal (si Pilotage juge la mesure nécessaire)

**Une seule** ESP32-2432S028R classique, ESP-IDF v5.5.6, câble USB/port série. Flasher **uniquement le BIN diagnostic T03** de l'artefact CI de la PR, vérifier le SHA-256 du binaire et conserver le **HEAD complet** correspondant. Ne pas confondre avec le binaire D13 de réception (ou avec le build normal). Idéalement faire une sauvegarde de la Flash/NVS avant tout flash; T03 n'ajoute pas d'écriture NVS, mais le parcours normal D09/D11 reste présent après diagnostic.

1. Flasher avec les `flasher_args.json`, bootloader et table de partitions **de l'artefact T03 correspondant** (pas d'offset inventé), puis ouvrir le moniteur à 115200 bauds.
2. Redémarrer et relever `T03 ASSET format=raw565be`, `row-rle565be`, `T03_RASTER` initial, `MEM phase=t03_complete`, `LCD_FAIL`, `timeouts`, `lcd_fault`; conserver les logs bruts.
3. Vérifier que le parcours D09 reprend et que l'écran/tactile fonctionnent normalement; une photo de la page normale et, si possible, une courte vidéo de démarrage suffisent. Ne pas demander 20 tirs ni de campagne de variantes pour ce spike.

**Critères d'interprétation :** `stack_margin_bytes>=1024`, `timeouts=0`, `fault=0`, DMA inchangé 5120, heap8 et largest8 rapportés sans diminution inexpliquée. Les chronos seuls ne prouvent pas la fidélité des couleurs/orientation sur écran; signaler cette limite séparément. En l'absence d'essai, **ON_DEVICE_UNVERIFIED**, jamais `ON_DEVICE_VERIFIED`.

## 6. Périmètre exclu et remise

Pas de changement dans `design/`, `BIBLE_PRODUIT.md`, IA, D07/D08, NVS, règles, ESP-NOW, G04/G05/G07 R1 ou intégration UI complète. D14 #69 et D15 #70 demeurent des missions Pilotage **ultérieures**. Révision technique, validation du contrat v1 et fusion : **Pilotage seul**.
