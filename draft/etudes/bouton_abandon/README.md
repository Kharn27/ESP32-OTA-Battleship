# BTN-ABD-01 — étude graphique ABANDON · Issue #149

**Statut : `CANDIDATE` · `ART_NOT_REVIEWED` · `ON_DEVICE_UNVERIFIED`.**
Aucun GO artistique PO n'est déduit de la CI ni de la comparaison. Cette étude n'altère ni l'original PO ni `assets/`, `maquettes/`, la Bible ou le firmware.

## Filiation du vrai original PO (nouvelle source active)

- **Original immuable** : `draft/Bouton_Abandon.png` de `main` (restauration PR #152).
- Taille : **1 814 386 octets** ; dimensions **2172 × 724 pixels** ; **RGBA**.
- SHA-1 **Git blob** : `3818cb092456e1ff222d8b2dacf9f9aa55266135`.
- SHA-256 **fichier PNG Git exact** : `1b7c026a5ebcf7283de9ce93bf740fab350aa4aaf22da80641c34fe421c8719f`.
- SHA-256 **flux RGBA décodé complet** : `d00d364479fcace09fa4f9339d9d4cf0a184cb0893c1c5f9507d0e8ab5bd1b96`.
- Limites de l'alpha > 1 du vrai original : **(31, 98) — (2141, 623)** ; obtenu depuis le vrai binaire récupéré dans un artefact d'Actions GitHub.
- Recadrage **(24, 95) — (2148, 626)** : master RGBA transparent **2124 × 531**, **ratio 4:1 exact**, contenant toute la silhouette alpha > 1 sans étirement ni recoloration.
- Exports correspondants **140 × 35** (recommandé) et **112 × 28** (compact), LANCZOS alpha prémultiplié ; agrandissements de contrôle x4 sans interpolation (`NEAREST`).

**Rectification de la provenance antérieure :** l'image jointe au chat avait **2048×682**, 1 870 797 octets et SHA-256 PNG `2637ff5eb00d42f427a07fa0a25baffd7b95c77566578d6e223ac8ecb6533690`. Elle **n'est PAS pixel-identique** à l'original Git : constaté dans le [run d'audit isolé #44156311202](https://github.com/Kharn27/ESP32-OTA-Battleship/actions/runs/44156311202). Les anciens candidats/rapport `rapport_reference_upload.json` ont donc été **supplantés par une régénération complète depuis le vrai original Git** ; conserver ce rapport ancien uniquement comme référence historique de l'erreur et ne pas l'utiliser pour valider la nouvelle production.

## Sorties graphiques de l'étude

- `exports/abandon_master_2124x531_RGBA.png` — master détouré grand format.
- `exports/abandon_140x35_RGBA.png` — proposition prioritaire PO.
- `exports/abandon_112x28_RGBA.png` — proposition compacte.
- `preuves/abandon_140x35_x4_NEAREST.png` et `preuves/abandon_112x28_x4_NEAREST.png`.
- `preuves/etude_abandon_320x240.png` et `preuves/etude_abandon_1280x960_NEAREST.png` : test clair/sombre.
- `preuves/comparatif_approuves_320x240.png` et `preuves/comparatif_approuves_1280x960_NEAREST.png` : les **vrais** sprites JOUER, PALMARÈS et ATTAQUER de `assets/boutons/` (référencés, jamais copiés dans `draft/`), avec ABANDON issu du Git original.
- `rapport_git_original.json` : provenance, empreintes SHA-256, dimensions et alpha, pour **neuf fichiers PNG**.

La planche comparative est une **preuve de travail**, pas une maquette du jeu ni un composant runtime. Les touches/enfoncements/désactivations relèvent de GUI-01 ; aucune variante non demandée n'est créée ici. Aucun format mémoire/firmware ni taille tactile n'est imposé.

## Contrôle autonome depuis un checkout propre de la PR

Utiliser **Pillow 12.3.0** pour reproduire les PNG et vérifier :

```bash
python3 -m venv /tmp/cyd-btnabd
/tmp/cyd-btnabd/bin/python -m pip install 'Pillow==12.3.0'
/tmp/cyd-btnabd/bin/python draft/etudes/bouton_abandon/audit_source.py --check
/tmp/cyd-btnabd/bin/python draft/etudes/bouton_abandon/generate.py --repo . --check-source
/tmp/cyd-btnabd/bin/python draft/etudes/bouton_abandon/generate.py --repo . --check-comparatif
/tmp/cyd-btnabd/bin/python draft/etudes/bouton_abandon/generate.py --repo . --check
python3 scripts/check_graphic_pipeline.py
```

**`--check` n'écrit aucun fichier** : il vérifie le vrai blob Git, SHA-256/tailles, reconstruction des **sept visuels ABANDON** pixel pour pixel depuis le fichier original, les **deux planches de comparaison** depuis les sprites approuvés et la cohérence des SHA-256 consignés dans **`rapport_git_original.json`**. Aucun `rapport.json` absent n'est attendu.

Pour reconstruire explicitement les exports sur une copie de travail :

```bash
/tmp/cyd-btnabd/bin/python draft/etudes/bouton_abandon/generate.py --repo .
```

Un essai local sur le vrai original extrait de GitHub Actions (avec les vrais sprites approuvés) a réussi `--check-source`, `--check-comparatif` et `--check`, en comparant les pixels. **Ce test local n'est pas une preuve de checkout du dépôt complet** : garder les résultats de CI/QA exact-HEAD dans `QA_2026-11-17.md` et la revue Pilotage.

## Avis artistique sans GO

Le 140×35 favorise la lisibilité du drapeau, des rivets et du texte. Le rendu noir/acier/or respecte la peinture d'origine. Le 112×28 est compact mais moins détaillé.

**Ne rien promouvoir dans `assets/` avant GO explicite du PO. Seul Pilotage peut fusionner la PR.**
