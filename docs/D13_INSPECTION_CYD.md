# D13 — Inspection post-partie et révélation volontaire du Capitaine (Issue #68)

**Mandat :** Issue [#68](https://github.com/Kharn27/ESP32-OTA-Battleship/issues/68), [ordre Pilotage](https://github.com/Kharn27/ESP32-OTA-Battleship/issues/68#issuecomment-6630470338) du 04/11/2026. Base `main` à l'ouverture `b57f2f4960fb28e39382b4a8ea241ce7deecabef` (D12 #71 fusionnée), branche `feat/issue-68-d13-inspection-resultat`. **Bible** §§14.4, 14.6, 14.10.3, 14.13. [D07](D07_SOLO_LIFECYCLE.md), [D10](D10_COMBAT_TECHNIQUE.md), [D11](D11_EVENTS_NVS_CYD.md), [D12](D12_PARCOURS_CYD.md), [TEST_POLICY](TEST_POLICY.md). G08 #55 approuvée sur les **fonctions** ; aucune reprise de G07-R1 artistiquement refusée, aucun rendu R2 final.

## Contrat C99 et confidentialité

**Composant `components/warship_inspection/`** : contrôleur C99 sans ESP-IDF/FreeRTOS, NVS, allocation, IA ni fichier d'assets. Il possède **zéro copie permanente** de la flotte CPU ; `game` est l'unique `WarshipLifecycle` privé de D09/D10. Il n'accède **jamais** à `game.captain`, aux secrets `WarshipSoloSession` ou aux champs D02/D06 pour la révélation.

- `warship_inspection_toggle(game, ui, hit)` n'accepte que la commande spécifique `WARSHIP_COMBAT_INSPECT` après RESULT **DEFEAT** et sur la vue **ATTAQUE**. Il interroge `warship_lifecycle_observe` puis `warship_lifecycle_set_captain_visible(!reveal_visible)` ; D07 reste l'unique autorité. **VICTOIRE, NUL, COMBAT, FEU, CONFIRM, DEPLOY/HOME et DÉFENSE** : aucune mutation, aucune révélation.
- `warship_inspection_overlay(game, observed, ui, frame)` s'exécute **après** `warship_combat_make_frame` (D10) et **avant** le rendu pixels. La projection `observed` est la vue publique D07 (flotte **humaine**, historiques, autorisation booléenne, aucun emplacement CPU). Le champ `frame.ships[100]` est **réutilisé en ATTAQUE** uniquement en RESULT DEFEAT avec `reveal_requested && reveal_visible`. La copie CPU est obtenue via `warship_lifecycle_copy_revealed_captain` **une fois par reconstruction**, validée comme `WarshipFleet` puis rasterisée sur **17 cases / 5 navires distincts**. **Cette copie locale `WarshipFleet` (25 octets) est effacée par écriture volatile avant retour**, y compris lors d'une erreur de validation. Pas de pointeur conservé. Les 100 cases de `ships[]` forment le **seul calque privé de présentation** dans le `WarshipCombatFrame` statique déjà existant.
- En ATTAQUE hors révélation, `ships[100]` est **explicitement effacé** avant chaque construction de calque, même si le frame a été réutilisé. En DÉFENSE, `ships[]` garde seulement les **17 cases humaines** issues de l'observation publique D07 ; l'état `reveal_visible` de D07 **survit** au changement de vue. Au retour ATTAQUE, la flotte volontaire est re-projetée depuis D07 sans aucun cache auxiliaire.
- `attacks[100]` demeure le tableau **inchangé** des tirs ratés, touchés et coulés historiques D10. `warship_combat_pixel` dessine les impacts **en priorité sur** les cases de navires révélés : la flotte découverte n'efface jamais l'historique. Le rendu est toujours **une seule grille 10×10**. Ni nouveau framebuffer ni PSRAM ni DMA additionnel.
- Dès **MASQUER** (y compris nouveau tap), **REJOUER**, **ACCUEIL**, avant **PRÊT** d'une nouvelle partie, `warship_inspection_clear_frame` efface **l'intégralité du frame statique** par écriture volatile. Aucune image résiduelle secrète n'est alors exposable ; le prochain frame est recréé depuis la projection D07 autorisée. Le code D12 traite toujours REJOUER/ACCUEIL avec ses fonctions d'origine. Pas de modification du registre D11, des IDs, des compteurs ni des événements terminaux.
- **Sécurité de la séparation :** la copie D07 ne sert qu'aux pixels, jamais à une stratégie de choix du CPU ; l'IA Capitaine continue à n'utiliser que l'état public. Aucun `ESP_LOG` ne contient les coordonnées/identités des navires révélés. Le flag visible autorise l'affichage mais **ne vaut jamais** consentement pour une transmission publique permanente.

## Affichage technique transitoire (pas G07-R1 / G07-R2)

Conserver D12 (320×240) : grille de 190×190, cellules 19 pixels à `x=22,y=24`, 3 boutons à droite REJOUER, ATTAQUE/DÉFENSE, ACCUEIL. **En RESULT DÉFAITE et vue ATTAQUE uniquement**, un **quatrième petit bouton technique** est placé en bas à droite, rectangle `x=225…315, y=203…233`, hitbox intérieure `x=227…313, y=205…231`, centre tactile recommandé `(270,220)`. Même détection B01 D10/D12, **40 ms DOWN, 60 ms UP, 120 ms entre commandes** ; refus des appuis maintenus, rebonds et entrées invalides.

| Après défaite | Texte du bouton | État de la grille |
|---|---|---|
| Avant toute demande | **VOIR SA FLOTTE** | Impacts existants, flotte CPU totalement invisible |
| Premier appui | **MASQUER** | Flotte complète du Capitaine + mêmes impacts |
| Appui suivant | **AFFICHER** | Flotte CPU effacée du frame, mêmes impacts |
| Nouvel appui | **MASQUER** | Flotte visible à nouveau, toujours mêmes impacts |
| Passage sur DÉFENSE | Aucun bouton de révélation | Seulement flotte humaine et impacts reçus |
| Retour ATTAQUE | État de visibilité conservé | Overlay ou masquage comme avant navigation |
| REJOUER / ACCUEIL | Aucun | Frame expurgé, historiques D07 de nouvelle session ou HOME |

En **VICTOIRE/NUL**, aucun quatrième bouton ; seule l'inspection libre ATTAQUE/DÉFENSE et REJOUER/ACCUEIL est accessible. Jamais de bouton « MONTRER MA FLOTTE », aucune boîte de dialogue, aucune fiche navire, aucun écran d'inspection séparé. Le rendu du texte 5×7 reste **technique** : typographie, couleurs, hitboxes définitives et assets G08 ne sont **pas validés artistiquement** par D13.

**Budget static à établir sur cible :** `sizeof(WarshipFleet)==25` par C99 (5×5 champs octet) ; **0 nouvelle allocation heap** ; `WarshipCombatFrame.ships[100]` déjà existant, **3 drapeaux de phase/révélation** ajoutés à la projection D10. Le CPU est **copié temporairement sur pile seulement lors du dessin volontaire ATTAQUE**, jamais en COMBAT. Les dimensions, DMA **5120 o**, pile main **4096 o** et partitions restent inchangés. Les valeurs Xtensa compilées (`idf.py size`, Flash/DRAM/IRAM) et les watermarks réels seront consignés sur le HEAD final.

## Test réel D07/D10/D12/D11 sur PC

Nouvelle suite `tests/test_warship_inspection.c` (intégrée au CMake hôte), **avec les vrais contrôleurs D07/D10/D12 et un registre `WarshipPalmares` à backend de test**. Scénarios déterministes de flottes indépendantes humaines `{0,20,40,60,80}`, CPU `{10,30,50,70,90}` et RNG de choix CPU = `0` ; l'algorithme Capitaine est **inchangé**. Les déroulés de tirs utilisant ces contextes passent par `select` + `fire` + `presented` / ACK réel D07, **sans affecter d'office le verdict**. Une vraie VICTOIRE, une vraie DÉFAITE et un vrai NUL sont ainsi reproductibles.

Essais exigés (et assertions conservées dans le code) : **18 suites complètes** (dont D13) sur **GCC puis Clang**, `-Wall -Wextra -Werror -pedantic -fsanitize=address,undefined`. Scénarios négatifs du dispatcher, autorisation D07, aucune copie révélée avant opt-in, grille ATTAQUE `ships=0`, 17 cases dévoilées avec cinq IDs, même `attacks[100]` à travers SHOW/HIDE et nav, marques de tirs toujours prioritaires, DEFENSE 17 cases humaines, maintien/double tap sans action différée, aucune donnée CPU accessible après MASQUER/REJOUER/HOME, aucune écriture supplémentaire du backend palmarès lors de l'inspection. **La compilation ESP32 et les tests CI ne valent pas preuve physique.**

## Mode de réception CYD déterministe — **explicite et isolé**

Pour une **CYD désignée avec accord PO/Pilotage uniquement**, un second firmware peut être compilé avec `WARSHIP_D13_CYD_SCENARIO=ON` **et** `WARSHIP_D11_NVS_BENCH=ON` dans un **répertoire de build séparé**. Le CMake **refuse** `WARSHIP_D13_CYD_SCENARIO=ON` si le namespace D11 n'est pas isolé. Ce mode remplace uniquement les sources RNG (flottes générées via **D01**, ciblage Capitaine via **D05** sur l'information publique), sans changer règles/moteur/IA, sans injecter de verdict, sans forcer `set_captain_visible`. Les tirs du joueur restent **manuels et simultanés**. Le firmware normal (options OFF) utilise toujours `esp_random()`.

