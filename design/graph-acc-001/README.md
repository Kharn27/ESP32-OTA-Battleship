# GRAPH-ACC-001 — Révision 5 : tir et impact réellement relus dans le composite

**Périmètre strict :** réponse au rejet artistique PO de la révision 4 (`277f38d173e7f75d1f228494092461d45f67d0e2`), directive Pilotage sur la PR #8. La scène, le titre et les deux boutons sont **verrouillés** ; aucun changement de direction artistique globale.

**Statut :** `CORRECTIF_GRAPHISTE_LIVRE_POUR_REVUE_PO — DESIGN_PREVIEW — ON_DEVICE_UNVERIFIED — PR_DRAFT_NON_MERGED`. Le PO **n'a pas encore validé** cette nouvelle correction.

## Ce qui est modifié

**1. Mise à feu ennemie.** Éclat orangé-blanc contrôlé près de l'armement/silhouette du navire ennemi, vers `(229,43)`. Son échelle reste petite au regard de la distance.

**2. Trajectoire.** Sillage de fumée sombre bleu-gris et grains cuivrés/ambre par endroits, depuis la bouche jusqu'à l'impact, suivant une légère courbe balistique passant de `(229,43)` vers `(281,184)`. La traînée est **assez matérialisée pour être suivie à l'écran natif**, mais texturée et rompue par l'atmosphère : pas de trait vectoriel uniforme, pas de grand laser blanc.

**3. Impact raté dans l'eau.** À droite et au-delà de la proue, vers `(280,187)`, sans contact avec le navire. Une colonne d'eau irrégulière, des embruns lumineux à granularité variable, des éclats de mousse et une dépression sombre se raccordent aux vagues et aux lumières du coucher de soleil. Les couleurs de la gerbe sont dérivées des pixels de la mer historique à cet emplacement ; blancs chauds côté lumière, cyan et bleu-gris sur les bords.

Les éléments graphiques s'inscrivent dans **le même plan de profondeur** que les vagues au lieu d'un pictogramme géométrique collé. Aucun aéronef, bateau, animation ou commande ajouté.

## Preuve d'immuabilité — comparatif avec le HEAD rejeté

| Zone / élément | Contrôle |
|---|---|
| Scène historique, cadrage, mer, coucher de soleil, navires, tirs amis | Fond historique original `872496d73a150dd5a016c219973e5865576b7e04` préservé, hors seule intervention de riposte |
| Titre `CYD WARSHIP` | Pixels inchangés par comparaison directe |
| `JOUER CONTRE MOUSSE`, `PALMARÈS` | Apparence, positions et zones tactiles inchangées |
| Total zones verrouillées | **10 533 / 10 533 pixels RGB identiques** avec `277f38d` |
| Pixels modifiés dans le composite | **1 693** pixels (uniquement effet de tir, impact et raccords dans l'eau) |
| Ancien effet presque invisible | Remplacé par mise à feu contrastée, trace fumée/ambre identifiable, gerbe plus volumétrique et mousse au niveau des vagues |

## Fichiers de revue
- `accueil_cyd_warship_320x240.png` — **PNG final composite 320 × 240**, référence de jugement PO, image fixe.
- `accueil_cyd_warship_x4.png` — **1280 × 960**, pixels répétés 4 × 4, nearest-neighbor strict, sans réinterpolation.
- `effet_riposte_320x240.png` — **calque transparent RGBA** actualisé. Effet local éditable (Krita/GIMP). Ce n'est pas le composite final.
- `accueil_cyd_warship.svg` — **inchangé par rapport au HEAD précédent** : fond narratif intégré, titre et boutons vectoriels d'origine, référence relative vers `effet_riposte_320x240.png` à conserver à côté du SVG. Le fond reste raster ; les rendus de police SVG peuvent différer du PNG de référence.
- `README.md` — présente note de revue et limites.

Rectangles tactiles **inchangés**, non contractuels : `JOUER CONTRE MOUSSE (8,177,156,27)` ; `PALMARÈS (8,208,114,27)`.

## Validation

Contrôles d'export effectués pour le PNG natif et le zoom ×4 sur poste. La **lisibilité finale à 100 % doit être jugée sur le PNG composite**, non sur le calque isolé. Une production graphique n'est **ni une approbation PO ni un test embarqué**.

**Aucun firmware, compilation Xtensa, mesure RAM/Flash runtime ou test physique CYD.** Aucune fusion de la PR par le Graphiste. L'état de la PR doit rester `DRAFT` jusqu'aux décisions du PO et du Pilotage.
