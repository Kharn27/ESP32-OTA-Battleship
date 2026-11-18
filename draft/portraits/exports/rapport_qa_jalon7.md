# PORTRAITS-04 — Jalon 7 / Issue #133 — Rapport Graphiste / DA

## Périmètre et origine
GO Pilotage daté du 18/11/2026 : [commentaire J7 #6977239439](https://github.com/Kharn27/ESP32-OTA-Battleship/issues/133#issuecomment-6977239439). Base immuable `main@afe8f44443d965a081bb075dcc20bc0f2fc31fa4`. Branche `design/issue-133-jalon7-quinze-portraits`, PR Draft liée à #133. **15 positions nouvelles** : 11 sur D, 4 sur E. Les manifestes J0–J6 totalisent 52 nouveaux IDs sans recouvrement, plus le **pilote A-L1-C1 conservé hors exports** ; donc 53 positions existantes +15 = **68/68 candidates seulement après fusion J7**.

**Diff prévu strictement sous `draft/portraits/exports/` :** 15 masters RGBA HD, 30 exports RGBA (15×48 + 15×64), 30 zooms exacts x4, 8 composites natifs 320×240 (4 pages clair/sombre), 2 contacts 1120×182, 6 avant/après 320×240 et trois textes (`export_jalon7.py`, `manifest_jalon7.json`, présent rapport) : **91 PNG + 3 textes = 94 ajouts**. Aucune modification d'origine PO, de J0–J6, d'assets, de maquette, de Bible ou de firmware.

## Extraction et homogénéité
Source D originale PO **1536×1024 RGBA** ; E **1448×1086 RGBA**. Les 15 rectangles exacts [x0,y0,x1,y1) sont consignés individuellement au manifeste, sans confondre cellule de repérage et portrait validé. Carrés master 307, 308 ou 362 pixels, avec marge transparente haute **22, 25, 45 ou 48** suivant l'identifiant. Conservation des couleurs, coiffes, barbes, cheveux, bijoux et vêtements sans filtre global. Les deux tailles sont calculées **indépendamment d'un même master HD** en BOX entier alpha prémultiplié, jamais l'une à partir de l'autre. Contours contrôlés sur fonds sombres et clairs. Les ×4 sont des répétitions exactes de pixels avec alpha RGBA intact.

**Parité Adobe natif / GitHub fusionné vérifiée** sur masters témoins inchangés : D-L1-C4 J5 SHA256 `7c58ffeee2255390fe342cf10b370e64b5148d677e1ac6bc17794ea9563843cf`, E-L3-C1 J6 `d39424caff2217a2e54be9d940c276358ed1abc2218c6061e1ec47ccadefb875`. Dans les deux cas, génération sur images Adobe en pleine résolution et comparaison SHA-256 **strictement identique** aux masters déjà sur `main`. Les cinq originaux GitHub doivent encore être recalculés lors du `--check` checkout complet.

## Masques locaux : suppression exclusive des fragments voisins
Six positions nécessitent sept masques rectangulaires sur masters dérivés, de politique `all` (mise à zéro RGBA). Coordonnées demi-ouvertes, aucune édition hors ROI :
- `D-L3-C3` : `[294,0,308,308)` (1467 px alpha>0) ; `[0,230,9,308)` (426 px alpha>0). **Total 1893 pixels** retirés, **0 canal** modifié hors ROI.
- `D-L3-C4` : `[278,0,307,307)` (2950 px alpha>0). **Total 2950 pixels** retirés, **0 canal** modifié hors ROI.
- `D-L4-C4` : `[277,0,307,307)` (5127 px alpha>0). **Total 5127 pixels** retirés, **0 canal** modifié hors ROI.
- `E-L4-C1` : `[344,0,362,362)` (1060 px alpha>0). **Total 1060 pixels** retirés, **0 canal** modifié hors ROI.
- `E-L4-C2` : `[348,0,362,362)` (984 px alpha>0). **Total 984 pixels** retirés, **0 canal** modifié hors ROI.
- `E-L4-C3` : `[344,0,362,362)` (2120 px alpha>0). **Total 2120 pixels** retirés, **0 canal** modifié hors ROI.

Tous les ID ont **une unique composante alpha≥6 à 48 ET 64 pixels** ; les 30 sorties ont été contrôlées. Les cinq plus fortes discontinuités de contour ont été inspectées à taille native; les fragments colorés faibles et les marges propres ont été surveillés. Ces corrections ne sont pas des retouches esthétiques globales.

## Preuves produites et traçabilité
- **91/91 PNG** listés dans `manifest_jalon7.json` avec chemin, SHA-256 complet 64 hex et taille réelle issue des octets déposés sous forme de blobs Git.
- **30 zooms ×4** : `preuves/jalon7_{ID}_{48|64}_x4.png`.
- **8 natifs** : `preuves/jalon7_p{1..4}_320x240_{sombre|clair}.png`.
- **2 contacts** : `preuves/jalon7_contact_p{1..2}_1120x182.png`.
- **6 avant/après** : `preuves/jalon7_correctif_{ID}_avant_apres_320x240.png`.
- Script `export_jalon7.py` : source reproductible avec `--check` lecture seule, hash de cinq originaux, régénération exacte, vérification du manifeste et connexité. Il conserve le pipeline J6 et les paramètres PNG déterministes.

## Contrôles réalisés / restant à Pilotage
**Dans les connecteurs** : recadrages HD D/E exacts (dimensions et coordonnées), témoins source D/E byte-parity PASS, 15×2 vérifications composantes alpha PASS, les sept ROIs vérifiées en pixels non transparents et zéro modification hors zone, contacts et previews 320×240 clair/sombre inspectés. Les valeurs de SHA et taille sont calculées directement sur les tableaux d'octets PNG transmis à GitHub ; ne préjugent pas du contrôle du checkout final.

**À exécuter indépendamment par Pilotage, sur le HEAD exact de la PR** :
```sh
python3 -m pip install Pillow==12.3.0
for n in 0 1 2 3 4 5 6 7; do python3 draft/portraits/exports/export_jalon${n}.py --check || exit 1; done
python3 scripts/check_graphic_pipeline.py
git status --short
```
Vérifier 91/91 SHA/taille réels du J7, 30 zooms nearest, les 8 natifs, deux contacts et six avant/après, les 30 connexités, l'intégrité des cinq sources PO, le diff 94 ajouts uniquement, la CI et l'exactitude des pixels. **Ces commandes sur checkout, la CI du nouveau HEAD et le test CYD physique ne sont PAS revendiqués comme exécutés dans cet environnement Graphiste.**

**Statut : `CANDIDATE — ON_DEVICE_UNVERIFIED`.** Aucun GO individuel PO, aucune promotion dans `assets/`, aucun regroupement canonique G/Q, aucune preuve matérielle. Seul Pilotage revoit et fusionne.

## Addendum correctif — six libellés de preuves (18/11/2026)

**Déclencheur :** revue bloquante Pilotage PR #156, [review #5959784211](https://github.com/Kharn27/ESP32-OTA-Battleship/pull/156#pullrequestreview-5959784211), QA indépendante [run #44337253445](https://github.com/Kharn27/ESP32-OTA-Battleship/actions/runs/44337253445). Le `--check` J7 comparait correctement les vrais fichiers : les six témoins avant/après différaient du rendu canonique.

**Cause démontrée :** pendant la livraison initiale, les six comparatifs 320×240 ont été composés avec les glyphes 3×5 du helper JavaScript `connector_jalon3.js`, qui ne définit pas les lettres **V, N, T, P, R et S** utilisées par les mots `AVANT` et `APRES`. Le `export_jalon7.py` versionné, lui, définit ces six glyphes et les peint. Ce sont donc **les preuves soumises**, pas le générateur Python ni les sprites, qui étaient incorrectes. Le générateur versionné reste volontairement **inchangé**.

**Réparation exacte et bornée :** dans chacun des six témoins RGBA 320×240, restauration du fond puis tracé des quatre libellés 3×5 conformément à `correction_comparison()` du script J7 : `AVANT` et `APRES` aux x=8/168, y=5/125, sur les rangées sombre et claire. **108 pixels RGBA modifiés par PNG, 0 pixel hors des quatre rectangles [8,5,28,10), [168,5,188,10), [8,125,28,130), [168,125,188,130)**. Aucune modification des portraits, de l'avant/après 48/64, des masques, des originaux PO ni de J0–J6. Les six PNG restent RGBA 320×240, 307528 octets chacun, avec l'encodeur déterministe PNG filter=0 / zlib DEFLATE stocké déjà contracté.

Nouvelles empreintes SHA-256 des **six seuls PNG modifiés**, inscrites dans `manifest_jalon7.json` :

| ID | SHA-256 révisé | Taille octets |
| --- | --- | ---: |
| `D-L3-C3` | `0ab55b931e7884eff50351031627ad08cb216aad8eb1fbb3d60a981a07c10af1` | 307528 |
| `D-L3-C4` | `f07a0cd71eea96ef0da4ad99e0f18290a2a0bc0ba7df0d3a3e4bc2bcdefae823` | 307528 |
| `D-L4-C4` | `fd5c68f86f7155442d5a24053c2b327c75ca6cd6129199633c3dbf2a37fdf319` | 307528 |
| `E-L4-C1` | `5974063f7c0e598391b8c625bdc1526040e567db4570ad4039761a27ac30e8a8` | 307528 |
| `E-L4-C2` | `dbc1e04d2971ae7c3ef8cea35bc5ea4889cbe4439a6a31712582c4246b30fbd6` | 307528 |
| `E-L4-C3` | `1df31e352a13c40953b1e3889b30f3cdf2d661686915aa4a81c7d18a157e77f8` | 307528 |

**Contrôle ciblé effectivement obtenu dans les connecteurs GitHub :** PNG binaires existants lus en base64, reconstruction PNG lossless aller/retour **identique à l'octet**, comparaison canal par canal avant/après, **108 pixels de glyphes uniquement** par preuve, dimensions/alpha préservés. Le SHA-256 corrigé de `D-L3-C3` (`0ab55b931e7884eff50351031627ad08cb216aad8eb1fbb3d60a981a07c10af1`) reproduit **exactement** la valeur attendue remontée dans la revue indépendante Pilotage. Tous les SHA-256 révisés ont été calculés sur les octets corrigés (SHA-256 du helper versionné, auto-vérifié sur `abc`).

**Reste à contrôler par Pilotage sur le nouveau HEAD :** exécuter réellement les huit scripts `export_jalon0.py` à `export_jalon7.py --check`, `scripts/check_graphic_pipeline.py`, le contrôle manifeste **91/91**, `git status --short`, et vérifier la CI **3/3** du nouveau HEAD. **Aucun de ces contrôles sur checkout complet ni test CYD physique ne doit être considéré comme exécuté du seul fait de cet addendum.** Statut toujours `CANDIDATE — ON_DEVICE_UNVERIFIED`, aucune validation artistique individuelle PO ni promotion `assets/` ; revue et fusion exclusivement par Pilotage.
