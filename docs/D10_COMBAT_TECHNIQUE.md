# D10 — Combat technique tactile VS Capitaine (Issue #57)

**Mandat Pilotage :** #57, base `main` `78eed0d336f580932183f8fbd96887f9737bbe1e`. Sources métier : Bible v0.17 §14.1, §14.4–5 et §14.10.1–3. Implémentation TECHNIQUE **sans validation graphique produit**. Livraison en PR Draft #58, branche `feat/issue-57-d10-combat-technique`; Pilotage reste seul habilité à fusionner.

## Architecture et confidentialité

- `main/d09_probe.c` conserve **une seule** instance statique privée `WarshipLifecycle`, un seul propriétaire LCD SPI2 / tactile SPI3, calibration T02-B et bande DMA de **5120 B**. Les états HOME → DEPLOY / START / AUTO / RETOUR / PRÊT de D09 ne sont pas changés.
- À PRÊT, `warship_combat_init()` active la vue **ATTAQUE**. Le rendu appelle `warship_lifecycle_observe()` vers la projection statique `WarshipLifecyclePublic`, puis `warship_combat_make_frame()`. **La routine de pixel ne reçoit qu'un `WarshipCombatFrame`** : aucun `WarshipLifecycle`, `WarshipSoloSession`, `WarshipBattle` ou flotte CPU privée dans le rendu. `frame.ships[100]` reste **entièrement nul sur ATTAQUE**, et ne décrit que les navires humains sur DÉFENSE.
- `warship_combat_apply()` dispatch les commandes via D07 `select/cancel/fire`, sur une observation publique fournie par stockage statique (évite une copie publique volumineuse sur la pile de FEU). **Aucun moteur D01/D02/D05/D06/D07 réécrit**, aucun accès IA aux positions humaines privées.
- Les valeurs `WarshipBattleShot` / `WarshipBattlePair` sont les **seules** informations CPU exposées par l'UI. En `SUNK`, seul le type `ship_id` est écrit en texte près de la coordonnée terminale ; les anciennes cases touchées ne sont jamais réattribuées visuellement à une épave. L'occupancy de la flotte CPU n'est jamais lue.
- Composant C99 `components/warship_combat_ui/` indépendant de FreeRTOS/ESP-IDF ; `warship_combat_render.c` affiche une police technique 5×7 en Flash. Aucun heap, framebuffer, PNG, microSD ou PSRAM.

## Parcours et ordre des opérations

1. `PRÊT` : passage D07 COMBAT → ATTAQUE (10×10, eaux inconnues), sans nouveau boot ni nouvelle instance de jeu.
2. Gestes : hit-test `x=22..211` / `y=24..213`, cellule de **19×19** pixels. Un tap sur case inconnue prépare une cible, un tap sur la même l'annule, un tap ailleurs remplace la cible. Le bouton ANNULER est également disponible. FEU est inerte sans sélection. Une case déjà jouée ne modifie pas la sélection. Aller-retour DÉFENSE/ATTAQUE préserve la cible D07.
3. `FEU` : D07 `fire` appelle le choix public Capitaine (D05), verrouille et résout **les deux tirs d'une seule manche** (D06/D02). La paire retournée est retenue dans `WarshipCombatUi.pending_pair` (information de résultat autorisée), mais la vue issue de `observe` masque encore les deux impacts.
4. Pendant un délai technique **provisoire de 180 ms**, ATTAQUE reste affichée avec `TIR...` ; tous les appuis sont consommés par le dispatcher B01, **sans queue ni relecture**, y compris navigation ou zone d'abandon non implémentée. Le geste FEU lui-même ne peut provoquer une seconde action.
5. Une fois l'effet arrivé à échéance, `warship_combat_make_frame(include_pending=1)` compose **simultanément** : l'impact humain sur la grille ATTAQUE, le résumé de la riposte CPU, l'alerte de gravité max, les types coulés autorisés et, si nécessaire, un verdict technique. `draw_combat(1)` transmet ce cadre complet via bandes LCD. **Ce n'est qu'après le retour `ESP_OK`** que `warship_combat_presented()` appelle `warship_lifecycle_ack_reveal()`. Une erreur LCD arrête le chemin **sans ACK**.
6. Après ACK, nouvelle sélection possible immédiatement ; pas de `CONTINUER` ni bascule automatique. L'événement D07 terminal est journalisé **une seule fois**, sans écriture D08/NVS ; le champ de bataille reste inspectable ATTAQUE/DÉFENSE, mais plus aucun tir n'est accepté.

