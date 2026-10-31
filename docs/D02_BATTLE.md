# D02 — Moteur de combat C99 / Issue #25

Référentiel métier exclusif : `BIBLE_PRODUIT.md` v0.17, §6, LOT1-001,
LOT1-CBT-001, LOT1-FIN-001, §14.10.1. Les scénarios §14.14 restent
`PROPOSÉS` ; les tests portent sur les décisions déjà acquises.

## API et propriétaire mémoire

Voir `components/warship_battle/include/warship_battle.h`.
L'application possède une unique `WarshipBattle` (état secret fixe,
aucun `malloc`). `warship_battle_init` valide les deux `WarshipFleet` de D01
et les **copie** indépendamment ; toute entrée invalide préserve le
contenu antérieur de la destination. D02 ne génère pas de flotte.

Le camp HUMAN tire sur la flotte CAPTAIN et réciproquement. Coordonnées
`x,y` dans `[0,9]`, origine en haut à gauche. Chaque camp gère sa cible
avec `warship_battle_select` (remplace), `warship_battle_cancel` (annule),
`warship_battle_fire` (FEU irréversible). Une case déjà attaquée ou une
sélection identique est refusée sans mutation. FEU sans sélection est refusé.
Un second FEU est une erreur, jamais une seconde attaque.

Phases :
- `PREPARING` : chaque camp peut choisir puis verrouiller sa propre cible.
  Le premier FEU n'applique **aucun** dégât et ne publie **aucun** résultat.
- `READY` : les deux FEU sont reçus, aucune cible n'est modifiable.
- `warship_battle_resolve` : transition explicite. Elle calcule les deux
  impacts dans un état temporaire, publie une **paire atomique**
  `WarshipBattlePair` et détermine le verdict **après** les deux attaques.
  Impossible de résoudre deux fois la même manche.
- `AWAIT_REVEAL` : en cas de poursuite, verrouillage logique de toute
  nouvelle sélection. `warship_battle_finish_reveal` est la notification
  technique **après** révélation synchronisée par la future UI ; elle ouvre
  immédiatement la manche suivante sans timer ni bouton CONTINUER.
- `FINISHED` : verdict définitif et actions de jeu interdites.
  `warship_battle_take_final` fournit une **seule fois** l'événement terminal
  consommable par le futur palmarès ; les consultations publiques ultérieures
  restent possibles sans nouveau verdict.

Les structures résultats ont une coordonnée d'impact, `MISS/HIT/SUNK` et
un identifiant de navire **uniquement lorsque SUNK**. Pour MISS/HIT,
`ship_id=WARSHIP_BATTLE_NO_SHIP` ; aucune donnée ne relie publiquement
les touches d'un même navire ni ne révèle les cases de son épave. Les
contacts par côté/diagonale restent valides ; aucun halo d'eau appliqué.

`warship_battle_observe` retourne une **copie** de `WarshipBattlePublic`
limitée aux tirs sortants et entrants **déjà résolus**, leur impact,
les types coulés annoncés et le verdict. Aucun pointeur interne, aucune
flotte, grille cachée, état de verrouillage adverse ou cible en attente.
Pour la future IA Capitaine, transmettre **uniquement** cette observation
(avec règles connues), **jamais** la structure `WarshipBattle` ni sa flotte
humaine. Le besoin d'afficher la propre flotte sur DÉFENSE sera traité via
un contrat futur séparé et autorisé, sans exposer l'adversaire.

Les buffers d'entrée/sortie doivent être distincts et valides. Les sorties
restent inchangées sur erreur, comme l'état du combat. Le moteur ne
gère pas la concurrence : la session appellera l'API en séquence, non
à partir de callbacks simultanés. Aucune allocation, NVS, timer, IA,
FreeRTOS, ESP-NOW, interface graphique ou tactile.

## Tests et vérification

Les cas de `tests/test_warship_battle.c` couvrent :
- copies D01 et rejet de l'une ou l'autre flotte invalide/NULL ;
- sélections successives, annulation, doubles sélections, coordonnées
  interdites, FEU sans cible, verrouillage du seul camp et FEU répété ;
- résultats absents avant le second FEU, ordre des FEU inversé avec
  même état et même paire finale, erreurs strictement atomiques ;
- touches, ratés, coulés par type pour les cinq navires identifiés,
  maintien des deux navires de longueur 3 et des navires en contact ;
- 17 manches chacune pour VICTOIRE, DÉFAITE et MATCH NUL réciproque,
  aucune priorité au premier tireur, terminal consommé une seule fois ;
- observations CPU comparées entre deux positions humaines différentes
  avec historique public identique ; cible secrète non publiée ;
- 100 manches avec 83 cases d'eau puis les 17 dernières occupées pour
  éprouver les bornes des deux historiques.

Commandes CI sur le HEAD exact (hôte Ubuntu 24.04) :

```sh
cmake -S tests -B build-host -G Ninja -DCMAKE_C_COMPILER=gcc -DWARSHIP_SANITIZERS=ON
cmake --build build-host --verbose
ctest --test-dir build-host --output-on-failure -V
# Répéter avec CMAKE_C_COMPILER=clang dans un autre build-dir.
```

`-std=c99 -Wall -Wextra -Werror -pedantic`, ASan/UBSan et les
régressions T01/T02/D01 incluses. Build cible ESP-IDF v5.5.6/Xtensa :
`idf.py set-target esp32 && idf.py build && idf.py size && idf.py size-components`.

Attention : la sonde T02 reste le firmware existant. Elle lie seulement
`warship_battle_phase(NULL)` pour assurer une présence minime du composant
au lien, **sans commencer une partie**. Un `size-components` affichant une
faible contribution **ne représente pas le coût complet** de la résolution,
qui est compilée mais éliminée par le linker jusqu'à son appel runtime.
Les mesures sur HEAD, la CI et le SHA-256 du binaire sont remis dans la PR.

Matériel CYD : `NON_APPLICABLE` pour D02, aucun test physique demandé.
