# G04-R2 — ATTAQUE VS Capitaine — référence artistique active

> **VALIDÉ ARTISTIQUEMENT PAR LE PO le 04/11/2026** ([décision PR #64](https://github.com/Kharn27/ESP32-OTA-Battleship/pull/64#issuecomment-6626211912)). **`DESIGN_PREVIEW — ON_DEVICE_UNVERIFIED`** : accord sur maquette PC uniquement, pas une certification de l'intégration CYD.

**Référence canonique unique à 320 × 240 (paysage) :**

![ATTAQUE VS Capitaine — G04-R2 native](g04_r2_attaque_320x240.png)

Zoom de contrôle ×4, agrandissement strict **NEAREST** :
![Zoom ATTAQUE 1280 × 960](g04_r2_attaque_x4.png)

## États fonctionnels, non concurrents

Quatre vignettes de comportement, sans créer d'autres écrans de référence :
![Vignettes — sans cible, FEU engagé, double résolution, retour DÉFENSE](g04_r2_etats_320x240.png)

[Zoom ×4 strict NEAREST](g04_r2_etats_x4.png)

- **Sans cible** : FEU indisponible. **FEU engagé** : sélection, DÉFENSE et ABANDON verrouillés jusqu'à la révélation conjointe des deux tirs.
- **Résolution** : les deux impacts sont affichés ensemble. **Retour DÉFENSE → ATTAQUE** : cible G5 conservée avant FEU, informations d'alerte acquittées sans effacer l'historique.
- **ABANDON** ouvre CONTINUER / ABANDONNER ; aucune sortie directe.

## Invariants de la maquette validée

- Une seule grille **10 × 10**, cellules **18 × 18 px**, surface **180 × 180 px**, origine **(21,53)**, colonnes A–J, lignes 1–10 ; tracé unique de 11 horizontales + 11 verticales d'un pixel. Mer continue sombre en arrière-plan : pas de tuiles ou de fausses séparations.
- Cible préparée **G5** (réversible avant FEU), impacts **TOUCHÉ C3/E3/E4/I8** et **RATÉ A8/B7/D1/F9/G2/H6/J4** : distinctions par forme en plus des couleurs, aucun navire adverse révélé.
- Panneau maritime intégré : commandes **FEU**, **DÉFENSE** avec notification `!` sans compteur, **ABANDON** secondaire ; **RIPOSTE B7 RATÉ** séparée de l'alerte ancienne **NON LUE COULÉ**.
- Style maritime peint (Capitaine, mer, laiton, bois, émail) conforme à la version acceptée ; les anciens rendus G04/G05/G07 R1 ne servent pas de référence artistique. L'écran G08 conserve sa validation propre.

## Sources et preuves à conserver

- `reference_concept_1448x1086.png` : **référence HD raster** des matières et du décor.
- `reference_controles_integres_1448x1086.png` : **master raster** du panneau droit intégré. Ses textes sont peints dans le raster, non des glyphes indépendants ; une décomposition est à étudier lors du contrat d'assets firmware.
- `build_g04_r2.py` : composition Python éditable (grille/mer/marqueurs/vignettes), **seuls quatre PNG d'interface** sont exportés : ATTAQUE native + zoom, états natifs + zoom.
- `generation-report.json` : SHA256 et tailles des deux références HD, du générateur et des quatre PNG conservés. `verify_g04_r2.py` : contrôle sur PNG réel, vérification NEAREST, lignes et faux quadrillages, différenciation des marqueurs, états, panneaux et mutations négatives.
- `verification-PC.txt` : provenance des tests graphiques historiques. [CI graphique contrôlée — SUCCESS #41701593193](https://github.com/Kharn27/ESP32-OTA-Battleship/actions/runs/41701593193) avant le nettoyage des seules preuves/comparatifs ; les **PNG artistiquement approuvés restent identiques**. La CI C99/ESP-IDF est contrôlée indépendamment sur le HEAD final.

**Reproduction sur Linux** (Python 3, police du paquet `fonts-dejavu-core` à `/usr/share/fonts/truetype/dejavu/DejaVuSansCondensed-Bold.ttf`) :

```sh
python3 -m pip install 'Pillow==12.3.0'
python3 design/ecrans/attaque/build_g04_r2.py
python3 design/ecrans/attaque/verify_g04_r2.py
```

### Nettoyage contrôlé / historique

Les anciens fichiers d'avant/après, comparatifs et images techniques intermédiaires ont été **retirés de l'arbre actif**, mais restent récupérables à la révision [`e5d97f97`](https://github.com/Kharn27/ESP32-OTA-Battleship/tree/e5d97f97a06efd0bab93b1fbebf3a973b9fb5ac1/design/ecrans/attaque). Aucune copie d'archive n'est ajoutée au dépôt. Seuls la référence active, ses variantes de comportement, ses deux masters raster et ses outils/preuves essentiels subsistent.

**Dette explicite :** la validation sur CYD réelle (TFT, stylet/hitboxes, lisibilité dans l'appareil, Flash/RAM et pack d'assets) reste **à réaliser pendant l'intégration firmware**. Aucun `ON_DEVICE_VERIFIED` n'est revendiqué. [Issue #45](https://github.com/Kharn27/ESP32-OTA-Battleship/issues/45) · [PR #64](https://github.com/Kharn27/ESP32-OTA-Battleship/pull/64).
