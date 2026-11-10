> **ARCHIVE / NON EXÉCUTABLE (10/11/2026).** Les sondes D14 COLOR A–E et
> DOOM_REFERENCE sont supprimées du build/CI de production. Les commandes,
> anciennes tailles et BIN mentionnés plus bas sont historiques, **à ne pas
> flasher pour la réception D14 actuelle**. Le profil LCD approuvé est désormais
> intégré inconditionnellement au firmware normal ; procédure actuelle :
> [D14_LCD_PRODUCTION.md](D14_LCD_PRODUCTION.md).

# D14 — Audit indépendant Pillow / PNG / RGB565 / BIN ESP32 — Issue #69

**Contrat Pilotage :** [ordre #6764371996](https://github.com/Kharn27/ESP32-OTA-Battleship/issues/69#issuecomment-6764371996) et [PR #91 #6764379881](https://github.com/Kharn27/ESP32-OTA-Battleship/pull/91#issuecomment-6764379881), 10 novembre 2026. Travail **sans nouveau flash**, branche existante D14, PR Draft non fusionnée. Aucune décision sur couleurs, gamma, inversion ou sources artistiques.

## Événement déclencheur / limites des photos

Le PO a physiquement comparé sur CYD le véritable accueil G01 PR90 et la référence PC. Il rapporte **ciel rouge/orange devenu rose pâle, mer, logo et boutons ternes**, tandis que la mire A de primaires directs lui paraît correcte. Le verdict métier reste **`CORRECTIONS_REQUISES`**. Les deux photos n'identifient pas à elles seules le SHA exact du BIN exécuté, ni des métriques UART. Il n'existe **aucune mesure colorimétrique instrumentée** : les prises de vue sont soumises à des expositions et balances différentes.

L'hypothèse « bug dans le décodeur PNG » doit être testée **indépendamment** ; l'ancien test D14 comparait un RAW Flash à une référence construite avec le **même** décodeur maison, ce qui était insuffisant pour valider la lecture du PNG source.

## Source et trois oracles réellement distincts

| Objet | Référence vérifiable |
|---|---|
| Visuel approuvé | PR graphique **#90**, `design/ecrans/accueil/accueil_cyd_warship_320x240.png` |
| Blob Git PR90 sur main | `7bd321c6ccbfc230802e20010bfa06e71a9099a9` |
| Source SHA-256 | `ca73f029e80c95c09b3c7defe1437bba3156280b2f2169465ee3f30014343873` |
| Format source | 320×240 PNG RGB8, 144890 octets, **76800 pixels** |
| SHA-256 RAW565BE attendu | `8a376f7ca4cf83ed31b146905b25c80455b71fdef194eee8c40ac00e2d5248dc` |
| Taille RAW565BE | 320 × 240 × 2 = **153600 octets** |
| Ancien générateur | `tools/d14_color_g01_export.py` → `t03_export.read_png` + `rgb565_be` |
| Nouvel oracle | `tools/d14_color_pillow_audit.py` → Pillow `Image.open(...).convert("RGB").tobytes()` et quantification **par masques indépendants**, sans appel à `rgb565_be` |

1. **PNG_SOURCE** : contrôle du SHA256 original et de la taille ; Pillow décode **230400 octets RGB888** de façon indépendante du parseur maison. Comparaison octet par octet des 76800 triplets Pillow vs `t03_export.read_png()`. Sur désaccord : premier (x,y), canal, deux triplets, octet et **filtre de la ligne PNG**. Un `PASS` signifie égalité RGB888 intégrale, pas seulement des hashes de test dérivés du même code.
2. **GENERATE_RGB565** : conversion Pillow indépendante des composantes (`((R & 0xF8)<<8) | ((G & 0xFC)<<3) | (B>>3)`), comparaison des **153600 octets** à la référence du générateur et au **tableau C réel généré** `g01_pr90_raw565`, extraction par analyse de sa définition C.
3. **COMPILED_FLASH** : une fois l'application réellement construite par ESP-IDF, recherche de **l'unique séquence contiguë de 153600 octets RAW565BE** dans `build-d14-color/cyd_warship.elf` **et** `build-d14-color/cyd_warship.bin`. Le script refuse l'absence, les occurrences multiples ou un contenu différent. Il enregistre les **offsets de fichier exacts**, les SHA256 des fichiers et les octets extraits. Cela confirme le **BIN de compilation**, **pas** une capture physique de la Flash ni des octets SPI sur la CYD.
4. **RECONSTRUCTION_PC** : seuls les **octets extraits du BIN** alimentent la reconstruction RGB565→RGB888 : `R8=(R5<<3)|(R5>>2)`, `G8=(G6<<2)|(G6>>4)`, `B8=(B5<<3)|(B5>>2)`. Pas de traitement couleur supplémentaire. Les écarts attendus maximum vs PNG RGB888 sont **7 rouge / 3 vert / 7 bleu** en valeur absolue ; le rapport donne valeurs **observées** et moyennes de chaque canal, plus des pixels témoins ciel, mer, bouton bronze, logo et soleil.

L'image reconstruite depuis le tableau C sur hôte est **distinctement étiquetée `GENERATED_C_FLASH_ARRAY_HOST_ONLY`** ; elle ne doit pas être présentée comme « preuve des octets de l'ESP32 » tant que l'extraction ELF/BIN n'a pas eu lieu. Le rapport du job Xtensa doit indiquer `APP_BIN_EXTRACTED`.

## Preuves et livrables automatiques

- **C99 hôte GCC/Clang ASan+UBSan** : le test `d14_g01_pr90_independent_pillow_oracle` donne au vrai décodeur C `t03_decode_rect()` une référence construite avec Pillow, indépendante du décodeur Python maison. Il vérifie **30 bandes DMA**, tous les pixels, ROI, limites de buffers. Le job hôte compare aussi Pillow RGB888 au parseur PNG et Pillow RGB565 au tableau C.
- **ESP-IDF v5.5.6 Xtensa esp32** : seul le **diagnostic D14 G01 TEST-ONLY** est inspecté au niveau ELF/BIN ; aucune mutation du binaire normal, de G01, des sources `design/`, du pilote LCD ou de la NVS. La CI exécute les vérifications après sa compilation.
- **Artefact de preuves CI** (nom `warship-d14-PILLOW-FLASH-AUDIT-<HEAD>`) : `report.json` ; `g01_reconstructed_from_esp32_flash.png` ; `g01_original_vs_esp32_flash.png` (source à gauche, octets BIN à droite, **aucun redimensionnement**) ; `g01_quantization_delta_actual.png` ; `g01_quantization_delta_x8_DEBUG_ONLY.png` (écarts amplifiés **uniquement pour les rendre visibles**, PAS une correction graphique) ; `g01_extracted_from_esp32_flash.rgb565be` ; `pillow_expected.rgb565be`, `README.txt`.

**Lecture de la chaîne causale :**

| Maillon | Ce que teste l'audit | Limite de preuve |
|---|---|---|
| `SOURCE_PNG` | Git blob, SHA-256, taille/mode | Validité artistique = PO, non outil |
| `PNG_DECODE` | Pillow vs maison, 76800 pixels RGB888 | Ne caractérise pas le LCD |
| `GENERATE_RGB565` | Pillow autonome vs C Flash | Erreurs de quantification 5/6/5 normales |
| `COMPILED_FLASH` | Recherche exacte des octets dans ELF + BIN | Ne valide pas la Flash effectivement programmée |
| `T03_COPY` | `t03_decode_rect` comparé à Pillow sous ASan/UBSan | Ne capture pas la bande DMA sur CYD |
| `ESP_LCD` | Compilation et revue du chemin `esp_lcd_panel_draw_bitmap`, RGB/BIG, case10 | SPI/commande/GRAM non lus sur la dalle |
| `PANEL_CURVE` | **NON TESTÉ** par cet audit host | Gamma/demi-teintes/optique exigent preuve matérielle séparée |

Si **Pillow et les octets réels du BIN concordent** et que la reconstruction PC conserve l'aspect artistique sauf quantification 565, une erreur du décodeur PNG ou de la source Flash **n'explique pas** le délavage. Les investigations suivantes doivent porter en priorité sur le transfert effectif vers `esp_lcd` et les **demi-teintes / courbe du panneau**. Les aplats 0/255 du diagnostic A précédent ne sont **pas** un test de ces demi-teintes. Une sonde future peut afficher rouge/orange, bleu et gris synthétiques **directement au DMA** sur **le même profil A, PWM40, case10** ; ne jamais cumuler B/C/D/E refusés. Une gamma seule sous inversion OFF est une hypothèse technique distincte de l'ancien mode D (inversion ON + gamma) et **ne sera pas appliquée sans protocole et arbitrage**.

## Exécution reproductible PC / CI

En partant du HEAD de la branche D14, avec **Python + Pillow==11.3.0** :

```sh
# Host : le générateur produit l'array et la référence du vieux pipeline ;
# l'audit refait le décodage + quantification indépendamment avec Pillow.
cmake -S tests -B build-host -G Ninja -DWARSHIP_SANITIZERS=ON
cmake --build build-host
ctest --test-dir build-host --output-on-failure

# ESP-IDF v5.5.6, même HEAD et même source G01 :
idf.py -B build-d14-color -D WARSHIP_D14_COLOR_DIAGNOSTIC=ON set-target esp32
idf.py -B build-d14-color -D WARSHIP_D14_COLOR_DIAGNOSTIC=ON build
python tools/d14_color_pillow_audit.py \
  --input design/ecrans/accueil/accueil_cyd_warship_320x240.png \
  --generated-c "$(find build-d14-color -name d14_color_g01_data.c -type f -print -quit)" \
  --elf build-d14-color/cyd_warship.elf \
  --bin build-d14-color/cyd_warship.bin \
  --out-dir build-d14-color/d14-pillow-audit
```

**À transmettre au Pilotage :** HEAD exact, [run CI 3/3], hashes, offsets ELF/BIN, `report.json` et **prévisualisation PC**, rapport causal par maillon. Aucune nouvelle campagne de flash avant discussion des conclusions avec le PO. `ON_DEVICE_UNVERIFIED` pour tout diagnostic LCD qui suivrait. **PR #91 reste Draft, non fusionnée ; D14 `CORRECTIONS_REQUISES` ; D15 #70 non démarrée.**
