# PORTRAITS-04 — J4 / Issue #133 — rapport Graphiste

## Périmètre et référence
Base : main@c280e5b5818115de30852b3d202c1f2456a73f3e. Branche design/issue-133-jalon4-huit-portraits.
8 nouveaux IDs : A-L2-C4, B-L2-C3, C-L2-C4, C-L3-C2, D-L1-C2, D-L1-C3, E-L1-C2, E-L2-C2. 29/68 déjà fusionnés ; 37/68 si acceptation/fusion par Pilotage ; 31 restants après.
Livrables : 8 masters HD, 16 RGBA 48x48/64x64, 16 zooms NEAREST x4, 4 composites 320x240 clair/sombre et 1 planche-contact 1120x182.
Tous les fichiers ajoutés sont dans draft/portraits/exports/. Aucun original PO, asset promu, maquette ou firmware modifié.

## Pipeline / source
Extraction depuis les cinq planches PO via crop HD documenté et création d'un master RGBA par ID. Le 48 et le 64 proviennent indépendamment du même master ; BOX aire entière + alpha prémultiplié ; RGB caché derrière alpha=0 nettoyé.
Algorithmes éprouvés réutilisés depuis export_jalon3.py et connector_jalon3.js (pas de seconde implémentation générale).
Étalon de contrôle : A-L1-C2 recalcule le même SHA256 que J1 sur master HD, 48 et 64 depuis la source Adobe native (parité des octets).
Valeurs SHA256 des cinq originaux PO documentées mais non recalculées dans la session connecteur : recomptage impératif sur checkout complet.

## Corrections locales
- A-L2-C4: rect [0,180,130,300), mode red, removed alpha>0 856; outside ROI 0 changed channels
- C-L3-C2: rect [0,45,145,130), mode red, removed alpha>0 740; outside ROI 0 changed channels
- D-L1-C3: rect [300,108,307,240), mode all, removed alpha>0 581; outside ROI 0 changed channels
Pour la variante D-L1-C3, fenêtre de 307px décalée de 1px à droite dans une cellule source de 308px ; pas d'étirement.
Les autres positions restent sans masque ; ornements, chevelures et drapés préservés. Exceptions par ID dans manifest_jalon4.json.

## Contrôles
Inspectés sur PC : 8 zooms 48×4, les 4 aperçus natifs 320×240 (clair/sombre), 8/8 connexités alpha>=6 = une seule composante principale.
Les empreintes et tailles du manifeste ont été calculées sur les octets PNG émis pour les 24 images master/export et les 21 preuves.
IMPORTANT : la recette export_jalon4.py --check est non destructive et compare les fichiers produits octet par octet ; elle valide aussi le FORMAT 64 hex ET le SHA256/taille RÉELS de 100% des fichiers déclarés (y compris preuves) sur disque.
Exécution checkout complet indépendante, CI du HEAD et scripts graphiques : NON RÉALISÉES dans cette session ; Pilotage les vérifie contradictoirement.

## Reproduction sur checkout exact
python3 -m pip install Pillow==12.3.0
for stage in 0 1 2 3 4; do python3 draft/portraits/exports/export_jalon${stage}.py --check || exit 1; done
python3 scripts/check_graphic_pipeline.py
git status --short

## Statut
CANDIDATE — ON_DEVICE_UNVERIFIED. Aucun GO artistique individuel PO ; aucune promotion assets/ ni test physique. Pilotage seul examine et fusionne.
