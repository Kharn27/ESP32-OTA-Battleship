# PORTRAITS-01 — pilote unique A / ligne 1 / colonne 1

**Issue :** #124 · **statut artistique : CANDIDATE, en attente du PO** · **matériel : ON_DEVICE_UNVERIFIED**.

## Sélection et provenance

- Origine PO : [`draft/portraits/portrait_A.png`](../portrait_A.png), 1448 × 1086, RGBA.
- SHA-256 original selon l'archive contrôlée de la PR #125 : `30e92fa1eed23733d812a521c6910390860620f906a0f3529e8410abe0bd9acc`.
- Emplacement pilote : **A-L1-C1**, haut gauche de la planche A (casquette marine, barbe blanche, uniforme rouge/or).
- Rect exact en pixels source : `x=0, y=49, largeur=362, hauteur=362`, soit la zone `[0,49,362,411)`.
- Ce portrait n'est **pas** attribué automatiquement au CPU Capitaine.
- Les cinq originaux PO demeurent immuables sur `main`. Cette branche n'en modifie aucun.

## Livrables de travail (pas des assets firmware)

| Fichier | Objet | Dimensions |
| --- | --- | --- |
| `source_HD_coupe.png` | Recadrage RGBA exact, sans retouche | 362 × 362 |
| `source_HD_nettoyee.png` | Copie de travail : 10 pixels vert-lime parasites rendus transparents, pixels invisibles normalisés | 362 × 362 |
| `pilote_32.png` | Hypothèse taille 32, réduction prémultipliée | 32 × 32 |
| `pilote_48.png` | Hypothèse taille 48 | 48 × 48 |
| `pilote_64.png` | Hypothèse taille 64 | 64 × 64 |
| `comparatif_320x240.png` | Planche PC native, trois tailles, fonds clair et sombre | 320 × 240 |
| `comparatif_320x240_x4_nearest.png` | **PNG réel ×4** : agrandissement par répétition exacte des pixels, sans interpolation | 1280 × 960 |
| `comparatif_x4_nearest.svg` | Agrandissement ×4 alternatif (natif intégré avec `image-rendering:pixelated`) | 1280 × 960 (affichage SVG) |
| `export.py` | Recette Python/Pillow de régénération **ou vérification non destructive `--check`** depuis l'original GitHub contrôlé par SHA-256 | Source éditable |
| `qa_repro_report.json` | Mesures sur GitHub Actions : comparaison RGBA et octets des exports régénérés versus les exports committés | Preuve de QA |

**Reprise du 12/11/2026 :** la première livraison utilisait pour les réductions une représentation Adobe et une recette de diminution non identique au filtrage Pillow. La vérification directe depuis l'original GitHub a quantifié **717 / 1 563 / 2 785 pixels différents** respectivement en 32/48/64. Les **sept PNG candidats et de comparaison ont été remplacés** par les rendus issus de `export.py` exécuté directement dans GitHub Actions contre le véritable `portrait_A.png` archivé (SHA-256 vérifié). **Deuxième passe : 0 pixel et 0 octet de différence sur les sept PNG** ; preuve [run #43259993102](https://github.com/Kharn27/ESP32-OTA-Battleship/actions/runs/43259993102), rapport `qa_repro_report.json`. Le script permet maintenant un contrôle pixel à pixel non destructif via `--check`.

**Contrôle du détourage / consigne PO :** les **10 pixels vert-lime** ciblés dans la copie de travail ont été inventoriés individuellement sur l'original (alpha = **1 à 3/255**, tous voisins de transparence à 2 px). Ce sont des pixels quasi invisibles de frange, non des aplats ni détails lumineux établis. Aucun autre contour coloré ou reflet artistique du portrait n'a été retouché. Les trois `pilote_*.png` conservent le **mode RGBA et l'alpha transparent**, sans fond incrusté ; fonds clair/sombre exclusivement dans les comparatifs de QA. Les PNG de la PR restent **candidats PC**, sans validation PO ni essai matériel.

## Contrôle artistique avant choix

- **32 px :** silhouette reconnaissable, regard et petits détails rapidement perdus.
- **48 px :** compromis visuel plausible à tester.
- **64 px :** meilleure lecture de la casquette, du visage et de la barbe ; occupation écran plus grande.
- Le cadrage ne masque ni la coiffe ni la barbe. L'alpha est conservé ; contrôler les franges sur un écran TFT réel.
- Contraste simultanément montré sur fond sombre et clair. Le test est une **planche PC**, pas une maquette d'écran du jeu ni une mesure de confort tactile.
- Aucune taille recommandée ici ne vaut contrat mémoire/format de production. Le DEV doit mesurer depuis Flash sur vraie CYD.

## Reproduction

Depuis la racine du dépôt (Python 3 + Pillow) :

```sh
python3 -m pip install Pillow
python3 draft/portraits/pilote_A_L1_C1/export.py --check  # compare sans modifier les fichiers
# Regénération volontaire :
python3 draft/portraits/pilote_A_L1_C1/export.py
python3 scripts/check_graphic_pipeline.py
```

La recette valide d'abord le SHA-256 de la planche originale, génère (ou, avec `--check`, compare pixel par pixel sans écriture) le recadrage, la copie nettoyée, les trois hypothèses, la planche native et `comparatif_320x240_x4_nearest.png` (rendu **réel**, sans interpolation). Elle imprime les SHA-256 de chaque sortie régénérée. Le PNG ×4 est également livré dans cette PR ; le SVG reste une variante d'affichage. Le redimensionnement ×4 du PNG livré a été construit par copie des pixels exacts, sans interpolation.

## Gouvernance

- Toutes les propositions restent **dans `draft/`**, sans ajout à `assets/manifest.json` et sans `maquettes/`.
- **PO :** doit valider le candidat et la lisibilité artistique ; aucun GO n'est présumé.
- **Pilotage :** contrôle le HEAD, la preuve et la PR Draft ; fusion exclusivement par Pilotage.
- **Développeur :** essai ultérieur `TEST-ONLY` sur ESP32-2432S028R, écran 320 × 240, sans PSRAM/microSD, puis photo, logs et mesures.
- **Statut :** `ON_DEVICE_UNVERIFIED` jusqu'à preuve physique. Aucun export en série ni intégration runtime.

Refs #124.