Les deux impacts sont révélés dans **un seul cadre logique** (pas deux transitions de jeu successives). La durée d'envoi physique du cadre est mesurable via `DRAW ... us=`, et le résultat réel à l'écran doit être validé sur CYD.

## DÉFENSE et alerte

Les `history.incoming[]` publics dessinent les impacts sur la flotte **humaine** de 17 cases. Le dernier élément seulement porte la coordonnée et `RATE/TOUCHE/COULE` (avec type si `SUNK`). En ATTAQUE, `last_cpu` et `has_summary` décrivent **la dernière riposte seulement**, remplacée au prochain FEU. L'alerte non consultée conserve `max(old, impact+1)` : `MISS=1 < HIT=2 < SUNK=3`. Même une dernière riposte `MISS` n'abaisse pas une ancienne alerte `SUNK`. Ouvrir volontairement DÉFENSE efface le résumé et l'alerte côté commande mais pas les historiques ; sa grille présente toujours la dernière riposte à consulter. Le retour ATTAQUE est explicite et ne restaure pas l'ancienne alerte.

## Géométrie strictement provisoire

Écran **320×240 paysage** ; grille **(22,24,190,190)** ; FEU **(225,32,91,38)** ; bascule DÉFENSE/ATTAQUE **(225,83,91,38)** ; ANNULER/verdict technique **(225,134,91,38)**. Bord intérieur de boutons : 2 px non interactifs. Aucune de ces coordonnées ne reprend ou valide les hitboxes G04/G05. La grille présente un tir par case et non une liste de cases d'épave. Le texte 5×7 et les couleurs restent des sondes techniques à mesurer au stylet sur le vrai TFT.

Le rendu reconstruit l'écran à partir du frame public et le transmet par **bandes de 8 lignes** sur l'unique buffer DMA existant, sans framebuffer complet. Le protocole de rendu est déterministe et borné ; les rafraîchissements sont bandés, mais **les rectangles d'optimisation supplémentaires ne sont pas encore implémentés**. Toute latence élevée ou scintillement nécessitera une correction après mesure réelle, pas une hypothèse de fluidité.

## Pile et métriques obligatoires

- Configuration inchangée : `CONFIG_ESP_MAIN_TASK_STACK_SIZE=4096` (garde de compilation D09 conservée), DMA 5120 B.
- Historique PO **D09 seulement** : `stack_margin_bytes=1520` après PRÊT sur le firmware D09 fusionné. **Ce chiffre ne qualifie PAS le chemin D10**. Le calcul et la décision Capitaine, les copies de D06 et la projection D07 peuvent diminuer la marge.
- D10 journalise `MEM phase=after_feu`, `after_reveal`, `after_rounds`, `running` : minimum historique `uxTaskGetStackHighWaterMark(NULL)` **en octets** ; `heap8`, `largest8`, `minimum8`, `dma_free`, `dma_largest` ; compteurs `bands`, `timeouts`, `lcd_fault`. Il est **interdit de valider** le critère de **1024 octets minimum** sans un vrai relevé CYD après FEU et plusieurs manches.
- Le binaire/empreinte, les tailles Flash/IRAM/DRAM et les logs de compilateurs résultent du job ESP-IDF v5.5.6, cible `esp32`, exécuté sur le **SHA exact de la PR**. Une CI verte ne prouve pas le fonctionnement physique.

## Tests hôte et CI

