# G04-R2 — ATTAQUE VS Capitaine — contre-revue corrective

> **`DESIGN_PREVIEW — ON_DEVICE_UNVERIFIED` · `CORRECTIONS_REQUISES — REFUS_ARTISTIQUE_PO_G04-R2` · nouveau correctif soumis pour contre-revue · accord artistique PO non reçu.**  
> Issue [#45](https://github.com/Kharn27/ESP32-OTA-Battleship/issues/45) · PR Draft [#64](https://github.com/Kharn27/ESP32-OTA-Battleship/pull/64) · Branche `design/g04-r2-attaque`. **Seul Pilotage fusionne.**

## Proposition principale 320 × 240 native

![Écran ATTAQUE VS Capitaine 320×240](g04_r2_attaque_320x240.png)

<details><summary>Zoom ×4 NEAREST strict — 1280 × 960</summary>

![Écran ATTAQUE agrandi sans interpolation](g04_r2_attaque_x4.png)

</details>

## Nouveau correctif : lisibilité des 100 cases (prescription Pilotage du 03/11/2026)

> Source du refus : [commentaire Pilotage #6609534686](https://github.com/Kharn27/ESP32-OTA-Battleship/pull/64#issuecomment-6609534686), suivant l'arbitrage artistique du PO. Les contrôles anciens prouvaient les coordonnées, **pas la lecture immédiate de 100 cases**.

**Comparatif direct avant/après à résolution native (deux captures de 320×240 pixels non redimensionnées, juxtaposition 640×260 avec bandeaux de 20 px) :**

![Avant : cent carreaux de mer distincts / Après : mer continue, quadrillage unique — deux images 320×240 natives](g04_r2_comparaison_avant_apres_640x260.png)

<details><summary>Zoom de contrôle ×4 NEAREST strict — 2560×1040</summary>

![Comparaison ×4 sans interpolation](g04_r2_comparaison_x4.png)

</details>

**Le seul changement pictural se situe dans le plateau 180×180 :** 27 577 pixels différents à l'intérieur des coordonnées `(21,53)…(201,233)` ; **zéro pixel modifié en dehors du plateau** par rapport au PNG de la livraison refusée. Le bandeau du capitaine, la perspective, le bois, les boutons, les textes, les impacts et le réticule **G5** sont conservés. L'ancienne image est conservée dans `g04_r2_avant_quadrillage_320x240.png` comme preuve exacte, pas comme référence active.

**Rendu proposé :** la précédente mosaïque de **100 crops indépendants de 18×18** est supprimée. `build_g04_r2.py` génère une **surface unique de 180×180**, à teinte et variations d'éclairage continues. Des rides courtes, irrégulières et peu contrastées évoquent l'océan, sans bordures artificielles aux multiples de 18 px. Au-dessus, **une seule grille autoritaire** de 11 verticales + 11 horizontales, épaisseur 1 px, coordonnées entières exactes. Les effets de coups sont dessinés ensuite **au-dessus** du quadrillage, à leurs positions initiales. La couche décorative reste subordonnée à la lisibilité.

**Diagnostic sans coups (pas une deuxième interface jouable) :** [mer continue nue 180×180](g04_r2_mer_continue_180x180.png) et [plateau 181×181 avec toutes les bordures, sans impacts](g04_r2_plateau_sans_impacts_181x181.png). La dimension de ce dernier est **181** pour inclure le pixel de bord x/y=180, tandis que la **surface des 100 cases reste 180×180**.

**Preuve PC en plus des anciens tests :** `verify_g04_r2.py` vérifie les **3 861 pixels de lignes uniques** dans le diagnostic sans impacts, l'absence de couleur de ligne ailleurs, le contraste de la mer, les ruptures moyennes aux anciens raccords de 18 px, la constance des 100 cases, la correspondance bit à bit des deux moitiés de la comparaison native, le zoom ×4, et l'absence de changement hors plateau. Contrôles négatifs : injection d'une **fausse ligne d'eau** détectée, suppression d'un pixel de grille détectée et altération d'un canal du PNG ×4 détectée. Mesures recueillies : luminance maximale de la mer **109,4** (<125) ; contraste/raccord moyen maximal **4,84** (<8) ; 0 modification hors plateau. Les tests sont des **garde-fous**, et **non une acceptation artistique automatique** : seule l'observation à 320×240 par le PO tranche le point « 100 cases immédiatement reconnaissables ».

**Status :** `DESIGN_PREVIEW — ON_DEVICE_UNVERIFIED`, **non validé PO**. Aucune modification à G05/G07/G08, Bible, firmware ou charte générale. La dette CYD reste reportée à l'intégration matérielle.

## Annexe visuelle ciblée — quatre états demandés (§2.5, Issue #45)

**Vignettes, pas quatre écrans concurrents :** A / sans cible → FEU indisponible ; B / FEU engagé → sélection, DÉFENSE et ABANDON verrouillés pendant l'animation ; C / résultats **des deux tirs révélés ensemble** (exemple illustratif G5 touché + B7 raté, non verdict séparé) ; D / consultation DÉFENSE puis retour ATTAQUE → cible G5 conservée avant FEU, résumé et alerte acquittés, impacts inchangés.

![Quatre vignettes ciblées des états UI à 320×240](g04_r2_etats_320x240.png)

<details><summary>Vignettes ×4 NEAREST strict — 1280 × 960</summary>

![Vignettes agrandies sans interpolation](g04_r2_etats_x4.png)

</details>

## Corrections suite à la contre-revue [#6606077196](https://github.com/Kharn27/ESP32-OTA-Battleship/pull/64#issuecomment-6606077196)

1. **Lisibilité RIPOSTE** : un seul libellé **`RIPOSTE B7 : RATÉ`**, 9px, centré verticalement dans le cartouche, sans deuxième ligne écrasée en bas. La boîte typographique est vérifiée dans le générateur (x214–299, y197–207, strictement dans la zone visible). **`NON LUE : COULÉ`** passe de **7 à 9px** et tient entièrement dans la zone d'alerte (x225–304, y220–230). L'alerte et la dernière riposte restent distinctes, sans compteur inventé.
2. **États visuels** : les quatre mini-vignettes ci-dessus sont des **PNG réels**, dessinés dans la source éditable Python. Aucune mécanique ajoutée : FEU sans cible, FEU engagé, résolution simultanée et consultation-retour DÉFENSE sont ceux de la Bible.
3. **Contrôle physique du quadrillage sur les PNG** : le vérificateur lit le fichier PNG réel et sonde indépendamment des métadonnées du rapport **121 intersections et 220 segments** sur les 11 lignes verticales et 11 horizontales de la grille, origine **(21,53)**, pas **18**, 100 cases de **18×18**, cadre **180×180**. Une mutation d'un pixel d'une ligne déclenche volontairement un échec ; la mutation d'un seul canal rouge du zoom ×4 reste également rejetée.
4. **Reproductibilité** : `Pillow==12.3.0`, police système `DejaVuSansCondensed-Bold.ttf` du paquet Debian/Ubuntu `fonts-dejavu-core` (chemin `/usr/share/fonts/truetype/dejavu/DejaVuSansCondensed-Bold.ttf`). Source Python éditable, sans prétention de source picturale vectorielle : l'illustration de fond est raster.

## Règles et périmètre

- Une **seule grille 10×10 ATTAQUE**, colonnes **A–J** et lignes **1–10** en continuité géométrique avec G08. La mer texturée est le plateau ; aucun navire ennemi ni silhouette de flotte révélée.
- Tir préparé en **G5** : réversible avant FEU et conservé au retour de DÉFENSE si FEU non engagé. Les tirs passés TOUCHÉ (`C3 E3 E4 I8`) et RATÉ (`A8 B7 D1 F9 G2 H6 J4`) sont des exemples fictifs, marqués par **formes** différentes (explosion / anneau d'écume), pas la couleur seule.
- **FEU** est dominant seulement avec une cible valide. **DÉFENSE** est une navigation volontaire ; l'indicateur `!` désigne une notification non consultée, **sans nombre**. La riposte **B7 RATÉ** et la gravité maximale plus ancienne **COULÉ** restent deux informations séparées ; consulter DÉFENSE les acquitte sans effacer l'historique des impacts.
- **ABANDON** ouvre **CONTINUER / ABANDONNER**, ne déclenche pas de sortie directe. FEU engage les deux tirs puis révèle leurs résultats synchronisés, sans interaction pendant l'animation.
- Ambiance illustration maritime riche (soleil/capitaine, bois/laiton, mer, éclairage) inspirée de la direction G01, **pas** des anciens habillages G04/G05/G07 R1 rejetés. G08 demeure approuvée pour son écran propre ; aucun fichier G08 n'est modifié ici.

## Source, provenance et reproduction PC

- `reference_concept_1448x1086.png` : illustration maritime **générée par ChatGPT**, référence picturale / atlas de matières **raster**, non asset firmware ni source vectorielle élément par élément.
- `build_g04_r2.py` : **source réellement éditable** du compositing, texte, pastiches de métal/bois, grille, cibles, effets, alertes et quatre vignettes ; tous les paramètres d'implantation sont dans le code.
- `g04_r2_attaque_320x240.png` / `_x4.png` : maquette native et zoom. `g04_r2_etats_320x240.png` / `_x4.png` : vignettes natives et zoom.
- `verify_g04_r2.py` : contrôle indépendant des **pixels des PNG relus**, du zoom NEAREST, des marques, des variantes, des rapports SHA256 et deux preuves négatives ; `generation-report.json` et `verification-PC.txt` : traces des exports réellement livrés.

Sur Linux avec Python 3 et le paquet système `fonts-dejavu-core` :

```sh
python3 -m pip install 'Pillow==12.3.0'
python3 design/ecrans/attaque/build_g04_r2.py
python3 design/ecrans/attaque/verify_g04_r2.py
```

**Preuves PC uniquement** : les contrôles ne valident ni visuellement le style auprès du PO ni matériellement le TFT, le contraste, les hitboxes stylet, la mémoire Flash/RAM et le contrat d'assets avec le Développeur. Aucune certification `ON_DEVICE_VERIFIED` revendiquée. Le matériel n'est **pas une condition de réception de cette maquette PC** ; cette dette demeure tracée pour l'intégration firmware.

**Décision attendue :** nouvelle contre-revue Pilotage sur le HEAD exact, puis présentation PO pour acceptation artistique, sans passage en Ready ni fusion par le Graphiste.
