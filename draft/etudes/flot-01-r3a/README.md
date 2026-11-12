# FLOT-01 — Sous-marin INTACT H — candidat R3-A (Issue #113)

> **CANDIDATE / DESIGN_PREVIEW — ON_DEVICE_UNVERIFIED.** Aucun nouveau GO PO, aucun test CYD. Cette PR est un arrêt artistique R3-A horizontal, **pas** la validation technique ni la production R3-B vertical.

## À inspecter en taille native

- [`sous_marin_intact_r3a_h_54x18.png`](sous_marin_intact_r3a_h_54x18.png) : **54×18 RGBA, alpha strictement 0/255**, proue droite, 3×18 px, aucun pixel de mer. Sur un écran ordinaire, il faut inspecter le fichier sans interpolation à sa taille native.
- [`sous_marin_intact_r3a_h_x4.png`](sous_marin_intact_r3a_h_x4.png) : **216×72 NEAREST exact**, contrôle et non ressource runtime.
- [`sillage_r3a_h_66x22.png`](sillage_r3a_h_66x22.png) : **couche d'interaction navire/mer autonome**, sans élément de coque. L'ancrage de cette couche place le coin gauche du navire à `(10, 0)` ; donc si la coque est dessinée à `(x, y)`, dessiner préalablement le sillage à `(x-10, y)` puis le navire à `(x, y)`.
- [`sillage_r3a_h_x4.png`](sillage_r3a_h_x4.png) : **264×88 NEAREST** d'inspection uniquement.
- [`r3a_sur_mer_sombre_320x240.png`](r3a_sur_mer_sombre_320x240.png) et [`r3a_sur_mer_claire_320x240.png`](r3a_sur_mer_claire_320x240.png) : contextes illustratifs à **320×240** avec une grille 10×10 de pas 18 ; le sous-marin est à C6–E6, origine de grille `(21,53)`, comme G05. Les mers sont des fonds de contrôle **fabriqués pour ce comparatif, non validés PO ni les vrais fonds G05**.
- [`r3a_detail_mer_sombre_x4.png`](r3a_detail_mer_sombre_x4.png) et [`r3a_detail_mer_claire_x4.png`](r3a_detail_mer_claire_x4.png) : zoom **264×88 NEAREST**, cadrant la totalité de l'effet d'eau (66×22) avec la coque ; **pas** un sprite 54×18.

## Pourquoi R3-A plutôt que R2

La coque est **reconstruite pixel par pixel**, à partir d'une silhouette de sous-marin hydrodynamique, avec modèle de lumière longitudinal, nuances métalliques discrètes, lignes de plaques interrompues, corrosion localisée, kiosque à facettes et mâts dissymétriques. Pas de conversion SVG, pas de détourage automatique du master G05 aplati, pas de pixels de mer importés. L'image de comparaison R3 H présentée au PO dans la conversation est une **référence d'intention**, pas un fichier de 54×18 fourni comme source canonique. L'ancien sous-marin `legacy` de G05 demeure la référence de qualité, non un asset source isolé.

Le **filet de dérive / sillage** est un effet situé sous la coque, avec turbulence derrière la poupe et éclats irréguliers près de la flottaison. Il n'est **pas incrusté** dans le sprite H transparent, et son débordement de 10 px vers la poupe et de 4 px sous le sprite exige un arbitrage Pilotage/Dev avant toute consommation firmware. La couleur doit être revue sur les vraies mers du jeu.

## Références existantes, à comparer sans les altérer

- Maquette historique [G05 DÉFENSE 320×240](../../../maquettes/defense/legacy/g05_r2_defense_320x240.png), [zoom ×4](../../../maquettes/defense/legacy/g05_r2_defense_x4.png), sous-marin C6–E6.
- Sprite réel [R2 H 54×18](../../../assets/flotte/sous-marin/sous_marin_intact_h.png), [R2 H ×4](../../../assets/flotte/sous-marin/sous_marin_intact_h_x4.png), historiquement approuvé puis remis en cause artistiquement pour R3.
- Original PO : [`draft/sous_marin_PO_A.jpeg`](../../sous_marin_PO_A.jpeg). Aucune modification de ces références.

## Reproduction / contrôles

Depuis le dossier de cette étude : `python3 build_r3a.py` puis `python3 build_r3a.py --check` ; dépendance Pillow **12.3.0** ; les PNG sont reconstruits avec la graine fixe `11303`, en pixels natifs puis zoom *NEAREST*. `--check` compare les octets des huit exports attendus, et valide dimensions, alpha binaire et conservation des pixels ×4. **Le PNG natif est l'artefact à juger à 1×.**

| Fichier | Taille / poids constaté | Statut |
| --- | --- | --- |
| Sprite H | 54×18, voir PNG | `CANDIDATE_NOT_RUNTIME` |
| Sillage | 66×22, indépendant | `CANDIDATE_EFFECT_NOT_RUNTIME` |
| Zooms / contextes | 216×72, 264×88, 320×240 | `INSPECTION_ONLY` |

**Côté technique** : le rendu de preuve utilise un simple alpha 0/255 ; pas de mélange partiel requis. PNG comme format de proposition, non nouveau format firmware imposé. Les coûts mémoire sur appareil, l'export RGB565 éventuel, les cases voisines et les performances de rendu restent **non vérifiés**. Le R2 validé et `assets/manifest.json` restent inchangés. Ni H R3 ni son sillage n'apparaissent dans le catalogue runtime avant validation PO.

## Blocage de chemin à traiter par Pilotage

L'Issue #113 prescrit encore **`design/flotte/navires/sous-marin/` seulement**. Or `AGENTS.md` sur `main` et `docs/GRAPHIC_PIPELINE.md` interdisent toute nouvelle contribution à `design/`. Candidat déposé ici sous **`draft/etudes/flot-01-r3a/`**, sans l'inscrire dans `assets/`, dans l'attente de l'alignement formel de l'Issue et du verdict du PO. **Cette PR ne saurait être fusionnée ni convertie en asset approuvé sans arbitrage Pilotage.**

**Stop R3-A** : PO juge le 1× sur mer claire/sombre ; après GO explicite seulement, le développement des orientations V et de la série H/V R3-B peut commencer. Le PO et Pilotage contrôlent les preuves ; **fusion par Pilotage seul**.

## Ajout — R3-H généré (12/11/2026)

Une **image R3-H supplémentaire** est mise à disposition pour revue PO/Pilotage sans écraser le candidat R3-A existant. Voir [R3H_GENERATED_REVIEW.md](R3H_GENERATED_REVIEW.md) pour le PNG source HD, l'export 54×18, l'écume séparée 66×22, les épreuves 320×240 et leurs zooms ×4. Statut **CANDIDATE / ON_DEVICE_UNVERIFIED**, STOP PO maintenu.
