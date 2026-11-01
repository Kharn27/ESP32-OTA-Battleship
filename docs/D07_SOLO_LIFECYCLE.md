# D07 — Cycle de partie C99 : AUTO, PRÊT, abandon, REJOUER (Issue #35)

**Source métier :** Bible produit v0.17 §14.1, §14.2, §14.6–8, §14.10.1 et §14.13, LOT1-PL-001, LOT1-FLT-001, LOT1-ACC-010, LOT1-REV-001/002/003, LOT1-ABD-001, LOT1-FIN-002/003/005. **Contrat de mission :** Issue #35 et vigilance Pilotage. **Base vérifiée :** `main` `f13be83d8f40a9b4703350ec103e7ac1720faef1` (D06 #34 fusionnée).

## Portée et séparation des responsabilités

`components/warship_lifecycle/` est un contrôleur C99 **headless** au-dessus de **D01** (deux générateurs de flottes indépendants) et **D06** (session de combat, IA publique D05 et résolution simultanée D02). Il ne duplique aucune règle de combat. Ni écran, ni RTOS/FreeRTOS, ni tactile, ni animation/timer, ni persistance NVS D08, ni ESP-NOW.

La couche applicative possède **un seul** `WarshipLifecycle` (contenant `WarshipSoloSession` ainsi que les deux flottes pré-PRÊT). Cette structure est **privée** : ne jamais la passer à l'UI, à Capitaine ni à un système de logs public. Il y a deux copies privées des flottes pendant le combat (positions de déploiement et propriété de D06) : ce choix borne les accès et permet de rejouer avec la formation humaine **sans lire directement les champs secrets de D06**. La mémoire supplémentaire et la pile associée sont à mesurer lors de l'intégration.

Le contrat de présentation n'utilise que :
- `warship_lifecycle_observe(game, &WarshipLifecyclePublic)` pour lire la **flotte humaine** (dès DEPLOY, toujours en COMBAT, pendant CONFIRM_ABANDON et RESULT), la phase, la vue publique **HUMAN** de D06 et les drapeaux de révélation. Le modèle UI ne contient **jamais** la flotte CPU. Les indicateurs `has_human_fleet`/`has_battle` explicitent les sections disponibles ; avant PRÊT, aucun historique de combat.
- `warship_lifecycle_fire(game, rng, ctx, &WarshipBattlePair)` pour remettre **une paire résolue** au contrôleur de présentation, qui affiche les deux impacts ensemble. `warship_lifecycle_ack_reveal(game, &event)` n'intervient **qu'après** la révélation visuelle synchrone. D06 masque toujours la dernière paire et son verdict dans les vues ordinaires avant acquittement.
- `warship_lifecycle_copy_revealed_captain(game, &fleet)` **uniquement** après DÉFAITE, consultation volontaire et état `visible=1`. Cette fonction projette, sous contrôle D07, sa **copie privée préexistante** de la flotte CPU ; elle ne lit ni ne modifie les champs internes D06. Le modèle UI ordinaire et les décisions de l'IA ne disposent d'aucun tel accès. Les cinq navires complets servent exclusivement à la superposition visuelle post-défaite ; les impacts historiques restent distincts.

**Limite fondamentale d'une copie fournie au rendu :** D07 peut refuser toute **nouvelle** lecture quand la flotte est masquée, mais ne peut pas effacer une copie préalablement reçue par l'appelant. **Obligation de l'intégrateur UI :** effacer/invalider la copie de présentation quand MASQUER, REJOUER ou ACCUEIL est demandé ; ne jamais connecter le pointeur vers une copie de la flotte à l'IA.

## Machine à états

