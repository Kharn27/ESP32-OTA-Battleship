# PORTRAITS-04 — Rapport QA Jalon 0 : étalon A–E

**Issue #133 — livraison CANDIDATE, non approuvée — ON_DEVICE_UNVERIFIED.** Branch `design/issue-133-jalon0-etalonnage-a-e`. **STOP de revue Pilotage avant production des 63 positions restantes.** Aucun rôle/personnage canonique attribué. Les regroupements G/Q n'impliquent aucune exclusion.

## Contenu

- **5 positions A–E**, avec **10 variantes en 48 et 64** : le pilote A-L1-C1 réutilise les deux PNG déjà présents dans `draft/portraits/pilote_A_L1_C1/` (aucun doublon), tandis que B/C/D/E ont **8 sprites nouveaux** et **4 masters HD carrés** sous `draft/portraits/exports/<ID>/`.
- **13 preuves PNG** dans [preuves/](preuves/) : **10 zooms NEAREST ×4** (incluant le pilote), **2 planches PC natives 320×240** avec fond clair/sombre et une **planche-contact 700×182** regroupant 5 IDs, 48/64, clair/sombre.
- Recette de régénération et `--check` non destructif : [export_jalon0.py](export_jalon0.py) ; manifeste de suivi [manifest_jalon0.json](manifest_jalon0.json).
- **Aucun fichier modifié** parmi les cinq originaux PO `draft/portraits/portrait_{A..E}.png`, le pilote, `assets/`, `maquettes/`, la Bible, ou le firmware.

## Sources et recadrages réels

L'étalon reprend les **vrais PNG PO** stockés sur GitHub. Recadrages géométriques séparés obtenus sur les originaux Adobe Creative Cloud de même nom et taille que les originaux GitHub, avec **zones exactes versionnées** ci-dessous. Les masters neutralisent seulement le RGB caché derrière les pixels d'alpha nul (aucun filtre de teinte ou de style). Les identités des fichiers source sont garanties par SHA-256 attendu dans le **script de reproduction**, mais **cette passe n'a pas exécuté ce script sur un checkout complet** : la correspondance cryptographique interstockage Adobe/GitHub doit donc être confirmée par Pilotage via `--check`.

| ID | Rectangle source [x0,y0,x1,y1) | Master et ajout transparent | Particularité soumise à revue |
|---|---|---|---|
| `A-L1-C1` | `[0,49,362,411)` | **Pilote existant inchangé**, 362 carré | Cas témoin, uniformité avec nouvelles propositions |
| `B-L1-C2` | `[362,49,724,411 )` | 362×362, marge haute 0 px | Haut des boucles proche du bord du recadrage. Confirmer l'absence de rognage perceptible au PO/Pilotage. |
| `C-L2-C1` | `[0,272,362,543 )` | 362×362, marge haute 45 px | Recadrage source non carré 362×271 ; marge transparente haute 45 ; le bas touche la limite de cellule. Vérifier épaulettes/cadre. |
| `D-L2-C3` | `[614,256,922,512 )` | 308×308, marge haute 22 px | Squelette et tricorne à l'éclat cyan ; extrémités de la coiffe proches des bords du cadre source 308×256. Revue contour cyan nécessaire. |
| `E-L4-C4` | `[1086,815,1448,1086 )` | 362×362, marge haute 45 px | Perroquet coloré au contact du couvre-chef et en bord gauche ; conserver séparation visuelle au 48. |

**Attention :** les cellules de l'inventaire A–E ne sont pas des recadrages de production ; c'est pourquoi les blocs C/D/E sont mis en carrés au moyen d'espace transparent plutôt que déformés par un resize d'une cellule rectangulaire.

## QA de pixels générés

Le **même master HD** sert aux variantes 48 et 64 par rééchantillonnage **BOX d'aire pondérée alpha prémultiplié, en entiers**, sans 48→64. Chaque sortie est un **PNG RGBA 8 bits**, alpha réel ; les pixels alpha=0 portent RGB=(0,0,0). Aucun fond incrusté. Les fichiers ×4 répliquent exactement chaque pixel en bloc 4×4, alpha compris.

