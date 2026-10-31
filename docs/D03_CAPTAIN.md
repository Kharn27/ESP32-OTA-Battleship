# D03 — IA Capitaine, fondation C99 (Issue #27)

**Source métier :** `BIBLE_PRODUIT.md` v0.17 §9, §14.3, §16 ; contrat D02 `warship_battle_observe(..., WARSHIP_BATTLE_CAPTAIN, &view)`. **D03 n'est pas l'IA Capitaine finale.**

## Frontière publique et API

- Composant `components/warship_captain/`, API `warship_captain_choose(const WarshipBattlePublic *view, WarshipCaptainRandom rng, void *rng_context, WarshipCaptainTarget *out)`.
- **Seul `WarshipBattlePublic` est fourni au moteur.** Aucun argument, champ d'état ni pointeur vers `WarshipBattle`, une flotte humaine ou une carte d'occupation cachée. Les observations de D02 comprennent uniquement les tirs déjà **résolus**. La session future doit passer la projection côté **CAPTAIN**, pas la projection humaine. Aucun accès à des cibles non résolues ni à l'ordre de verrouillage.
- L'API est **sans état persistant** : le contexte RNG appartient à l'appelant ; une suite identique d'observations et d'échantillons rend le même tir. Elle n'utilise ni allocations, ni RNG système, ni timer, ni ESP-IDF, ni RTOS. Appel séquentiel ; pointeurs valides, non chevauchants, détenus par l'appelant. Le retour `WARSHIP_CAPTAIN_OK` écrit une paire `(x,y)` 0..9 jamais tirée. En cas d'erreur, le buffer de sortie reste inchangé.
- Statuts : `BAD_ARGUMENT` (pointeur manquant), `BAD_OBSERVATION` (compteur >100, désaccord outgoing/incoming, verdict hors enum, coordonnée/impact/type invalide, duplication de cible ou de SUNK-ID dans une même série), `NO_TARGET` (partie terminée ou 100 cases déjà attaquées), `RANDOM_FAILURE` (callback non zéro). Les éléments non comptés des tableaux D02 ne sont pas lus. `NO_TARGET` et entrées invalides n'invoquent pas le RNG. Sur observation jouable non pleine, **exactement un appel RNG**, même si une seule cible reste libre.
- Contrôle des deux séries pour sécuriser la frontière D02, puis seule la série **outgoing** oriente la décision. La série **incoming** peut changer sans changer le tir pour un même aléa tant qu'elle reste valide. Les informations de SUNK ne donnent qu'un ID/type, jamais la répartition secrète des anciennes cases touchées.

## Politique de choix (technique, non règle produit)

1. Construire deux masques temporaires bornés de 100 octets : cases `used` et touches non coulées `hit`, **exclusivement depuis les observations publiques**.
2. Explorer d'abord les **paires adjacentes de HIT** (les plus récentes en premier) : leur alignement n'est qu'une **hypothèse** ; si une extrémité alignée D/G ou E4/E7 est encore inconnue, choisir parmi les prolongements libres. Un MISS sur l'un laisse l'autre prioritaire.
3. Sinon, essayer les **voisins orthogonaux inconnus** de la touche HIT la plus récente ayant des voisins accessibles : l'exploration survit aux MISS successifs. Pas de halo d'eau autour des navires. Les `SUNK` ne sont pas traités comme une carte d'épave : leur `ship_id` ne sert pas à attribuer des anciennes touches.
4. Sinon, recherche distribuée entre **quatre zones de 5×5** : privilégier les zones les moins déjà explorées, avec une parité de recherche variable et tirage pour départager les coordonnées. Si une parité est épuisée, explorer l'autre. Ce choix implémente CAP-CPU-001/002/010 sans matrice de probabilité des placements ni trajectoire rigide universelle. La répartition par zones et l'ordre des voisins sont des détails d'implémentation révisables, **pas des règles métier**.

Complexité déterministe bornée : parcours d'au plus 100 éléments par série pour la validation, au plus 100 pour construire les masques, au plus 100 touches historiques pour la recherche des prolongements, puis au plus quatre parcours de 100 cases pour la recherche générale. Chaque appel effectue zéro ou **un tirage aléatoire** et aucune boucle sans borne. Empreinte constante ; aucune copie de l'observation de 803 octets dans l'IA. La mesure `sizeof` et les métriques de pile locales sont à distinguer du runtime Xtensa réel.

