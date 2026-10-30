# CYD WARSHIP

**I-009 — Bataille navale sur ESP32 CYD** · Dépôt technique : `ESP32-OTA-Battleship` (nom inchangé).

Bataille navale électronique pour ESP32 CYD classique (ESP32-2432S028R, écran tactile 320 × 240), pensée comme un jeu familial et tactique. L'exploration artistique vise un **pixel art de jeu indépendant contemporain**, riche, expressif et immersif ; **aucune charte graphique n'est encore validée par le PO**.

**État : exploration fonctionnelle. Aucun développement n'est engagé.**

**LOT 1 :** bataille classique complète sur une CYD contre **Capitaine**. Le parcours de l'accueil, les règles actives, les exclusions et les décisions remplacées sont définis **uniquement dans la [Bible produit](BIBLE_PRODUIT.md)** (§ 14–16). Ce README est un point d'entrée, pas une seconde spécification.

## Référence produit

➡️ [BIBLE_PRODUIT.md](BIBLE_PRODUIT.md) — vision, contraintes, règles décidées, variantes de jeu, parcours, décisions et questions ouvertes.

🎨 [MAQUETTES.md](MAQUETTES.md) — **index de la maquette d'accueil retenue et des recherches graphiques historiques** : sept anciens SVG UX et dix fichiers A–F retirés de `main` sur décision PO, sans réécriture Git. Archives : [maquettes UX](https://github.com/Kharn27/ESP32-OTA-Battleship/tree/a768e6cabbdfba1b793f3c77c0d75f0dae001c54/assets/maquettes), [A/B/C](https://github.com/Kharn27/ESP32-OTA-Battleship/tree/a768e6cabbdfba1b793f3c77c0d75f0dae001c54/assets/explorations/2026-10-29-attaque-themes), [D/E/F](https://github.com/Kharn27/ESP32-OTA-Battleship/tree/a768e6cabbdfba1b793f3c77c0d75f0dae001c54/assets/explorations/2026-10-29-attaque-layouts) et [PR #1](https://github.com/Kharn27/ESP32-OTA-Battleship/pull/1). Les deux scènes narratives retirées de `main` restent consultables via la [PR #2](https://github.com/Kharn27/ESP32-OTA-Battleship/pull/2) et leur [dossier historique figé](https://github.com/Kharn27/ESP32-OTA-Battleship/tree/872496d73a150dd5a016c219973e5865576b7e04/assets/explorations/2026-10-29-bataille-en-cours). **Aucune référence visuelle de production approuvée.**

La bible distingue explicitement les éléments **DÉCIDÉS**, **PROPOSÉS**, **À ARBITRER**, **À VÉRIFIER** et **REPORTÉS**. Elle constitue une synthèse de travail, pas un cahier des charges validé dans son ensemble.

## Organisation du développement multi-agents

Les [GitHub Issues du dépôt](https://github.com/Kharn27/ESP32-OTA-Battleship/issues) constituent le **backlog officiel**, piloté et mis à jour directement par le Chef de projet à l'aide du connecteur GitHub. Les PR contiennent les livraisons, revues et preuves. Le GitHub Project expérimental n'est pas requis et n'est pas la source de vérité.

- [Workflow multi-agents et suivi par Issues](docs/WORKFLOW_AGENTS.md) — types, statuts, priorités, responsables, branches, preuves et fusions.
- [Politique de preuves et de tests](docs/TEST_POLICY.md) — PC, build cible, CI, CYD physique et ESP-NOW à deux cartes.
- [Modèle d'Issue](.github/ISSUE_TEMPLATE/user_story.yml) et [modèle de PR](.github/pull_request_template.md).

Ce dispositif ne crée pas de backlog fonctionnel et ne lance aucun développement. Les champs de statut et d'attribution sont portés par chaque Issue, sans tableau séparé à administrer.

## Intention

Un jeu naval familial, lisible et fluide au stylet, avec une identité maritime expressive. Les expériences V1.0, contraintes, variantes et choix de règles sont décrits dans la **Bible produit** ; ils ne sont pas redéfinis ici.

## Origine

- [I-009 dans Le Grenier Geek](https://github.com/Kharn27/Le-Grenier-Geek/blob/main/plans/I-009-bataille-navale-cyd/IDEE.md).
- [Maquettes d'intention 320 × 240](https://github.com/Kharn27/Le-Grenier-Geek/tree/main/plans/I-009-bataille-navale-cyd/assets) (non figées).

Aucune user story, issue technique, architecture ou planification n'est créée à ce stade.