| ID | SHA-256 export 48 | SHA-256 export 64 | Alpha=0 (48/64) | Alpha partiel (48/64) |
|---|---|---|---|---|
| `A-L1-C1` | `5abbcacd3981e6b426c93542dcb6289712afc4ca49b7f38dc5c35c9345360dee` | `2241bf1306772bbca3164356529a4fb8290f9ef33ae89aaa9b29d083572967da` | 692/1242 | 1612/2854 |
| `B-L1-C2` | `df2b5b5c024eda68c2707a81898caad75676fb874131212c754af371c2d1e56d` | `b9a4e6521d808d5ba71b40c3960cd18f33152d369a8ee93c6fc9132476fe7dca` | 630/1128 | 1674/2968 |
| `C-L2-C1` | `159e62a3da5d263d270a58d4feb3f5eba244fddac5948757c9118a7f2474ed26` | `bf84eff6d018e17fdf4fd673528a28b2e41273187c6e3b6d89ba153b48dc79d0` | 882/1560 | 1422/2536 |
| `D-L2-C3` | `b935298bb7f8e5cbb3dd25ce0508820251a67a30fcf0232c34e706a4eca45f93` | `072bc66c7da0042d5f86a0e8e526351a3d0baec7ce1099cb6380f090755ff3e0` | 616/1137 | 1688/2959 |
| `E-L4-C4` | `ff587247a24aaeda329732d7d35d40caf683af87d4cf0d210441c3c546b59c58` | `1169a4e3568578b639d76269f0213aa125ec8a627abdcfb19da63d1edff0c3a9` | 771/1378 | 1533/2718 |

Les SHA-256 des **12 nouveaux PNG** (4 masters + 8 sprites) ont été calculés sur les binaires relus depuis GitHub, via implémentation SHA-256 éprouvée sur vecteur `abc`. Ils figurent intégralement dans le manifeste JSON. Les SHA-256 du pilote proviennent de [qa_repro_report.json](../pilote_A_L1_C1/qa_repro_report.json) et de sa validation PC historique. Les 13 PNG de preuves sont committés, leurs identifiants Git blob fournissent une deuxième traçabilité, mais la **comparaison binaire d'un `--check` sur checkout propre n'a pas encore été attestée**.

## Consommation et méthode de vérification

- **Référence du code :** Python 3.11+ et Pillow **12.3.0** (version de la recette). Export PNG stable via zlib stocké, sans filtre de ligne (filtre 0), sans métadonnées, SHA-256 calculé en sortie.
- **Version de Pillow** : dépendance de lecture/recadrage du PNG source, **les pixels rendus sont calculés en entiers par la recette versionnée**.
- **Vérification à exécuter par Pilotage depuis la racine du checkout :**

```sh
python3 -m pip install Pillow==12.3.0
python3 draft/portraits/exports/export_jalon0.py --check
python3 scripts/check_graphic_pipeline.py
```

`--check` **n'écrit aucun fichier**. Il doit d'abord vérifier le SHA-256 des **5 PNG PO**, puis comparer les **4 masters, 8 variantes et 13 preuves** octet pour octet avec les sorties régénérées, et vérifier le réemploi de la paire pilote.

**Tests effectués :** rendu et contrôle visuel PC des cinq sujets dans les deux tailles sur clair/sombre ; vérification en-têtes 48/64, présence d'un alpha réel, absence de RGB parasite sous alpha nul, SHA-256 du lot de sprites versionnés ; intégrité des originaux Git connue par SHA Git de la PR d'archivage. **Tests non encore effectués :** exécution indépendante de `--check` sur checkout GitHub complet, validation artistique PO, essai exact sur CYD 320×240 et mesure de mémoire Flash/RAM.

## Points de vigilance STOP PILOTAGE

1. **Haut de chevelure de B-L1-C2 :** vérifier la marge ; risque de contact visuel avec le bord du recadrage. Ajuster si besoin le rectangle et toutes ses variantes, **sans retoucher les originaux**.
2. **C-L2-C1 et E-L4-C4 :** proportions d'occupation similaires malgré des images source en cellules courtes ; contrôler la lecture du visage et des bijoux à 48.
3. **D-L2-C3 :** qualité des contours cyan et lisibilité des yeux du squelette à 48, sans fond sombre incrusté.
4. **Taille apparente et hauteur du regard :** comparer A/B/C/D/E en **1× 48 et 64**. Les marges constituent une **proposition d'étalonnage**, pas un contrat PO validé.
5. **Diff/exécution CI :** vérifier sur le HEAD exact de PR, consulter les éventuelles remarques Codex et ne passer à J1 (8 à 16 candidats) **qu'après STOP TECHNIQUE Pilotage**.

**Statut de décision :** GO de *production de candidats 68/68* acquis le 16/11/2026, GO **artistique des individus non acquis**. #134 (promotion `assets/`) est une étape séparée, hors du présent lot.
