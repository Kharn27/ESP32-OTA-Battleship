# GRAPH-ACC-001 — Correction de composition pour revue PO

**Statut :** `DESIGN_PREVIEW — PO_ARTISTIC_APPROVAL_PENDING — ON_DEVICE_UNVERIFIED`.

## Continuité imposée par le retour PO
Cette révision corrige le rejet artistique de la PR #8. La **référence obligatoire** est la version narrative originale `01_bataille_narrative_320x240.png` à l'arbre Git historique `872496d73a150dd5a016c219973e5865576b7e04`, fichier `assets/explorations/2026-10-29-bataille-en-cours/01_bataille_narrative_320x240.png`. La scène photographique/illustrée originale est **reprise au pixel près comme fond**, sans recadrage, recoloration, reconstruction du navire, de l'ennemi, des tirs existants, du ciel ou de la mer. Elle ne devient pas pour autant un asset firmware validé. La version historique 02 plus pixelisée n'est pas supprimée, et le degré de pixelisation n'est **pas arbitré**.

### Adaptations minimales
- Gerbe de riposte **dans l'eau à droite de la proue** (autour de x=273, y=180), séparée du navire. Trajectoire secondaire orangée issue de la zone de l'ennemi lointain ; le tir ami initial reste intact.
- Titre **CYD WARSHIP**, discret en haut à gauche, avec bande fine de contraste.
- Deux boutons sur l'eau en bas-gauche, sans panneau commun : `JOUER CONTRE MOUSSE` (deux lignes, exactement le même libellé) et `PALMARÈS`.
- Aucun aéronef additionnel ; les silhouettes et autres détails **déjà présents dans la référence** sont conservés.

### Rectangles tactiles pressentis (hypothèses, non contrat firmware)
- JOUER CONTRE MOUSSE : `x=9,y=176,w=164,h=33`.
- PALMARÈS : `x=9,y=213,w=132,h=23`.
Deux zones sans chevauchement, espacées de 4 pixels ; boutons au stylet à confirmer **uniquement après validation esthétique PO**.

### Livrables et nature des sources
- `accueil_cyd_warship_320x240.png` : rendu final natif exact 320 × 240, composition originale et surimpression.
- `accueil_cyd_warship_x4.png` : agrandissement 1280 × 960 **nearest-neighbor strict**.
- `accueil_cyd_warship.svg` : source éditable SVG autonome ; la couche de fond est un PNG historique **embarqué sans modification**, les superpositions sont des formes pixel indépendantes. Il s'agit d'une source éditable pour les ajouts, **pas** d'une vectorisation éditable de l'illustration historique. Aucune archive A–F n'est restaurée sur `main`.

### Avant/après de la COMPOSITION (et non exploration artistique)
| Sujet | Scène de référence (AVANT) | Correction de l'accueil (APRÈS) |
|---|---|---|
| Cadrage, ciel, soleil, montagnes, ambiance | composition historique | **identique au pixel près hors surimpressions** |
| Navire principal, ennemi endommagé, tirs existants | présents | **inchangés** |
| Riposte manquée | non explicitement visible | trace secondaire et gerbe à l'eau près de la proue |
| Titre officiel | absent | bande discrète et titre supérieur |
| Accès LOT 1 | absents | deux seules actions superposées sur l'eau |
| Aéronefs facultatifs | éléments existants seulement | aucun aéronef nouveau |
| Pixelisation | originale, choix PO ouvert | fond original **sans transformation de grain** |

**Ne pas traiter le rendu comme validé par le PO.** Une fois le rendu artistique accepté, prévoir séparément un contrôle CYD physique de la lisibilité et du stylet. Aucun firmware n'a été modifié ou testé. Pas de mesure RAM/Flash runtime (PNG de revue, pas format firmware).
