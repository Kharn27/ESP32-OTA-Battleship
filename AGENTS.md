# AGENTS.md — Règles permanentes du dépôt CYD WARSHIP

**Applicabilité :** tous les agents (Pilotage, Développeur C, Graphiste/DA, PO Assistant) et toutes les contributions au dépôt. Lire ce fichier **avant toute modification ou proposition de PR**, puis consulter `docs/GRAPHIC_PIPELINE.md` pour les travaux visuels.

## Autorité et responsabilités

- **PO humain :** seul habilité à valider artistiquement une ressource ou une maquette et à décider des règles et parcours du jeu.
- **Pilotage / Architecte :** seul chargé des contrôles, de l'acceptation technique et de la fusion effective des PR.
- **Graphiste / DA :** crée les assets et les maquettes selon le pipeline ; ne s'attribue pas un GO PO.
- **Développeur :** consomme les ressources validées, propose les formats firmware et vérifie les contraintes CYD ; ne confond pas prévisualisation et asset utilisable.
- **PO Assistant :** met en forme les décisions, sans approuver au nom du PO.

`BIBLE_PRODUIT.md` demeure **l'unique autorité pour les règles du jeu**. Ce fichier organise les livraisons graphiques ; il ne crée aucune règle métier. Les Issues servent au pilotage et les PR aux livraisons (voir `docs/WORKFLOW_AGENTS.md`).

## Architecture graphique obligatoire

| Dossier | Propriétaire du contenu | Règle |
| --- | --- | --- |
| `draft/` | Originaux PO, brouillons et variantes de travail | Conserver les originaux tels que remis ; ne pas les traiter comme validés ou intégrables. Ne modifier un original PO que sur mandat explicite. |
| `assets/` | **Éléments individuels réutilisables validés artistiquement par le PO** | Images, boutons, fonds, logos, navires, effets ; chaque export visuel est déclaré dans `assets/manifest.json` avec empreinte SHA-256 et référence de décision PO. |
| `maquettes/` | **Compositions d'écrans** | Une maquette assemble des IDs d'assets existants et des primitives UI, sans créer de copie privée d'un composant réutilisable. Déclarer les dépendances dans `maquettes/<ecran>/screen.json`. |
| `docs/graphisme/` ou `docs/GRAPHIC_PIPELINE.md` | Contrats, charte, flux de travail | Documentation et conventions ; aucune copie de sprites ou d'écrans comme source runtime. |
| `design/` | **Ancien emplacement retiré** | Le dossier est supprimé de l’arbre courant ; aucune nouvelle contribution n’y est autorisée. Les contenus historiques sont gelés sous `maquettes/*/legacy/` ou reclassés dans `draft/etudes/`. |

Ne jamais faire passer un brouillon ou un rendu d'écran pour un composant validé. Une **validation artistique PO sur PC** n'est **pas** une validation sur la **CYD physique** ; marquer explicitement `ON_DEVICE_UNVERIFIED` si non prouvé. Un asset approuvé ne change pas silencieusement : changement de pixels = nouvelle empreinte, preuve de décision et revue.

## Protocole de travail de chaque agent

1. Lire les règles de ce dépôt et la Bible pour tout besoin fonctionnel ; vérifier l'état de la branche et de l'Issue.
2. Classer tout fichier visuel dans `draft/`, `assets/` ou `maquettes/` **avant** de le créer. Pour les créations candidates, travailler sur branche/PR, pas directement sur `main`.
3. Pour un asset approuvé : noter **source, ID stable, chemin, SHA-256, décision PO, statut matériel** dans le manifeste. Pour une maquette : référencer les IDs d'assets et fournir l'aperçu natif **320 × 240** et ses fichiers sources/placement.
4. Exécuter les contrôles de structure de `scripts/check_graphic_pipeline.py` et présenter les preuves **effectivement obtenues**.
5. Remettre Issue, branche, PR, HEAD exact, validations et limites. **Seul Pilotage fusionne**, après revue sur le nouveau HEAD.

**Interdictions :** dupliquer un sprite dans un dossier de maquette ; remplacer un asset validé sans traçabilité PO ; prétendre qu'une CI valide le graphisme ou le matériel ; mélanger réorganisation et retouches artistiques ; modifier `Le-Grenier-Geek` sans mandat.

## Agents ChatGPT dans des projets distincts

La présence de ce fichier ne garantit pas que ChatGPT lira spontanément le dépôt. **Chaque projet d'agent doit recevoir une fois une instruction persistante** : « Avant chaque mission CYD WARSHIP, lis `AGENTS.md` sur `main` et les documents qu'il référence ; respecte leurs règles, même si mon prompt ne les répète pas ; signale toute impossibilité d'accès avant de modifier le dépôt. » Les règles versionnées sont la référence commune, et non les souvenirs de conversations indépendantes.
