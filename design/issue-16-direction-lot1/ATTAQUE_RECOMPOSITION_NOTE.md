# G01 — ATTAQUE recomposée : « La mer est le plateau »

**Statut :** `CORRECTIONS_REQUISES — RECOMPOSITION_ARTISTIQUE_ATTAQUE — DESIGN_PREVIEW — ON_DEVICE_UNVERIFIED`.  
**Issue :** #16 ; **PR :** #19 (DRAFT). **Référence de comparaison :** V2.1 / `ba7d649adb54be781d79705addc466450d52fd4b`.  
**Périmètre :** proposition **ATTAQUE uniquement**. Ne constitue ni une charte validée, ni une proposition d'intégration firmware.

## Concept de composition

**La mer est le plateau** : le paysage n'est plus un petit encart autonome. Un horizon crépusculaire traverse toute la largeur ; la grille 10×10 flotte en surimpression directement sur un seul corps d'eau animé par des crêtes, courant de reflets, lumière fragmentée et vagues asymétriques **sans motif répété par case**. Le navire de premier plan à droite, sa fumée, ses superstructures et son sillage sont **illustratifs et hors de la grille** : il ne révèle pas les positions d'une unité adverse. La coque appartient physiquement à la mer et son sillage épouse la ligne de flottaison. Aucun projectile dessiné comme un trait, aucun impact plaqué sur un objet.

Le feuillage de décor est remplacé par du relief marin et une silhouette nautique reconnaissable à l'échelle réelle. Le bouton **FEU** est un élément de commande chaud en cuivre orangé, aussi lisible que dominant. **ATTAQUE** et **DÉFENSE** sont des pavillons/plaques compacts de la même scène, plutôt qu'un bandeau de télémétrie occupant toute la partie supérieure.

## Contrat visuel fonctionnel de la maquette

- Une seule grille ATTAQUE 10×10 ; 18 px par case **sur ce concept** (23,49) à (203,229). Les en-têtes A–J et 1–10 restent visibles. Taille et précision **à vérifier physiquement S02**.
- Cible **G5** indiquée par quatre coins cyan et répétée sur la commande **CIBLE G5**, sans toucher au fonctionnement de la sélection et de FEU.
- Ratés : colonne d'écume turquoise/blanche ancrée à une couronne d'eau. Touchés : gerbe de flammes orange/jaune sur remous sombres. Chaque symbole n'occupe que sa case ; aucun groupe de touches ne dessine un navire ou une épave adverse.
- Commande **DÉFENSE** : `DERN. E5 TOUCHÉ` et, de façon **indépendante**, `NON LU / COULÉ`. Le scénario signifie que le dernier tir de Capitaine touche E5, alors qu'un naufrage plus ancien n'a pas encore été consulté. **Aucun compteur.** L'ouverture DÉFENSE acquitte le résumé et l'alerte, sans effacer les impacts historiques. Ce fonctionnement vient de la Bible §14.10.2, non d'une nouveauté graphique.
- Navigation, abandon, animations et verrouillage FEU restent exactement définis par la Bible ; la preview est un **état statique avant FEU** et ne simule pas de clic ni de rendu embarqué.

## Comparaison avec la V2.1 et les trois premières planches

| Aspect | V2.1, non retenue artistiquement | Recomposition proposée |
| --- | --- | --- |
| Composition | Grille en rectangle à gauche et paysage cadré en vignette à droite | Océan et horizon continus ; grille comme carte de tir sur l'eau ; bateau intégré hors grille |
| Rapport art/jeu | Interface tactique décorée ; deux zones nettement disjointes | Scène de jeu illustrée ; bateau, soleil, sillage et interface participent à la même scène |
| Matière de l'eau | Micro-motifs répétitifs à l'échelle des cellules | Bandes ondulantes, remous et reflets discontinus sur toute la surface, indépendants du quadrillage |
| Impacts | Éclats simplifiés en croix / écumes peu dimensionnées | Flammes triangulaires et écumes en gerbe avec contact aqueux, silhouettes plus expressives |
| Commandes | Cadres/plaques empilés, FEU en cartouche | Pavillons tactiles dans le ciel et commande de cuivre plus théâtrale |
| Intention des trois premières planches | Perdue dans la V2/V2.1 | Retour à la chaleur crépusculaire, mer turquoise, navire évocateur et action expressive — **sans** reprendre leurs proportions HD ni leur surcharge d'informations |

