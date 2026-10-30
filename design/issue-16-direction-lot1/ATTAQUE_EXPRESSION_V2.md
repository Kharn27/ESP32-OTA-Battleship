# G01 / Issue #16 — Proposition ATTAQUE expressive V2 (arbitrage PO)

**Statut de cette proposition :** `DESIGN_PREVIEW — ON_DEVICE_UNVERIFIED — EN_ATTENTE_RETOUR_ARTISTIQUE_PO`.
**Statut de la direction V1 en PR #19 :** rejet artistique explicite PO. Il ne devient pas « validé » par l'ajout de la V2.
**Périmètre de la reprise :** **une seule** nouvelle preview ATTAQUE au vrai 320×240 et son zoom ×4. Ni DÉFENSE, ni planche de conventions finale, ni écran d'accueil modifié. L'ancienne branche et ses fichiers restent pour mémoire et comparaison.

## Références et parti pris

- Source métier : `BIBLE_PRODUIT.md` v0.17 (§14.1, §14.4, §14.5, §14.10.2 et §15), sans ajouter de règles.
- Référence d'identité acceptée : accueil `design/graph-acc-001/` / PR #8, non modifié.
- Références de **style uniquement** : trois planches illustratives initiales fournies dans la conversation de lancement G01 (coucher de soleil, mer turquoise, écume, plaques cuivrées, typographie maritime, actions expressives). Ces planches 1448×1086 ne sont pas des exports runtime, des grilles approuvées ou des éléments copiés dans la V2.
- Retour PO et Pilotage : PR #19, commentaires 6529112559 et 6529712012. La V1 géométrique est artistiquement rejetée. **Spectacle en périphérie, repos visuel au centre du combat.**

## Conception de la V2

La zone maritime crépusculaire (phare, côte rocheuse, soleil et reflets) est concentrée dans une petite fenêtre sur la droite. Les 100 cases d'eau ont une texture délicate, mais assez uniforme pour ne pas concurrencer les informations. Les repères d'impact sont dessinés à l'intérieur de la surface de chaque case : écume blanche et relief turquoise pour un raté ; braise rouge, noyau jaune et petites étincelles pour une touche ; quatre angles cyan pour une sélection. La commande principale FEU est orange et la plus saillante, hors grille. Les plaques cuivrées ajoutent une identité navale de jeu sans reprendre un tableau de bord militaire.

### Exemple sémantique intentionnel sur l'onglet DÉFENSE

L'écran montre **« Dernier : raté »** et, indépendamment, **« Alerte non lue : COULÉ »**. C'est le scénario décidé au §14.10.2 : un navire humain a coulé à une manche antérieure, le joueur n'a pas consulté DÉFENSE, puis Capitaine a raté la dernière riposte. L'alerte maximale non consultée reste donc « COULÉ » ; le résumé de dernière riposte dit bien « raté ». Le badge **n'a aucun compteur**.

Variante sémantique future **non dessinée** : si le dernier tir touche, sa coordonnée et le résultat (par exemple « E5 touché ») prennent la place du court texte « Dernier : raté », sans écraser la gravité maximale. À l'ouverture volontaire de DÉFENSE, le **résumé sur la commande et l'alerte** sont acquittés ; les impacts historiques restent visibles sur la grille du joueur. Aucune bascule différée durant l'animation FEU.

Les touches sur ATTAQUE ne sont pas reliées à une coque, une ombre d'épave ou un groupe de cellules. L'annonce d'un navire coulé est **textuelle et par type**, sans révéler l'appartenance des cases (§14.4). Aucun ennemi n'est dessiné dans la fenêtre décorative.

## Livrables de cette étape limitée

- `attaque_g01_v2.py` : code source éditable pour les couleurs, les coordonnées, la scène de crépuscule, les marqueurs, les plaques et tous les textes ; chaque composant peut être modifié indépendamment.
- `attaque_expression_layers.ora` : source *raster en quatre calques* OpenRaster, éditable notamment dans Krita/GIMP. Le texte du fichier ORA est pixellisé (pas un calque typographique vectoriel) ; **les libellés restent éditables dans le script**.
- `attaque_expression_320x240.png` : composite natif, sans rééchantillonnage d'une planche HD.
- `attaque_expression_x4.png` : 1280×960, réplication des pixels ×4 en nearest-neighbor.

### Contrôles PC intégrés au générateur

Dimension native 320×240 ; zoom ×4 1280×960 ; comparaison stricte des pixels nearest-neighbor ; contrôle que le fichier ORA contient 4 calques et que son composite `mergedimage.png` égale la preview. Vérifier visuellement les deux tailles et les libellés après chaque modification.

## Réserves et questions PO

- **Proposé, non validé :** niveau d'ornement des cadres, contraste du bleu de la grille, palette or/cuivre, effet de braise/écume et vignette du phare. L'accueil approuvé n'est pas une charte complète.
- **À éprouver S02 :** cases ~18×18 px (peuvent être trop petites au stylet), largeur de la commande DÉFENSE, confort de FEU, lettrage de 10–12 px, contraste et délai d'affichage sur vraie dalle. Le texte d'alerte tient ici sur deux lignes mais une variation plus longue peut nécessiter une forme courte.
- **Contrat technique futur :** aucun budget chiffré RAM/Flash, aucun format/compression runtime, aucune cible de FPS ni police embarquée présumés. Les PNG et l'ORA ne sont pas des assets firmware.
- **Provenance :** illustration originale dessinée directement en pixels avec Pillow ; pas d'assets tiers visuels ni d'image générative incorporée. Police hôte DejaVu Sans Condensed (non distribuée). L'illustration d'accueil n'est pas modifiée ni incorporée.

**Arbitrage artistique demandé au PO :** cette V2 réintroduit-elle suffisamment le caractère de « véritable petit jeu indépendant » des premières planches, tout en préservant le confort visuel de la grille ? Si oui, ses principes serviront de base **proposée** à la déclinaison ultérieure DÉFENSE/conventions ; si non, concentrer les corrections artistiques sur cette seule preview avant toute propagation.