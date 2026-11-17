# BTN-ABD-01 — ABANDON · étude graphique (Issue #149)

**Statut : `CANDIDATE · ART_NOT_REVIEWED · ON_DEVICE_UNVERIFIED`.**
**PR #150 DRAFT ; GO artistique PO non obtenu. Fusion exclusivement Pilotage.**

## 1. Original PO et filiation à vérifier

L'original PO est présent, inchangé, sur `main` : `draft/Bouton_Abandon.png`,
**1 814 386 octets**, **Git blob SHA-1 `3818cb092456e1ff222d8b2dacf9f9aa55266135`** (restauration PR #152).
Aucun fichier original n'est ajouté ou modifié ici.

La **pièce jointe réellement utilisée** dans cette étude mesure **2048×682 RGBA**,
1 870 797 octets, SHA-256 du fichier
`2637ff5eb00d42f427a07fa0a25baffd7b95c77566578d6e223ac8ecb6533690`.
Elle a un **encodage différent** de la source Git. Sa comparaison **pixel à pixel n'a pas encore été exécutée contre le Git exact**.

La preuve reproductible est `empreinte_pixels_upload.json` :
l'empreinte SHA-256 des **5 586 944 octets RGBA décodés** de la pièce jointe est
`da7ca8f59aac3c27e02d9fb0b5966ee6ea048389221834610c1d11845ec818a5`.
`generate.py --repo . --check-source` exige l'identité du blob Git,
des dimensions et de tous ces pixels ; en cas de divergence, le processus échoue
et **aucun export ne doit être considéré comme dérivé de l'original PO**.
Cette commande **n'a pas encore été exécutée avec succès sur un checkout réel**.

## 2. Candidats graphiques (sources et exports)

Dérivés **de la pièce jointe**, sans redessin : découpe `(24,90)-(2024,590)`,
master RGBA **2000×500** ; réduction LANCZOS en alpha prémultiplié,
exports **140×35** et **112×28** ; zooms ×4 strict **NEAREST**.

- `exports/abandon_master_2000x500_RGBA.png`
- `exports/abandon_140x35_RGBA.png` — préférence de la DA
- `exports/abandon_112x28_RGBA.png` — taille compacte, moins détaillée
- `preuves/etude_abandon_320x240.png` et son zoom ×4
- `preuves/abandon_{140x35,112x28}_x4_NEAREST.png`
- `rapport_reference_upload.json` — hashes SHA-256 et alpha de **ces 7 PNG seulement**, avec référence **explicite** à l'upload (pas à Git).

Les candidats sont conservés noirs/acier/or, avec drapeau, plaque et rivets.
Pas de recoloration rouge, pas de variantes d'état `pressed`/`disabled` inventées.

## 3. Comparatif publié avec les vrais boutons approuvés

La **planche native 320×240** et son zoom **1280×960 NEAREST** sont
désormais publiés :

- `preuves/comparatif_approuves_320x240.png`
- `preuves/comparatif_approuves_1280x960_NEAREST.png`

Ils assemblent sans redimensionnement ni modification les **vrais blobs de
`main`** JOUER (140×32), PALMARÈS (140×30) et ATTAQUER ON (140×35)
avec ABANDON candidat (140×35), sur fonds clair et sombre.
Les empreintes **Git blob SHA-1 des quatre entrées et des deux planches**,
ainsi que la disposition, figurent dans `rapport_comparatif_approuves.json`.
Aucune duplication des sprites approuvés dans la PR.

Le rendu natif a été **effectivement affiché et examiné** :
le 140×35 est plus lisible et préserve mieux le drapeau et les reliefs que le
112×28. Cette appréciation n'est **pas** une validation PO ni matérielle.

## 4. Reproductibilité depuis un checkout propre

**Dépendance conseillée : Pillow 12.3.0** (version des exports originaux).
Depuis la racine du vrai dépôt, après checkout de la HEAD de la PR :

```bash
python3 -m pip install 'Pillow==12.3.0'
python3 draft/etudes/bouton_abandon/audit_source.py --check
python3 draft/etudes/bouton_abandon/generate.py --repo . --check-source
python3 draft/etudes/bouton_abandon/generate.py --repo . --check-comparatif
python3 draft/etudes/bouton_abandon/generate.py --repo . --check
python3 scripts/check_graphic_pipeline.py
```

**`--check` est strictement en lecture seule.** Il ne réclame **plus un
`rapport.json` inexistant** dans la PR : il vérifie la provenance Git exacte
via le SHA-256 des pixels RGBA, les **7 exports par comparaison de pixels avec
la source Git**, le contenu et les SHA-256 de `rapport_reference_upload.json`,
et les deux planches via les **vrais blobs approuvés** (9 PNG).
Les PNG de comparaison peuvent avoir un encodage différent du rendu Pillow,
sans que leurs **pixels RGBA** soient différents.

**Résultats réellement obtenus :** fabrication des deux PNG comparatifs et
inspection à 320×240 sur PC ; cohérence indépendante de la composition
(bitmap font/compositing) vérifiée localement sur image de test ;
Git blobs des quatre entrées et deux sorties vérifiés par le connecteur.
**Résultats non obtenus ici :** `--check-source PASS`, `--check PASS`,
`scripts/check_graphic_pipeline.py PASS` sur un **checkout complet** ;
absence d'accès aux octets du PNG Git >1 Mo dans cet environnement.
La CI build GCC/Clang/ESP-IDF était verte sur **l'ancienne HEAD**
`b626e0c543124005c5db901274ee64b8e5e2faf4` ; les nouveaux commits
doivent être contrôlés séparément.

**En cas d'échec de `--check-source` : ne pas forcer un PASS, ne pas déclarer la
filiation prouvée ; analyser les dimensions et pixels réels, puis demander
l'arbitrage PO ou régénérer depuis l'original avec un nouveau HEAD.**

## 5. Gouvernance

Étude exclusivement dans `draft/etudes/bouton_abandon/` ;
aucun changement de `draft/Bouton_Abandon.png`, `assets/`, `maquettes/`,
firmware ou `BIBLE_PRODUIT.md`. `ABANDON` graphique ne remplace pas le
texte de dialogue `CONTINUER / ABANDONNER` (BN-UX-003, LOT1-ABD-001,
LOT1-UX-011). Format et budget firmware, taille tactile et rendu sur la vraie
CYD restent à valider avec le Développeur/Pilotage.

**La PR #150 reste DRAFT ; aucune fusion sans la revue de Pilotage et aucun
GO artistique implicite.** Refs #149.
