# G08 — Inspection post-défaite VS Capitaine / Issue #55

**Statut : DESIGN_PREVIEW — ON_DEVICE_UNVERIFIED**. Validation artistique PO requise. Revue et fusion réservées au Pilotage.

## Objet

Un seul champ de bataille **ATTAQUE G07** avec grille de 100 cases de 18 x 18 pixels ; aucune fenêtre supplémentaire, aucun dialogue, aucun FEU. Le générateur Python G08 réutilise le code graphique G07 fusionné (mer, escorte militaire, grille, impacts, boutons cuivrés, verdict non tactile) depuis le même dépôt, sans modifier les fichiers G07.

La grande maquette 320 x 240 représente **B : flotte révélée volontairement**. Une planche limitée 320 x 240 illustre A/B/C. Sources éditables SVG/Python, PNG natifs et zoom x4 NEAREST.

## Machine à états visuelle

| État | Flotte adverse | Bouton proposé | Transition |
|---|---|---|---|
| **A** après verdict compact DÉFAITE | **SECRÈTE** ; seuls impacts visibles | **VOIR SA FLOTTE** (décidé PO) | premier appui volontaire vers B |
| **B** après révélation | 5 navires / 17 segments sous les impacts | **MASQUER FLOTTE** (proposé DA) | appui vers C |
| **C** après masquage | **SECRÈTE à nouveau**, marques inchangées | **AFFICHER FLOTTE** (proposé DA) | appui immédiat vers B |

**DÉCIDÉ PO :** la visibilité B ou C est préservée lors de ATTAQUE → DÉFENSE → ATTAQUE ; la vue DÉFENSE garde sa propre flotte (non refondue). Les commandes REJOUER et ACCUEIL subsistent ; l'issue du combat et le palmarès sont intacts. Aucun autre navire sélectionnable, aucune fiche, aucun FEU. La commande n'existe qu'après DÉFAITE VS Capitaine, ni en VICTOIRE ni en NUL.

## Exemple de données, entièrement FICTIF

Cinq navires adverses sans recouvrement : **B2–F2 (5)**, **I4–I7 (4)**, **D6–F6 (3)**, **A8–A10 (3)**, **G9–H9 (2)**. **12 TOUCHÉ, 27 RATÉ, 5 cases ennemies encore intactes** : F2, I7, F6, A10, H9. La flotte ennemie n'est donc pas entièrement détruite. En parallèle, la dernière salve simultanée de Capitaine a détruit la flotte humaine sur DÉFENSE : il s'agit bien d'une DÉFAITE cohérente, et non de la VICTOIRE G07 renommée.

Les tirs sont identiques cellule par cellule sur A, B et C ; les coques apparaissent sous les marqueurs, et n'introduisent jamais de nouvelle explosion. Les navires peuvent légalement se toucher, mais les positions illustratives sont simplement valides.

## Implantation 320 x 240 et zones proposées (NON homologuées sur CYD)

- Grille : 100 cases, ancrage cellule (21+18*c, 53+18*r), cases 18 x 18.
- Haut : ATTAQUE information ; DÉFENSE tactile (101,4,103,34) ; RÉSULTAT/DÉFAITE/VS CAPITAINE texte seul sans relief, non tactile.
- Actions tactiles : REJOUER (212,146,104,28) ; ACCUEIL (212,180,104,27) ; VOIR/MASQUER/AFFICHER FLOTTE (212,212,104,24).
- Inscription d'état (208,49,110,27), informative et non cliquable.
- Ratés : écume claire G07 ; touches : explosions cuivre/écume G07 ; coques acier gris-vert sous les marques ; mer, lumière et escorte militaire G07 préservées.
- Fonte DejaVu Sans PC et tailles ≈ 7–12 px héritées G07 : **aucune police firmware choisie ni confort stylet validé**.

**Proposé DA :** formes, palette de coques, indicateur d'état, textes MASQUER FLOTTE et AFFICHER FLOTTE, zones précises. **À VÉRIFIER CYD :** lecture native TFT, contrastes, hitbox de 24 pixels, export Flash et coûts RAM, méthode d'affichage sans framebuffer exigé. Aucune consommation mémoire annoncée.

## Livrables

- build_g08.py : **source de dessin éditable**, indépendante de la génération d'images, import des primitives existantes G07.
- g08_inspection_320x240.svg et .png : vue B principale.
- g08_inspection_x4.png : 1280 x 960 **nearest-neighbor strict**.
- g08_variantes_320x240.svg et .png : A/B/C, parcours et persistance.
- g08_variantes_x4.png : 1280 x 960 **nearest-neighbor strict**.
- g08_etat_a_320x240.svg et g08_etat_c_320x240.svg : états cachés éditables.
- generation-report.txt : logs de contrôles et empreintes SHA256.

Reproduction depuis la racine du dépôt (Python 3 + CairoSVG + Pillow) :

    python3 -m pip install cairosvg pillow
    python3 design/issue-55-g08-inspection/build_g08.py

**Assertions non tautologiques :** 5 navires continus et distincts (5/4/3/3/2), 100 cases, 17 cellules occupées, hits disjoints des misses, 5 segments ennemis intacts ; contrôle du SVG réellement produit (100 data-cell, coques uniquement en B, calque des coques sous impacts, tirs A/B/C à coordonnées identiques, verdict sans cadre tactile, quatre vraies commandes, absence FEU), vérification des PNG 320 x 240 et zoom 1280 x 960 reproduisant exactement chaque pixel en carré 4 x 4. Rapport SHA256 des fichiers du HEAD.

**Provenance :** scène originale G07 du dépôt (réutilisée par import) ; dessin G08 codé et éditable ici. L'image conversationnelle générée initialement à 1448 x 1086 **n'est pas employée** : elle n'est pas conforme à l'Issue ni considérée comme un asset runtime. Aucun tiers visuel supplémentaire, aucune modification G01–G07 ou firmware.

**Décision artistique PO en attente.** Statut exclusivement DESIGN_PREVIEW / ON_DEVICE_UNVERIFIED ; aucune preuve d'essai physique.
