# I-009 — Scènes narratives « Bataille en cours » (archives)

**ARCHIVEE / EXPLORATION_D_AMBIANCE / NON_VALIDEE_PO / NON_RUNTIME**

> **FRONTIÈRE PRODUIT :** ces scènes montrent, à titre **strictement narratif**, un navire ennemi visible, touché et en feu, avec des tirs vers l'horizon. **Ce ne sont pas des écrans de gameplay** et elles n'autorisent jamais la révélation des positions inconnues de la flotte adverse en combat. Les règles du LOT 1 demeurent celles de [BIBLE_PRODUIT.md](../../../BIBLE_PRODUIT.md).

Cette archive regroupe deux générations d'images OpenAI du 29 octobre 2026 : contre-torpilleur au premier plan, tir vers l'horizon, océan au couchant et adversaire touché. Les **originaux PNG exacts et les exports natifs** sont désormais **versionnés dans le dépôt sur `main`**. Ils ont été vérifiés par comparaison des Git blob SHA-1 du dépôt aux octets locaux et par rapprochement avec leurs tailles et empreintes SHA-256.

## Aperçus authentiques au format CYD — PNG 320 × 240

| 01 — Narrative | 02 — Plus pixelisée |
|:---:|:---:|
| ![01 en 320x240](01_bataille_narrative_320x240.png) | ![02 en 320x240](02_bataille_pixelisee_320x240.png) |
| [Original PNG 1448 × 1086](01_bataille_narrative_original_1448x1086.png) | [Original PNG 1448 × 1086](02_bataille_pixelisee_original_1448x1086.png) |
| [Zoom ×4 sans interpolation](01_bataille_narrative_zoom4x.png) | [Zoom ×4 sans interpolation](02_bataille_pixelisee_zoom4x.png) |

[Comparatif des deux exports natifs côte à côte, 640 × 240](COMPARATIF_natif_2x320x240.png).

Les quatre anciens JPEG (`*_revue.jpg` et `*_preview_320x240.jpg`) sont conservés uniquement comme **copies de consultation avec pertes**. Les PNG liés ci-dessus sont les références documentaires.

## Provenance exacte

| Version | Identifiant de génération OpenAI | Original exact | Réduction écran CYD |
|---|---|---|---|
| 01 — Narrative | `a7162bd7-fb6e-49cd-9961-a46bca06b5f5` | `01_bataille_narrative_original_1448x1086.png` | `01_bataille_narrative_320x240.png` |
| 02 — Plus pixelisée | `2674048b-ed4f-45a0-9107-02e4fcc94b57` | `02_bataille_pixelisee_original_1448x1086.png` | `02_bataille_pixelisee_320x240.png` |

**La génération 02 est une nouvelle sortie générative**, et non un agrandissement de 01. Les deux originaux PNG 1448 × 1086 sont **inchangés**. Les prévisualisations 320 × 240 ont été réduites avec Pillow/LANCZOS sans recadrage (rapport 4:3) ; les zooms 1280 × 960 emploient le plus proche voisin (NEAREST), vérifié pixel à pixel. Le comparatif 640 × 240 juxtapose directement les deux exports natifs sans autre interpolation.

Consulter [PROVENANCE.md](PROVENANCE.md), [MANIFEST_SHA256.md](MANIFEST_SHA256.md) (dimensions, tailles, SHA-256 et Git blob SHA-1) et [IMPORT_ORIGINAUX.md](IMPORT_ORIGINAUX.md) (journal de la levée du blocage initial). Le document [CONTROLES.txt](CONTROLES.txt) reprend les contrôles locaux.

## Intention et réserves

Océan bleu pétrole et turquoise, écume froide et reflet or/corail au coucher du soleil, fumée anthracite. Le rapport entre le **navire proche** et le **navire endommagé lointain** raconte une bataille sans constituer une proposition de système de révélation des flottes.

**DESIGN_PREVIEW uniquement :** aucune charte validée, aucun asset runtime prêt pour ESP32, aucun budget Flash/RAM mesuré, aucune compilation ni aucun test tactile sur CYD physique.

Les [PR #1](https://github.com/Kharn27/ESP32-OTA-Battleship/pull/1) et [PR #2](https://github.com/Kharn27/ESP32-OTA-Battleship/pull/2) sont fusionnées. [MAQUETTES.md](../../../MAQUETTES.md) distingue les archives historiques A–F de ces deux scènes narratives, toujours **non validées par le PO**.

**Statut : ORIGINAUX ET EXPORTS ARCHIVÉS SUR `main` (PR #2 fusionnée) — CONTRÔLES BINAIRES CONFORMES — NON_VALIDEE_PO — NON_RUNTIME — ON_DEVICE_UNVERIFIED.** La fusion documentaire ne constitue aucune approbation artistique ni autorisation d'intégration au firmware.
