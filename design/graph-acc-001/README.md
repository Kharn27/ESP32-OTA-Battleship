# GRAPH-ACC-001 — Révision 4 : retouche ciblée du tir et de l’impact

**Retour PO reçu** : composition historique actée ; titre et boutons OK en principe ; **tir ennemi et impact dans l’eau à corriger**, sans nouvelle exploration.

**Statut demandé :** `RETOUR_PO_CIBLE — SCENE_ACTEE — TITRE_OK_EN_PRINCIPE — BOUTONS_OK_EN_PRINCIPE — CORRECTIONS_GRAPHIQUES_APPLIQUÉES — PO_VALIDATION_FINALE_PENDING — PR_DRAFT_NON_MERGED`.

## Intervention circonscrite

- **Scène** : conserve pixel pour pixel le fond maritime narratif de `872496d73a150dd5a016c219973e5865576b7e04` (original `01_bataille_narrative_320x240.png`). Aucun changement de palette globale, perspective, ciel, navires, nuages, vagues ou tirs alliés.
- **Titre et boutons** : repris **pixel pour pixel** de la révision 3 (`cabee0d8c4263a8b3599d657aee368afed6fa507`). Zones contrôlées : titre (8,9)-(120,30), commandes (7,176)-(166,205) et (7,207)-(126,237). **10 533 pixels** de ces trois zones contrôlés identiques. Texte et contours inchangés.
- **Tir ennemi** : suppression du « laser » continu, trop chaud et trop blanc. Point de mise à feu près de la silhouette ennemie `(229,43)` ; déplacement suivant une trajectoire balistique sous-jacente `(229,43) → (278,184)`, mais tracé rendu par une **trace atmosphérique irrégulière très ténue**, quelques reflets cuivrés dispersés et légère fumée. Pas de segments rectilignes graphiques.
- **Impact manqué** : centre vers `(278,186)`, loin de la coque et dans la mer. Ancienne étoile blanche supprimée au profit d'une **écume discontinue et d'embruns courts**, sur une bande de vague plus sombre et des reflets de l'eau. Couleurs localement mélangées à celles du pixel maritime d'origine ; petites particules à transparence variable, pas de contour régulier ou de sprite découpé.
- **Grain** : fond historique **inchangé**. Pixelisation toujours à arbitrer par le PO.

## Livrables

- `accueil_cyd_warship_320x240.png` — rendu natif 320 × 240, référence de validation esthétique.
- `accueil_cyd_warship_x4.png` — strict ×4 nearest-neighbor (1280 × 960).
- `accueil_cyd_warship.svg` — source éditable : fond historique intégré, zones UI vectorielles inchangées, nouvelle couche de riposte indépendante.
- `effet_riposte_320x240.png` — **couche transparente RGBA isolée** pour retouche ciblée dans Krita/GIMP/Aseprite ; SVG et couche à conserver dans le même dossier.
- `README.md` — cette note de composition et de contrôle.

## Avant / après : la zone corrigée uniquement

| Critère | HEAD précédent | Nouvelle correction |
|---|---|---|
| Mise à feu | flash + trait lumineux apparent | flash discret, condensation/particules |
| Trajectoire | courbe brillante continue | légère traînée décomposée, atmosphère visible au travers |
| Impact | gerbe blanche en étoile | fines projections irrégulières mêlées à l'écume et aux reflets |
| Scène, UI | retenues en principe | **strictement préservées** |

Les rectangles tactiles restent (8,177,156,27) et (8,208,114,27) : **aucune modification UX**. Environ 1 060 pixels seulement changent par rapport au précédent PNG ; les retouches se concentrent sur la zone du tir et de la gerbe.

## Niveau de preuve et réserves

`DESIGN_PREVIEW` uniquement : PNG natif inspecté en aperçu sur ordinateur, génération ×4 par réplication exacte. **Pas de build, pas de test CYD, pas d'estimation empirique du coût mémoire firmware**. L'asset de revue n'est pas un asset runtime.

Le fichier SVG conserve des objets texte et des plaques éditables ; comme avant, ses moteurs de rendu peuvent présenter de petites différences par rapport au raster validé : **PNG natif = référence visuelle**. Source SVG liée au PNG transparent externe pour préserver la matière plutôt que lui substituer des formes vectorielles lisses.

**PO : validation artistique finale non acquise. PR #8 à maintenir en brouillon. Pilotage seul responsable de sa revue et de la fusion.**
