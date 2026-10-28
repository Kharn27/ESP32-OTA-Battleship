# ESP32-OTA-Battleship

Bataille navale électronique pour ESP32 CYD classique (ESP32-2432S028R, écran tactile 320 × 240), pensée comme un jeu familial, tactique et rétro.

**État : exploration fonctionnelle. Aucun développement n'est engagé.**

## Référence produit

➡️ [BIBLE_PRODUIT.md](BIBLE_PRODUIT.md) — vision, contraintes, règles décidées, variantes de jeu, parcours, décisions et questions ouvertes.

🎨 [MAQUETTES.md](MAQUETTES.md) — **galerie illustrée** : trois maquettes historiques conservées et quatre esquisses UX reconstituées, toutes en SVG 320 × 240 avec leur statut.

La bible distingue explicitement les éléments **DÉCIDÉS**, **PROPOSÉS**, **À ARBITRER**, **À VÉRIFIER** et **REPORTÉS**. Elle constitue une synthèse de travail, pas un cahier des charges validé dans son ensemble.

## Direction du jeu

- Duel naval fluide et intuitif au stylet ; immersion rétro sans ralentir le gameplay.
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
