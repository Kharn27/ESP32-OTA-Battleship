# CYD WARSHIP

**I-009 — Bataille navale sur ESP32 CYD** · Dépôt technique : `ESP32-OTA-Battleship` (nom inchangé).

Bataille navale électronique pour ESP32 CYD classique (ESP32-2432S028R, écran tactile 320 × 240), pensée comme un jeu familial et tactique. L'exploration artistique vise un **pixel art de jeu indépendant contemporain**, riche, expressif et immersif ; **aucune charte graphique n'est encore validée par le PO**.

**État : exploration fonctionnelle. Aucun développement n'est engagé.**

**DÉCIDÉ — LOT 1 (depuis Bible v0.12) :** une partie classique complète sur une CYD **VS Capitaine** ; **Mousse et Amiral** sont reportés à des lots ultérieurs. L'ancien libellé « JOUER CONTRE MOUSSE » de l'issue graphique #6 est historique ; la maquette d'accueil a été retenue par le PO puis fusionnée dans la [PR #8](https://github.com/Kharn27/ESP32-OTA-Battleship/pull/8).

**DÉCIDÉ pour l'accueil LOT 1 (complément PO après v0.15) :** titre **CYD WARSHIP**, scène maritime fixe au coucher du soleil, riposte ennemie manquée près du navire principal et commandes **JOUER** et **PALMARÈS**, de même taille visuelle sur la maquette retenue. **JOUER** conduit **directement au déploiement VS Capitaine** dans le LOT 1 ; le combat commence après **PRÊT**. **LOT1-ACC-010 — DÉCIDÉ :** une sortie vers l'accueil est obligatoire pendant le déploiement (initial ou de revanche) avant PRÊT, sans forfait ni nouvelle partie comptabilisée ; libellé et graphisme à concevoir. L'ancien écran de sélection d'adversaire, décidé en v0.16, est **reporté** jusqu'à disponibilité de plusieurs adversaires ; Mousse, Amiral et VS CYD restent reportés. **Maquette retenue au stade DESIGN_PREVIEW / ON_DEVICE_UNVERIFIED**, sans validation runtime ni ergonomie tactile. [Cadrage détaillé dans la Bible, § 15](BIBLE_PRODUIT.md#15-identité--cyd-warship--et-écran-daccueil--cadrage-po-du-29-octobre-2026-v08).

## Référence produit

➡️ [BIBLE_PRODUIT.md](BIBLE_PRODUIT.md) — vision, contraintes, règles décidées, variantes de jeu, parcours, décisions et questions ouvertes.

🎨 [MAQUETTES.md](MAQUETTES.md) — **index de la maquette d'accueil retenue et des recherches graphiques historiques** : sept anciens SVG UX et dix fichiers A–F retirés de `main` sur décision PO, sans réécriture Git. Archives : [maquettes UX](https://github.com/Kharn27/ESP32-OTA-Battleship/tree/a768e6cabbdfba1b793f3c77c0d75f0dae001c54/assets/maquettes), [A/B/C](https://github.com/Kharn27/ESP32-OTA-Battleship/tree/a768e6cabbdfba1b793f3c77c0d75f0dae001c54/assets/explorations/2026-10-29-attaque-themes), [D/E/F](https://github.com/Kharn27/ESP32-OTA-Battleship/tree/a768e6cabbdfba1b793f3c77c0d75f0dae001c54/assets/explorations/2026-10-29-attaque-layouts) et [PR #1](https://github.com/Kharn27/ESP32-OTA-Battleship/pull/1). Les deux scènes narratives retirées de `main` restent consultables via la [PR #2](https://github.com/Kharn27/ESP32-OTA-Battleship/pull/2) et leur [dossier historique figé](https://github.com/Kharn27/ESP32-OTA-Battleship/tree/872496d73a150dd5a016c219973e5865576b7e04/assets/explorations/2026-10-29-bataille-en-cours). **Aucune référence visuelle de production approuvée.**

La bible distingue explicitement les éléments **DÉCIDÉS**, **PROPOSÉS**, **À ARBITRER**, **À VÉRIFIER** et **REPORTÉS**. Elle constitue une synthèse de travail, pas un cahier des charges validé dans son ensemble.

## Direction du jeu

- Jeu naval fluide et intuitif au stylet ; recherche d'un univers maritime en pixel art moderne sans ralentir le gameplay, avec lisibilité à confirmer sur la CYD réelle.
- Mode standard : un tir secret par joueur, résolution simultanée.
- Variante classique alternée et variantes tactiques avec salves et composition de flotte.
- Palmarès persistant propre à chaque CYD et consultable à la demande par l'adversaire.
- Fin de partie : la flotte du vainqueur reste secrète, sauf s'il choisit lui-même de la révéler.
- Liaison directe ESP-NOW : avis de faisabilité favorable de l'architecte ; CRÉER / REJOINDRE retenu, découverte et reconnexion à vérifier sur matériel.
- Mode solo contre CPU **décidé pour la cible V1.0 dans tous les modes**, avec trois difficultés et compositions tactiques adaptées.
- Coupures radio temporaires : suspension et reprise cohérente **visées en V1.0** ; reprise après redémarrage **reportée**.
- Réserve d'idées futures conservée dans la bible, sans élargir tacitement le périmètre V1.0.

## Origine

- [I-009 dans Le Grenier Geek](https://github.com/Kharn27/Le-Grenier-Geek/blob/main/plans/I-009-bataille-navale-cyd/IDEE.md).
- [Maquettes d'intention 320 × 240](https://github.com/Kharn27/Le-Grenier-Geek/tree/main/plans/I-009-bataille-navale-cyd/assets) (non figées).

Aucune user story, issue technique, architecture ou planification n'est créée à ce stade.
