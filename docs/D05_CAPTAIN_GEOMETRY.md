# D05 — Capitaine : géométrie des tailles restantes (Issue #33)

**Référence contractuelle :** Bible produit v0.17 §14.3 / §16, CAP-CPU-009 et CAP-CPU-010, CAP-TEST-001 ; Issue #33. **Base de développement :** `main` `5fb8120c8e72d6bebf5f03bb507dd643ea8a94fe` (D04). Cette note décrit une heuristique **technique** ; elle ne crée aucune règle produit.

## Périmètre / frontières de confidentialité

La signature `warship_captain_choose(view, rng, ctx, out)` est inchangée. Capitaine ne reçoit que `WarshipBattlePublic` du point de vue CAPTAIN et le RNG injectable. La géométrie n'examine **que** l'historique `outgoing` et ses `SUNK` (type `ship_id` annoncé). Elle n'accède ni à la flotte humaine, ni à l'occupation réelle, ni à l'appartenance des touches anciennes à une épave. L'historique `incoming` est validé comme avant, mais n'influence pas le choix.

Types publics D01 : patrouilleur 2, sous-marin 3, destroyer 3, croiseur 4 et porte-avions 5. Les deux navires de longueur 3 ont des **IDs distincts** : le naufrage de l'un n'élimine pas l'autre. Le plus petit navire encore à flot (longueur minimale parmi les IDs non annoncés `SUNK`) suffit à éliminer les corridors trop courts pour *tous* les navires restants. La flotte ne sert pas à résoudre des hypothèses de placement : seule la fonction D01 `warship_fleet_ship_length(id)` fournit une longueur publique.

## Critère géométrique conservateur

**Uniquement si `pursue()` D04 ne trouve aucune piste exploitable**, une cible non tirée est crédible si elle appartient à une suite droite **horizontale ou verticale** d'au moins la plus petite longueur restante, constituée de cases publiquement permissives :

- **inconnue** : traversable et sélectionnable ;
- **`HIT` ancien non attribué** : traversable pour mesurer la longueur, mais **jamais sélectionnable à nouveau** ;
- **`MISS`** : bloque le segment (eau certaine) ;
- **case effectivement ciblée et marquée `SUNK`** : bloque le segment (elle ne peut pas être une case d'un autre navire encore à flot). Il ne s'agit **pas** de reconstruire les autres cases de l'épave.

Exemple : après `SUNK patrouilleur`, une poche de deux cases inconnues encadrée de `MISS` ou de carrés `SUNK` ne peut contenir aucun bateau restant (minimum trois) ; un corridor de trois cases reste possible. Un `HIT` connu peut compléter la longueur d'un tel corridor, malgré son état déjà tiré. Les navires peuvent être en contact orthogonal/diagonal : **aucun halo** n'est créé, et aucune ancienne touche n'est attribuée arbitrairement à une épave.

Le critère est **nécessaire, non suffisant**. Il peut conserver une case qui ne serait en réalité viable pour aucun navire : un `HIT` permissif peut appartenir à une autre épave, les autres bateaux peuvent empêcher une configuration globale, ou plusieurs segments peuvent se chevaucher. Ces **faux positifs volontaires** protègent l'équité et la prudence, sans résoudre la flotte secrète ni chercher l'ensemble des placements.

## Ordre des décisions et souplesse

`pursue()` D04 n'est **pas modifiée** : paires alignées, indices plus récents et gestion chronologique `SUNK` sont conservés à l'identique, en particulier le cas P2 `B2 HIT → E4 HIT → E5 SUNK → D5 HIT`. Le filtre n'affecte **que** `explore()` lorsqu'aucune piste ne donne de cible.

`explore()` conserve les quatre zones 5×5, la parité issue du tirage aléatoire et la répartition selon l'activité publique, mais restreint sa liste candidate aux cases géométriquement crédibles. La parité bascule lorsque son groupe de candidats est vide. Les choix restants sont départagés par le même tirage et non par une énumération de placements ou un classement probabiliste.

Si aucun candidat géométrique n'existe (historique artificiel, toutes les longueurs annoncées coulées alors que des cases inconnues subsistent, contradiction globale), la recherche revient à **l'ensemble des cases jamais tirées**. Ainsi, même une unique case finale isolée reste attaquable. `NO_TARGET` est rendu lorsque la partie est terminale ou les 100 cases sont déjà tirées.

## Bornes et erreurs

Toutes les itérations sont bornées par 100 cases, cinq types, dix positions par axe ; aucun heap, objet global mutable, OS, écran ni radio. Les masques locaux existants `used`, `hit`, `sunk_hint` (100 octets chacun) restent utilisés ; un masque `candidates` local de 100 octets est ajouté **uniquement dans `explore()`**. Cette empreinte est **une comptabilité de buffers C**, pas une mesure de pile Xtensa : compilateur, inlining et ABI peuvent changer les tailles réelles.

Le contrat RNG demeure : zéro appel sur `BAD_ARGUMENT`, `BAD_OBSERVATION` ou `NO_TARGET` ; exactement un appel si au moins une case reste jouable (même cible forcée ou filtre vide). `RANDOM_FAILURE` préserve la sortie, comme toutes les erreurs. Aucun modulo n'est effectué sur zéro candidat : le repli garantit au moins une case candidate avant la sélection de parité.

## Matrice de vérification

Nouvelle suite `warship_captain_geometry` : exclusion de la poche de deux cases après `SUNK patrol` avec corridor crédible, transposition, 64 échantillons ; deux IDs taille 3 ; patrouilleur restant ; porte-avions seul restant ; HIT ancien voisin d'un `SUNK` ; absence de halo ; permutation de `incoming` ; repli dernière case et cinq `SUNK` artificiels ; all 100 `NO_TARGET` ; erreurs et unicité RNG ; variété de choix sur une même observation.

Régression à conserver **sans modifier les tests historiques** : `warship_core_contract`, `t02_touch_map_boundaries`, `warship_fleet_contract`, `warship_battle_contract`, `warship_captain_foundation`, `warship_captain_tracks`. Les preuves d'exécution et la comparaison des tailles liées D04/D05 doivent être enregistrées dans la PR sur le **HEAD final**. Commandes de référence :

```sh
cmake -S tests -B build-gcc -G Ninja -DCMAKE_C_COMPILER=gcc -DWARSHIP_SANITIZERS=ON
cmake --build build-gcc
ctest --test-dir build-gcc --output-on-failure -V
cmake -S tests -B build-clang -G Ninja -DCMAKE_C_COMPILER=clang -DWARSHIP_SANITIZERS=ON
cmake --build build-clang
ctest --test-dir build-clang --output-on-failure -V
idf.py set-target esp32
idf.py build
idf.py size
idf.py size-components
sha256sum build/cyd_warship.bin
```

La CI existante épingle ESP-IDF v5.5.6, cible Xtensa `esp32`, et archive le binaire. Elle vérifie le commit exact avant les tâches. Les métriques `idf.py size` et `size-components` sont des **empreintes statiques du code effectivement lié** ; comme l'application T02 ne fait qu'appeler Capitaine avec des paramètres nuls, ces chiffres **ne démontrent pas la taille de toute l'heuristique ni une pile/heap runtime**. Aucun essai CYD n'est prévu pour cet algorithme pur : `ON_DEVICE: NON_APPLICABLE`. Les essais de difficulté ressentie CAP-TEST-001 auront lieu à l'intégration du jeu.