```sh
# Normal : compilation, puis flash via VS Code Build → Flash → Monitor.
idf.py set-target esp32
idf.py build
sha256sum build/cyd_warship.bin

# DIAGNOSTIC D13 uniquement sur carte autorisée :
idf.py -B build-d13-reception -D WARSHIP_D11_NVS_BENCH=ON -D WARSHIP_D13_CYD_SCENARIO=ON set-target esp32
idf.py -B build-d13-reception -D WARSHIP_D11_NVS_BENCH=ON -D WARSHIP_D13_CYD_SCENARIO=ON build
sha256sum build-d13-reception/cyd_warship.bin
# Utiliser les flash args de CE répertoire; ne pas mélanger prod et bench.
```

L'UART doit annoncer `D13_SCENARIO=ARMED deterministic_rng=1 NVS=warship_d11lab TEST_ONLY` et `D11_BENCH=ARMED namespace=warship_d11lab`. **Le palmarès de production `warship/palmares` et la clé `warship/event_seq` ne sont ni lus ni modifiés par ce binaire**, seuls les compteurs du banc isolé peuvent évoluer. D11-BENCH peut inscrire **une VICTOIRE diagnostique initiale** sur première entrée avec clés de banc absentes ; il s'agit d'un témoin technique existant D11, **pas d'une partie réellement jouée**. Relever le baseline du banc avant d'y enregistrer la vraie défaite reproductible. Ne pas appeler ces valeurs PALMARÈS produit.

