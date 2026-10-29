# I-009 — Workflow GitHub multi-agents (proposition de référence)

**Portée :** organisation du travail sur [ESP32-OTA-Battleship](https://github.com/Kharn27/ESP32-OTA-Battleship). **GitHub Project :** [CYD WARSHIP](https://github.com/users/Kharn27/projects/1/views/1). Le Project est un **tableau de pilotage** et non un second dépôt. Configuration des champs/vues à effectuer dans l'interface GitHub ; ce document ne prétend pas les avoir créés.

**Maturité :** conventions de pilotage, sans création de backlog fonctionnel ni déclenchement de mission firmware/graphisme. La Bible produit et les décisions explicites du PO restent les sources métier ; une Issue ne peut les contredire.

## 1. Responsabilités — une source de vérité commune

| Acteur | Autorité et responsabilité |
| --- | --- |
| PO humain | Décide du produit, règles, périmètre, acceptation artistique et arbitrages impactant le joueur ; réalise les tests physiques demandés avec un protocole précis |
| Assistant PO | Consolide exigences, traçabilité et contradictions ; soumet des PR documentaires ; n'invente pas les règles |
| Pilotage / Architecte C-ESP32 | Transforme les besoins validés en US testables ; ordonne, priorise, attribue ; définit interfaces, risque et preuves ; **revoit, décide et effectue les fusions de PR** |
| Développeur C / ESP32 | Implémente le firmware et les tests prévus par ses US, compile et présente les preuves dans une PR |
| Graphiste / DA | Prépare les compositions, assets, sources et exports prévus par ses US ; fait valider les choix artistiques au PO avant acceptation finale |

**Agents ChatGPT distincts :** ne pas présumer qu'ils lisent les conversations des autres ni qu'ils se contactent automatiquement. L'Issue (contrat), la PR (livraison), le HEAD et les liens de preuves constituent la passation. Un humain transmet la mission entre projets ChatGPT lorsque nécessaire. Un compte GitHub commun n'identifie pas à lui seul l'agent exécutant : écrire explicitement le **responsable**.

**Dépôts :** `ESP32-OTA-Battleship` est le dépôt de développement. `Le-Grenier-Geek` est une boîte à idées : **aucune Issue, branche, PR ni modification** dans ce dépôt sauf mandat explicite distinct.

## 2. Modèle de backlog — GitHub Issues

Une **Issue = une mission bornée** : `[US-DEV]`, `[US-GRAPH]`, `[DOC-PO]`, `[SPIKE]`, `[BUG]` ou `[TECH]`. Utiliser le [modèle d'Issue](../.github/ISSUE_TEMPLATE/user_story.yml), complété par le Pilotage avant attribution. Les IDs pérennes restent les numéros GitHub (`#NN`) ; un identifiant métier peut être ajouté au titre, sans remplacer le lien.

Chaque mission précise : **source PO**, valeur, inclus/exclus, responsable, dépendances/interfaces, critères d'acceptation, tests et preuves, risques, points d'arbitrage et conditions de clôture. Privilégier des US petites et vérifiables ; isoler une incertitude matérielle ou radio dans un `SPIKE`. Ne pas créer à l'avance des dizaines de tickets à partir d'une Bible encore incomplète.

**Dépendances :** renseigner les liens `bloquée par #NN` / `bloque #MM` dans les Issues ; utiliser les fonctionnalités GitHub natives si disponibles. Ne paralléliser que les travaux indépendants, avec une interface suffisamment stable. Une demande qui modifie l'expérience joueur repart au PO.

## 3. Configuration recommandée du GitHub Project

La création du Project ne configure **pas** automatiquement ces champs. Les définir dans GitHub Projects lorsque le tableau est prêt :

| Champ Project | Valeurs recommandées | Usage |
| --- | --- | --- |
| **Status** (single select) | À qualifier ; Prête ; En cours ; En revue ; Bloquée ; Terminée | Flux réel des missions |
| **Type** (single select) | US-DEV ; US-GRAPH ; DOC-PO ; SPIKE ; BUG ; TECH | Nature du travail |
| **Responsable** (single select) | Pilotage ; Développeur ; Graphiste ; Assistant PO ; À attribuer | Agent, indépendant des assignees GitHub |
| **Priorité** (single select) | P0 critique ; P1 haute ; P2 normale ; P3 basse | Ordre d'exécution fixé par Pilotage |
| **Lot** (single select) | LOT 1 ; V1.0 futur ; Hors lot ; À préciser | Portée décidée, pas une promesse de livraison |
| **Validation** (single select, facultatif) | À définir ; PC ; Build cible ; CI ; Matériel requis ; Matériel vérifié ; N/A | Vue synthétique ; les **preuves réelles** restent dans les Issues et PR |

**Vues :** (1) `Backlog` en table, trié par priorité et dépendances ; (2) `Kanban` groupé par Status ; (3) `Roadmap` **seulement après** définition de jalons fiables. Pour éviter de perdre une mission, le lien Issue doit rester l'identifiant canonique. Les automatisations de statut ne doivent pas déclarer `Terminée` une US dont les preuves obligatoires manquent.

**Définition des statuts :**
- **À qualifier :** besoin non exploitable, critères ou autorité insuffisants.
- **Prête :** US complète, dépendances libérées, agent attribué, tests exigibles connus.
- **En cours :** agent travaille sur la branche de l'Issue.
- **En revue :** PR remise avec HEAD et preuves, décision Pilotage attendue.
- **Bloquée :** dépendance, preuve ou arbitrage manquant ; noter le blocage et son propriétaire.
- **Terminée :** critères acceptés et preuves exigées acquises ; PR effectivement fusionnée si une PR est nécessaire. Si le matériel est obligatoire mais non vérifié, **ne pas clore** la validation : garder une Issue bloquée ou un suivi matériel lié explicite.

Le tableau GitHub Projects regroupe les mêmes Issues du dépôt et peut afficher des PR ; il n'est pas une base produit indépendante.

## 4. Git, branches et PR

- Brancher depuis le dernier `main`. Préfixes : `feat/issue-NN-sujet`, `fix/issue-NN-sujet`, `design/issue-NN-sujet`, `docs/issue-NN-sujet`, `spike/issue-NN-sujet`.
- Une PR poursuit un objectif et référence son Issue (`Refs #NN`). Par défaut, **ne pas mettre `Closes #NN`** : une fusion ne doit pas clôturer automatiquement une mission encore soumise à preuve physique.
- Fournir un **HEAD exact**, le diff, les décisions et contrats respectés, les tests exécutés et non exécutés, ainsi que les limites. [Modèle de PR](../.github/pull_request_template.md).
- Utiliser `Draft` pendant le travail ; remettre pour revue une fois les vérifications disponibles. Ne pas profiter d'une correction pour refondre une autre partie du produit.
- **Pilotage revoit le HEAD courant** (pas un vieux SHA), la portée, les tests, la CI éventuelle, les contraintes C/mémoire/radio ou l'intégrité graphique ; puis rend `CORRECTIONS_REQUISES`, `EN_ATTENTE_DE_PREUVES`, `EN_ATTENTE_ARBITRAGE_PO` ou `ACCEPTEE_POUR_FUSION`.
- Après chaque correction / nouveau push : **nouvelle vérification sur le nouveau HEAD**. **Seul Pilotage exécute la fusion** et vérifie GitHub avant de déclarer `FUSIONNEE`.
- Une PR purement documentaire ou une maquette `DESIGN_PREVIEW` peut être fusionnée sans prétendre à un test matériel ; l'intégration firmware restera distincte. Pour une US exigeant une validation physique, les critères et la politique [TESTS](TEST_POLICY.md) définissent le blocage.
- Pas de changement de cible matérielle, framework, ESP-IDF, budgets RAM ou règles du jeu sans décision et preuve adaptées.

## 5. Passation et continuité des conversations

**Message de remise minimum :** `Issue #NN — agent — branche — PR #MM — HEAD complet — preuves — état (tests PC / build / CI / matériel) — blocages — action suivante`.

En cas de conversation longue ou d'agent remplacé, tenir dans l'Issue/PR un résumé autonome des décisions et du résultat ; ne pas se reposer sur la mémoire d'un autre projet ChatGPT. Les tâches et décisions sur GitHub sont actualisées **au fil de l'eau** et non rétroactivement affirmées sans contrôle.

## 6. Avant de démarrer le développement

Ce workflow n'est **pas** un mandat de codage. Quand le PO aura livré un besoin suffisamment stable, Pilotage proposera un MVP, les dépendances et un petit premier lot de **SPIKE/US** à attribuer. À confirmer à ce moment : framework/version ESP-IDF, écran/tactile/rotation, budgets mesurés, politique Flash/assets et outils de test. Aucun serveur ni accès Internet n'est requis pour le jeu entre deux CYD ; ESP-NOW fera l'objet de preuves sur **deux cartes**.
