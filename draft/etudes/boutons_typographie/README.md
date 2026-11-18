# BTN-TYPO-01 · Issue #157 — Harmonisation des lettrages

**Statut au 18/11/2026 : `VALIDATED_PO_PC` pour les deux variantes B uniquement (JOUER 140×32, PALMARÈS 140×30).** Les variantes A ne sont pas retenues. `ON_DEVICE_UNVERIFIED` : aucune validation sur CYD physique. Le GO artistique est tracé dans [Issue #157, commentaire PO](https://github.com/Kharn27/ESP32-OTA-Battleship/issues/157#issuecomment-6983850624). Étude exclusivement dans `draft/etudes/boutons_typographie/`, **sans promotion `assets/` ni intégration firmware**.

## Motivation et références authentiques

Le PO juge JOUER/PALMARÈS plus petits visuellement que ATTAQUER/ABANDON. Le changement est **limité aux mots peints**. Les rivets, le métal, le gouvernail, le trophée, la palette et les cadres des quatre boutons sont conservés.

Sources canoniques `main`, **jamais copiées ou modifiées dans ce dossier** :

| Ressource | Dimensions | SHA-1 blob Git |
| --- | --- | --- |
| `assets/boutons/bouton_jouer_original_1107x252_RGBA.png` | 1107×252 | `f81c510060af5aeee231bea50df420d04db7b320` |
| `assets/boutons/bouton_jouer_CYD_140x32_RGBA.png` | 140×32 | `ddd7f95eee37ba3759a108b92f536c515d8d78e8` |
| `assets/boutons/bouton_palmares_original_1163x250_RGBA.png` | 1163×250 | `06a53829d508ae7366338b14a187a655edd6505c` |
| `assets/boutons/bouton_palmares_CYD_140x30_RGBA.png` | 140×30 | `0c3ac936ebc0c9cbe20d3e9e0c5fe29be289985d` |
| `assets/boutons/attaquer_on_140x35_RGBA.png` | 140×35 | `7ac6667492a96e4a2ad6d7445ab68a504576324b` |
| `draft/etudes/bouton_abandon/exports/abandon_140x35_RGBA.png` **PR #150** | 140×35 | `d3c3d3e0da3700cf5d6bf863990e0e460d7d76a9` |

La référence ABANDON est figée au commit de PR #150 `efdc669ef15425c5db342c30de9e07b82ba93dd8` et chargée par `git show` (ou URL GitHub strictement épinglée si le commit n'est pas dans le checkout). Elle reste **candidat**, pas approuvée.

## Deux intensités par bouton (format CYD constant)

| Bouton | A — modérée, HD largeur/hauteur | B — affirmée, HD largeur/hauteur | Format final |
| --- | --- | --- | --- |
| **JOUER** | ×1,13 / ×1,15 | ×1,23 / ×1,29 | **140×32** RGBA |
| **PALMARÈS** | ×1,075 / ×1,13 | ×1,135 / ×1,24 | **140×30** RGBA |

La valeur est **le facteur effectif d'agrandissement du lettrage masqué HD**, pas un redimensionnement de la plaque ni un étirement du sprite CYD. La différence d'intensité est volontairement plus douce en largeur pour le mot long PALMARÈS, afin de laisser respirer le trophée et les rivets.

### Méthode et limites de fidélité

`generate.py` reprend **les lettres dorées et leur bordure noire dans l'art HD approuvé**, localise leurs faces claires, garde leur motif de dégradé et découpe seulement leur relief. L'ancien lettrage est nettoyé localement par inpainting sous un masque plus large que le contour recopié (pour éviter les ombres fantômes), puis les lettres authentiques sont agrandies et recomposées sur la plaque intacte. La plaque n'est jamais redimensionnée ; les décorations et bandes de pourtour sont vérifiées **pixel-identiques** aux originaux HD (`protected_changed_pixels = 0` / audit du rapport).

Les seuls pixels modifiés sont ceux de la **zone du texte** et de la reconstitution du métal précédemment caché par ce texte. Cette retouche HD est *éditable/reproductible via la recette*, mais ne prétend pas reconstituer les pixels authentiques cachés derrière les lettres initiales. Aucun nouveau glyphe ni police arbitraire n'a été utilisé. L'accent du È est préservé.

## Contenu de l'étude

- `maitres/` : **4 masters HD RGBA** (2 JOUER, 2 PALMARÈS), géométrie originale.
- `exports/` : **4 véritables sprites candidats CYD**, dimensions originales.
- `preuves/` : **trois écrans à 320×240 natif** — actuel, A modérée, B affirmée — et **leurs zooms ×4 NEAREST 1280×960**, plus contact comparatif `960×240`. Chaque écran montre JOUER, PALMARÈS, ATTAQUER ON approuvé et ABANDON candidat, chacun à échelle 1:1 sur fond clair et sombre.
- `rapport.json` : SHA-256, poids, dimensions, dépendances/objets Git, coordonnées des masques et régions protégées.
- `generate.py` : source reproductible ; `requirements.txt` : versions épinglées.

Les planches sont des **preuves graphiques**, et non une maquette d'écran de jeu ou une preuve d'intégration firmware.

## Reproduction sur un checkout Git avec accès au commit de PR #150

```bash
python3 -m venv /tmp/cyd-typo-157
/tmp/cyd-typo-157/bin/pip install -r draft/etudes/boutons_typographie/requirements.txt
/tmp/cyd-typo-157/bin/python draft/etudes/boutons_typographie/generate.py --repo . --check
python3 scripts/check_graphic_pipeline.py
```

`--check` reconstruit les quatre masters HD, les quatre exports, les sept vues de contrôle et `rapport.json` ; compare tous les octets aux fichiers publiés et **n'écrit rien**. Le champ `status: CANDIDATE / ART_NOT_REVIEWED / ON_DEVICE_UNVERIFIED` de `rapport.json` est le **statut historique imprimé par la recette lors de la génération avant le GO** ; la décision PO ultérieure est tracée dans l'Issue #157 et ci-dessus. La recette et le rapport de reproductibilité restent donc byte-identiques. Pour régénérer sur un *checkout de travail* uniquement, omettre `--check`. Si une source canonique ou le candidat ABANDON épinglé change, le script refuse la ressource. Le ZIP local ou un rendu d'écran ne remplace pas cette filiation.

## Validation artistique du PO — variantes B uniquement

**GO artistique du PO donné le 18/11/2026** pour **JOUER B 140×32 et PALMARÈS B 140×30** ; les variantes A restent de simples essais archivés, non retenus. Le choix B renforce la présence des mots tout en préservant les ornements et dimensions. Aucun état tactile ni comportement nouveau n'est validé.

**Aucune modification dans `assets/` ni `maquettes/`, ni `draft/Bouton_*.jpeg`. Pas de statut `ON_DEVICE_VERIFIED` sans essai matériel.** Le PO donne le GO artistique ; seul Pilotage décide et effectue la fusion, puis une promotion éventuelle fera l'objet d'une autre mission. Refs #157 ; liée contextuellement à PR #150, non modifiée.
