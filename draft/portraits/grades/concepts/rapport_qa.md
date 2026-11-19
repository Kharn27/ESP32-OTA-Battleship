# Contrôle PC — PORTRAITS-GRADES-DA / PR 1

**Statut :** `DESIGN_PREVIEW — CANDIDATE — ON_DEVICE_UNVERIFIED`.

## Contrôles exécutés localement le 19/11/2026

| Vérification | Résultat | Preuve / réserve |
|---|---|---|
| Génération SVG autonome Node.js | PASS | `node build_concepts.mjs <sortie>` : 10 SVG |
| Régénération sans écrire | PASS | `node build_concepts.mjs <sortie> --check` : 10 SVG octet pour octet identiques |
| Six études | PASS | `grade_{homme,femme}_{mousse,capitaine,amiral}_etude.svg` (96×96, fond transparent) |
| Visage identique homme : Mousse = Capitaine = Amiral | PASS | Comparaison RGBA exacte CairoSVG/Pillow de la zone `(30,38,66,73)`, 36×35 = 1 260 pixels, deux paires sans différence |
| Visage identique femme : Mousse = Capitaine = Amiral | PASS | Même zone, même méthode, deux paires sans différence |
| Transparence des concepts | PASS | PNGs de contrôle RGBA, pixel (0,0) alpha 0 sur les six études |
| Deux simulations natives | PASS | `apercu_sombre_320x240` et `apercu_clair_320x240`, rendu 320×240 sur PC |
| Inspection zoom ×4 NEAREST | PASS | PNG d'étude 1280×960 calculés sans interpolation et examinés visuellement sur PC |
| Lecture à 48 pixels sur clair et sombre | À ARBITRER PO | Silhouette des trois coiffes et contraste des vêtements visibles ; microdétails de médaille/grain de beauté non garantis |
| `python3 scripts/check_graphic_pipeline.py` sur dépôt complet | NON EXÉCUTÉ | Environnement local non raccordé au checkout complet du dépôt ; contrôle CI / Pilotage après PR |
| Tests CI sur HEAD GitHub de PR | NON EXÉCUTÉ au moment du rendu local | À relier et contrôler après publication du commit |
| Vérification physique CYD | NON TESTÉ | Aucune photographie de la carte ni firmware de test |

Vérification supplémentaire réalisée : `python3 check_concepts.py` contrôle tous les SHA-256 du manifeste, la sortie PNG de CairoSVG par rapport aux dix SVG, les sorties d’épreuve, le visage RGBA et le ×4 NEAREST **en lecture seule** (PASS).

Les SVG `apercu_*` sont des **épreuves** et non des sprites runtime ; les PNG de contrôle sont uniquement des rendus locaux. Toute différence entre le HEAD GitHub et ces fichiers impose une nouvelle vérification.
