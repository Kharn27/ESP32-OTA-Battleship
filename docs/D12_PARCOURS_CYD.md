# D12 — Parcours joueur technique sur ESP32 CYD : QUITTER, ABANDONNER, REJOUER, ACCUEIL

**Issue :** [#66](https://github.com/Kharn27/ESP32-OTA-Battleship/issues/66) · **Base validée :** `main` `0227b3c87440c1d590c0d1342d0e530ca2ac0bd1` (D11 PR #65 fusionnée) · **Branche :** `feat/issue-66-d12-parcours` · **Autorité de fusion :** Pilotage exclusivement.

**Sources :** Bible §14.1/14.4/14.6–14.8/14.10, [D07](D07_SOLO_LIFECYCLE.md), [D10](D10_COMBAT_TECHNIQUE.md), [D11](D11_EVENTS_NVS_CYD.md), [TEST_POLICY](TEST_POLICY.md). Les emplacements de boutons sont **une interface technique provisoire**, sans approbation des maquettes G04/G05/G07-R2, ni intégration G08/D13.

## Contrat d'intégration C99

- **Un seul propriétaire :** `main/d09_probe.c` conserve son unique `static WarshipLifecycle game` privé. Aucune copie de la flotte Capitaine vers les contrôles, rendus ou logs. La projection `WarshipLifecyclePublic` déjà utilisée par D10, enrichie seulement du **numéro de phase D07**, pilote le raster D12.
- `components/warship_journey/` : dispatcher **C99 sans ESP-IDF, TFT, NVS ni allocation** ; reçoit un hit D10 débouncé et appelle **uniquement les transitions D07 existantes**. Un résultat `NOT_HANDLED` laisse D10 gérer CELL, FEU et ATTAQUE/DÉFENSE, sans nouveau moteur. Aucun événement n'est construit par l'UI.
- `warship_journey_input_step` réutilise **la seule** instance de `WarshipCombatInput` et ses seuils D10 : **40 ms DOWN, 60 ms UP, 120 ms inter-action**. Les coordonnées du contrôle reconnu proviennent de la même grille de hitboxes en toutes vues et phases ; les appels sont ensuite filtrés **par l'état D07**. Cette technique permet QUITTER même depuis DÉFENSE, tout en empêchant qu'un tap sur grille DÉFENSE ne sélectionne une cible. Avant rendu de la nouvelle phase, `warship_combat_input_discard` annule le geste courant ; pour revenir HOME/DEPLOY, `warship_deploy_input_discard` exige le relâchement frais du stylet.
- `COMBAT` : le bouton inférieur n'est **plus ANNULER**, mais **QUITTER** (le joueur peut toujours annuler sa cible en retouchant **la même case** via D10). D07 `request_abandon` ouvre la confirmation uniquement en SELECTING, et **jamais** en AWAIT_REVEAL. La flotte, l'historique, la sélection et la vue ATTAQUE/DÉFENSE ne changent pas.
- `CONFIRM_ABANDON` : CONTINUER appelle `decide_abandon(0)`, reprend la session **bit-à-bit inchangée** et n'envoie **aucun** événement D11. ABANDONNER appelle `decide_abandon(1)` exactement **une fois**, puis transmet **l'unique valeur D07 VOLUNTARY_FORFEIT** à `d11_runtime_event`. D11 garde intégralement la responsabilité des ID monotones, du snapshot 44 octets et de la réconciliation NVS. Aucune **partie** VS Capitaine n'est créditée pour un forfait.
- **Statut NVS sans faux succès** : `d11_runtime_event` restitue désormais le vrai `WarshipEventStatus` à D12. `APPLIED`/`ALREADY_APPLIED` sont confirmés ; `PENDING`/`WRITE_UNCERTAIN`/`RELOAD_REQUIRED` et autres erreurs retournent `WARSHIP_JOURNEY_SCORE_UNCONFIRMED`, avec `D12_FORFEIT_NOT_CONFIRMED_NVS` et le code D11 dans les logs. Le contrôleur a déjà choisi HOME dans D07 ; aucune tentative de rollback du forfait n'est inventée. Le **retry D11 borné toutes les 10 s** reste intact, sans nouveau ledger ni écriture cachée.
- **FEU en cours** : si `ui->pending`, tout geste (grille/onglet/QUITTER) est consommé sans effet et sans mise en attente. La paire complète est d'abord dessinée au LCD (`ESP_OK`), puis D10 `presented` ACK D07, et **ensuite seulement** D11 reçoit un événement terminal V/D/N. Le verdict entraîne le rendu des commandes de résultat.
- **RESULT** : REJOUER appelle `warship_lifecycle_replay` (même flotte humaine, CPU régénéré indépendamment, coïncidence légitime, historiques vierges, retour DEPLOY et PRÊT obligatoire). L'échec RNG préserve le résultat/les boutons sans nouveau score. ACCUEIL appelle `leave_result` et efface la session sans forfait, ni nouvelle écriture. L'inspection ATTAQUE/DÉFENSE D10 reste possible entre-temps ; **G08 n'est pas intégrée**.
- **DEPLOY, y compris revanche** : RETOUR via D07 `leave_deploy` => HOME, 0 forfait/0 partie ; AUTO/PRÊT restent D09. L'accueil technique D09 contient le bouton START existant, sans menu produit nouveau.

## Hitboxes techniques pour une CYD 320 × 240, paysage

**Gauche** : grille 10 × 10 inchangée, 19 px/case à partir de `(22,24)`, ATTAQUE/DÉFENSE une seule grande grille. **Droite** : trois rectangles fixes `x=225…315`, de 91×38 px, avec une bordure de 2 px non cliquable. Les centres faciles à atteindre au stylet sont `(270,50)`, `(270,100)`, `(270,150)`.

| Phase | Haut `y=32…69` | Milieu `y=83…120` | Bas `y=134…171` |
|---|---|---|---|
| COMBAT, ATTAQUE | FEU (si cible valide) | DÉFENSE | QUITTER |
| COMBAT, DÉFENSE | FEU inactif | ATTAQUE | QUITTER |
| FEU en cours | TIR… (bloqué) | Bloqué | QUITTER visuellement grisé, **bloqué** |
| CONFIRM_ABANDON | CONTINUER | BLOQUÉ | ABANDONNER |
| RESULT | REJOUER | ATTAQUE / DÉFENSE | ACCUEIL |
| DEPLOY / HOME | D09 : AUTO / PRÊT / RETOUR | D09 : START | selon D09 |

Le texte est volontairement sans accent sur l'écran 5×7. Les libellés/hitboxes définitifs, palettes, sprites et comportement artistique restent à décider hors D12. Aucun framebuffer plein écran, pas d'assets ni PSRAM ; raster D10 conservé avec seulement la phase publique ajoutée.

## Tests hôte / CI et limites

```sh
cmake -S tests -B build-host -G Ninja -DCMAKE_C_COMPILER=gcc -DWARSHIP_SANITIZERS=ON
cmake --build build-host
ctest --test-dir build-host --output-on-failure -V

cmake -S tests -B build-clang -G Ninja -DCMAKE_C_COMPILER=clang -DWARSHIP_SANITIZERS=ON
cmake --build build-clang
ctest --test-dir build-clang --output-on-failure -V
```

Nouvelle suite `d12_technical_journey` : **vrai D07 + D10 + D11 avec backends de stockage injectés** ; RETOUR avant PRÊT, confirmation CONTINUER bit-à-bit sans écriture, QUITTER depuis DÉFENSE, forfait confirmé une seule fois, refus de commande pendant FEU y compris maintien à travers ACK, visualisation de paire sur grille publique sans navire CPU, vrais verdicts D07, résultat/REJOUER, erreur RNG sans mutation, AUTO après revanche, RETOUR sans nouvelle partie, ACCUEIL, écriture NVS ratée avant/après commit et réconciliation par D11. Tests historiques des 16 suites D01–D11 inchangés.

**Attention** : `HOST_TESTED`, `TARGET_BUILD_VERIFIED`, `CI_PASSED` et `ON_DEVICE_VERIFIED` sont des preuves distinctes attachées au HEAD exact de la PR. Les valeurs de heap, fragmentation, pile et latence tactile réellement observées sur CYD ne peuvent **pas** être dérivées d'un build PC/ESP-IDF. La pile main reste **4096 B**, le DMA **5120 B**, sans budget silencieusement augmenté. Gate matériel : **`stack_margin_bytes >= 1024`** pendant **FEU et parcours D12**, aucune faute SPI/LCD/DMA.

## Fiche PO — VS Code : Build → Flash → Monitor (1 CYD)

**Préconditions** : une ESP32-2432S028R classique désignée par Pilotage, alimentation/câble USB et COM/TTY connu, VS Code + extension Espressif ESP-IDF **v5.5.6** cible **esp32**. Conserver les éventuels réglages utilisateur de `.vscode/settings.json` (notamment le fichier à trois paramètres du PO) : **ne pas le modifier**. Flasher un BIN **issu du HEAD D12 de la PR** et noter son **SHA-256 après extraction**, pas simplement le SHA Git du code. Firmware de **production** D11 (pas le diagnostic `warship_d11lab`).

1. **Sauvegarder si nécessaire la NVS de la carte choisie** et obtenir l'accord sur le banc. Ne pas lancer `erase-flash`, `Full Clean`, `nvs_flash_erase` ni reformatage. Vérifier la configuration cible **4096 B** pour main et partition inchangée, puis ouvrir le dépôt dans VS Code.
2. Dans l'extension ESP-IDF : sélectionner la cible **esp32**, le port de la CYD ; lancer **Build**, puis **Flash**, puis **Monitor**. Les commandes CLI équivalentes pour une compilation propre sans effacement sont `idf.py set-target esp32`, `idf.py build`, `idf.py -p /dev/ttyUSB0 flash monitor` (adapter le port ; ne pas employer `erase-flash`). Si le firmware CI fourni est flashé, utiliser ses `flasher_args.json`/binaires du **même** artefact, jamais mélanger avec un build local. Relever version du boot, SHA Git, SHA-256 du BIN **réellement flashé**, carte/port et heure.
3. Au boot, relever les lignes `D11_NVS_INIT` et `D11_COUNTERS` des **cinq compteurs**. Depuis DEPLOY, essayer **AUTO → RETOUR** ; HOME apparaît avec START. Recommencer START → DEPLOY → RETOUR ; vérifier que `games`/`voluntary_forfeits` n'ont pas changé. Tester START → AUTO → PRÊT ; entrée COMBAT.
4. Cibler une case de la grille, ouvrir QUITTER via le bouton à **(270,150)**, vérifier l'écran de confirmation et l'absence de NVS write. Appuyer CONTINUER à **(270,50)** : la grille/visée préparée et la vue ATTAQUE/DÉFENSE doivent être conservées, aucun forfait. Refaire QUITTER depuis DÉFENSE après bascule volontaire ; vérifier que le même chemin est disponible.
5. Refaire QUITTER puis **ABANDONNER** à **(270,150)** : retour HOME sans partie VS Capitaine ; chercher `D12_NAV`, `D11_EVENT stage=post_d07_ack status=APPLIED` et `D11_COUNTERS voluntary_forfeits=ancien+1` (si NVS réussit). Aucun double crédit au maintien/double appui. **Si l'état est `WRITE_UNCERTAIN`/`PENDING`**, collecter `D12_FORFEIT_NOT_CONFIRMED_NVS` et les `D11_EVENT`; ne pas annoncer de sauvegarde, laisser le retry D11 faire sa seule tentative bornée ou signaler l'erreur à Pilotage.
6. Redémarrer la CYD **sans effacer NVS** et relire les cinq compteurs : même valeur de forfait, `games` inchangé. Jouer ensuite **au moins deux FEU + deux ACK** ; pendant les 180 ms de blocage appuyer/maintenir QUITTER, ATTAQUE/DÉFENSE et grille ; aucun effet avant/après ACK ni confirmation différée. Vérifier les impacts du joueur et de Capitaine révélés ensemble, et l'historique.
7. Poursuivre jusqu'à un verdict V/D/N : une seule partie et une seule catégorie augmentée dans D11. Sur RESULT, inspecter librement ATTAQUE/DÉFENSE ; appuyer **REJOUER (270,50)**. Vérifier grille humaine précédente, CPU indépendante et cachée, historique neuf, AUTO possible, PRÊT nécessaire. Depuis ce DEPLOY de revanche, **RETOUR** => HOME, aucun nouveau forfait ni deuxième partie. Lors d'une autre fin de partie, **ACCUEIL (270,150)** => HOME, sans nouveau score.
8. Conserver les logs UART complets des phases `MEM phase=after_action`, `after_feu`, `after_reveal`, `after_rounds`, `after_nvs_event`, **`after_d12_nav`**, **`after_d12_result`** ; `heap8`, `largest8`, `minimum8`, `dma_free`, `dma_largest`, `stack_margin_bytes`, `lcd_fault`, `timeouts`, ainsi que `D12_NAV` et `D11_EVENT`. Seuil **pile main >=1024 B sur toute la séance**, heap/largest stables sans fuite, **0** SPI/LCD/DMA erreur. Photos simples des trois écrans de commande : confirmation, résultat, revanche.
9. Rendre au Pilotage un tableau étape/PASS-FAIL, l'identité du firmware BIN/SHA, les logs UART et réserves. **Aucune preuve matérielle D12 ne peut être annoncée sans ce retour**. La dette de validation NVS D11 / D08 #36 reste indépendante de la validation de la navigation.

**Non couverts D12 :** PALMARÈS produit, ressources G04/G05/G07-R2, G08/D13 inspection post-défaite, retour de session après extinction, ESP-NOW, deuxième CYD. Aucune nouvelle règle de gameplay.
