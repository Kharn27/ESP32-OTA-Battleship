# PORTRAITS-04 — Rapport QA Jalon 1 : 8 positions A–E

**Issue #133 · Responsable : Graphiste / Directeur artistique · Statut : `CANDIDATE — ON_DEVICE_UNVERIFIED`.**

Base `main@6b3c965263408eb7104494a9c6da75e75b11efa2` ; branche `design/issue-133-jalon1-huit-portraits`. Aucun portrait validé artistiquement, aucun GO de promotion `assets/`.

## Contenu et progression

8 positions : `A-L1-C2`, `A-L1-C3`, `B-L1-C1`, `B-L1-C3`, `C-L1-C1`, `C-L3-C1`, `D-L1-C5`, `E-L1-C4`. Chaque ID possède un `master_HD.png`, un `portrait_48.png` et un `portrait_64.png` issus du même master, sans étirement ni déduplication. 16 preuves NEAREST ×4, 4 comparatifs 320×240 natifs clair/sombre, une planche-contact 1120×182. **21 PNG de preuve**, à ne pas traiter comme sprites firmware.

Après validation de ce lot : 13/68 positions couvertes (26 variantes) ; 55 restantes. Les cinq témoins J0 ne sont ni modifiés ni copiés.

## Extraction et réserves

| ID | Planche | Rectangle réel [x0,y0,x1,y1) | Master/marge | Vigilance |
|---|---|---|---|---|
| `A-L1-C2` | A | `[362,49,724,411]` | 362×362, top 0px | Chapeau marine à ancre, boucles et épaulettes |
| `A-L1-C3` | A | `[724,49,1086,411]` | 362×362, top 0px | Bandeau rouge et tresses |
| `B-L1-C1` | B | `[0,49,362,411]` | 362×362, top 0px | Cheveux argentés, ornements latéraux, drapé violet |
| `B-L1-C3` | B | `[724,49,1086,411]` | 362×362, top 0px | Bandeau bordeaux et grands anneaux |
| `C-L1-C1` | C | `[0,0,362,272]` | 362×362, top 45px | Tricorne large et barbe blanche |
| `C-L3-C1` | C | `[0,543,362,815]` | 362×362, top 45px | Tricorne et plume rouge |
| `D-L1-C5` | D | `[1229,0,1536,256]` | 307×307, top 25px | Bonnet bleu et pipe, fumée |
| `E-L1-C4` | E | `[1086,0,1448,272]` | 362×362, top 45px | Capuche sombre et ancre |

Les cellules de l'inventaire ne sont pas prises pour des recadrages finaux : coordonnées documentées par ID. Nettoyage limité au RGB caché sous alpha=0 ; aucun filtre colorimétrique global.

## Méthode / QA

Python >=3.11, Pillow==12.3.0 ; génération `PNG RGBA 8 bits`, DEFLATE stored sans métadonnées, BOX par aire pondérée et alpha prémultiplié en entiers. Les 48 et 64 sont calculés indépendamment depuis le master, jamais 48→64. Les preuves ×4 utilisent NEAREST exact. Le manifeste `manifest_jalon1.json` donne pour chaque sortie le SHA-256, les tailles et le bilan alpha.

Quatre étalons J0 B/C/D/E ont été recalculés depuis leurs planches Adobe de même nom et dimensions : les SHA-256 des sorties 48 correspondent **exactement** aux quatre valeurs J0 fusionnées. Les SHA-256 des cinq originaux du dépôt sont des **références attendues**, non une mesure nouvellement effectuée sur un checkout local. Leur vérification par `--check` est nécessaire.

Contrôles encore à réaliser sur checkout propre et sur le HEAD exact de la PR :

```sh
python3 -m pip install Pillow==12.3.0
python3 draft/portraits/exports/export_jalon0.py --check
python3 draft/portraits/exports/export_jalon1.py --check
python3 scripts/check_graphic_pipeline.py
git status --porcelain
```

Le mode `--check` est exclusivement lecture, compare les PNG régénérés octet à octet et contrôle les cinq SHA sources. Les vérifications indépendantes et la CI ne sont **pas revendiquées comme passées** avant exécution effective.

## Revue et limites

Inspecter les planches 320×240 et ×4, position des yeux et tailles apparentes, tricornes/plume, cheveux B-L1-C1, fumée D-L1-C5, détails sombres E-L1-C4 et franges alpha sur les deux fonds. L'analyse de l'occupation et du cadrage demeure candidate ; le PO peut demander des corrections ponctuelles.

**Hors périmètre :** `assets/`, `maquettes/`, `BIBLE_PRODUIT.md`, firmware, originaux PO et cinq témoins J0. Aucun test physique CYD, aucune mesure Flash/DRAM, aucun personnage canonique attribué. **Fusion réservée à Pilotage.**

Refs #133.