## Matrice de traçabilité D03 / CAP-CPU

| Décision | Statut D03 | Comportements vérifiés |
| --- | --- | --- |
| CAP-CPU-001 | **PARTIEL initial** | recherche des cases inconnues répartie par zones, aucune répétition |
| CAP-CPU-002 | **PARTIEL initial** | parité variable, RNG injectable, plusieurs trajectoires selon graine |
| CAP-CPU-003 | **COUVERT sur cas élémentaires D03** | HIT isolé E5 → D5/F5/E4/E6 ; poursuivi après MISS |
| CAP-CPU-008 | **PARTIEL élémentaire uniquement** | E5/F5 → D5/G5 ; retour après prolongements impossibles, **sans révision avancée** |
| CAP-CPU-010 | **PARTIEL initial** | aucune optimisation probabiliste exhaustive |
| CAP-CPU-004 / 005 / 006 / 007 / 009 | **NON COUVERT** | révisions contradictoires, pistes multiples persistantes, ambiguïtés de naufrage et tailles restantes : tranche ultérieure |
| CAP-CPU-008 avancé | **NON COUVERT** | reconstitution de pistes impossibles, priorités complexes : tranche ultérieure |

Le retour vers la recherche après épuisement d'un prolongement n'implique **aucune certitude** quant à la position des navires ; les touches de navires adjacents ne sont jamais classées par ID. Les décisions au-delà des scénarios élémentaires sont **explicitement différées**.

## Scénarios et reproduction de tests

| Critère #27 | Scénario hôte |
| --- | --- |
| Recherche organisée et non figée | 12 graines × 70 tirs sans touche : coordonnées uniques, bornes, dispersion des quatre quadrants, variation des premières cibles |
| Touche isolée | E5 et les 16 masques de voisins déjà attaqués, coins et bords, poursuite après MISS |
| Deux touches alignées | E5/F5, D5 MISS → G5, G5 MISS → D5 ; axe vertical ; fallback légal après blocage des deux extrémités |
| 99/100 cases | unique case restante 99 coups ; `NO_TARGET` à 100, sans écriture ni tirage |
| Incohérences/erreurs | valeurs hors enum, coordonnées hors grille, types invalides, SUNK en double, tirs en double, compteur >100, incoming/outgoing incohérents, NULL, RNG en échec ; absence de mutation |
| Secret public | D02 réel : deux flottes humaines D01 différentes, observation CAPTAIN initiale puis après tirs identiques à l'eau ; **mêmes vues publiques et même aléa → même cible** ; aucune lecture de `WarshipBattle` côté IA |

GCC/Clang C99 strict + ASan/UBSan via CI sur HEAD exact :

```sh
cmake -S tests -B build-host-gcc -G Ninja -DCMAKE_C_COMPILER=gcc -DWARSHIP_SANITIZERS=ON
cmake --build build-host-gcc
ctest --test-dir build-host-gcc --output-on-failure -V
cmake -S tests -B build-host-clang -G Ninja -DCMAKE_C_COMPILER=clang -DWARSHIP_SANITIZERS=ON
cmake --build build-host-clang
ctest --test-dir build-host-clang --output-on-failure -V
```

Cible : ESP-IDF v5.5.6, `idf.py set-target esp32 && idf.py build && idf.py size && idf.py size-components && sha256sum build/cyd_warship.bin` dans `.github/workflows/t01-build.yml`.

**Mesure mémoire :** `idf.py size-components` mesure uniquement les sections conservées au lien, et non le poids théorique de toutes les fonctions de l'archive. Le log D03 de boot vérifie le rejet des arguments NULL, **sans exécuter de stratégie sur une vraie partie**. Si LTO/fonction sections retire une portion du moteur, sa taille n'est pas le coût complet futur de session, même avec un lien réussi. RAM/pile/latence sur carte : **NON TESTÉ** ; aucun essai physique PO demandé, **ON_DEVICE : NON_APPLICABLE**.

## Exclusions et risques connus

Pas de CAP-CPU-004/005/006/007/009 ni CAP-CPU-008 avancé, pas de politique complète après SUNK ni révision contradictoire des pistes, aucune garantie de taux de victoire ni validation de difficulté ressentie. Pas d'orchestration de session, flotte CPU, UI/tactile, ESP-NOW, NVS, palmarès ou timer. La prochaine US pourra introduire un petit état de pistes ou enrichir la stratégie sans exposer de donnée secrète.