```text
                         init
                          |
                          v
                  HOME / ACCUEIL
                          |
             start(RNG humain, RNG CPU) --échec: HOME intact
                          |
                          v
                    DEPLOY / AUTO <------------------+
                   (5 bateaux déjà visibles)         |
                      |  |    |                      |
           AUTO répété |  |    | QUITTER             |
           (D01 humain)|  |    +----------> HOME     |
                      |  |                           |
                      +--+                           |
                         | PRÊT                      |
                         v                           |
                 COMBAT (D06 SELECTING)              |
                      |     |                        |
                   FEU|     |demander abandon        |
                      |     v                        |
                      |  CONFIRM_ABANDON             |
                      |     |           |            |
                      | CONTINUER   ABANDONNER       |
                      |     |           +--> HOME + FORFAIT_VOLONTAIRE
                      |     v                        |
                      |   COMBAT                      |
                      v                              |
             COMBAT (D06 AWAIT_REVEAL)                |
             aucun tir, abandon refusé                |
                      | ACK après révélation          |
             +--------+-------------+                |
             | CONTINUE             | final           |
             v                      v                 |
           COMBAT                 RESULT              |
                          inspection sans score       |
                          éventuelle révélation       |
                              |      |                |
                  ACCUEIL ----+      +---- REJOUER --+
                                         RNG CPU
```

**PRÊT** est le **seul** engagement : `warship_lifecycle_ready()` appelle `warship_solo_init()` avec les deux flottes D01 précédemment préparées. AUTO ne déclenche aucun combat ; avant PRÊT `leave_deploy()` revient HOME **sans événement**. Après PRÊT, aucun `leave_deploy()` autorisé : pour renoncer, le joueur demande/valide explicitement l'abandon. Pendant FEU (D06 `AWAIT_REVEAL`), `request_abandon()` et toute sélection sont rejetés **sans mise en attente** ; la future UI ignore également la navigation et les boutons pendant l'animation.

`warship_lifecycle_request_abandon()` ouvre une confirmation uniquement en COMBAT et D06 SELECTING. Tant qu'elle est ouverte, les commandes de jeu sont bloquées ; `decide_abandon(0)` reprend la session **inchangée** et émet `NONE`. Seul `decide_abandon(1)` retourne `HOME` et émet **une** occurrence `VOLUNTARY_FORFEIT` ; elle efface les flottes/combats privés, sans fabriquer un verdict D06 et sans résultat VS CAPITAINE. Demande répétée, erreur et incident technique n'émettent rien.

Après chaque manche, `ack_reveal()` s'appuie sur D06 : elle émet `NONE` en continuant ou **une seule** valeur `VICTORY`, `DEFEAT` ou `DRAW` en consommant D06 `take_final()` une fois. La phase RESULT interdit tout tir et autorise inspection, `leave_result()` sans forfait et `replay()` après résultat. `replay()` conserve l'ancienne **flotte humaine**, génère à nouveau **la flotte CPU** avec D01 sur RNG indépendant et retourne DEPLOY, sans PRÊT implicite. Les historiques et dommages D06 sont effacés, ainsi que l'autorisation/visibilité de révélation. Une nouvelle flotte peut coïncider par hasard avec la précédente ; ce n'est **pas un échec**.

`set_captain_visible(1)` est recevable **exclusivement en RESULT DÉFAITE** ; le premier appel représente la demande `VOIR SA FLOTTE`. `set_captain_visible(0)` masque ensuite, puis `1` réaffiche à volonté. `reveal_requested` persiste pendant l'inspection (pas de notion de changement de vue stockée ici), `reveal_visible` reste indépendant. VICTOIRE et NUL ne donnent pas cette interface ; les tirs et verdicts historiques ne sont jamais altérés par la consultation.

## Événements D08 — livraison synchrone, sans NVS

Les sorties `WarshipLifecycleEvent` sont **des événements**, pas des totaux ou des écritures mémoire persistantes :

| Transition | Valeur transmise exactement une fois |
|---|---|
| AUTO, PRÊT, QUITTER avant PRÊT, CONTINUER, REJOUER, ACCUEIL après résultat | aucune |
| `ack_reveal` non terminal | `NONE` |
| `ack_reveal` d'une dernière paire, après D06 `take_final` | `VICTORY` / `DEFEAT` / `DRAW` |
| `decide_abandon(0)` | `NONE` |
| `decide_abandon(1)` confirmé après PRÊT | `VOLUNTARY_FORFEIT` |

