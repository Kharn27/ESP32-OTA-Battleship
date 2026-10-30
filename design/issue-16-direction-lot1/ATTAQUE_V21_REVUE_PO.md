# G01 / Issue #16 — ATTAQUE V2.1, reprise artistique ciblée

**Statut** : `CORRECTIONS_REQUISES — DESIGN_PREVIEW — ON_DEVICE_UNVERIFIED — SOUMIS_A_ARBITRAGE_ARTISTIQUE_PO`.

**Base étudiée** : `attaque_g01_v2.py` et PNG V2 sur branche `design/issue-16-direction-lot1`, HEAD initial `ccc6f9d4c5bed2d7f4d189202d82df3a8a723229`. Revue Pilotage `#issuecomment-6533078329` du 30 octobre 2026.

**Périmètre réel** : ATTAQUE **seulement** ; même architecture, sans ajouter de panneau. Ni planche DÉFENSE, ni composants, ni écran d'accueil, ni modifications métier.

## Avant / après artistique — V2 → V2.1 (cinq points)

1. **Surface de mer :** aplat turquoise à quelques segments → huit bases bleu-vert et une micro-trame de vagues à plusieurs valeurs, ombres sous les crêtes, lumière ponctuelle parcimonieuse. Les marques de tirs restent au-dessus de l'eau.
2. **Écume et impacts :** éclaboussures et croix simplifiées → sprays ancrés dans la ligne d'eau, noyaux chauds entourés de remous sombres/teal, fragments de pixels intra-case. Aucune coque ennemie dévoilée.
3. **Crépuscule :** vignette isolée → nuages étagés, mer éclairée par des reflets discontinus, tête rocheuse et phare éclairés ; les coloris bleus/ambre du paysage se retrouvent dans les marges et les liserés de la grille.
4. **Matériaux UI :** cadres cuivrés plats → rainures, volumes sombres, liserés en laiton, reflets directionnels et rivets à l'échelle du pixel. FEU conserve ses coordonnées et son dominance visuelle.
5. **DÉFENSE :** dernier tir et alerte de gravité conservés dans leur commande existante ; le cas désormais affiché est `Dernier : E5 touché` tandis qu'une alerte ancienne `COULÉ` demeure **NON LUE**, indépendante et sans compteur.

## Invariants explicitement préservés

- Écran natif **320 × 240** ; zoom ×4 = 1280 × 960 en **nearest-neighbor strict**.
- **Une seule grille ATTAQUE de 10 × 10 cases** : origine (23,55), pas 18 ; en-têtes A–J / 1–10 ; même sélection **G5**.
- FEU occupe toujours le même grand périmètre tactile visuel : x=205..318, y=179..236. Palette et relief affinés, bouton ni réduit ni déplacé.
- Touches rouges distinctes des ratés blancs; aucun navire adverse, aucun groupement d'impacts en épave.
- Bandeau DÉFENSE séparant information du **dernier tir** et **gravité maximale non consultée** : `E5 touché` n'efface pas `COULÉ`. Absence de compteur. L'acquittement n'est pas simulé : c'est une maquette d'état avant consultation.
- Aucun nouveau panneau, icône métier, règle, animation ou trajet de tir.

## Sources, exports et preuves

- `attaque_g01_v21.py` : source de production **de preview**, autonome en Python/Pillow ; réutilise les fonctions de composition V2 en les renforçant avant l'assemblage. Le fichier embarque la base V2 et peut être édité indépendamment. Python hôte ≠ firmware CYD.
- `attaque_v21_320x240.png` : image native en RGB.
- `attaque_v21_x4.png` : multiplication exacte des pixels, sans interpolation.
- `attaque_v21_layers.ora` : source raster éditable (4 calques : ambiance, grille/historique, interface/FEU, écume périphérique), vérification interne de l'image fusionnée.
- `comparatif_v2_v21.png` : deux panneaux natifs 320×240 côte à côte, sans agrandissement ; les deux versions diffèrent intentionnellement aussi par le **scénario de dernier tir** pour prouver sa capacité d'affichage.

Le script produit des assertions de dimensions, exactitude du ×4 et égalité du composite ORA avec le PNG RGB. Contrôle visuel à 320 × 240 et ×4 effectué ; risques d'écran réel non levés.

## Provenance et statut technique

- Pixel art natif original dessiné par code de création graphique. **Une étude Photoshop/Firefly** de stylisation a été utilisée comme **recherche d'ambiance** et **n'est pas intégrée** aux pixels, aux sources ni aux exports remis : la génération ne garantissait pas l'exactitude typographique/fonctionnelle.
- Polices de conception via les fichiers DejaVu Sans Condensed du système hôte (non distribués dans le dépôt). Libellés modifiables dans le `.py` ; textes rasterisés dans l'ORA.
- **Non validés** : contraste et rendu effectifs de la dalle, précision stylet (case ~18 px), confort texte 9–12 px, latence de dessin et budget Flash/RAM. Contrat de conversion/runtime à définir avec le développeur lors de S02. Aucune preuve physique ni `ON_DEVICE_VERIFIED`.

**Décision PO attendue :** la V2.1 atteint-elle le niveau qualitatif de pixel art maritime souhaité, en maintenant un combat lisible à 320×240 ? Sinon, préciser exclusivement les zones encore insuffisantes. Pilotage garde seul la décision de suite, de revue et de fusion.