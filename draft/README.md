# I-009 / PR #76 — Deux fonds océaniques PO (DRAFT)

Le PO a fourni **deux images sources distinctes** (A et B) le 06/11/2026. Les références ici sont des réencodages JPEG de transfert, **sans création visuelle**, pas une reconstruction vectorielle. Ne pas revendiquer une identité binaire avec les originaux.

## Références mises à disposition
- `ocean_A_reference_1195x896.jpeg` : référence A du PO.
- `ocean_B_reference_1195x896.jpeg` : référence B du PO.
- `ocean_A_320x240.jpeg` et `ocean_B_320x240.jpeg` : prévisualisations natives immédiates.

## Quatre exports PNG demandés
Sur la branche de PR #76 : `python3 draft/export_ocean.py` (Python 3, Pillow) génère :
- `draft/ocean_A_320x240.png`, `draft/ocean_B_320x240.png`
- `draft/ocean_A_x4.png`, `draft/ocean_B_x4.png` (1280 × 960, **NEAREST exact**, 1 pixel = 4 × 4)
Le script assure les contrôles de dimensions, pixels répliqués et SHA-256.

## Consigne PO
**Adapter ces deux images existantes. C'est tout.** Aucun SVG de substitution, texture procédurale, redraw, retouche de palette, nouvelle direction artistique, grille ou interface. Le dossier `design/ocean/` de la PR #76 actuelle est une proposition hors sujet : ses huit fichiers sont à retirer du diff. Les quatre PNG calculés depuis ces références deviennent le seul livrable graphique de cette correction. Les JPEG de transfert sont des références, non des assets firmware homologués. Soumettre le nouveau HEAD exact à la validation du PO et à la contre-revue du Pilotage ; ne pas fusionner.
