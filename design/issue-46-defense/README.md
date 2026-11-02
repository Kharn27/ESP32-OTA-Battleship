# G05 — DÉFENSE VS CAPITAINE · Issue #46

**Graphiste / Directeur artistique · LOT 1 · P1 · `DESIGN_PREVIEW — ON_DEVICE_UNVERIFIED`.**  
**Validation artistique PO : EN ATTENTE.** Revue et fusion **Pilotage exclusivement**. La PR de référence fonctionnelle G04 #48 reste **Draft** et **n'est pas une charte visuelle acceptée**.

### Maquette et sources

- [Aperçu natif 320 × 240](g05_defense_320x240.png)
- [Contrôle agrandi ×4 : 1280 × 960 NEAREST](g05_defense_x4.png)
- [Source SVG éditable](g05_defense_320x240.svg) (groupes sémantiques nommés, cinq définitions G02 intactes)
- [Script de contrôles et exports PC](export_previews.py)

Une seule **grille DÉFENSE 10×10** : la **propre flotte du joueur** (5 navires, 17 cases), jamais une grille de ciblage ni une seconde grille ATTAQUE. La commande **ATTAQUE / REVENIR** est volontaire et secondaire, **aucun FEU** sur cette vue.

### Scène illustrée et hiérarchie

L'océan est **continu**, sous la grille et jusqu'au lointain : ciel à bandes de coucher de soleil, soleil partiellement voilé, cap côtier avec phare, vagues turquoise irrégulières et reflets qui s'effacent sous les cases. Les cinq silhouettes proviennent **intégralement des définitions SVG G02 de `main`**, chemins et couleurs **strictement inchangés** : leurs formes sont ancrées à la même perspective sur la mer. Les impacts sont insérés dans l'eau et sur les coques **sans stickers ni trajectoires dessinées**. Les fanions/plaques cuivrées font écho à G01 ; pas de pile de tableaux de bord ou d'encart maritime autonome.

La **grille 180×180** domine, avec axes **A–J verticaux (lignes)** et **1–10 horizontaux (colonnes)**, pas 18 px. À droite, la mention de l'état **PATROUILLEUR COULÉ** est un **constat sur sa propre flotte**, pas une liste de tirs ni une alerte non lue. La légende RATÉ / TOUCHÉ / COULÉ est complémentaire aux **formes** des impacts (écume claire, gerbe orange sur eau sombre, fêlure/rouge et remous de submersion). Un voisinage légal entre navires **ne fusionne pas leurs silhouettes**.

### Fixture visuelle (positions compatibles G02, contrôlées dans le SVG)

Convention : **lettre de ligne + numéro de colonne**, comme G03 et G04. Il s'agit d'un scénario illustratif, **pas d'une séquence IA imposée**.

| Navire humain | Taille / orientation | Cases (17 au total) |
| --- | --- | --- |
| Porte-avions | 5 H | B2 B3 B4 B5 B6 |
| Sous-marin | 3 H | F3 F4 F5 |
| Contre-torpilleur | 3 V | E6 F6 G6 |
| Croiseur | 4 V | D9 E9 F9 G9 |
| Patrouilleur | 2 H | H7 H8 |

**Contacts licites :** F5/F6 par côté (sous-marin / contre-torpilleur) et H8/G9 en diagonale (patrouilleur / croiseur). **Aucun chevauchement, aucune sortie de grille.**

| Case | Statut historique visible | Sens |
| --- | --- | --- |
| I2 | RATÉ ancien | Gerbe écume sur eau |
| F6 | TOUCHÉ ancien | Coque humaine touchée, non coulée |
| H7 | TOUCHÉ ancien | Première atteinte du patrouilleur |
| H8 | COULÉ lors d'un coup antérieur | Seconde case du patrouilleur, type **PATROUILLEUR** annoncé ; les deux traces restent visibles |
| A7 | RATÉ **dernière riposte** | L'attaque la plus récente a manqué, après le naufrage ; aucun effacement des coups antérieurs |

**Cohérence explicite avec l'exemple G04 :** avant la consultation, le bouton côté ATTAQUE peut afficher **« dernier tir RATÉ »** et **« alerte maximale COULÉ »** du naufrage antérieur. En ouvrant DÉFENSE, **les deux informations sur la commande ATTAQUE sont acquittées** (pas de compteur, pas de mémoire de coordonnées dans ce bouton), tandis que **toutes les traces A7/I2/F6/H7/H8 restent représentées** sur la grille DÉFENSE. Les statuts historiques ne sont pas des alertes « non lues ».

### Mini-contrat d'états et navigation vers D07