Les événements résultats et forfait sont **mutuellement exclusifs pour chaque engagement** : le premier conduit à RESULT, le second retourne HOME. Aucune deuxième commande valide ne peut en émettre un autre pour le même combat. Le futur D08 consomme l'événement **au retour réussi** de la transition, incrémente une fois la catégorie adéquate et gère lui-même la persistance. Aucun canal ou queue d'événements implicite, aucune comptabilisation au simple affichage ou à la consultation. **Conséquence documentée :** l'appelant doit remettre l'événement à D08 immédiatement et ne pas l'ignorer ; les coupures et la persistance transactionnelle restent du ressort de D08. Ne pas présenter une erreur/coupure comme forfait.

## Erreurs, aléa et ressources

Les RNG sont injectés avec **contextes distincts** pour génération humaine, génération CPU et tir Capitaine ; D01 consomme cinq tirages par flotte, même si disposition identique. Le code D07 n'impose ni qualité de hasard, ni distribution uniforme, ni exclusion de répétition. En échec d'une génération (y compris au second appel lors de start), **aucune flotte ni phase** n'est remplacée : le candidat est préparé dans une variable locale, puis commité seulement après succès. En échec RNG pour FEU, D06 garde session/pair intacts et permet retentative. Toute erreur d'API laisse l'objet de contrôle et les sorties appelant inchangés, sous réserve de RNG non réentrant et de pointeurs valides non aliasés. L'état interne d'un RNG externe, lui, peut déjà avoir avancé.

Le composant ne fait aucune allocation dynamique, utilise des tableaux bornés définis par D01/D02 et des copies locales C99. Les mesures `sizeof(WarshipLifecycle)` / `sizeof(WarshipLifecyclePublic)` des tests hôte sont **des tailles de structures sur hôte** ; copies locales de `WarshipSoloSession`, `WarshipBattlePublic` dans D06, tableaux internes D05 et piles d'appels en profondeur s'ajoutent. Le `size-components` de l'application T02 ne lie qu'un appel nul ; **il n'est pas le coût mémoire du jeu complet**. Les **watermarks de pile et heap réellement sur Xtensa** doivent être acquis à l'intégration UI (séparément du contrôle statique des budgets). Le contrôleur D07 a le statut matériel **ON_DEVICE: NON_APPLICABLE**.

## Vérifications et reproduction

Suite `warship_lifecycle_session` : HOME, erreurs d'API, D01 génération humaine+CPU, AUTO répété/same-placement/échec RNG, sortie pré-PRÊT, PRÊT sans démarrage anticipé, D06 sélection/FEU/paire, absence de fuite avant ACK, abandon refusé pendant animation, confirmation invalidée/CONTINUER intact/ABANDONNER unique, scénario headless **VICTOIRE, DÉFAITE et NUL simultané** à travers D02+D05 réels, terminal unique, révélation sur demande seulement après défaite, masquer/réafficher, REJOUER humain préservé et CPU regeneré, historique vierge, RNG erreur au replay. Toutes les suites T01–D06 doivent rester intactes.

```sh
cmake -S tests -B build-gcc -G Ninja -DCMAKE_C_COMPILER=gcc -DWARSHIP_SANITIZERS=ON
cmake --build build-gcc
ctest --test-dir build-gcc --output-on-failure -V
cmake -S tests -B build-clang -G Ninja -DCMAKE_C_COMPILER=clang -DWARSHIP_SANITIZERS=ON
cmake --build build-clang
ctest --test-dir build-clang --output-on-failure -V

# Job GitHub Actions épinglé à ESP-IDF v5.5.6, cible esp32 Xtensa :
idf.py set-target esp32
idf.py build
idf.py size
idf.py size-components
sha256sum build/cyd_warship.bin
```

La CI doit vérifier **HEAD exact**. Taille liée Flash/IRAM/DRAM et SHA-256 du firmware appartiennent à la PR #40 finale. `main/app_main.c` n'appelle qu'une **sonde de liaison nulle** D07, sans initialiser de partie sur le matériel : build cible ≠ essai sur carte réelle. Tests physiques et UX seront définis avec l'intégration, hors D07. **D08 reste en attente.**
