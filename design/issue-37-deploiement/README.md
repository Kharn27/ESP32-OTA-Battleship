# G03 — DÉPLOIEMENT VS CAPITAINE (Issue #37)

**Responsable :** Graphiste / DA · **Statut :** `DESIGN_PREVIEW — ON_DEVICE_UNVERIFIED` · **Décision PO sur cet écran :** en attente.  
**Branche :** `design/issue-37-deploiement` · **Dépendances :** identité [G01 #16](https://github.com/Kharn27/ESP32-OTA-Battleship/issues/16), flotte validée [G02 #30](https://github.com/Kharn27/ESP32-OTA-Battleship/issues/30) / PR #32 fusionnée.  
**Périmètre :** composition 320×240 PC **uniquement**, sans firmware ni test stylet.

## Aperçus et source

- [Maquette native 320×240](g03_deploiement_320x240.png)
- [Zoom ×4 (1280×960, NEAREST)](g03_deploiement_x4.png)
- [Source vectorielle éditable](g03_deploiement_320x240.svg)
- [Export et contrôle reproductibles](export_previews.py)

**Un écran, une seule grille** : la flotte **humaine** de cinq bâtiments est visible dès l'entrée, placée légalement et prête à confirmer. Les navires affichés sont les cinq définitions SVG originales de G02, insérées **sans aucune retouche** des chemins ou couleurs. Le fond maritime turquoise continu, les accents cuivre/ambre et les contours sombres s'appuient sur G01, sans changement des assets G01/G02. Aucun navire ennemi n'est représenté.

### Géométrie vérifiable

Origine visuelle grille `(18,42)`, pas `18×18`, 10 lignes × 10 colonnes ; repères lettres A–J et chiffres 1–10. Paramètres de géométrie **de preview**, non hitboxes.

| Bâtiment | Longueur | Orientation | Cases occupées |
| --- | ---: | --- | --- |
| Porte-avions | 5 | H | B2, C2, D2, E2, F2 |
| Sous-marin | 3 | H | C6, D6, E6 |
| Contre-torpilleur | 3 | V | F5, F6, F7 |
| Croiseur | 4 | V | I4, I5, I6, I7 |
| Patrouilleur | 2 | H | G8, H8 |
| **Total** | **17** | H/V | **17 distinctes** |

**Contacts admis :** Sous-marin / Contre-torpilleur par côté `E6 / F6` ; Patrouilleur / Croiseur en diagonale `H8 / I7`. Ni collision, ni sortie de grille. Les contacts sont légaux en LOT 1, sans séparation artificielle ajoutée. Le validateur lit et vérifie les `<use data-*> ` et les transformations **réellement représentées** dans la source.

## Actions et mini-passation UI vers D07 #35

| Région visuelle proposée (x,y,w,h) | Action métier | État montré | États futurs à préparer |
| --- | --- | --- | --- |
| **AUTO** `208,47,108,48` | Refaire un **placement complet** aléatoire, répétable | Disponible | visuel pressé/retour disponible ; grille renouvelée |
| **PRÊT** `208,104,108,54` | Seule confirmation qui **démarre le combat** VS Capitaine | Disponible, accent cuivre/ambre + coche | visuel pressé/transition, protection contre double activation à spécifier côté UI |
| **RETOUR** `208,167,108,54` | Revenir à **ACCUEIL**, sans forfait tant que PRÊT n'a pas été activé ; également en revanche | Disponible, secondaire | visuel pressé puis retour accueil |
| Grille `18,42,180,180` | Afficher le placement humain automatiquement préparé ; pas d'entrée de placement manuel | Cinq coques visibles, aucune sélection | rafraîchir entièrement après AUTO, reprise flotte précédente en revanche selon Bible |

Les rectangles sont des **surfaces visuelles proposées**, pas des coordonnées tactiles définitivement arrêtées. Décalage de zone active, bords de l'écran, taille réelle de la pointe du stylet et tolérances doivent être éprouvés après T02-B / S02. Le libellé **RETOUR** et les emprises doivent être soumis au PO/Pilotage ; ce document **ne les valide pas**.

Aucune commande rotation, déplacement, glisser-déposer ou autre adversaire. **AUTO ne commence jamais la partie** ; PRÊT la commence. Il n'existe pas de choix intermédiaire de Capitaine. Cette maquette d'état « prête à confirmer » n'est ni une capture firmware, ni une spécification de séquencement événementiel.

## Production, provenance et qualité

Reproduire depuis la racine du dépôt :

```sh
python3 -m pip install 'CairoSVG==2.7.1' 'Pillow==11.3.0'
python3 design/issue-37-deploiement/export_previews.py
```

Le script vérifie les 5 types/longueurs, 17 cellules uniques, l'absence de débordement/chevauchement, les deux contacts, la transformation des positions et **l'égalité textuelle des cinq définitions G02** avec leur source sur main. Il exporte SVG → PNG 320×240 → PNG 1280×960 par `NEAREST` ; égalité RGBA complète contre le zoom attendu, contrôle négatif RGB. Le PNG est quantifié pour l'export PC, **sans décision** sur la palette ou le format embarqué.

**Inspection PC :** vue native et zoom ×4, hiérarchie lisible des trois actions ; contrastes des micro-libellés, espacements au stylet et angles de la dalle à examiner sur la vraie CYD. Le contenu du bandeau de contexte et la granularité des détails de flotte sont des propositions artistiques. **Sources des navires :** dessins originaux G02, exacts ; **nouveau fond, plaques et pictogrammes G03 :** dessins vectoriels originaux, sans image ni sprite externe. Typographie : `DejaVu Sans Condensed` du poste hôte, **substitut non embarqué et non fourni**.

**Points ouverts :** validation PO de la composition, nom RETOUR/hiérarchie, précision T02-B sur matériel, choix de fonte/glyphes accentués, méthode de rendu/rafraîchissement, stratégie de stockage en Flash et mesures RAM/latence par le Développeur. Aucun budget, atlas, conversion RGB565, bibliothèque firmware ou dimension de hitbox n'est fixé.

**Réception attendue :** Pilotage revoit le HEAD/diff, puis demande l'avis artistique explicite du PO. Fusion **par Pilotage uniquement**. `DESIGN_PREVIEW — ON_DEVICE_UNVERIFIED`.

Références : [Issue #37](https://github.com/Kharn27/ESP32-OTA-Battleship/issues/37), [D07 #35](https://github.com/Kharn27/ESP32-OTA-Battleship/issues/35), [Bible v0.17](../../BIBLE_PRODUIT.md), [politique de test](../../docs/TEST_POLICY.md), [workflow](../../docs/WORKFLOW_AGENTS.md).
