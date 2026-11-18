# PORTRAITS-04 — Jalon 6 / Issue #133 — Rapport Graphiste

## Référence / périmètre
**Base exacte** : `main@e414d9f224dc3419acd006c2c80521c5a7ca2b0d`. **Branche** : `design/issue-133-jalon6-huit-portraits`. Huit IDs nouveaux, parmi les 23 restants après J5 : `C-L4-C1 , C-L4-C2 , C-L4-C3 , C-L4-C4 , E-L3-C1 , E-L3-C2 , E-L3-C3 , E-L3-C4`.
Avant J6 : **45/68** positions candidates fusionnées (90 variantes). Si Pilotage accepte et fusionne J6 : **53/68 = 77,9 %**, 106 variantes 48/64, **15 restent** (D=11 ; E=4 ; C terminé). Aucun ID J0–J5 modifié ou dédoublonné.

**48 nouveaux fichiers uniquement sous `draft/portraits/exports/`** : 8 masters RGBA HD, 16 sprites RGBA 48×48/64×64, 16 agrandissements exacts NEAREST ×4 (192 ou 256), 4 aperçus **320×240 natifs** sombre/clair, 1 contact 1120×182, une recette Python, un manifeste à empreintes et ce rapport. Zéro changement dans `assets/`, `maquettes/`, firmware, Bible, originaux PO ni artefacts J0–J5.

## Cadrage, alpha et identité visuelle
Planches PO **C** et **E**, extraction native fidèle : carrés master 362×362, pour C dernière ligne recadrages y=[815,1086) et marge transparente haute 45 ; pour E troisième ligne y=[543,815) et marge haute 45. Toutes les coordonnées demi-ouvertes et dérogations figurent par ID au manifeste. 48 et 64 proviennent **indépendamment du même master** par BOX entier en alpha prémultiplié, sans interpolation 48→64 ni correction colorimétrique globale. RGB caché pour les pixels alpha=0 normalisé à zéro (0 pixel non nul observé dans ce lot).

**Parité source contrôlée** : les PNG des masters fusionnés J5 `C-L1-C4` et `E-L1-C3` ont été recalculés depuis les sources Adobe natives et les SHA-256 obtenus sont **strictement identiques** aux deux SHA de `manifest_jalon5.json` sur `main`. Les cinq empreintes SHA des originaux PO sont vérifiées par le script sur checkout complet, et non revendiquées comme nouvelle mesure dans l'outil de transport.

### Nettoyage local exclusif
- `E-L3-C2`, ROI master **[348,0,362,362)**, politique `all`. Motif : fragments étrangers de la cellule voisine au bord droit, décorrélés de la silhouette principale. **1 259 pixels alpha>0 supprimés**. Contrôle explicite : **0 canal modifié hors ROI**. Le rendu 48 et 64 possède désormais **une seule composante alpha>=6**, sans supprimer les lunettes, mèches et col fourré du sujet. Aucun masque sur les sept autres.
- Aucun traitement de netteté, lissage ou rognage global ; coiffes, tatouages, ornements et volumes préservés. Justifications par ID dans le manifeste.

## Contrôles de production réalisés
- **8/8 sprites à une composante alpha>=6 à 48 ET 64**, marges, yeux et volume apparent comparés sur les deux fonds.
- **45 PNG générés** (24 master+exports et 21 preuves). SHA-256 64 hex et taille calculés sur les **mêmes octets PNG** que ceux transmis au dépôt GitHub.
- **16 zooms nearest ×4 exacts**, 4 aperçus 320×240 clair/sombre, 1 contact ; revue visuelle PC native et zoom.
- `export_jalon6.py --check` a été dérivé du pipeline J5 : lecture seule, vérification des cinq originaux PO SHA, régénération octet-à-octet des **45 PNG**, puis SHA et taille réels pour **45/45 fichiers déclarés**, en refusant empreinte non hexadécimale/abrégée.

## Contrôles indépendants **à exécuter par Pilotage**
```sh
python3 -m pip install Pillow==12.3.0
for n in 0 1 2 3 4 5 6; do python3 draft/portraits/exports/export_jalon${n}.py --check || exit 1; done
python3 scripts/check_graphic_pipeline.py
git status --short
```
**Non exécutés par Graphiste dans cet environnement** : le `--check` sur checkout complet pour J0–J6, le validateur du pipeline, la CI sur HEAD PR et l'essai sur CYD physique. Ne revendiquer aucun PASS anticipé. À confirmer indépendamment : 45/45 SHA et tailles, 5 sources PO, 16 NEAREST, 4 natifs, connexité des deux tailles, masque et diff isolé.

**Statut immuable : `CANDIDATE — ON_DEVICE_UNVERIFIED`.** GO PO global = autorisation de produire les candidats, pas GO esthétique individuel. Aucune promotion vers `assets/` ou caractère canonique. Pilotage seul revoit, décide et fusionne.
