# PORTRAITS-04 / Jalon 2 - Rapport QA: 8 portraits candidats A-E

**Issue #133 - Graphiste / DA - CANDIDATE / ON_DEVICE_UNVERIFIED.**

Base `main@965039849944de7fe64d361476884a594e897a21`, branche `design/issue-133-jalon2-huit-portraits`. Aucun original PO ni candidat des jalons J0/J1 n'est modifie. 8 positions nouvelles, aucune attribution de personnage canonique ou dedoublonnage.

## Cadrages et particularites

| ID | Source | Rect source [x0,y0,x1,y1) | Master / marge | Particularite |
| --- | --- | --- | --- | --- |
| `A-L2-C1` | A | `[0,592,362,954]` | 362 px; top 0 px | Tresse rousse, lunettes et fourrure |
| `A-L2-C2` | A | `[362,592,724,954]` | 362 px; top 0 px | Moustache, lunettes, frange lat-rale rouge - contr-ler |
| `B-L2-C1` | B | `[0,592,362,954]` | 362 px; top 0 px | Tatouages, cheveux relev-s, boucle blanche |
| `B-L2-C4` | B | `[1086,592,1448,954]` | 362 px; top 0 px | Insigne ancre, longue-vue au premier plan |
| `C-L2-C2` | C | `[362,278,724,543]` | 362 px; top 48 px | Col -lev-, haut source +6px contre pollution rang-e |
| `C-L2-C3` | C | `[724,278,1086,543]` | 362 px; top 48 px | Pipe/fum-e, haut source +6px |
| `D-L2-C2` | D | `[307,260,614,512]` | 307 px; top 27 px | Tatouages, haut source +4px |
| `E-L2-C4` | E | `[1086,278,1448,543]` | 362 px; top 48 px | Chef, grande toque, haut source +6px |

## Livrables et methodologie

- 8 masters RGBA, 16 portraits 48x48 / 64x64, 16 preuves NEAREST x4, 4 comparatifs 320x240 natifs clair/sombre, 1 contact sheet 1120x182. Au total: 45 PNG nouveaux.
- Deux tailles derivees independamment du meme master HD. BOX par aire ponderee et alpha premultiplie en entier; PNG RGBA8 sans metadata, zlib stored. Zero des canaux RGB caches sous alpha nul. Aucun filtre colorimetrique global.
- Les recadrages C/D/E decalent legerement la limite superieure pour retirer les residus de la rangee precedente. Les marges sont documentees par ID dans le manifeste.
- Certaines franges rouges/orange et contours semi-transparents proviennent des sources PO. Surveiller le bord gauche sur A-L2-C2, les cheveux, la fumee de C-L2-C3, le gabarit des visages et les elements d'uniforme. Revue artistique par le PO encore requise.

## QA a effectuer sur un checkout complet

```sh
python3 -m pip install Pillow==12.3.0
python3 draft/portraits/exports/export_jalon0.py --check
python3 draft/portraits/exports/export_jalon1.py --check
python3 draft/portraits/exports/export_jalon2.py --check
python3 scripts/check_graphic_pipeline.py
git status --porcelain
```

Une reproduction du temoin `A-L1-C2/portrait_48.png` de J1 a donne une correspondance octet pour octet. La verification independante du checkout GitHub, CI, SHA d'origine et essai sur CYD restent a effectuer et ne sont **pas declares PASS** ici.

Progression cible apres acceptation J2: **21/68 positions, 42 variantes, 47 positions restantes**. Pas de `assets/`, `maquettes/`, Bible, firmware, promotion #134 ou role canonique. Fusion reservee a Pilotage.

Refs #133.

## Reprise suite à la revue Pilotage #5921914474 — correction locale J2

**Référence de l'écart :** `D-L2-C2` bord droit (48 px, x=47), et frange rouge de `A-L2-C2` en bord gauche. Base visuelle avant correction `abc8f6e028abffa75cae93cf11c7577d4c2e7a3b`.

### Intervention circonscrite sur les deux masters dérivés

- **D-L2-C2 :** master 307×307, masque rectangulaire `[292,0,307,307)`, soit 1 031 pixels alpha non nul supprimés. Le personnage principal s'arrête à x=286 avant nettoyage (alpha>0), et les résidus sont séparés à partir de x=292. Les groupes parasites de 4 et 5 pixels signalés sur x=47 dans le portrait 48 ont été exclus du master, sans retoucher le visage, les tatouages, la barbe ni l'épaule.
- **A-L2-C2 :** master 362×362, masque `[0,200,10,235)`, soit 203 pixels alpha non nul supprimés. Les résidus très faiblement opaques suivent la frange de cheveux roux de `A-L2-C1` au même bord et aux mêmes hauteurs. Aucune retouche de la moustache, de la casquette, des lunettes ou du buste.
- L'intégrité des **cinq originaux PO** demeure vérifiée par les SHA de la recette J2 ; aucun fichier source sous `draft/portraits/portrait_*.png` n'a été modifié. J0/J1 et six autres nouveaux IDs de J2 ne sont pas concernés.

### Livrables et reproduction

La recette `export_jalon2.py` applique les masques sur les RGBA **dérivés**, puis recalcule séparément chaque 48 et 64 par BOX pondéré / alpha prémultiplié. Mise à jour des quatre composites natifs clair/sombre, du contact 1120×182 et des quatre zooms NEAREST ×4 des deux IDs.

Deux preuves supplémentaires 320×240, chacune avec **AVANT à gauche / APRES à droite**, rangée sombre puis claire et tailles 48 et 64 :
- `preuves/jalon2_correctif_D-L2-C2_avant_apres_320x240.png`
- `preuves/jalon2_correctif_A-L2-C2_avant_apres_320x240.png`

Le manifeste `manifest_jalon2.json` contient les empreintes recalculées, dimensions, comptes alpha et bbox inclusives. **Total attendu : 23 preuves J2** (16 ×4, 4 natifs, 1 planche-contact et 2 comparatifs correctifs).

### Niveau de vérification

- Avant correction : CI 3/3 PASS et QA indépendante Pilotage PASS sur `abc8f6e...`, comme documenté dans la revue.
- Sur le **nouveau HEAD correctif**, la CI et les commandes sur un checkout intégral restent à confirmer par les résultats effectifs ; ne pas transposer les PASS du précédent HEAD.
- Contrôles requis : `export_jalon0.py --check`, `export_jalon1.py --check`, `export_jalon2.py --check`, `scripts/check_graphic_pipeline.py`, SHA cinq originaux, RGBA 48/64, zoom ×4 NEAREST, nouvelles preuves natives et arbre Git propre.
- **Statut inchangé : CANDIDATE — ON_DEVICE_UNVERIFIED.** Aucun GO artistique PO, aucune promotion `assets/`, aucun GO J3 ; Pilotage seul revoit puis fusionne.
