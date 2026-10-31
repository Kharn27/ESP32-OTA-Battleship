# D04 — Capitaine : pistes, contradictions et naufrages ambigus (Issue #29)

**Contrat autorisé :** Bible produit v0.17 §9, §14.3 et §16 ; principes CAP-CPU-004/005/006/007 et 008 avancé, CAP-TEST-001. **CAP-CPU-009 (tailles restantes) reste hors D04.** Cette tranche poursuit le composant D03, sans implémenter la session ou l'IA complète.

## Frontière secret/public et ABI préservées

Signature **inchangée** : `warship_captain_choose(const WarshipBattlePublic *view, WarshipCaptainRandom rng, void *ctx, WarshipCaptainTarget *out)`. L'appelant fournit strictement `warship_battle_observe(battle, WARSHIP_BATTLE_CAPTAIN, &view)`. Aucun champ ni argument `WarshipBattle`, `WarshipFleet`, carte d'occupation, appartenance des anciennes touches ou coordonnées de l'épave. Les seules observations CPU utilisées pour le choix sont les tirs `outgoing` résolus ; `incoming` est validé mais n'oriente pas le choix. Un `SUNK` contient le point d'impact et **le type**, jamais les autres cellules de l'épave.

Le moteur reste **sans état persistant**. Les pistes sont **reconstruites à partir de l'historique public** à chaque appel : deux situations publiques et deux tirages identiques rendent exactement le même choix, indépendamment des positions secrètes. L'API et les statuts D03 sont conservés : `BAD_ARGUMENT`, `BAD_OBSERVATION`, `NO_TARGET`, `RANDOM_FAILURE`, avec sortie non modifiée en erreur. Aucun tir répété, zéro tirage sur `NO_TARGET`/entrée invalide, exactement un tirage sur observation jouable, aucune allocation, aucun I/O ou RTOS.

## Algorithme technique (non nouvelle règle PO)