| Situation métier **DÉCIDÉE** | Réponse UI proposée par G05 (**non validée PO**) |
| --- | --- |
| DÉFENSE ouverte depuis ATTAQUE, hors animation FEU | Montrer immédiatement la grille humaine et ses impacts. DÉFENSE active en titre, commande **ATTAQUE / REVENIR**. |
| Résumé de dernière riposte / alerte maximale sur ATTAQUE avant ouverture | Les deux sont **acquittés lors de l'ouverture DÉFENSE** ; ne pas afficher un duplicata sur DÉFENSE. Les marques restent sur sa grille. |
| Plusieurs manches sans ouvrir DÉFENSE | Les coups précédents restent enregistrés ; le dernier résumé côté ATTAQUE se **remplace**, et la gravité non consultée conserve son **maximum** (RATÉ < TOUCHÉ < COULÉ) avant ouverture. |
| Retour volontaire sur ATTAQUE | Basculer sans menu ; **préserver l'éventuelle cible offensive précédemment choisie**. La vue DÉFENSE est consultative : pas de nouvelle cible ni tir. |
| FEU en cours d'animation côté ATTAQUE | Bascule et demande d'abandon **ignorées sans mise en attente** jusqu'à la révélation simultanée. Aucune ouverture DÉFENSE différée. |
| Partie en cours, demande d'abandon | Conserver le contrat **CONTINUER / ABANDONNER** après PRÊT, sans retour gratuit. G05 n'invente aucune commande ou modalité nouvelle ; implantation d'un accès à la sortie à revoir avec l'ensemble du combat. |
| Après résultat définitif | Historique des dégâts toujours inspectable via les deux vues. Les transitions/verdict final relèvent d'une autre US, **pas dessinés en G05**. |

La commande **ATTAQUE** est seulement une **proposition d'implantation visuelle**, sans hitbox spécifiée pour le firmware. Les positions suggérées sont :

| Région visuelle conceptuelle | Repères pixels natifs | À vérifier |
| --- | --- | --- |
| Grille humaine (non interactive pour le tir) | x18, y42, 180×180, pas 18 | Lisibilité des coques et touches à 18 px ; aucune action de ciblage |
| Titre DÉFENSE / adversaire | bandeau y0–28 | Articulation avec la vue ATTAQUE définitive (G04 non validée visuellement) |
| Plaque PATROUILLEUR COULÉ | env. x210–316, y87–121 | Texte et hiérarchie d'un événement humain coulé |
| Repères d'impacts latéraux | env. x210–312, y130–174 ; légende x18–200, y225–239 | Densité des micro-libellés, ne pas confondre évènement et alerte |
| **ATTAQUE / REVENIR** | env. x208–317, y178–220 | Dimensions/retour au stylet, tailles et bord de l'écran **non validés** |

### Compromis, statut et provenance

**En natif 320×240 et à ×4 sans interpolation :** hiérarchie « flotte/grille → impacts → ATTAQUE », absence de FEU, contraste blanc/orange/rouge et conservation des navires visibles sont examinés sur PC. **Points fragiles** à soumettre au PO : coordonnées 8.6–9 px, légende 8.2–10.6 px, sous-texte REVENIR 8.6 px, contraste de la mer sur écran lumineux, lecture simultanée H7/H8 coulé et contact diagonal H8/G9. **TFT, tactile et stylet non essayés** ; aucune largeur de hitbox, aucun test de police embarquée.

**Provenance :** composition G05, eau/ciel/phare/plaque/effets/typo en SVG original dessiné pour cette maquette. Les cinq définitions G02 sont **textuellement reprises** depuis `design/issue-30-navires-flotte/g02_grille_320x240.svg` de `main`, vérifiées par le script ; ni source G02 ni autre ressource existante n'a été éditée. Police **DejaVu Sans Condensed** pour le raster PC : ni transmise, ni approuvée comme police firmware. **Aucune image tierce, aucun asset runtime.**

**Pour un éventuel mandat ultérieur d'assets (non démarré) :** on pourrait séparer motifs de remous, icônes d'impact raté/touché/coulé, armatures cuivre, états de commandes et petites textures d'eau en composants réutilisables **après contrat technique**. Ces éléments **ne sont pas livrés comme sprites indépendants ici**. Aucune API, compression, atlas, format Flash, palette ou budget RAM/PSRAM n'est décidé.

### Tests PC reproductibles

À la racine du dépôt (Python 3, CairoSVG et Pillow) :

```bash
python3 -m pip install 'CairoSVG==2.7.1' 'Pillow==12.3.0'
python3 design/issue-46-defense/export_previews.py
```

Le script lit **le SVG effectivement exporté** et la référence G02 : égalité des cinq définitions `<defs>`, coordonnées `data-col/data-row`, 17 cases distinctes et intérieures, contacts F5/F6 et H8/G9, les cinq historiques cohérents avec eau/coque, dernière riposte A7 ratée, 10×10/axes A–J/1–10, retour ATTAQUE sans FEU, aucune information d'alerte non lue reportée dans DÉFENSE. Il génère PNG **320×240**, zoom **1280×960 NEAREST** puis compare **tous les pixels RGBA** et un contrôle négatif RGB. Ce n'est **pas** une preuve de runtime.

Références : [Bible produit](../../BIBLE_PRODUIT.md) §§14.1/14.4–5/14.10 ; [G01](../issue-16-direction-lot1/CONTRAT_GRAPHIQUE.md) ; [G02](../issue-30-navires-flotte/) ; [G03](../issue-37-deploiement/) ; [G04 UX provisoire PR #48](https://github.com/Kharn27/ESP32-OTA-Battleship/pull/48) ; [Politique de test](../../docs/TEST_POLICY.md). **Avis artistique PO puis contre-revue et fusion Pilotage requis.**
