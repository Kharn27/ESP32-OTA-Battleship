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
