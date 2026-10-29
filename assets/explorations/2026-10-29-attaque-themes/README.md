# Exploration artistique ATTAQUE — trois thèmes candidats (29 octobre 2026)

**Statut : PROPOSÉ / DESIGN_PREVIEW — NON VALIDÉ par le PO.**
Ce dossier archive trois propositions visuelles **SVG natifs 320 × 240** pour `LOT1-UX-001/002/003`.
Ces fichiers sont des **sources éditables et visualisables directement sur GitHub**, non des assets firmware, ni des captures de la vraie CYD.

| A — Abysses électriques | B — Archipel | C — Escadre |
| :---: | :---: | :---: |
| ![Abysses](A_abysses_electriques.svg) | ![Archipel](B_archipel.svg) | ![Escadre](C_escadre.svg) |
| Bleu profond / néon turquoise / corail | Azur / sable / orange | Pétrole / acier / cuivre |

## Contexte commun, conforme à la Bible produit v0.6

- Classique **10 × 10 VS MOUSSE**, mono-tir simultané, **une seule grille ATTAQUE** à la fois.
- E6 est sélectionnée mais **non verrouillée** ; C3 est déjà touchée ; H8 est déjà manquée.
- FEU verrouille irrévocablement la cible sans déclencher de bascule automatique.
- DÉFENSE reste consultable immédiatement, avec **alerte de navire touché non consultée**.
- Les cases déjà attaquées ne sont plus sélectionnables ; **la flotte adverse reste secrète**.
- Les illustrations de navire hors-grille représentent **votre escadre**, pas des positions ennemies.

## Retour exploratoire reçu

Le PO apprécie **les trois ambiances**, et a suggéré la possibilité de **thèmes interchangeables**.
**Ce retour ne vaut pas approbation d'une direction graphique, ni décision d'implémenter la sélection de thèmes.**
Un deuxième axe d'étude compare **différentes dispositions de l'écran** (grille centrale, navigation latérale, barre de commandes basse).

## Preuves et réserves

- Les 3 SVG ont le canevas `320 × 240` et restent éditables ; les exports PNG natifs et zooms 4× sont **disponibles dans le dossier exploratoire fourni au PO hors GitHub**, non versionnés dans cette PR.
- **Aucun test au stylet sur CYD** ; les cellules font 15 × 15 px et la lisibilité des libellés reste à éprouver.
- Les SVG emploient du vectoriel et des typographies de PC : **formats d'exploration, pas sprites optimisés**.
- Aucune estimation RAM/Flash de rendu, aucune compilation firmware, aucune PR à fusionner sans revue Pilotage.
- Voir [les notes artistiques](NOTES_ARTISTIQUES.md) pour intention, palettes, animations et compromis.

**Aucune charte graphique validée. Aucune modification du dépôt d'idées Le-Grenier-Geek.**

## Exploration complémentaire des agencements

À la demande du PO, les [trois variantes D/E/F](../2026-10-29-attaque-layouts/README.md) comparent **la disposition** des zones tactiles indépendamment des univers de couleur A/B/C. Les six pistes demeurent **NON VALIDÉES**.
