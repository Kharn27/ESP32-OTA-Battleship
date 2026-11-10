# FLOT-01 — Sous-marin intact / premier jalon PO

**Issue #92** — I-009 CYD WARSHIP — **`DESIGN_PREVIEW — ON_DEVICE_UNVERIFIED`**.

## Portée exacte

Première proposition artistique **INTACT uniquement**, soumise au **STOP PO**.
Aucun état **TOUCHÉ** ni **COULÉ** n'a été dessiné ou homologué. Ce pilote n'est pas un asset flash/runtime approuvé et ne modifie aucun élément de gameplay.

| Livrable | Fonction | Format |
|---|---|---|
| `sous_marin_intact_h.svg` | source éditable, orientation horizontale | SVG 54×18 |
| `sous_marin_intact_v.svg` | source orientée dérivée de H | SVG 18×54 |
| `sous_marin_intact_h.png` | sprite autonome H, proue à droite | PNG RGBA 54×18 |
| `sous_marin_intact_v.png` | sprite autonome V, proue en haut | PNG RGBA 18×54 |
| `preuve_grille_320x240.png` | H sur grille 10×10, position G02 C6–E6 ; H/V secondaires | PNG natif 320×240 |
| `preuve_grille_v_320x240.png` | V sur la même grille, position d'essai C6–C8, **NON canon G02** | PNG natif 320×240 |
| `preuve_alpha_clair_sombre_320x240.png` | deux fonds d'eau de contraste différent, H/V dans chacun | PNG natif 320×240 |
| `preuve_comparaison_g02_320x240.png` | comparaison de silhouette G02 / proposition FLOT-01 | PNG natif 320×240 |
| `preuve_*_x4_nearest.png` | zoom exact 4× de chaque preuve native | PNG 1280×960 |
| `manifest.json` | dimensions, ancrages, SHA-256 et octets des exports | JSON |
| `export_and_check.py` | génération reproductible et validation `--check` sans écriture | Python |

## Sources artistiques

- [G02 approuvé, `design/flotte/g02_grille_320x240.svg`](../../g02_grille_320x240.svg) : **fuseau, kiosque, rapport 3 cases**, palette et emprise du Sous-marin approuvés pour concept PC. La silhouette G02 est reproduite uniquement dans le panneau comparatif, sans altérer le fichier source.
- **PO `draft/sous_marin_PO_A.jpeg`** : référence d'identité visuelle pour l'état intact, coque d'acier gris patiné, détail supérieur et plusieurs mâts centraux. Le JPEG du dépôt est une **copie réencodée**, non un sprite transparent et non une preuve pixel à pixel de l'original PO.
- [G01 `CONTRAT_GRAPHIQUE.md`](../../../fondations/CONTRAT_GRAPHIQUE.md) : encrage froid, nuances marines, accents chauds limités ; le navire doit être autonome et ne contient **ni mer peinte, ni gerbe, ni damier**.
- Dessin vectoriel/pixel original de production basé sur ces références de projet ; aucun contenu provenant d'une banque d'images, aucune police redistribuée.

Les détails du PO (lignes de coque, mâts, kiosque central) sont **interprétés** à 54×18 ; un simple réduction de la scène HD aurait effacé la plupart des indices et incrusté la mer. Les reflets et la patine sont volontairement simplifiés afin d'être évalués sur le TFT 320×240.

## Géométrie et alpha

- G02 : grille **10×10**, origine **(19,39)**, pas **18 px**, longueur **3**.
- Horizontal canonique C6–E6 : ancre absolue **(55,129)**, rectangle de l'asset **[55,109) × [129,147)**. Vertical d'essai : **(55,129)**, emprise **18×54**, C6–C8 **pour inspection uniquement**, sans changer la pose G02 approuvée.
- Export PNG de type RGBA réel (`mode=RGBA`) et alpha 0 ou 255 ; tous les pixels hors coque sont transparents, aucun fond maritime n'est présent dans le sprite. La présence de transparence doit être constatée dans les pixels, pas déduite de `.png`.
- La rotation V est réalisée par transposition discrète NEAREST du **raster H** ; le SVG V reste une **source éditable dérivée** avec transformation -90°. Une rasterisation directe de V selon un moteur SVG peut différer sur quelques pixels de contour à cause du placement de traits ; l'export PNG canonique est celui du script. Cette nuance est documentée, et aucun pixel n'est interpolé dans le PNG orienté.
- La comparaison claire/sombre applique **les mêmes fichiers PNG H/V** sur deux fonds, sans changer la couleur du navire ni truquer son détourage.

## Reproduire les exports et les preuves

Dans le dossier `design/flotte/navires/sous-marin/` :

```sh
python3 -m pip install 'Pillow==12.3.0' 'CairoSVG==2.8.2'
python3 export_and_check.py
python3 export_and_check.py --check
```

`--check` ne crée et ne remplace aucun fichier : il recalcule les artefacts en mémoire, compare leurs octets, vérifie le canal alpha, la taille, l'ancrage G02, la rotation 90° et chaque agrandissement NEAREST **1280×960**. Les SHA-256 et octets mesurés sont dans `manifest.json`. Les dépendances mentionnées concernent uniquement **l'outillage PC de preuve**, pas le firmware ESP32.

## Flash / RAM / points à arbitrer avec Pilotage et Développeur

- Coût brut théorique **par orientation** sans compression : `54×18×4 = 3 888 octets RGBA`, soit **7 776 octets** pour les deux orientations si elles étaient chargées intégralement en RGBA ; ce **n'est pas une mesure RAM runtime**, ni un budget validé.
- Les tailles **PNG compressées effectives** sont inventoriées par le script et le manifeste ; elles ne disent pas la consommation RAM ni le temps de décodage.
- À confirmer avec le Développeur : mécanisme réel de lecture Flash, décodage PNG RGBA, mélange alpha sur **fond maritime variable**, ancrage à la grille, rotation ou fichiers préorientés, coûts de pile/mémoire et latence TFT. `PNGdec` annonce le support du type PNG truecolor+alpha, mais une conversion RGBA vers RGB565 sur fond uniforme ne vaut pas automatiquement mélange avec l'océan déjà dessiné case par case ; **ne pas décréter de pipeline firmware**.
- Mêmes points à réévaluer au STOP PO pour les futurs états localisés TOUCHÉ / COULÉ : sprite de coque versus marqueur d'impact par case. Aucune combinatoire d'images n'est décidée ici.

## Contrôle artistique / statut

Visuels inspectés au **320×240 natif** et en **×4 NEAREST**, sur mer claire, mer sombre, et sur une grille à cellules de 18 px. Le rendu est un **avant-projet** : lisibilité du kiosque à 1× à confirmer par le PO. Aucune évaluation de TFT, de stylet ou de performance matérielle n'a eu lieu : **`ON_DEVICE_UNVERIFIED`**.

**Prochaine décision : PO sur l'INTACT H/V**, puis seulement Pilotage/Dev sur le contrat des dégâts. **Aucune fusion par Graphiste.**