1. Préparer des masques de 100 octets chacun, exclusivement publics : `used` (tout tir), `hit` (HIT dont l'identité reste inconnue), `sunk_hint` (uniquement les **HIT observés avant un SUNK**, à distance immédiate orthogonale ou diagonale de son carré connu). Les HIT acquis **après** ce SUNK ne sont jamais déclassés par lui. `sunk_hint` est **uniquement une priorité souple**, jamais une carte d'épave ou une exclusion d'eau ; toute case encore inconnue reste ciblable. Un HIT antérieur ne reçoit aucun `ship_id`.
2. Reconstituer les pistes par ordre des indices HIT (plus récents d'abord), avec deux catégories : d'abord les indices **non ambigus chronologiquement** vis-à-vis des SUNK, puis les **anciennes touches proches d'un SUNK postérieur**, **sans jamais les effacer**. La proximité géographique seule ne déclasse jamais un HIT **postérieur** au naufrage. Une paire qui contient au moins un HIT frais reste une piste de première priorité. Dans chaque catégorie : examiner les paires adjacentes alignées dont une extrémité reste libre, ensuite les voisins orthogonaux inconnus d'une touche. Une paire est toujours une **hypothèse**, même si les navires se touchent.
3. Quand C4/F4 sont MISS autour de D4/E4 HIT, aucun prolongement horizontal libre ne subsiste : essayer les voisins orthogonaux d'une touche, puis une autre piste. Le même mécanisme fonctionne pour un axe vertical contredit. Un seul MISS sur une extrémité laisse l'autre prioritaire. Les HIT des autres pistes sont conservés dans l'historique et redeviennent prioritaires quand la piste actuelle s'épuise.
4. Un SUNK conduit à **préférer un autre indice public frais** s'il existe, sans affirmer que les HIT voisins font partie de l'épave. En l'absence d'autre piste, il est légitime de revenir aux HIT ambigus proches du SUNK. Avec quatre impacts dans une zone et annonce d'un navire de trois cases, au moins une hypothèse d'autre navire demeure exploitable ; D04 n'en identifie **jamais la case exacte par connaissance secrète**. D01 autorise le contact orthogonal et diagonal.
5. En l'absence de HIT exploitable, le moteur conserve la recherche D03 répartie en quatre quadrants 5×5, parité variable et aléa injectable, sans analyse exhaustive des placements ou probabilités.

**CAP-CPU-009** : point d'extension ultérieur en filtrage de plausibilité géométrique avant sélection parmi les cellules publiques inconnues, basé uniquement sur les types coulés connus et longueurs classiques. **Aucun tel filtrage n'est implémenté en D04.**

L'heuristique de voisinage de SUNK n'est ni un rayon d'épave ni une attribution de touches. Seules les touches **déjà observées** lors d'un naufrage peuvent devenir des indices ambigus : un HIT nouveau voisin est nécessairement postérieur et conserve sa priorité. La proximité peut concerner des navires différents ; seule la priorité de recherche change, jamais leur statut.

## Matrice de réception observable (nouvelle suite `warship_captain_tracks`)

| Principes | Scénarios dans les tests C99 |
| --- | --- |
| CAP-CPU-004 | D4/E4 HIT, C4/F4 MISS, aucun SUNK : sélection orthogonale encore libre, puis poursuite après autre MISS. Axe vertical contradictoire G4/G5, G3/G6 MISS ; si voisins tous occupés, retour légal à la recherche générale |
| CAP-CPU-005 | Deux HIT indépendants C3/H8, dans les deux ordres chronologiques : piste récente d'abord, MISS n'interrompt pas la poursuite, puis retour à l'autre quand les voisins sont épuisés |
| CAP-CPU-005/008 | Paire F6/G6 et autre piste B2 : extension de paire, une extrémité MISS → persévérance sur l'autre |
| CAP-CPU-006 | B2 HIT et E5 HIT puis E6 SUNK type patrol : priorité à B2 ; après épuisement, retour au HIT E5 ambigu |
| CAP-CPU-007 | D5/E5/F5 HIT, G5 SUNK annoncé de type trois cases : C5 reste candidat, aucune détermination de quelle touche est dans l'épave |
| CAP-CPU-006/007 | E5 HIT et F6 SUNK en diagonale : pas de halo d'eau ; E5 garde des voisins ciblables |
| **Correctif chronologie post-SUNK (revue Pilotage)** | B2 HIT, E4 HIT, E5 SUNK (patrouilleur), D5 HIT **après** : prochain tir orthogonal à D5 (RNG 0..7), jamais A2 ; cas miroir G5, paire mixte E4 pré-SUNK/E3 post-SUNK → E2, puis retour à B2 si les voisines de D5 sont épuisées. Vérification RED sur le code précédent et GREEN sur le HEAD corrigé. |
| Équité | Même `outgoing`, deux historiques `incoming` publics différents et même RNG ⇒ même choix. Le test D03 **conservé** utilise en plus deux flottes humaines D01 réellement distinctes, D02 réel, mêmes observations CAPTAIN et mêmes décisions |
| Robustesse | 99 tirs → dernière case ; 100 tirs → `NO_TARGET` sans RNG ni écriture ; duplications et coordonnées/compteurs incorrects → `BAD_OBSERVATION` ; RNG en échec → `RANDOM_FAILURE` ; sortie préservée |
| Régression D03 | Suite `warship_captain_foundation` conservée intégralement : E5 isolé, E5/F5 avec D5 ou G5, 12 graines × 70 tirs organisés, erreurs et secret réel D02 |

La stratégie est **bornée** : validation des deux historiques de 100 tirs au maximum, construction de trois masques locaux (300 octets au total) ; `sunk_hint` est alimenté **pendant la lecture chronologique** du tableau public, avec seulement les HIT déjà rencontrés, au maximum deux catégories × deux passes × 100 tirs, chacune avec au plus quelques voisins ; sinon scans D03 sur 100 cases. Un appel RNG au maximum. **Aucune copie du `WarshipBattlePublic` de 803 octets**, ni objet persistant ou heap.

## Preuves et reproduction

GitHub Actions sur le HEAD exact de la PR :
```sh
cmake -S tests -B build-gcc -G Ninja -DCMAKE_C_COMPILER=gcc -DWARSHIP_SANITIZERS=ON
cmake --build build-gcc
ctest --test-dir build-gcc --output-on-failure -V
cmake -S tests -B build-clang -G Ninja -DCMAKE_C_COMPILER=clang -DWARSHIP_SANITIZERS=ON
cmake --build build-clang
ctest --test-dir build-clang --output-on-failure -V

# Cible ESP32 Xtensa, ESP-IDF v5.5.6 :
idf.py set-target esp32
idf.py build
idf.py size
idf.py size-components
sha256sum build/cyd_warship.bin
```

Le `main/app_main.c` T02 n'est **pas modifié** : il référence D03 pour vérification du lien, sans vraie partie. `size-components` mesure seulement les sections **effectivement conservées**, pas le coût de future orchestration du CPU. Les tailles et estimations de pile hôte ne sont pas des mesures de pile Xtensa. Aucune campagne matérielle demandée : `ON_DEVICE: NON_APPLICABLE`.

## Exclusions / limites

Pas de CAP-CPU-009 ni conformité Capitaine complète, pas d'évaluation qualitative de difficulté ressentie, Mousse/Amiral, AUTO/PRÊT/FEU, écran, tactile, NVS, palmarès, ESP-NOW ou deuxième CYD. La priorité de pistes est une heuristique contrôlée, **non** un algorithme définitif imposé par la Bible. La validation matérielle de gameplay reste distincte.
