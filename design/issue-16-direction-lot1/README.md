# G01 — Direction visuelle LOT 1 / Issue #16

**Livraison courante : consolidation documentaire G01** — DESIGN_PREVIEW / ON_DEVICE_UNVERIFIED / EN_ATTENTE_DÉCISION_PO. **PR #19 DRAFT, non fusionnée**. Aucune modification graphique d'ATTAQUE ou de l'accueil par cette consolidation.

## Document actif

**[CONTRAT_GRAPHIQUE.md](CONTRAT_GRAPHIQUE.md)** — source de référence provisoire de G01 : identité à conserver, couleurs, boutons, marqueurs, typographies, états, inventaire candidat d'assets, références retenues / variantes rejetées / points ouverts, coordination Développeur et essais S02.

**La décision attendue du PO concerne l'identité comme base de conception, et non chaque pixel d'un écran de combat.** Le bandeau supérieur et la densité de la grille ATTAQUE sont des **points d'attention des futures missions d'écran**, pas un nouveau chantier artistique G01.

## Dossier des preuves et références

| Classe | Liens | Statut |
| --- | --- | --- |
| **Accueil retenu en conception** | [PR #8](https://github.com/Kharn27/ESP32-OTA-Battleship/pull/8) | Continuité d'univers, non runtime |
| **Trois premières planches illustrées** | Intentions consignées dans [Issue #16](https://github.com/Kharn27/ESP32-OTA-Battleship/issues/16) et la [note](ATTAQUE_RECOMPOSITION_NOTE.md) | Ambition artistique retenue, pas gabarits/exports validés |
| **Témoin graphique proposé G01** | [Note de recomposition](ATTAQUE_RECOMPOSITION_NOTE.md) ; [PNG 320×240](attaque_recomposition_320x240.png) ; [zoom x4](attaque_recomposition_x4.png) | ATTAQUE « La mer est le plateau », **non validée écran** |
| **Sources éditables courantes** | [Python](attaque_recomposition.py) ; [OpenRaster 4 calques](attaque_recomposition_layers.ora) ; [SVG tracé Adobe auxiliaire](attaque_recomposition_trace_editable.svg) | Preview uniquement, pas firmware |
| **V1/V2/V2.1 rejetées artistiquement** | [V2](ATTAQUE_EXPRESSION_V2.md) ; [V2.1](ATTAQUE_V21_REVUE_PO.md) ; ancienne série produite par render_g01.py | Archives pour comparaison, pas direction active |
| **A/B/C, D/E/F et vieilles UX/narratives** | [MAQUETTES.md](../../MAQUETTES.md) | Archives NON RETENUES ; distinctes des trois planches illustrées |

**Tests S02 non réalisés :** 18 px/case de concept, écran/stylet, bouton FEU, contrastes, textes 9–14 px, état verrouillé, latence et budgets embarqués. **Contrat DEV ouvert :** police, palette/format, transparence, partition/Flash, redraw, mémoire et timings. La preuve PC ne démontre aucune faisabilité firmware.

**Gouvernance :** [Bible produit](../../BIBLE_PRODUIT.md) source métier ; [workflow](../../docs/WORKFLOW_AGENTS.md) ; [politique de test](../../docs/TEST_POLICY.md). Seul le **PO** accepte l'identité ; **Pilotage** prend en charge synchronisation, CI, revue du nouveau HEAD et éventuelle fusion. PR #19 reste **DRAFT**, Issue #16 **ouverte** avant décision.
