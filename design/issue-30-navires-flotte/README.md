# G02 — Les cinq navires classiques (Issue #30)

**Responsable :** Graphiste / DA. **Statut : DESIGN_PREVIEW — ON_DEVICE_UNVERIFIED.** Proposition unique, sans validation PO de G02.

## Livrables GitHub : sources SVG vectorielles éditables

- [Planche complète](g02_planche_flotte.svg) : cinq profils distincts, vue de dessus, tailles 2/3/3/4/5.
- [Test de grille 320×240](g02_grille_320x240.svg) et [agrandissement ×4](g02_grille_x4.svg) : grille 10×10, 18 px/case provisoire.
- [États du Croiseur](g02_etats_320x240.svg) et [×4](g02_etats_x4.svg) : intact, partiellement touché, coulé, **uniquement sur DÉFENSE du joueur**.
- [Orientations](g02_orientations_320x240.svg) et [×4](g02_orientations_x4.svg) : Patrouilleur 2 et Porte-avions 5, H et V.

Les SVG contiennent des éléments géométriques éditables, aucune police intégrée, aucune image tierce et des couleurs issues du socle G01. **Exports versionnés au format SVG** : les copies PNG raster doivent être générées localement via `export_previews.py` (CairoSVG + Pillow) ; les PNG ne sont pas présents dans cette branche. Pour les fichiers PNG vérifiés en conversation, consulter l'archive de remise distincte. Ils sont des **sources de présentation**, pas des sprites firmware validés. L'export PNG 320×240 et zoom 1280×960, accompagné du code auteur Pillow et du contrôle d'occupation, est fourni séparément au Pilotage dans la remise G02.

## Reproduction des exports et vérification géométrique

```sh
python3 -m pip install cairosvg pillow
python3 design/issue-30-navires-flotte/validate_fleet.py
python3 design/issue-30-navires-flotte/export_previews.py
```

`validate_fleet.py` contrôle 5 navires, 17 cellules distinctes, orientations mixtes, contact orthogonal et diagonal. `export_previews.py` génère les PNG 320×240 et 1280×960 par agrandissement **NEAREST**, ainsi que la planche raster. **Les tests de géométrie ont été exécutés sur PC ; les exports des SVG du dépôt sont reproductibles mais n'ont pas été exécutés sur CYD.**

## Épreuve de contact sans superposition

| Bâtiment | Orientation | Cases |
|---|---|---|
| Porte-avions (5) | H | B2–F2 |
| Sous-marin (3) | H | C6–E6 |
| Contre-torpilleur (3) | V | F5–F7 |
| Croiseur (4) | V | I4–I7 |
| Patrouilleur (2) | H | G8–H8 |

Total **17 cases distinctes** ; Sous-marin/Contre-torpilleur **par côté**, Croiseur/Patrouilleur **en diagonale**. Les coques et leurs bords restent séparés par la marge interne de chaque case. Le contraste matière/formes distingue les deux 3 cases.

## Direction artistique

Socle G01 accepté par le PO : **« la mer est le plateau »**. Mer continue turquoise, reflets organiques, cuivre et ambre du crépuscule, contours d'encre froide, même lumière de dessus pour chaque coque. Patrouilleur = vedette courte ; Sous-marin = fuseau/kiosque ; Contre-torpilleur = proue fine, deux tourelles ; Croiseur = trois groupes d'artillerie et passerelle ; Porte-avions = long pont, piste et îlot asymétrique.

## Portée, preuves et questions ouvertes

**PROPOSÉ :** formes de coques, détails, style des impacts et contraste. **VALIDÉ PO :** identité artistique générale G01 seulement ; **G02 en attente d'avis PO**. **NON TESTÉ SUR CYD :** reconnaissance réelle à 18 px, stylet, rendu TFT, tactile, fontes, Flash/RAM/temps d'affichage. T02/S02 et Développeur fixeront formats, compression, masques et budgets. **Aucune règle métier ni écran complet modifié.** Ce dossier ne révèle jamais la flotte cachée de Capitaine sur ATTAQUE. La forme en croix des dommages coulés n'est qu'une hypothèse de représentation de la flotte propre, pas une annonce de l'épave ennemie.

Provenance : dessin original G02, sans image ni police tierce distribuée. Le texte des SVG utilise une fonte sans-serif hôte comme substitut de prévisualisation.

Références : [Issue #30](https://github.com/Kharn27/ESP32-OTA-Battleship/issues/30), [G01](../issue-16-direction-lot1/CONTRAT_GRAPHIQUE.md), [Bible](../../BIBLE_PRODUIT.md), [politique de test](../../docs/TEST_POLICY.md). Refs #30, **pas Closes #30**.