**Scénario de défaite reproductible, sans faux verdict :** depuis PRÊT en mode diagnostic, sélectionner **les seize premières cases adverses connues par la fixture de test** (les navires D01, non affichés par le firmware normal) pour obtenir des TOUCHÉ/COULÉ, en saisissant successivement les coordonnées :

```
F1 G1
F2 G2 H2
F3 G3 H3
F4 G4 H4 I4
F5 G5 H5 I5
```

Ensuite, tirer **une seule fois sur les cases d'eau restantes**, dans l'ordre de lecture ligne 1→10, colonne A→J, en **sautant toute case déjà ciblée** et toutes les cases de la flotte listées ci-dessus, ainsi que **J5** (17e case du dernier navire). En mode test uniquement, Capitaine suit ses décisions D05 sur informations publiques avec RNG fixé à 0 ; après un nombre **borné par 100 manches** (cf. suite hôte D13), il coule la flotte humaine et D07 produit **DÉFAITE** après le dernier ACK. À aucun moment le code du runtime ne lit directement la flotte secrète ni ne simule un résultat. **La dernière case J5 reste cachée jusqu'à VOIR SA FLOTTE**, avec des touches et des ratés visibles pour contrôler la superposition. Cette longue séquence est destinée exclusivement à la reproductibilité technique d'une défaite ; elle ne constitue pas un mode de jeu produit.

## Fiche de réception PO — VS Code Build → Flash → Monitor (1 CYD)

