# I-009 — Workflow GitHub multi-agents (Issues comme backlog)

**Portée :** organisation du travail directement dans les [GitHub Issues du dépôt ESP32-OTA-Battleship](https://github.com/Kharn27/ESP32-OTA-Battleship/issues). Les Issues sont **la source de vérité opérationnelle** et les PR portent les livraisons. Le tableau GitHub Projects créé par le PO est **facultatif et non utilisé** pour le pilotage : la connexion ChatGPT GitHub ne permet pas d'en piloter les cartes ni les champs. Aucune action humaine de déplacement de cartes n'est nécessaire.

**Maturité :** conventions de pilotage, sans création de backlog fonctionnel ni déclenchement de mission firmware/graphisme. **`BIBLE_PRODUIT.md` est l'unique référence métier** : les décisions PO applicables doivent y figurer avant attribution d'une US. Les maquettes, ADR, Issues historiques et échanges ne sont que des références complémentaires ; ils ne peuvent créer ou modifier une règle produit.

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

Chaque mission précise brièvement : **décisions applicables de `BIBLE_PRODUIT.md` (§ / IDs)**, valeur, inclus/exclus, responsable, dépendances/interfaces, critères testables, preuves requises et blocages éventuels. **Une US détaille le besoin sans ajouter de règle métier** ; les autres sources ne valent que comme compléments. Ne pas recopier la Bible ni ses justifications historiques, ni imposer de spécifications techniques hors mission. Un champ sans objet indique **« Non applicable » avec une justification brève**. Isoler une incertitude matérielle ou radio dans un `SPIKE` plutôt que d'inventer une règle. Préférer quelques US bornées à un backlog artificiellement exhaustif.

**Dépendances :** renseigner les liens `bloquée par #NN` / `bloque #MM` dans les Issues ; utiliser les fonctionnalités GitHub natives si disponibles. Ne paralléliser que les travaux indépendants, avec une interface suffisamment stable. Une demande qui modifie l'expérience joueur repart au PO.

## 3. Pilotage opérationnel : Kanban logique **dans les Issues**

Aucun GitHub Project, champ personnalisé Projects ou label précréé n'est nécessaire. Le [formulaire d'Issue](../.github/ISSUE_TEMPLATE/user_story.yml) porte des champs **dans le corps même de l'Issue**, consultables et modifiables par Pilotage via le connecteur GitHub :

| Champ dans l'Issue | Valeurs | Auteur de la mise à jour |
| --- | --- | --- |
| **Type de mission** | US-DEV, US-GRAPH, DOC-PO, SPIKE, BUG, TECH | Pilotage au découpage |
| **Statut Pilotage** | À qualifier, Prête, En cours, En revue, Bloquée, Terminée | Pilotage, après contrôle de l'état réel |
| **Agent responsable principal** | Développeur, Graphiste, Assistant PO, Pilotage, À attribuer | Pilotage |
| **Priorité** | À déterminer, P0 bloquant, P1 haute, P2 normale, P3 basse | Pilotage |
| **Lot** | À préciser, LOT 1, V1.0 futur, Hors lot | Pilotage |

**Convention de titres :** `[US-DEV] ...`, `[US-GRAPH] ...`, `[DOC-PO] ...`, `[SPIKE] ...`, `[BUG] ...`, `[TECH] ...`. L'identifiant unique reste le numéro GitHub `#NN`, sans réutilisation ni renumérotation.

**Cycle de vie :**
- **À qualifier** : demande non exploitable, source/critères/autorité manquants ; pas encore attribuable.
- **Prête** : besoin validé et **toutes les décisions PO applicables intégrées à `BIBLE_PRODUIT.md`**, Issue concise et testable, dépendances connues et libérées, agent identifié, preuves attendues définies. Une décision PO nouvelle non encore consignée dans la Bible bloque ce statut pour toute US qui en dépend.
- **En cours** : agent réalise le périmètre autorisé sur une branche.
- **En revue** : PR avec HEAD et preuves remise au Pilotage ; les corrections repassent par ce statut sur chaque nouveau HEAD.
- **Bloquée** : attente d'une dépendance, d'une preuve exigée ou d'un arbitrage PO ; consigner explicitement la raison, son propriétaire et l'étape pour débloquer.
- **Terminée** : tous critères/contrôles de l'étape acceptés, fusion effectivement confirmée lorsque nécessaire, preuves obligatoires acquises. **Clôture GitHub de l'Issue seulement alors.**

**Journalisation** : à chaque transition significative, Pilotage **met à jour le champ Statut Pilotage dans le corps de l'Issue**, puis ajoute un bref commentaire `ancien → nouveau ; preuve / PR ; blocage / action`. Le commentaire n'est pas un remplacement de l'état courant. Le connecteur peut modifier le corps d'une Issue, ses commentaires et son état ouvert/fermé. Ne pas inventer un état Projects différent.

**Recherche et vues sans maintenance manuelle** :
- [Issues ouvertes](https://github.com/Kharn27/ESP32-OTA-Battleship/issues?q=is%3Aissue+is%3Aopen) — backlog vivant, toutes catégories.
- [Issues clôturées](https://github.com/Kharn27/ESP32-OTA-Battleship/issues?q=is%3Aissue+is%3Aclosed) — livraisons terminées.
- [PR ouvertes](https://github.com/Kharn27/ESP32-OTA-Battleship/pulls?q=is%3Apr+is%3Aopen) — travaux livrés / en revue.
- Les recherches GitHub `in:body` et les préfixes de titre permettent des **filtres de lecture indicatifs** par type/statut ; la référence exacte est le champ **Statut Pilotage** de chaque Issue. À la demande, Pilotage produit un récapitulatif Kanban depuis les Issues et PR réelles, sans table secondaire à maintenir.

**Pas de doubles sources de vérité** : aucun `BACKLOG.md` à synchroniser à la main, ni tableau Projects à actualiser. Un label n'est pas indispensable ; si des labels GitHub existent et sont utiles, Pilotage peut les utiliser en complément, mais **ne doit pas dépendre d'une capacité non disponible de création de labels**.

**Validation matérielle distincte** : une PR peut être fusionnée dans un jalon technique défini avec vérification CYD différée, mais **une Issue dont un critère obligatoire reste non démontré ne passe pas à Terminée**. Garder l'Issue ouverte et marquer `Bloquée`, ou ouvrir un suivi matériel lié lorsqu'une séparation de jalon a été explicitement prévue.

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

Ce workflow n'est **pas** un mandat de codage. Sur la base du **périmètre LOT 1 décidé par le PO**, Pilotage proposera **le découpage, les dépendances et les missions de réalisation du LOT 1** (US et éventuels SPIKE), sans redéfinir le produit. Les difficultés qui modifieraient l'expérience joueur retournent au PO pour arbitrage et mise à jour de la Bible avant qu'une US concernée soit « Prête ». À confirmer techniquement au démarrage : framework/version ESP-IDF, écran/tactile/rotation, budgets mesurés, politique Flash/assets et outils de test. Aucun serveur ni accès Internet n'est requis pour le jeu entre deux CYD ; ESP-NOW fera l'objet de preuves sur **deux cartes**.
