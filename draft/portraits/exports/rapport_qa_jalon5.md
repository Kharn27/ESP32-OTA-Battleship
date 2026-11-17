# PORTRAITS-04 — Jalon 5 / Issue #133 — Rapport Graphiste

## Référence / couverture

Base : `main@10319362ee975658326b78bc689712392093c2ac`. Branche `design/issue-133-jalon5-huit-portraits`. Huit IDs J5 : C-L1-C4, C-L3-C3, C-L3-C4, D-L1-C4, D-L2-C1, D-L2-C4, E-L1-C3, E-L2-C1. Couverture fusionnée avant J5 = **37/68** (74 variantes). **45/68 seulement après fusion Pilotage**, alors **23 positions resteraient**.

Livrables : 8 master HD, 16 PNG RGBA 48×48/64×64, 16 zooms ×4 nearest sans interpolation, 4 preuves 320×240 natives clair/sombre, un contact 1120×182, recette Python, manifeste SHA-256 et ce rapport. **48 fichiers exclusivement `draft/portraits/exports/`** ; ne change ni originaux, ni J0–J4, ni `assets/`, ni `maquettes/`, ni firmware, ni Bible.

## Extraction et normalisation

Sources C/D/E Adobe natives et parité confirmée à l'octet sur trois masters J4 existants : C-L2-C4, D-L1-C2, E-L1-C2. Découpes exactes, carrés HD et décalages transparents par ID dans le manifeste. Master RGBA avec nettoyage RGB derrière alpha=0 ; BOX entier alpha prémultiplié ; 48 et 64 dérivés indépendamment du master, **jamais upscale 48→64** ; x4 = nearest. Portraits visibles sur deux fonds, lumières préservées ; aucune correction globale de colorimétrie.

## Retouche locale unique

- `D-L2-C4` : `[300,80,307,307)` en coordonnées du master 307×307 ; masque `all` ; **583 pixels alpha>0 supprimés** pour les éclats de la cellule voisine à droite. **0 pixel/canal modifié hors rectangle**. Avant masque : trois composantes alpha>=6 en 48 ; après : une seule.
- Les 7 autres ne reçoivent **aucun masque** ; coiffes, cheveux, bijoux, barbe et fumée conservés. Exceptions de cadrage par ID dans `manifest_jalon5.json`.

## Contrôles réalisés

- 8/8 sprites 48 et 8/8 sprites 64 possèdent une seule composante alpha>=6 ; panneaux PC de quatre groupes 320×240 clair/sombre inspectés, comparatif complet et zooms x4.
- 24 sorties masters/variants + 21 preuves en PNG réellement générés, SHA256 et taille calculés sur ces mêmes octets transmis à GitHub.
- La recette `export_jalon5.py --check` est lecture seule : reconstitue chaque PNG et compare **octet à octet**, puis vérifie **SHA-256 réel, format 64 caractères hexadécimaux et taille réelle** des 45 fichiers référencés dans le manifeste ; vérifie aussi les 5 SHA originaux PO sur checkout intégral.
- **À faire par Pilotage** : exécuter `--check` J0–J5 et `scripts/check_graphic_pipeline.py` sur checkout complet du HEAD ; vérifier CI, hashes des 45 preuves et diff. Aucune exécution locale full checkout ni preuve on-device revendiquée.

## Vérification indépendante (lecture seule)

```bash
python3 -m pip install Pillow==12.3.0
for stage in 0 1 2 3 4 5; do python3 draft/portraits/exports/export_jalon${stage}.py --check || exit 1; done
python3 scripts/check_graphic_pipeline.py
git status --short
```

**Statut : CANDIDATE — ON_DEVICE_UNVERIFIED.** Aucun GO artistique individuel du PO ni promotion `assets/`. Pilotage seul revoit le HEAD et fusionne.
