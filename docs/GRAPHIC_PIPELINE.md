# Pipeline graphique — CYD WARSHIP (I-009)

**Statut : contrat organisationnel validé en principe par le PO le 10/11/2026, en cours d'implémentation (Issue #99).** La présente PR est une **fondation** ; aucune migration des contenus de `design/` n'est implicite.

## Les trois espaces

1. **`draft/`** : originaux et brouillons remis par le PO, non modifiés par défaut. Une proposition peut être comparée dans une Issue ou une PR, mais elle n'est pas un asset approuvé.
2. **`assets/`** : une bibliothèque de **composants individuels réutilisables**, acceptés artistiquement par le PO et indexés par un ID stable dans `assets/manifest.json`. Sous-familles recommandées : `fonds/`, `boutons/`, `logos/`, `navires/`, `effets/`. Les sources éditables et scripts peuvent être associés à leur composant, sans copier ses exports dans les écrans.
3. **`maquettes/`** : une bibliothèque d'**écrans assemblés** et documentés, un dossier par écran (accueil, déploiement, attaque, défense, fin-partie, palmarès...). Chaque écran porte un `screen.json` qui cite les **IDs** du manifeste, un aperçu natif 320 × 240 et les sources/recettes de composition. Un aperçu aplati est une **preuve de design**, jamais un asset runtime implicite.

Une maquette peut créer du texte, des grilles, des rectangles et des repères de mise en page (primitives UI), mais **pas cacher de nouveaux sprites** dans son fichier source ou son dossier. Toute nouvelle image réutilisable passe par `draft/` ou le processus d'acceptation de `assets/`. Pour les dépendances exactes, `assets/manifest.json` est la référence et les maquettes ne copient pas les composants.

## Validation et statuts séparés

| État | Portée |
| --- | --- |
| `DRAFT` / `CANDIDATE` | Proposition ou production en cours, non utilisable comme référence validée |
| `VALIDATED_PO_PC` | GO artistique explicite du PO, lié à une Issue/PR/commentaire ; ressource admise dans `assets/` ou maquette acceptée |
| `ON_DEVICE_UNVERIFIED` | Pas de preuve de rendu réel sur la CYD (statut matériel par défaut, même après GO PO) |
| `ON_DEVICE_VERIFIED` | Preuve datée et identifiable sur la cible/firmware exacte ; ne s'infère jamais d'un PNG PC ou d'une CI |

Le manifeste stocke l'état **artistique approuvé** et, séparément, `device_status`. Un changement d'empreinte SHA-256 d'un export approuvé exige une nouvelle revue et la référence d'acceptation correspondante. Le validateur vérifie la structure et l'intégrité des fichiers, **pas l'authenticité de la décision humaine** : Pilotage contrôle les liens de GO.

## Contrats des fichiers

### `assets/manifest.json`

```json
{
  "schema_version": 1,
  "assets": [
    {
      "id": "accueil.bouton_jouer",
      "path": "assets/boutons/bouton_jouer.png",
      "sha256": "<empreinte SHA-256 du fichier validé>",
      "source_ref": "draft/Bouton_jouer.jpeg",
      "po_validation_ref": "https://github.com/Kharn27/ESP32-OTA-Battleship/pull/90#issuecomment-...",
      "device_status": "ON_DEVICE_UNVERIFIED"
    }
  ]
}
```

Cet exemple est illustratif et **ne déclare pas** une ressource existante dans le nouveau catalogue. Un fichier graphique présent sous `assets/` mais absent du manifeste est refusé.

### `maquettes/<ecran>/screen.json`

```json
{
  "schema_version": 1,
  "id": "accueil",
  "size": [320, 240],
  "assets": ["accueil.bouton_jouer"],
  "preview": "maquettes/accueil/accueil_320x240.png",
  "status": "CANDIDATE"
}
```

Le statut peut devenir `VALIDATED_PO_PC` **avec** `po_validation_ref`. Les images de prévisualisation peuvent être des compositions aplaties ; l'intégration firmware reconstruira l'écran selon le contrat technique. Les coordonnées de placement, zones tactiles et états doivent être fournis dans une note ou une source éditable. **Pas d'import direct depuis `draft/` ou `design/`** dans une nouvelle maquette.

## Application et migration contrôlée

- **Étape A — ce contrat :** nouveaux dossiers, manifeste, fichiers de règles et contrôle GitHub Actions ; les fichiers sous `design/` ne sont pas déplacés.
- **Étape B — inventaire contradictoire :** relever les références déjà approuvées (par ex. accueil PR #90), les candidats, les scripts et toutes les dépendances. Ne pas promouvoir un rendu candidat par simple déplacement.
- **Étape C — migration :** déplacer, sans changement de pixels, uniquement les assets approuvés ; recréer les maquettes en référençant ces assets, mettre à jour chemins/scripts/index et valider les empreintes. Gérer explicitement les scènes aplaties, non réutilisables comme sprites.
- **Étape D — fermeture de l'héritage :** supprimer `design/` seulement quand plus aucun lien ou script actif ne l'utilise, puis rendre le blocage des ajouts hérités effectif en CI.

Pendant la transition, les PR graphiques historiques déjà ouvertes nécessitent une coordination Pilotage ; elles ne sont **pas automatiquement approuvées** ni automatiquement invalidées. La réorganisation n'accorde aucun nouveau GO PO et ne remplace pas les tests physiques.

## Contrôles

Depuis la racine du dépôt :

```bash
python3 scripts/check_graphic_pipeline.py
```

Le workflow GitHub Actions exécute cette vérification à chaque PR touchant la gouvernance ou les nouveaux dossiers. Il vérifie notamment les chemins, références, hashes et l'absence de copies binaires identiques. **Une CI verte signifie « structure cohérente », pas « esthétique validée » ou « testé sur CYD ».**

## Consigne persistante — à installer une fois dans chaque projet ChatGPT

> Pour CYD WARSHIP, lis au début de chaque mission le fichier `AGENTS.md` de la branche principale du dépôt `Kharn27/ESP32-OTA-Battleship`, puis les règles du pipeline qu'il référence. Applique ces règles et les décisions PO sans attendre qu'elles soient recopiées dans les prompts. N'utilise aucun brouillon comme asset approuvé ; travaille en Issue/PR et ne fusionne pas (fusion réservée au Pilotage). Si tu ne peux pas consulter le dépôt, signale-le explicitement.

Cette configuration doit être faite **dans les instructions persistantes de chaque projet ChatGPT**, car les agents de projets différents ne partagent pas automatiquement leurs instructions ni leur contexte.
