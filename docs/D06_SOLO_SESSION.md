# D06 — Session solo C99 : FEU, Capitaine, résolution et révélation (Issue #34)

**Contrat :** Issue #34 ; Bible produit v0.17 §14.1, §14.3, §14.4, §14.10.1 et §16. Base `main` `ec473010f28575650feb7c49f8c68893c9e980de`, D01 à D05 déjà fusionnées. Cette note documente le contrat **technique**, sans créer de nouvelle règle produit.

## Responsabilités et frontière du secret

Le module `components/warship_solo/` orchestre une seule partie contre Capitaine. Il **réutilise** sans duplication les validations de flottes D01, les sélections, tirs et verdicts D02, et le décideur C99 D05.

- `WarshipSoloSession` est une **structure de stockage propriétaire** (pas d'allocation dynamique) destinée au contrôleur applicatif ; elle contient `WarshipBattle` et donc **les deux flottes secrètes**. Elle ne doit jamais être communiquée directement au rendu, à l'écran ou à l'IA.
- `warship_solo_observe(session, &public)` est la **frontière UI** : `WarshipSoloPublic` ne contient que l'historique public **vue HUMAN**, la phase et une sélection humaine éventuelle. Pas de flotte ni de carte d'occupation adverse, ni de cible CPU secrète en attente.
- Pour **choisir sa cible**, le moteur observe `warship_battle_observe(&staged, WARSHIP_BATTLE_CAPTAIN, &public_cpu)` AVANT tout verrouillage/tir de la manche et transmet **uniquement** cet objet à `warship_captain_choose(&public_cpu, rng, ctx, &target)`. Ni `WarshipBattle`, ni `WarshipFleet`, ni cible humaine préparée ne franchissent cet appel. L'aléa est injecté par le client via `WarshipCaptainRandom`.
- Seul un `SUNK` informe du **type** du navire coulé (champ `ship_id`) ; il ne révèle jamais l'appartenance des autres `HIT` à son épave. La paire `WarshipBattlePair` contient strictement les deux tirs **déjà résolus**, leurs impacts, types `SUNK`, numéro de manche et verdict.

**Séparation des rôles :** la couche applicative possède l'objet `WarshipSoloSession` et gère le passage de `WarshipSoloPublic` / `WarshipBattlePair` au rendu. D06 n'implémente ni entrée tactile, ni emplacement de bouton, ni temporisation, ni navigation ATTAQUE/DÉFENSE, ni module NVS.

## Machine à états : une résolution, deux temps

```text
       init(flottes valides)
                  |
                  v
        SELECTING (avant FEU) <-------------+
        - sélectionner/remplacer              |
        - annuler, FEU nécessite cible         |
                  | FEU: CPU choisit            |
                  | 2 locks D02, resolve D02    |
                  | commit atomique de la paire |
                  v                             |
       AWAIT_REVEAL (toujours,                 |
       même si verdict définitif)              |
       - aucun nouveau tir/sélection           |
       - le rendu révèle la paire ensemble     |
                  | ack_reveal                  |
                  +---------- si CONTINUE -----+
                  |
                  +---------- si verdict final
                                         |
                                         v
                                     FINISHED
                                - aucun autre tir
                                - take_final une fois
```

1. **Avant FEU** : `warship_solo_select()` modifie/remplace la cible, `warship_solo_cancel()` l'annule. Les erreurs D02 (hors bornes, tir précédent, cible identique, absence de sélection) sont projetées dans les statuts D06, sans écriture en erreur.
2. **FEU** : `warship_solo_fire(session, rng, ctx, &pair)` valide les paramètres et la sélection, effectue **toute la manche sur une copie locale** de `WarshipBattle`, observe la vue CPU publique, demande exactement un tirage RNG via Capitaine, sélectionne le CPU, verrouille les deux cibles, appelle **une seule fois** `warship_battle_resolve()`. L'événement `pair` et la session sont remplacés seulement après succès intégral. Un second FEU en `AWAIT_REVEAL` retourne `ALREADY_FIRED`.
3. **Résolution ≠ révélation** : `warship_solo_fire` renvoie la paire déjà résolue au **contrôleur de présentation**, pour qu'il montre les deux effets au même instant pertinent. L'UI publique `warship_solo_observe` **masque la dernière paire tant que l'acquittement n'a pas eu lieu** : les deux compteurs historiques sont décrémentés pour la projection seulement, les entrées correspondantes mises à zéro, le verdict projeté à `CONTINUE`. Cela empêche une lecture publique anticipée du résultat via l'observation, mais le contrôleur doit évidemment conserver `pair` pour l'affichage.
4. **Acquittement** : `warship_solo_ack_reveal()` est un événement **explicite** que le contrôleur appelle **après** affichage simultané des résultats. Si D02 était en `AWAIT_REVEAL`, la session appelle `warship_battle_finish_reveal()`, puis permet immédiatement de sélectionner la manche suivante. Si D02 est `FINISHED`, la session passe `FINISHED` **sans appeler** `finish_reveal` (D02 interdit cet appel terminal). Sans acquittement, pas de nouvelle sélection.
5. **Événement terminal** : `warship_solo_take_final()` délègue le mécanisme D02 `warship_battle_take_final()` uniquement depuis `FINISHED`. Avant acquittement, `NO_EVENT`; après le premier succès, les suivants `NO_EVENT`, sans altérer le résultat. Les deux tirs de la dernière manche ont déjà été résolus pour VICTOIRE, DÉFAITE ou NUL. Aucun compteur persistant ni palmarès dans D06.

## Politique d'erreurs et retentative

Toutes les sorties utilisateur restent **inchangées** en erreur, ainsi que la session lors des erreurs de `select/cancel/fire/init` (hypothèse contractuelle : pointeurs valides, objets indépendants, RNG non réentrant qui ne modifie pas la session). L'erreur RNG `RANDOM_FAILURE` intervient **avant** toute mutation de la session : aucun FEU n'est engagé, aucune paire partielle publiée, la sélection humaine reste modifiable et une retentative de FEU est possible. Les appels sans sélection ne consomment **aucun** RNG ; une manche FEU validée en consomme exactement **un** via D05. `ALREADY_FIRED` / phase interdite consomment zéro aléa. Un RNG ayant échoué peut avoir avancé son propre état de contexte : le module ne peut ni garantir ni restaurer l'état d'un callback externe.

Le module ne demande pas à D02 de résoudre une manche partielle. Les échecs inattendus de D02/D05 renvoient `INTERNAL_ERROR` sans commit ; ils indiquent un défaut d'intégration, et ne sont pas convertis en comportement joueur. Les échecs de pointeurs/état invalide retournent `BAD_ARGUMENT`. L'API ne permet aucun accès direct aux méthodes D02 pour les contrôleurs UI ; D06 ne met pas de commandes en attente.

## Coûts, portée et tests

C99 `-Wall -Wextra -Werror -pedantic`, buffers bornés, aucun heap, écran, FreeRTOS, NVS ou ESP-NOW. Les principales valeurs à contrôler sont `sizeof(WarshipSoloSession)`, `sizeof(WarshipSoloPublic)`, les copies temporaires `WarshipBattle staged` et `WarshipBattlePublic public_cpu` sur pile, plus les buffers internes de Capitaine. Leur taille source n'est **pas une mesure de pile Xtensa**, ni un watermark runtime. `idf.py size[-components]` ne comptabilise que les symboles **effectivement liés** par le programme T02, pas un jeu de production.

La nouvelle suite `warship_solo_session` couvre initialisation et flottes invalides ; sélection, remplacement, annulation, double FEU, tir déjà effectué, phases interdites, RNG en échec et retentative sans demi-résolution ; vue de présentation masquée avant acquittement puis historique public révélé ; non-omniscience via flottes humaines différentes et observation CPU commune ; parties headless multi-manches déterministes ; **trois fins réelles** produites par D02+D05 : VICTOIRE, DÉFAITE et NUL simultané au tour exact de la défaite simulée, plus consommation terminale unique. Les sept suites historiques T01–D05 doivent rester inchangées et vertes.

Exécution CI sur HEAD exact (GitHub Actions du dépôt) :

```sh
cmake -S tests -B build-gcc -G Ninja -DCMAKE_C_COMPILER=gcc -DWARSHIP_SANITIZERS=ON
cmake --build build-gcc
ctest --test-dir build-gcc --output-on-failure -V

cmake -S tests -B build-clang -G Ninja -DCMAKE_C_COMPILER=clang -DWARSHIP_SANITIZERS=ON
cmake --build build-clang
ctest --test-dir build-clang --output-on-failure -V

# Cible Xtensa, ESP-IDF v5.5.6 :
idf.py set-target esp32
idf.py build
idf.py size
idf.py size-components
sha256sum build/cyd_warship.bin
```

Dans `main/app_main.c`, un **appel de liaison à paramètres nuls**, retournant `BAD_ARGUMENT`, conserve la sonde matérielle T02 : il ne lance aucun jeu, ne tire jamais et ne constitue pas une validation de session en conditions physiques. **ON_DEVICE : NON_APPLICABLE** pour D06 isolé. Le câblage déploiement, REJOUER, abandon et palmarès est **D07/D08**, la conformité qualitative de Capitaine en partie et les animations sur CYD sont des validations ultérieures.

Les résultats des jobs, taille firmware liée, empreintes IRAM/DRAM/Flash, SHA-256 et comparatif D05/D06 figurent dans la PR associée à l'Issue #34, sur son HEAD final.
