# I-009 — Origine des scènes de bataille en cours

**ARCHIVEE / EXPLORATION_D_AMBIANCE / NON_VALIDEE_PO / NON_RUNTIME**

Deux images issues de générations OpenAI dans la conversation d'exploration artistique I-009 du 29 octobre 2026. Cette archive sert à étudier une **atmosphère de jeu indépendant contemporain en pixel art**, pas un écran tactique.

## Scène et confidentialité du gameplay

Navire du joueur au premier plan, navire ennemi touché au loin, tirs, feu et fumée, océan au coucher du soleil. **La représentation d'un navire ennemi identifié et touché constitue une illustration narrative, jamais une révélation autorisée de positions secrètes de flotte en combat.** L'affichage produit demeure régi par les décisions du LOT 1.

## Origine et exactitude

- **Génération 01 :** `a7162bd7-fb6e-49cd-9961-a46bca06b5f5`, scène de bataille narrative.
- **Génération 02 :** `2674048b-ed4f-45a0-9107-02e4fcc94b57`, **nouvelle sortie générative** visant un style plus pixelisé ; **pas** un agrandissement exact de 01.
- Originaux de génération : PNG 1448 × 1086 conservés inchangés et contrôlés dans le dépôt.
- Exports CYD : réduction 320 × 240 par Pillow/LANCZOS, sans recadrage.
- Zooms ×4 : 1280 × 960 par redimensionnement NEAREST des exports CYD, égalité pixel à pixel vérifiée.
- Comparatif : juxtaposition 640 × 240 sans interpolation supplémentaire.
- Absence de source multicouche ou d'assets sprite structurés. Images 2D d'exploration, non optimisées pour le firmware.

## Aperçus des fichiers PNG versionnés

| Version | Original PNG | Export natif 320 × 240 | Zoom ×4 |
|---|---|---|---|
| 01 — Narrative | [Original](01_bataille_narrative_original_1448x1086.png) | ![320×240](01_bataille_narrative_320x240.png) | [Zoom](01_bataille_narrative_zoom4x.png) |
| 02 — Plus pixelisée | [Original](02_bataille_pixelisee_original_1448x1086.png) | ![320×240](02_bataille_pixelisee_320x240.png) | [Zoom](02_bataille_pixelisee_zoom4x.png) |

[Comparatif natif 640 × 240](COMPARATIF_natif_2x320x240.png) — [MANIFEST_SHA256.md](MANIFEST_SHA256.md) — [CONTROLES.txt](CONTROLES.txt).

**Contrôles réalisés :** dimensions, tailles, empreintes locales SHA-256, blob Git SHA-1 comparés aux entrées de l'arbre GitHub, et équivalence des pixels pour réductions, zooms et comparatif.

**Non réalisés :** compilation firmware, consommation Flash/RAM de rendu, essai tactile / couleurs de dalle CYD physique. **NON_VALIDEE_PO** : aucune charte artistique n'est approuvée par cette archive.
