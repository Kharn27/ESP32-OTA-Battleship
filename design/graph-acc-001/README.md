# GRAPH-ACC-001 — Révision 3 : réponse à la contre-revue Pilotage

**Directive source** : [PR #8 — commentaire 6503053992](https://github.com/Kharn27/ESP32-OTA-Battleship/pull/8#issuecomment-6503053992).  
**Statut** : `CORRECTIONS_APPLIQUÉES — DESIGN_PREVIEW — PO_ARTISTIC_REVIEW_PENDING — ON_DEVICE_UNVERIFIED`. PR maintenue en brouillon.

## Composition et arbitrages
La scène narrative originale `872496d73a150dd5a016c219973e5865576b7e04` (`assets/explorations/2026-10-29-bataille-en-cours/01_bataille_narrative_320x240.png`) est **préservée au pixel près** hors seules surimpressions. Coucher de soleil, navire principal, ennemi endommagé, relief de la mer, tirs déjà existants et aéronefs présents ne sont pas redessinés ; aucun avion supplémentaire. La granularité du fond n'est pas modifiée : **niveau de pixelisation toujours ouvert pour le PO**.

1. **Origine du tir** : petit éclat sur la silhouette ennemie en `(229,43)` (au lieu de `(232,55)`). Trajectoire **unique et continue** `M229 43 C250 74 252 128 281 184`, corail/cuivrée, distincte des tirs alliés historiques.
2. **Impact raté** : autour de `(280,187)`, dans l'eau, à droite de la proue du navire principal, sans couvrir la coque ; gerbe à lignes courbes, reflets froids, mousse et embruns en accord avec les vagues. Aucune gerbe géométrique blanche triangulaire.
3. **Boutons** : deux **plaques navales individuelles** aux coins chanfreinés, métal vert-bleu oxydé, fines bordures cuivrées rappelant le soleil, reliefs et transparence laissant percevoir l'eau ; lettrage proportionnel anti-aliasé et contraste ivoire.
4. **Titre** : directement posé sur le ciel en ivoire chaud et légèrement souligné, **sans bandeau noir**.

## Hypothèses de zones tactiles (non contractuelles)
- `JOUER CONTRE MOUSSE` : (8,177,156,27).
- `PALMARÈS` : (8,208,114,27). Hauteur améliorée **23 → 27 px**, 5 px de marge inférieure, séparation 4 px. Confort à vérifier après validation artistique PO ; pas de test CYD demandé à ce stade.

## Comparatif avant/après
| Élément | HEAD rejeté `aeb8ac24` | Correction |
|---|---|---|
| Scène narrative | conservée | conservée, pixel par pixel hors ajouts |
| Tir ennemi | cinq fragments, origine (232,55) | départ visible (229,43), **courbe continue** |
| Gerbe | blanche, anguleuse et isolée | mousse et gerbes courbes aux couleurs de l'eau |
| Commandes | rectangles bleu-noir, texte pixel rigide | plaques cuivrées semi-transparentes, texte proportionnel |
| PALMARÈS | 23 px | **27 px** |
| Titre | bande noire | lettrage direct sur ciel |

## Livrables
- `accueil_cyd_warship_320x240.png` : PNG natif 320×240.
- `accueil_cyd_warship_x4.png` : zoom 1280×960 **nearest-neighbor**, chaque pixel répliqué 4×4.
- `accueil_cyd_warship.svg` : fond raster **embarqué** non éditable en vectoriel ; surimpressions organisées par groupes éditables (forme et texte). Les moteurs SVG peuvent avoir un anti-aliasing légèrement différent ; **le PNG natif fait foi**.
- `README.md` : présente note et zones tactiles.

**Limites** : prévisualisation PC, pas de build firmware, pas de mesure Flash/RAM runtime, pas de test matériel CYD ni validation PO présumée. PR à conserver brouillon et non fusionnée. Aucun ajout hors GRAPH-ACC-001.
