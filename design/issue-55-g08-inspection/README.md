# G08 — Inspection post-défaite VS Capitaine — reprise graphique conforme à la référence PO

**Issue :** #55 · **PR Draft :** #56 · **Statut :** `DESIGN_PREVIEW — ON_DEVICE_UNVERIFIED` · **Périmètre :** maquette PC uniquement.

## Ce qui a changé après le retour PO du 03/11/2026

Toutes les images de G08 sont désormais une **recomposition graphique fidèle de l'illustration partagée par le PO dans la conversation** : capitaine pixel art au coin supérieur gauche, lune et silhouette navale à droite, mer profonde, or des cadres, encarts bleu marine, cartouche vert, coques d'acier et commandes en volume. Il ne s'agit plus de la précédente illustration dessinée avec des pictogrammes génériques. Le fond et les ornements de l'illustration de référence sont conservés, **la grille et les états ont été redessinés** pour satisfaire la défaite et les transitions imposées.

La capture originale `reference_design_PO.png` contient un exemple graphiquement inspirant mais **non conforme au scénario de défaite** : tous les segments visibles y sont marqués touchés. Dans cette livraison, cinq segments restent **intacts** alors que la flotte humaine est hypothétiquement détruite (DÉFENSE n'est pas redessinée).

## Parcours livré

| État | Flotte sur ATTAQUE | Cartouche | Commande réversible |
|---|---|---|---|
| A, après verdict compact | cachée | FLOTTE SECRÈTE | **VOIR SA FLOTTE** |
| B, après demande volontaire | les 5 coques visibles **sous** les marques | 5 NAVIRES RÉVÉLÉS | **MASQUER** |
| C, après masquage | cachée, tous les impacts conservés | FLOTTE MASQUÉE | **AFFICHER** |

**Règles décidées, inchangées :** révélation uniquement sur **DÉFAITE VS Capitaine**, pas de confirmation, pas de fenêtre indépendante, pas de FEU, pas de modification des tirs, DÉFENSE, REJOUER et ACCUEIL accessibles. La visibilité B/C doit être conservée sur **ATTAQUE → DÉFENSE → ATTAQUE** (contrat d'intégration ; démonstration par les états, non firmware). Les cartouches vert émeraude sont **informatifs/non tactiles** et le verdict reste informatif ; seules les commandes du bas sont tactiles.

### Scénario de démonstration (strictement fictif)

Grille 10×10 (100 cases), 5 navires 5/4/3/3/2 = 17 cellules sans recouvrement, **12 TOUCHÉ** et **20 RATÉ** (disjoints), **5 segments ennemis intacts**. Les coordonnées de touches et de ratés sont **identiques** entre A, B et C. `scenario_G08.json` décrit l'intégralité du scénario, indépendamment des images. L'intention produit est de comprendre le résultat et les navires, pas d'ajouter un compteur d'inspection.

## Livrables

- `g08_inspection_320x240.png` : état principal **B** (flotte révélée), aperçu natif.
- `g08_inspection_x4.png` : état B **1280×960**, ×4 strictement `NEAREST`.
- `g08_etat_a_320x240.png`, `g08_etat_b_320x240.png`, `g08_etat_c_320x240.png` : états A/B/C natifs indépendants.
- `g08_etat_a_x4.png`, `g08_etat_b_x4.png`, `g08_etat_c_x4.png` : leurs agrandissements sans interpolation.
- `g08_variantes_320x240.png` : planche côte à côte des **3 maquettes natives** (960×240) ; `g08_variantes_x4.png` : planche ×4.
- `g08_etat_a_layers.ora`, `g08_etat_b_layers.ora`, `g08_etat_c_layers.ora` : **sources éditables multicouches OpenRaster (GIMP/Krita)**, en résolution artistique 1448×1086, couches 01 référence / 02 grille & navires / 03 commandes & état.
- `build_g08_reference.py`, `reference_design_PO.png`, `scenario_G08.json` : source reproductible et provenance. `generation-report.txt` : empreintes et assertions de génération.

### Mini-contrat UI, proposé DA (ni homologue tactile ni contrat runtime)

| Zone | Coordonnées en aperçu 320×240, arrondies | Rôle |
|---|---|---|
| En-tête (0–319, 0–39) | portrait, verdict et titre | information, non tactile |
| Grille (27–169, 50–178) | 10×10 ; ~14,2×12,8 px/case dans l'illustration | ATTAQUE, impacts visibles, non sélectionnable après verdict |
| État (180–301, 54–95) | vert à droite | information, non tactile |
| Légende (176–306, 104–181) | raté/touché/navire | information, non tactile |
| 4 boutons en bas (5–318, 194–228) | VOIR / MASQUER / AFFICHER ; REJOUER ; DÉFENSE ; ACCUEIL | commande tactile (étendue exacte à valider) |

**Compromis explicitement non homologué :** le respect de l'illustration de référence implique ici des cases visuelles ~14×13 px et non **18×18 px** comme le langage G07. La question est soumise à Pilotage/PO ; **cette maquette ne constitue pas une autorisation de changer le contrat 18×18 ou l'expérience G07**, et ne peut être déclarée acceptable sans arbitrage/contrôle. Les marges de touches et la lisibilité réelle sur écran 2,8 pouces restent à mesurer.

Les textes des boutons, les palettes, le niveau de détail, la maquette HD/ORA et les valeurs de contraste sont des **PROPOSITIONS DA**. Les règles métier décrites ci-dessus sont **DÉCIDÉES dans la Bible §14.13**. La satisfaction artistique et le fonctionnement matériel sont **À VÉRIFIER** ; aucun budget Flash/RAM, format binaire runtime ou essai physique n'est revendiqué.

## Reproduire et contrôler

```bash
python3 -m pip install pillow
python3 build_g08_reference.py
```

Le générateur vérifie la liste des navires, l'absence de recouvrement, les sets disjoints, le scénario de défaite et l'identité pixel par pixel des x4 `NEAREST`. Il produit des PNG et des calques ORA. Les sorties sont des **maquettes PC**, pas des ressources firmware.

## Provenance

- `reference_design_PO.png` : **image fournie directement par le PO dans le dialogue de travail le 03/11/2026**, reprise visuelle demandée explicitement. Auteur exact et licence externe non revendiqués ; pas de stock externe ajouté.
- `build_g08_reference.py` : reconception des calques grille/navires, impacts, boutons et états pour le présent projet.
- Typographie de génération : DejaVu Sans Condensed Bold installée sur l'environnement graphique de build ; **aucune police embarquée distribuée** ni contrainte firmware imposée.
- Respect de G01/G07 : identité maritime, verdict et ATTAQUE uniques, mais **nouvel habillage demandé par le PO** à revalider explicitement sur nouveau HEAD.

**Fusion et validation : exclusivement Pilotage.**