La comparaison aux trois planches se fonde sur les intentions consignées dans les notes G01 (coucher de soleil, mer, navires, cuivre, action expressive), **pas** sur une fidélité pixel à pixel : leurs originaux 1448×1086 ne sont pas des gabarits runtime approuvés. La V2.1 demeure conservée telle quelle dans la PR pour revue contradictoire.

## Livrables et reproductibilité

- `attaque_recomposition.py` : **source originale éditable** qui dessine et compose quatre calques indépendants. Les couleurs, navire, mer, grille, coordonnées, libellés et marques peuvent être modifiés dans ce fichier ; génération PC Pillow.
- `attaque_recomposition_layers.ora` : source **raster à quatre calques**, modifiable dans Krita/GIMP ; textes rasterisés dans l'ORA mais éditables dans le générateur Python.
- `attaque_recomposition_320x240.png` : rendu de décision natif en RGB 320×240.
- `attaque_recomposition_x4.png` : 1280×960 produit **uniquement** par nearest-neighbor.
- `attaque_recomposition_trace_editable.svg` : tracé vectoriel Adobe **complémentaire** du rendu, éditable comme chemins ; ne remplace pas les calques nommés ni la source d'auteur.

**Commande de reproduction sur PC :** `python -m pip install Pillow && python design/issue-16-direction-lot1/attaque_recomposition.py design/issue-16-direction-lot1/`.

**Contrôles :** dimensions natives et ×4, égalité stricte pixel-à-pixel du zoom avec le nearest-neighbor, intégrité de l'ORA fusionnée, conservation de 4 calques. L'automatisation temporaire de dépôt (voir Actions de la branche) a vérifié les dimensions et l'équivalence du zoom et vérifié que le Python et l'ORA récupérés sont strictement identiques aux sources auteur.

Empreintes SHA-256 des fichiers auteurs inspectés localement :
- PNG natif : `84575c573bd320700dfe5880f00513ac1c422b28095b110db572a27df5926630`
- PNG zoom : `d4fe1f74156012fb9da5d25957d38df68c3a9ecac130420645464d6790de71ec`
- PY : `7b22095f8d01c133afa4e0e6aa02a373f9d943d07ab6b6294075f1cc3e7fd633`
- ORA : `1dddfe28c2563a228c8120a58bf6dc5dcb11f836787866500a3d30227200c616`

## Provenance, réserves et décision

**Dessin original réalisé en pixels à la taille native**, avec Pillow ; aucune image tierce intégrée aux exports. Une image d'étude d'atmosphère a été générée dans Adobe Firefly pendant l'exploration ; **aucun pixel de cette image n'est repris** dans la maquette de production ou dans les sources. Vectorisation Adobe d'appoint du PNG final. Polices PC DejaVu Sans Condensed (non distribuées et non contractualisées pour le firmware).

**Non testés :** CYD réelle, tactile, contraste LCD, lisibilité de textes 9–14 px, faisabilité de FEU et de la bascule DÉFENSE au stylet, décodage Flash/RAM, coûts d'animation et performances. Aucun budget matériel ni format d'asset runtime n'est déduit de ces aperçus. S02 et le contrat Développeur/Pilotage restent requis.

**Action :** arbitrage artistique **explicite du PO** sur cette recomposition ATTAQUE, avant extension à DÉFENSE, G02–G04 ou validation d'une charte. **Pilotage seul effectue revue et fusion ; PR reste DRAFT et non fusionnée.**

Références : `BIBLE_PRODUIT.md` §1, §2, §14.1/14.4/14.5/14.10.2/14.13 et §15 ; `docs/WORKFLOW_AGENTS.md` ; `docs/TEST_POLICY.md` ; PR #8 ; Issue #16 ; PR #19.
