# GRAPH-ACC-001 — Accueil CYD WARSHIP — remise image PO

**Mission :** première maquette complète de l’accueil CYD WARSHIP, LOT 1.
**Origine :** dernière image générée et explicitement retenue par le PO dans la conversation, avec deux boutons de même taille, libellés **JOUER** et **PALMARÈS**, titre **CYD WARSHIP**, navire au premier plan tirant vers les bâtiments ennemis éloignés. Aucun changement artistique effectué lors de l’export.

## Livrables de la branche (exactement trois fichiers graphiques)

1. `accueil_cyd_warship_320x240.png` : composite RGB de **320 × 240 px**. Mise à l’échelle depuis la composition source 1448 × 1086 par filtre **Lanczos**, en une seule passe. PNG de référence pour revue à l’échelle native.
2. `accueil_cyd_warship_x4.png` : **1280 × 960 px**, agrandissement par voisin le plus proche depuis le PNG natif, chaque pixel répliqué sur un carré **4 × 4**, sans interpolation.
3. `accueil_cyd_warship.svg` : conteneur SVG **autonome** intégrant la composition raster originale 1448 × 1086 sans lien externe. Source ouvrable et modifiable dans un logiciel graphique ; **ce n’est pas une illustration vectorielle à composants indépendants** ni un fichier en calques éditables. L’image originale haute définition y reste préservée. Les moteurs de rendu SVG peuvent effectuer un rééchantillonnage différent : le PNG 320 × 240 fait foi.

**Aucun calque transparent isolé livré.** Le fichier `effet_riposte_320x240.png` hérité des corrections précédentes est supprimé du tree actif ; le nouvel SVG n’y fait aucune référence.

## Arbitrages PO transmis

- **Reprendre exactement la dernière image sélectionnée**, pas les anciens rendus de la PR #8.
- Titre et boutons conservés dans le style de cette image, boutons **de mêmes dimensions visuelles**.
- Premier libellé réduit à **JOUER** ; second inchangé **PALMARÈS**.
- Le navire allié au premier plan tire en direction des bâtiments lointains ; le ciel, les bâtiments, la mer, les tirs, la gerbe et les reflets de **cette même image** sont préservés par le simple export.
- L’illustration fait l’objet d’une maquette d’accueil, **pas** d’un asset runtime confirmé ni d’un arbitrage supplémentaire sur la pixelisation.

## Contraintes et contrôles

- **ASSET_EXPORT_VERIFIED (PC)** : dimensions 320 × 240 et 1280 × 960 ; réplication x4 testée au niveau des pixels ; PNG RGB décodable ; SVG XML syntaxiquement valide ; fond intégré auto-contenu et vérifié identique à l’image source.
- **DESIGN_PREVIEW / ON_DEVICE_UNVERIFIED** : aucun build ESP32, aucune mesure Flash/RAM, aucun test tactile ou contrôle physique du rendu CYD. Les tailles fichiers ne démontrent pas la faisabilité Flash.
- **Écran 320 × 240** ; hypothèses de rectangles tactiles héritées des versions précédentes **invalidées pour cette nouvelle composition**. Il faudra mesurer les deux plaques dans les nouveaux exports avant tout contrat de coordonnées firmware.
- **Statut PR : DRAFT / NON FUSIONNÉE**. La décision de fusion est réservée au Pilotage, après décision PO.

## Histoire du changement

Cette livraison remplace les anciennes scènes de la PR #8 par **un seul écran composite complet**, sans accumulation de variantes. Les PNG natifs et zoom ont été écrasés. Le SVG a été reconstruit autour de la nouvelle source de manière autonome ; le calque transparent de riposte a été retiré du tree courant.