Suite `d10_combat_ui_technical` : gestes B01 (READY non réémis, tenue, rebond, invalidité, appui bloqué consommé, nouvelle pression), bornes de 10×10, annulation/remplacement/FEU sans cible, selection conservée en navigation, projection de la seule flotte humaine DÉFENSE, cache CPU vide ATTAQUE, paire non révélée avant composition, ACK unique, tirs répétés refusés, suite de manches CPU réelles jusqu'au verdict terminal, décision d'arrêt. Simulation de `HIT → SUNK → MISS` via `warship_combat_max_alert` et la projection du résumé ; ne se substitue pas à une observation matérielle.

Reproduction : `cmake -S tests -B build-gcc -G Ninja -DCMAKE_C_COMPILER=gcc -DWARSHIP_SANITIZERS=ON && cmake --build build-gcc && ctest --test-dir build-gcc --output-on-failure -V` ; idem `clang`. La CI comporte **2 jobs hôte + 1 job ESP-IDF** et sauvegarde le binaire et les fichiers de flash.

## Protocole PO — 1 CYD, exclusivement après autorisation Pilotage

**Identifier avant flash :** PR #58, HEAD 40 caractères, version ESP-IDF v5.5.6, cible `esp32`, carte ESP32-2432S028R (révision), source du binaire (CI ou local), `sha256sum build/cyd_warship.bin` **du fichier effectivement flashé**, flasher_args.json / ports / outil / sauvegarde éventuelle. **Ne pas effacer toute la Flash/NVS.** Flash via les offsets fournis par `flasher_args.json` et conserver journal UART 115200.

1. Démarrer : vérifier `BOOT_MODE=D09 direct boot`, `STACK_CONFIG ...=4096`, `LCD_PROFILE_PASS case=10` ; écran de déploiement direct 5 bateaux / 17 cases. Tester AUTO, RETOUR puis START, sélectionner PRÊT.
2. Après PRÊT : `D10_COMBAT_ENTER view=ATTAQUE`, grille inconnue, pas de bateaux CPU. Sur ATTAQUE, toucher une case, changer, annuler ; FEU sans cible inactif ; sélectionner une autre case puis FEU.
3. Pendant `TIR...` tapoter DÉFENSE / cellule / zone ANNULER ; vérifier qu'aucun événement différé n'est appliqué. **Après la révélation**, l'impact ATTAQUE et l'indication de riposte DÉFENSE apparaissent dans le même état. Contrôler `D10_PAIR_VISIBLE_ACK round=1` **une seule fois**.
4. DÉFENSE volontaire : vérifier le dernier tir adverse au bon endroit sur les navires humains et l'acquittement d'alerte ; revenir ATTAQUE, vérifier historique et impossibilité de tirer une case jouée. Préparer une **deuxième manche FEU** et répéter.
5. Recueillir **toutes** les lignes `MEM phase=after_feu`, `after_reveal`, `after_rounds`, `running`, `DRAW ... bands= ... us=`, `COUNTERS`, ainsi que toute erreur `TOUCH_SPI_ERROR`, `LCD_REFRESH_STOP`, `D10_PAIR_DRAW_FAIL_NO_ACK`, DMA timeout, SPI fault, reset ou anomalie d'affichage/tactile. Critère bloquant : **`stack_margin_bytes >= 1024`** après FEU et sur plusieurs manches, heap et plus gros bloc stables, aucun défaut SPI/LCD/DMA.
6. Fournir UART horodaté, SHA du BIN flashé, révision de carte, nombre de manches effectuées, photos courtes ATTAQUE/FEU/DÉFENSE et verdict `PASS/FAIL` motivé. Ne pas confondre `ON_DEVICE_PO_REPORTED_PASS` avec une preuve indépendante.

## Limites explicites

**Hors D10 :** actifs G04/G05/G07/G08 définitifs, effets de fin produit, révélation volontaire de la flotte CPU après défaite, REJOUER/ACCUEIL en production, demande d'abandon / forfait / NVS D08, PALMARÈS, radio ESP-NOW et deuxième CYD. La phase RESULT reste un arrêt technique inspectable ; le D07 terminal n'est **pas** persisté. Le seuil de pile, la lisibilité au stylet, les latences d'écran et l'identité du binaire réel restent **NON TESTÉS sur matériel** avant retour PO.