1. Identifier une **ESP32-2432S028R classique** unique, port UART, version/rev, câble et alimentation. Conserver `.vscode/settings.json` à trois paramètres habituel du PO **intact** ; choisir **ESP-IDF v5.5.6, cible esp32**, sans `Full Clean`, `erase-flash`, `nvs_flash_erase` ni reformatage. Sauvegarder la NVS si nécessaire, surtout pour la carte de test. Ne jamais mélanger l'artefact firmware de production et le BIN D13 réception.
2. Télécharger l'artefact **BIN/ELF/flash args** correspondant au **HEAD exact de la PR #73**. Vérifier localement **SHA-256 du BIN réellement flashé**, noter HEAD 40 caractères, mode OFF/ON, port, date. Dans VS Code, lancer **Build → Flash → Monitor** (ou utiliser les flash args/les quatre binaires d'un **même** artefact CI). Baud UART 115200.
3. D'abord sur le **firmware de production D13 (OFF)** : HOME/DEPLOY, AUTO, PRÊT, au moins **2 FEU/2 ACK**, bascule ATTAQUE/DÉFENSE et QUITTER/CONTINUER. Aucun bouton de révélation CPU avant un RESULT DÉFAITE ; aucune case CPU apparaissant sur ATTAQUE.
4. Ensuite, **avec accord explicite**, flasher le BIN **D13 réception isolée**. Relever `D13_SCENARIO=ARMED`, `D11_BENCH=ARMED` et le baseline `D11_COUNTERS` (l'événement VICTOIRE synthétique initial D11-BENCH reste identifiable). Suivre la séquence de 16 touches suivies de cases d'eau décrite ci-dessus, jusqu'au verdict **DÉFAITE**, par le vrai moteur. Relever `D10_PAIR_VISIBLE_ACK` terminal et `D11_EVENT` de la seule défaite jouée ; aucune fraude artificielle de résultat.
5. **Avant le premier opt-in :** prendre une photo de la grille ATTAQUE **MASQUÉE**, verdict compact, impacts visibles ; vérifier **absence des navires Capitaine et du bouton MONTRER MA FLOTTE**. Utiliser le petit bouton **VOIR SA FLOTTE** à `(270,220)`; prendre une photo **RÉVÉLÉE** avec **5 navires, 17 cases**, impacts conservés au-dessus.
6. Passer sur **DÉFENSE**, photo de la flotte humaine seulement, revenir **ATTAQUE** : la visibilité doit persister. Appuyer **MASQUER**, photo **REMASQUÉE** : 0 case de flotte CPU, **même historique d'impacts**, score D11 inchangé. **AFFICHER** doit restituer les 17 cases, sans nouveau crédit. Maintenir le stylet >200 ms et simuler un relâchement/repress court : **une seule action**.
7. Tester **REJOUER** lorsque les navires CPU sont affichés : retour DEPLOY, flotte humaine conservée, pas de flotte CPU ni drapeau visible ni nouveau score ; avant PRÊT, RETOUR→HOME sans forfait. Rejouer un second cycle et tester **ACCUEIL** depuis RESULT : pas de copie résiduelle ni d'écriture NVS. Sur une VICTOIRE/NUL éventuelle, vérifier inspection sans révélation (suite hôte D13 déterministe ; test sur appareil **NON TESTÉ** tant qu'absence de rapport).
8. Rebooter sans effacer NVS : les compteurs réels de production doivent être **identiques** avant/après passage par le firmware D13 de réception isolé ; le compteur bench de défaite doit rester enregistré mais ne remplace pas une preuve produit. Reflasher le firmware de production et vérifier `D13_SCENARIO=OFF`, `D11_BENCH=OFF`.
9. Archiver UART complet (`D13_INSPECTION status=APPLIED`, `COMBAT_FRAME d13_available/d13_visible`, `MEM phase=after_d13_toggle/after_d12_result/after_feu/after_reveal/after_nvs_event`, `D11_COUNTERS`, erreurs SPI/LCD/DMA), photos **MASQUÉ/RÉVÉLÉ/REMASQUÉ**, modèle carte, port, commandes et SHA. Mesurer **heap8 / largest8, minimum8, dma_free/dma_largest, stack_margin_bytes**, temps `DRAW ... us=` pour transitions : aucune fuite heap et **watermark main >= 1024 octets** après FEU et toutes consultations, zéro erreur SPI/LCD/DMA.

**Preuve matérielle : NON TESTÉE par le développeur.** `ON_DEVICE_PO_REPORTED_PASS` uniquement après retour reproductible du PO ; `ON_DEVICE_VERIFIED` réservé au contrôle Pilotage. Si un passage test échoue, remettre HEAD exact, logs et étape, sans conclure à un fonctionnement physique non vérifié.

**Hors D13 :** assets G07-R2/G08 de production, contraste final/stylet approuvé par PO, analyses enrichies de navires, second écran et mode radio, nouvelle persistance NVS, reprise de partie après coupure, une flotte CPU révélée au-delà de la défaite volontaire.
