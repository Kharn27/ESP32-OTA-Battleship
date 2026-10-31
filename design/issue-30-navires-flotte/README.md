# G02 — Design des cinq navires classiques (Issue #30)

**Responsable :** Graphiste / DA · **Statut :** DESIGN_PREVIEW — ON_DEVICE_UNVERIFIED · **Avis artistique PO G02 :** en attente.  
**Branche / PR :** `design/issue-30-navires-flotte` · [PR Draft #32](https://github.com/Kharn27/ESP32-OTA-Battleship/pull/32) · Refs #30 (pas Closes).

## Les livrables canoniques dans le dépôt

| Vue | Source SVG éditable | Export PNG natif | Preuve PNG zoom x4 NEAREST |
|---|---|---|---|
| Flotte complète | [Planche 960×560](g02_planche_flotte.svg) | [Planche PNG 960×560](g02_planche_flotte.png) | Non applicable (planche non 320×240) |
| Grille joueur 10×10 | [SVG 320×240](g02_grille_320x240.svg) | [PNG 320×240](g02_grille_320x240.png) | [PNG 1280×960](g02_grille_x4.png) |
| Trois états Croiseur, joueur DÉFENSE | [SVG 320×240](g02_etats_320x240.svg) | [PNG 320×240](g02_etats_320x240.png) | [PNG 1280×960](g02_etats_x4.png) |
| Orientation Patrouilleur et Porte-avions | [SVG 320×240](g02_orientations_320x240.svg) | [PNG 320×240](g02_orientations_320x240.png) | [PNG 1280×960](g02_orientations_x4.png) |

**Chaîne normative unique :** SVG versionné → rasterisation PNG **320×240** (CairoSVG) → agrandissement **1280×960 depuis le PNG natif** (Pillow, `Image.Resampling.NEAREST`). Les SVG `*_x4.svg` sont des commodités vectorielles historiques, **pas** des preuves d'agrandissement exact. La planche et les images générées par l'**ancien script Pillow autonome** transmis en archive lors de la première remise sont des **explorations NON NORMATIVES**, pas des références de comparaison ni des sources de production.

Ces sources sont éditables et relèvent exclusivement du **concept PC** ; elles ne sont pas des sprites finaux destinés à la Flash.

## Contrôle reproductible des vrais placements SVG

Le validateur [validate_fleet.py](validate_fleet.py) lit chaque balise `<use href="#nav-*">` dans le **SVG de grille effectivement rendu**, collecte ses `data-col/data-row/data-length/data-orient`, puis vérifie la transformation `translate` et `rotate(-90)` de la coque dessinée. Il vérifie indépendamment la grille native et son SVG x4, et refuse toute divergence des positions :

| Navire | Orientation | Emprise attestée dans les SVG |
|---|---|---|
| Porte-avions 5 | horizontale | B2–F2 |
| Sous-marin 3 | horizontale | C6–E6 |
| Contre-torpilleur 3 | verticale | F5–F7 (translate **109,165**) |
| Croiseur 4 | verticale | I4–I7 (translate **163,165**) |
| Patrouilleur 2 | horizontale | G8–H8 |

**17 cases distinctes.** Contact **par côté** Sous-marin / Contre-torpilleur (E6/F6) et **en diagonale** Patrouilleur / Croiseur (H8/I7). Pas de superposition. Les silhouettes verticales restent dans la colonne identifiée, grâce à la transformation cohérente avec la grille de 18 px/case (uniquement banc d'essai graphique).

## Génération et tests PC

Depuis la racine du dépôt :

```sh
python3 -m pip install 'CairoSVG==2.7.1' 'Pillow==11.3.0'
python3 design/issue-30-navires-flotte/validate_fleet.py
python3 design/issue-30-navires-flotte/export_previews.py
```

`export_previews.py` contrôle **la taille réelle 320×240**, **1280×960**, **l'égalité exacte du raster agrandi à l'image native redimensionnée en NEAREST**, et un échantillon correspondant pour chaque pixel natif. Les sept PNG sont **versionnés dans cette PR**.

**Preuve exécutée :** [GitHub Actions G02, run #41254090648 — PASS](https://github.com/Kharn27/ESP32-OTA-Battleship/actions/runs/41254090648), contrôle des vrais `<use>` et génération/commit des PNG, avec logs détaillés. Cette vérification graphique ne fait **pas** partie des jobs hôte/ESP32 génériques.

## Direction artistique et limites

Identité G01 acceptée par le PO : **« La mer est le plateau »**, mer turquoise continue, reflets organiques, cuivre et ambre, encrage froid, éclairage cohérent. Une proposition principale de cinq coques distinctes : Patrouilleur = vedette courte ; Sous-marin = fuseau avec kiosque ; Contre-torpilleur = étrave/tourelles ; Croiseur = plusieurs groupes d'artillerie ; Porte-avions = longue piste et îlot asymétrique. Les deux modèles de trois cases ne reposent pas sur la seule couleur.

**Proposé, NON validé G02 :** personnalité, détails, lisibilité réelle, impact et état coulé. Les états de dommages illustrent **la propre flotte du joueur** (DÉFENSE), sans silhouette/épave de Capitaine sur ATTAQUE. **Non testé sur CYD :** stylet, contraste TFT, formats, masques, Flash/RAM, latence, police. T02/S02 et le Développeur établiront l'interface runtime sans présumer d'un budget ou d'un format.

**Provenance :** dessin original G02, sans image tierce ni police embarquée livrée ; fonte sans-serif hôte comme substitution de preview. **Aucun changement métier, firmware, accueil ou G01.**

Références : [Issue #30](https://github.com/Kharn27/ESP32-OTA-Battleship/issues/30), [revue Pilotage](https://github.com/Kharn27/ESP32-OTA-Battleship/pull/32#pullrequestreview-5724899315), [contrat G01](../issue-16-direction-lot1/CONTRAT_GRAPHIQUE.md) et [politique de preuve](../../docs/TEST_POLICY.md).
