# D01 — Flotte classique C99 (Issue #23)

Source métier : `BIBLE_PRODUIT.md` v0.17, §4, §14.1, §14.2, §14.7.
Ce document décrit l'implémentation ; il ne crée pas de règle métier.

## Contrat

`warship_fleet.h` définit cinq identités fixes, une origine `(x,y)` de coordonnées
0..9, une orientation H/V et une longueur contrôlée contre l'identité. Chaque
`WarshipFleet` contient exactement cinq enregistrements de cinq octets ; 25
octets sur la plateforme hôte testée. L'ordre des enregistrements n'est pas
contractuel ; les IDs doivent être exactement 0,1,2,3,4, une fois chacun.

`warship_fleet_validate` vérifie identités, longueurs, orientation, limites et
chevauchements. Le format origine+orientation+longueur impose par construction
rectitude et contiguïté. Les contacts orthogonaux/diagonaux sont acceptés.

`warship_fleet_generate` retourne une flotte complète et valide, sans
allocation dynamique. Il appelle le callback RNG exactement cinq fois en cas
de succès ; un callback échouant interrompt immédiatement et retourne
`WARSHIP_FLEET_RANDOM_FAILURE` sans toucher à la destination. La génération
et `warship_fleet_copy` ne publient leur résultat qu'après validation complète.
Le consommateur garde la propriété et la responsabilité de la mémoire.

Le modèle contient les positions secrètes ; **ne pas le transmettre à la future
IA de Capitaine** : son API d'observation publique reste à concevoir dans son
Issue propre. Aucune sauvegarde NVS ni orchestration de revanche ici.

## Terminaison de AUTO

Pour chaque ID 0..4, un entier `uint32_t` externe choisit l'indice initial
parmi 200 placements candidats `10*10*2` (origine, orientation), puis
l'algorithme parcourt au plus 200 candidats cycliquement à la recherche du
premier emplacement libre et dans la grille. Le tableau local de 100 octets
représente uniquement les occupations pendant la construction.

Il existe **20 blocs horizontaux disjoints de longueur cinq** sur la grille
10x10 (deux par ligne). Au moment de placer n'importe lequel des cinq
navires, au plus 12 cellules sont occupées. Chaque cellule occupée ne peut
bloquer qu'un de ces blocs disjoints : au moins huit blocs restent vides.
Chaque navire est de longueur inférieure ou égale à cinq, donc un placement
horizontal est forcément libre. Le scan des 200 candidats le découvrira.
Le nombre de sondes est borné à 1000, les appels RNG à cinq, et une source
constante ne peut pas provoquer de blocage. La distribution n'est pas
statistiquement uniforme et une répétition identique est autorisée.

L'appelant fournit plus tard un adaptateur RNG approprié pour la cible ;
aucun choix d'entropie ni API ESP-IDF n'est imposé par ce module.

## Reproduction des tests

```sh
cmake -S tests -B build-host-gcc -G Ninja -DCMAKE_C_COMPILER=gcc -DWARSHIP_SANITIZERS=ON
cmake --build build-host-gcc
ctest --test-dir build-host-gcc --output-on-failure -V
cmake -S tests -B build-host-clang -G Ninja -DCMAKE_C_COMPILER=clang -DWARSHIP_SANITIZERS=ON
cmake --build build-host-clang
ctest --test-dir build-host-clang --output-on-failure -V
```

Les 10 000 graines déterministes (chacune relancée deux fois), RNG constant,
erreurs RNG 1..5, cas limites géométriques, identités distinctes et atomicité
sont vérifiés dans `tests/test_warship_fleet.c`. Les tests T01/T02 restent
intégrés dans le même CTest.

Pour ESP-IDF v5.5.6 / `esp32`, la CI exécute `idf.py build`,
`idf.py size` et `idf.py size-components`, sans essai CYD physique.
L'initialisation T02 reste inchangée hormis un journal de preuve de liaison
du composant. Voir la PR pour les mesures au HEAD exact.

Limites : aucune exigence d'uniformité RNG ; les mesures hôte de `sizeof`
ne sont pas des mesures Flash/DRAM cibles ; le firmware ne génère pas encore
de flotte au boot. Le RNG utilisé par la future application sera distinct du
RNG déterministe des tests.
