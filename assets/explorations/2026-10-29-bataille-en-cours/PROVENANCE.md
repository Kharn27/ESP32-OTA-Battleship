# I-009 — Exploration de l’âme du jeu : scène de bataille en cours

**Statut : EXPLORATION ARTISTIQUE — NON VALIDÉE PO.**

Deux images issues de générations d’images ChatGPT, produites le 29 octobre 2026 dans le cadre de l’itération « bataille en cours » : navire principal, adversaire touché au loin, trajectoires de tir et océan au coucher du soleil.

## Provenance et exactitude

- Outil : génération d’images OpenAI dans la conversation de conception artistique I-009 (pas une capture du firmware).
- L’image `01` constitue l’illustration initiale de la scène de bataille.
- L’image `02` est une **nouvelle sortie générative** demandée comme aperçu plus pixelisé de `01` : **ce n’est pas un agrandissement déterministe, pixel par pixel, de la première image**.
- Les deux originaux sont livrés dans leur format PNG et leurs dimensions natifs de sortie de génération (1448 × 1086). Les prévisualisations 320 × 240 sont des **réductions** locales avec Pillow, méthode LANCZOS, sans nouvelle génération. Les zooms 4× sont des agrandissements exacts au plus proche voisin de ces exports 320 × 240.
- Aucun calque/source éditable structurée n’est disponible ; les PNG sont des sources images, pas des fichiers de sprites prêts à embarquer.
- Aucun test sur CYD physique, aucune validation artistique PO, aucune validation de coût Flash/RAM ni rendu en firmware.

## Fichiers

| Version | Original OpenAI | Preview écran CYD | Zoom 4× du preview | Identifiant génération |
|---|---|---|---|---|
| 01_bataille_narrative | [01_bataille_narrative_original_1448x1086.png](01_bataille_narrative_original_1448x1086.png) | ![01_bataille_narrative en 320×240](01_bataille_narrative_320x240.png) | [01_bataille_narrative_zoom4x.png](01_bataille_narrative_zoom4x.png) | `a7162bd7-fb6e-49cd-9961-a46bca06b5f5` |
| 02_bataille_pixelisee | [02_bataille_pixelisee_original_1448x1086.png](02_bataille_pixelisee_original_1448x1086.png) | ![02_bataille_pixelisee en 320×240](02_bataille_pixelisee_320x240.png) | [02_bataille_pixelisee_zoom4x.png](02_bataille_pixelisee_zoom4x.png) | `2674048b-ed4f-45a0-9107-02e4fcc94b57` |

## Réserves et vérifications

- Toutes les résolutions et checksums sont contrôlés sur les fichiers de ce dossier ; pour les valeurs détaillées voir `CONTROLES.txt`.
- Statut GitHub : PR graphique de documentation, **sans changement du firmware**.
- Aucune direction artistique n’est approuvée implicitement par la présence de ces images.
