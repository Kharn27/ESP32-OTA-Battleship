# PORTRAITS-04 — J3 / Issue #133 — rapport de contrôle du Graphiste

## Périmètre
- **Base exacte :** main@66fc9ddeae3625206cfc2125223ea72d11175b61.
- **8 nouvelles positions :** A-L1-C4, A-L2-C3, B-L1-C4, B-L2-C2, C-L1-C2, C-L1-C3, D-L1-C1, E-L1-C1.
- **Aucun chevauchement** avec les 21 positions fusionnées de J0/J1/J2. Couverture après acceptation Pilotage : **29/68**, **39 restantes**.
- **40 PNG dédiés aux 8 positions** : 8 masters HD, 16 exports (48/64), 16 zooms ×4 NEAREST ; et **5 PNG transversaux** : 4 composites natifs 320×240 (2 pages × fond clair/sombre) + contact 1120×182.
- Scripts et métadonnées uniquement sous `draft/portraits/exports/`, aucun changement autorisé de PO, assets, maquettes ou firmware.

## Contrôle de traitement
- Crops des huit positions depuis les cinq planches A–E (dimensions A/B 1448×1086, C/E 1448×1086, D 1536×1024), chacune avec cadrage et marge haute spécifiés dans le manifeste.
- Export **48×48 RGBA** et **64×64 RGBA** calculés indépendamment du **même master HD**, intégration BOX pondérée par l'aire et alpha prémultiplié, nombres entiers ; jamais de 48→64.
- Export PNG sans métadonnées, zlib stored et filtres PNG 0 pour rendre les résultats comparables octet par octet. Zoom ×4 sans interpolation.
- Contrôle de calibrage réellement effectué sur `A-L1-C2` (J1) : découpe Adobe normalisée contre master GitHub existant **mêmes RGBA et mêmes octets PNG** ; export BOX J3 reproduit aussi exactement les octets du 48px J1.
- Chaque sprite J3 48px présente **une seule composante connectée au seuil alpha ≥ 6** dans le contrôle Graphiste ; les 8 versions zoom 48 ont été inspectées visuellement et les tailles 64 sur contact et composites natifs.
- Composites natifs clair/sombre sur **320×240**, y compris états 48 et 64 ; contact global avec les deux fonds à tailles natives.
- **Alpha à 255 : 0 pixel sur tous les 16 exports J3**, comme les jalons antérieurs : réserve matérielle d'opacité apparente maintenue.

## Nettoyages locaux et précautions
- `A-L1-C4` : rectangle master demi-ouvert `[0,80,31,258 )`, 1 pixels alpha>0 supprimés ; frange latérale étrangère, sujet central préservé.
- `B-L2-C2` : rectangle master demi-ouvert `[349,75,362,323 )`, 204 pixels alpha>0 supprimés ; supprime la pollution de cellule voisine à droite sans couper la coiffe.
- `C-L1-C2` : rectangle master demi-ouvert `[0,50,10,210 )`, 137 pixels alpha>0 supprimés ; débordement marginal de la cellule voisine, tricorne préservé.
- `B-L1-C4` : masque large écarté après inspection de la découpe et du 48px ; la version retenue préserve le manteau et ne présente pas de composante parasite détachée.
- `A-L2-C3` : ancien essai de masque sans pixel alpha>0 supprimé ; retiré de la recette (aucun effet visuel).
- Les originaux PO et toutes les 21 positions antérieures restent inchangés dans le diff. Zéro déduplication G/Q ; aucun rôle canonique.

## Reproduction et preuves — à exécuter sur checkout complet du HEAD J3
```bash
python3 -m pip install Pillow==12.3.0
python3 draft/portraits/exports/export_jalon0.py --check
python3 draft/portraits/exports/export_jalon1.py --check
python3 draft/portraits/exports/export_jalon2.py --check
python3 draft/portraits/exports/export_jalon3.py --check
python3 scripts/check_graphic_pipeline.py
git status --short
```
Le mode `--check` est strictement en lecture seule : compare les PNG committés aux octets régénérés et vérifie les SHA-256 de tous les originaux PO. **Cette exécution intégrale J0–J3 n'a pas été effectuée ici** (pas de checkout Git complet utilisable depuis l'environnement de travail). Elle relève de la QA indépendante Pilotage sur le HEAD exact. La CI de la nouvelle PR est aussi à constater après ouverture, sans anticiper son résultat.

## Statut et gouvernance
**CANDIDATE — ON_DEVICE_UNVERIFIED**. La revue graphique sur PC n'est ni un GO artistique individuel du PO, ni une vérification physique sur CYD. **Aucune promotion dans `assets/` ni fusion par le Graphiste.** Pilotage doit examiner le HEAD J3 exact et décider de l'acceptation technique.
