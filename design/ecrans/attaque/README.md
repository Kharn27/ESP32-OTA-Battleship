# G04-R2 — ATTAQUE VS Capitaine — proposition native 320 × 240

> **Statut : `DESIGN_PREVIEW — ON_DEVICE_UNVERIFIED` · `EN REVUE PO` — NON VALIDÉ PO.**
> Issue [#45](https://github.com/Kharn27/ESP32-OTA-Battleship/issues/45) · Mission Graphiste · Branche `design/g04-r2-attaque` · Aucun asset firmware, aucune homologation tactile.

## Aperçu principal — taille réelle 320 × 240

![G04-R2 ATTAQUE native 320×240 — proposition](g04_r2_attaque_320x240.png)

<details><summary>Zoom ×4 — 1280 × 960, NEAREST strict (inspecter les pixels)</summary>

![Zoom G04-R2 ×4 nearest](g04_r2_attaque_x4.png)

</details>

## Intention et décision d'adaptation

La proposition est une **recomposition native** du **concept illustré fourni dans la conversation** : coucher de soleil, mer comme plateau, bois/cuivre, horizon, petit capitaine, marques d'écume et d'impact. Le concept HD original reste fourni **uniquement comme référence de direction** : [illustration 1448 × 1086](reference_concept_1448x1086.png). Il ne s'agit **pas** d'un simple rééchantillonnage : le passage direct de 1448×1086 à 320×240 réduisait le plateau à environ 150 px, rendait textes et zones de commande trop petites et noyait les repères tactiques. Les décorations originales sont donc condensées ; les coordonnées et textes utiles sont redessinés à l'échelle native.

La continuité G08 est **fonctionnelle / géométrique**, pas un décalque esthétique de G07 R1 (qui a été rejetée). **Grille G08 de référence :** origine `(21,53)`, cases `18×18`, surface 180×180. Les **colonnes A–J** sont en haut et les **lignes 1–10** à gauche, comme dans G08. Une seule grille est visible. Aucun navire ni silhouette adverse n'est affiché avant la défaite et la demande volontaire décrites par G08 (hors de cet écran G04).

**Nouvelle grammaire proposée DA, à soumettre au PO :** coucher de soleil illustratif en bandeau discret, mer turquoise organique continue sous la grille, encadrement marin boisé, boutons FEU/cuivre chaud et DÉFENSE/bleu profond, anciens coups dessinés comme marques *en contact avec l'eau*. Suppression des bibelots HD qui diminueraient les cases. Aucun élément de G07 R1 rejeté n'est repris comme style.

## Scénario de preuve et contrat d'états (illustratif, sans nouvelle règle)

| Région / état | Représentation proposée | Contrat fonctionnel conservé |
| --- | --- | --- |
| Grille `(21,53)` — 10×10 | 100 cases de 18px ; croix/gerbes **TOUCHÉ**, anneaux d'écume **RATÉ** | Coup passé bloqué ; aucune silhouette ennemie, aucune attribution d'épave |
| **G5** préparée | Quatre coins ivoire/cyan et petit point central, sans gerbe de résultat | Cible inconnue réversible avant FEU, conservée après un aller-retour DÉFENSE |
| **FEU** `(210,56)–(317,94)` | Cuivre orangé dominant, activé et cible G5 indiquée | Sans cible : état visuel désactivé ; au déclenchement, verrouiller sélection/bascule/abandon ; résultats synchronisés après animation |
| **DÉFENSE** `(210,98)–(317,129)` | Bouton bleu de navigation avec `!` **non numérique** | Navigation volontaire immédiate, sauf FEU engagé ; acquitte le résumé et l'alerte, sans effacer les impacts |
| **ABANDON** `(210,132)–(317,153)` | Plaque secondaire mate, drapeau | Ouvre `CONTINUER / ABANDONNER`, ne quitte jamais directement |
| Historique `(210,156)–(317,190)` | Deux tirs illustratifs, `E7 TOUCHÉ`, `C2 RATÉ` | Historique persistant des impacts (extraits affichés, pas de suppression de tirs) |
| Dernière riposte `(210,193)–(317,211)` | Fond ivoire **`RIPOSTE : B7 RATÉ`** | Seulement le dernier tir du Capitaine ; indépendant de l'alerte maximale |
| Alerte non lue `(210,214)–(317,234)` | Bandeau rouge et avertissement **`NON LUE : COULÉ`** | Alerte plus ancienne et maximale non encore consultée, **sans compteur inventé** |

**Mini-annotations d'états (pas d'autres grands écrans)** : sans cible inconnue sélectionnée, FEU est désactivé ; pendant l'animation de FEU, cible, navigation et abandon sont indisponibles, les deux coups étant révélés ensemble ; revenir de DÉFENSE conserve G5 si aucun FEU n'a été lancé. La consultation de DÉFENSE acquitte résumé/alerte et n'efface pas les marques de grille. L'écran post-verdict et la révélation volontaire restent du ressort de G07/G08 et **ne sont pas redessinés ici**.

## Sources, génération et preuves PC

- `g04_r2_attaque.svg` : **source vectorielle native entièrement éditable**, avec tracés/rectangles/texte SVG réels, pas de bitmap encapsulé.
- `build_g04_r2.py` : génération déterministe des calques vectoriels/composition, export PNG avec CairoSVG, zoom ×4 avec Pillow NEAREST, rapport SHA256.
- `verify_g04_r2.py` : contrôle indépendant du XML SVG, 22 segments de grille, coordonnées/impacts, textes, dimensions relues, **comparaison pixel à pixel des 1 228 800 pixels agrandis**, test négatif sur un **canal R modifié** qui doit échouer, hashes réels.
- `generation-report.json` et `verification-PC.txt` : résultats de l'export et de la vérification PC sur les fichiers remis.
- `reference_concept_1448x1086.png` : **référence d'intention issue de génération d'image ChatGPT**, non asset embarqué, non source native. Les fichiers de maquette finale sont redessinés par source vectorielle scriptée.

Reproduire sur PC :

```bash
python3 -m pip install Pillow CairoSVG
python3 design/ecrans/attaque/build_g04_r2.py
python3 design/ecrans/attaque/verify_g04_r2.py
```

La source SVG et les PNG se superposent visuellement via CairoSVG sur PC. La police hôte proposée est **DejaVu Sans Condensed**, non embarquée ni licenciée comme ressource firmware. Les contours restent pixel-alignés autant que possible ; les dégradés et glyphes utilisent le rendu PC.

## Qualité et réserves

- **Vérifié PC :** 320×240 exact, cadrage 180×180, 18px/case, source réellement éditable, FEU/DÉFENSE/ABANDON et résumé/alerte distincts, aucune grille ennemie révélée, zoom NEAREST exact, contrôle négatif PASS, SHA rapportés.
- **À juger par le PO :** style maritime, richesse suffisante sans surcharge, contraste des marqueurs, équilibre décor/grille, libellés et emplacement proposés, cohérence artistique avec G08 (qui reste accepté pour son écran propre).
- **À vérifier sur la CYD physique et avec le Développeur :** finesse des textes de 6,5–9 px, confort des zones de 18 px au stylet, perception des touches ratées/touchées, alerte, contraste TFT, format/couleurs, consommation Flash/RAM, rendu CPU. Ces valeurs **ne sont pas homologuées** et aucune carte SD/PSRAM n'est supposée.
- **Hors périmètre :** écrans G05-R2/G07-R2/G08, règles de la Bible, firmware, assets runtime/format de compression.

**Le PO valide l'art. Pilotage examine et fusionne seul la PR.**
