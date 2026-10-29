# I-009 — Remise pour revue Pilotage

## Références et périmètre
- Issue liée : \`Refs #...\` (ne pas utiliser \`Closes #...\` tant que tous les critères ne sont pas acceptés).
- Type : US-DEV / US-GRAPH / DOC-PO / SPIKE / BUG / TECH.
- Responsable / agent :
- Base cible : \`main\`
- **HEAD exact soumis à la revue :** \`<SHA complet>\`
- Résumé des changements et **exclusions** :

## Critères d'acceptation
- [ ] Critère(s) de l'Issue respecté(s), avec renvoi aux preuves
- [ ] Dépendances, interfaces et décisions PO respectées
- [ ] Aucun changement hors périmètre ni nouvelle règle de jeu implicite

## Preuves (ne jamais confondre les niveaux)
| Niveau | État | Preuve / commande / lien |
| --- | --- | --- |
| Tests hôte (PC) | PASS / FAIL / NON EXÉCUTÉ / N/A | |
| Analyse C / sanitizers (si requis) | PASS / FAIL / NON EXÉCUTÉ / N/A | |
| Compilation ESP32 cible | PASS / FAIL / NON EXÉCUTÉ / N/A | |
| GitHub Actions / CI | PASS / FAIL / ABSENTE / NON EXÉCUTÉE | |
| Tests CYD physique | PASS / FAIL / NON TESTÉ / N/A | |
| Tests deux CYD / ESP-NOW (si requis) | PASS / FAIL / NON TESTÉ / N/A | |
| Mémoire, Flash et piles (si requis) | MESURÉ / NON MESURÉ / N/A | |

**En cas de preuve matérielle requise :** noter firmware/HEAD, matériel, procédure, logs et identité de la personne ayant réalisé le test ; ne jamais déduire un succès matériel de la seule compilation.

## Cas négatifs / limites connues
- Pertes radio, doublons, hors ordre, redémarrage, états invalides, bornes mémoire ou autres risques pertinents :
- Dette / limites résiduelles explicitement acceptables :
- Validation PO à obtenir, le cas échéant :

## Checklist de livraison
- [ ] Diff limité à la mission
- [ ] Fichiers et sources/export graphiques cohérents si concernés
- [ ] Tests et résultats **réels** fournis, échecs non masqués
- [ ] Aucune dépendance au dépôt d'idées \`Le-Grenier-Geek\` modifiée
- [ ] HEAD exact actualisé après le dernier push

## Décision réservée au Pilotage
\`CORRECTIONS_REQUISES\` / \`EN_ATTENTE_DE_PREUVES\` / \`EN_ATTENTE_ARBITRAGE_PO\` / \`ACCEPTEE_POUR_FUSION\` / \`FUSIONNEE\`

> Une CI verte n'est ni une revue de code, ni un test CYD. Le Pilotage contrôle le dernier HEAD et effectue personnellement la fusion.
