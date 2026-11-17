# BTN-ABD-01 — Étude ABANDON (Issue #149)

**Statut : CANDIDATE / ART_NOT_REVIEWED / ON_DEVICE_UNVERIFIED.**

## Provenance des pixels

- Original PO immuable sur `main` : `draft/Bouton_Abandon.png`, 1 814 386 octets, SHA-1 Git `3818cb092456e1ff222d8b2dacf9f9aa55266135` (préservé via PR #152). **Il n'a pas été modifié.**
- Entrée réelle utilisée pour cette prévisualisation locale : fichier image joint à la conversation le 17/11/2026, 2048×682 RGBA, 1 870 797 octets, SHA-256 `2637ff5eb00d42f427a07fa0a25baffd7b95c77566578d6e223ac8ecb6533690`, Git blob `80a68ede2a2ee06656c4cb0bb4aedc6bd4a3fb3d`.
- **Différence d'octets constatée :** ce fichier joint n'est PAS l'original Git bit pour bit. Il sert de référence visuelle fidèlement cadrée, sans prétention d'égalité pixel à pixel avec l'original Git non lisible via le connecteur. Les exports inclus reflètent cette pièce jointe, et non une version récupérée de force depuis GitHub.
- Tous les détails de palette, lettrage, rivets, drapeau et relief sont ceux de la pièce jointe ; aucune recoloration ni redessin.

## Construction non destructive

- Zone opaque significative (alpha > 1) du fichier joint : `(29,94) .. (2019,587)`.
- Marges d'origine essentiellement transparentes, notamment en haut/bas ; quelques pixels alpha=1 épars à l'extérieur de la silhouette.
- Découpe de conservation `(24,90,2024,590)` : master **2000×500**, ratio **4:1**. Tous les pixels de la silhouette (alpha > 1) sont conservés ; aucune dilatation/déformation.
- Exports : **140×35** et **112×28** RGBA, réduction `LANCZOS` sur pixels alpha prémultipliés pour éviter les franges sombres. Les couleurs du drapeau et des lettres ne sont pas retouchées.
- Zoom ×4 : strict `NEAREST`, sans création de détail ou interpolation.
- Mini-planche 320×240 : **étude sur fonds clair/sombre**, ce n'est **pas** une maquette d'écran et cela ne vaut pas implantation tactile validée.

**Disponibilité sur cette branche GitHub :** le script et les notes sont présents ; les PNG ci-dessous ont été produits et contrôlés localement à partir de l'upload, mais les octets binaires ne sont pas encore transférés dans cette PR. Cette PR ne constitue donc pas encore la livraison finale de #149. Les vrais PNG sont disponibles dans le ZIP remis au PO ; la génération depuis l'original Git reste à exécuter et à vérifier. **Ne pas valider cette étude comme terminée.**

## Fichiers (sorties générables, en attente de publication dans la PR)

- `exports/abandon_master_2000x500_RGBA.png`
- `exports/abandon_140x35_RGBA.png`
- `exports/abandon_112x28_RGBA.png`
- `preuves/etude_abandon_320x240.png`, `preuves/etude_abandon_1280x960_NEAREST.png`
- `preuves/abandon_140x35_x4_NEAREST.png`, `preuves/abandon_112x28_x4_NEAREST.png`
- `generate.py`, script régénérable, `--check` en lecture seule
- `rapport.json` : SHA-256, poids, dimension, mode et transparence de chaque export

## Procédures

Dépendance : Pillow. **Source Git exacte (après checkout)** :

```bash
python3 -m pip install Pillow
python3 draft/etudes/bouton_abandon/generate.py --repo . --input draft/Bouton_Abandon.png --out draft/etudes/bouton_abandon
python3 draft/etudes/bouton_abandon/generate.py --repo . --input draft/Bouton_Abandon.png --out draft/etudes/bouton_abandon --check
python3 scripts/check_graphic_pipeline.py
```

Le script vérifie le Git blob de l'original ; la recette refuse une source non conforme ou des dimensions inattendues. Sur **la pièce jointe**, la vérification a été réellement exécutée ainsi :

```bash
python3 generate.py --input /mnt/data/image.png --allow-reference --repo /mnt/data --out /mnt/data/BTN-ABD-01
python3 generate.py --input /mnt/data/image.png --allow-reference --repo /mnt/data --out /mnt/data/BTN-ABD-01 --check
# CHECK PASS: 7 PNG + rapport.json; NO FILES WRITTEN
```

**Attention :** l'original Git n'ayant pu être téléchargé ici, le lancement sur les octets Git et son résultat `--check` demeurent **À VÉRIFIER** ; ne pas extrapoler l'exécution réalisée sur l'upload.

## Comparatif avec les boutons approuvés

Le script détecte, dans un checkout réel, les fichiers existants suivants, et produit **en plus** une planche côte à côte 320×240 (fonds clair/sombre) avec les vrais boutons dans le même ordre :

- `assets/boutons/bouton_jouer_CYD_140x32_RGBA.png`
- `assets/boutons/bouton_palmares_CYD_140x30_RGBA.png`
- `assets/boutons/attaquer_on_140x35_RGBA.png`
- `draft/etudes/bouton_abandon/exports/abandon_140x35_RGBA.png` (candidat)

Les autres éléments approuvés **ne sont pas disponibles en bytes dans ce conteneur local**. En conséquence les planches `comparatif_approuves_*` ne sont **pas** incluses dans le paquet local : leur fabrication est prévue après checkout Git exact et reste **non vérifiée** ici. Aucun visuel approuvé n'a été reconstitué approximativement.

## Avis de direction artistique et gouvernance

À **140×35**, drapeau, rivets et « ABANDON » sont lisibles dans une planche 320×240. À **112×28**, la plaque et le mot restent reconnaissables, mais le détail du tissu et les micro-reflets décroissent. Le 140×35 est la **proposition préférée** pour confrontation à la charte. Le style conserve l'ardoise/acier et l'or du PO : **pas de rouge ajouté sans GO**. La couleur n'est pas un indicateur logiciel d'état ici.

La commande graphique `ABANDON` n'est **pas** le texte obligatoire du dialogue `CONTINUER / ABANDONNER` : aucun changement métier. Le sprite n'impose aucun nouvel état `ON/OFF`, `pressed` ou `disabled`. L'abandon ne fonctionne qu'aux étapes définies par BN-UX-003, LOT1-ABD-001, LOT1-UX-011. Les choix de taille tactile et le budget RAM/Flash restent à valider avec Pilotage/Développeur. Aucun test sur ESP32 CYD n'a eu lieu.

**Aucun déplacement vers `assets/`, aucune approbation artistique implicite, aucun merge.** Le PO décide de l'art, Pilotage examine la PR et fusionne seul. `Refs #149`.