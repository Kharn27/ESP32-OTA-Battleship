# BTN-ATK-01 — Candidats ON/OFF (Issue #139)

**Statut : CANDIDATE / ON_DEVICE_UNVERIFIED — aucun GO artistique PO — aucune intégration firmware.**
Branche : `design/issue-139-btn-atk-01`. Périmètre : `draft/etudes/bouton_attaquer/` uniquement.

## Références originales PO (inchangées sur main)

| Fichier | Octets GitHub | Git blob SHA-1 |
|---|---:|---|
| `draft/Bouton_Attaquer_on.png` | 1 874 024 | `e7b663608fa56c452fb189fb3fd96ea03bd33aef` |
| `draft/Bouton_Attaquer_off.png` | 1 520 779 | `d425e792a713167f667f80de52ffe010c6281fba` |

**Réserve importante de traçabilité.** Dans l'environnement de préparation, le connecteur GitHub a permis de vérifier les chemins, tailles et Git blobs, **mais pas de lire les octets PNG** : SHA-256, dimensions exactes et alpha des originaux GitHub restent à contrôler sur checkout avec `qa.py`. Pour ne pas bloquer toute étude, les candidats graphiques ci-dessous proviennent de copies visuellement correspondantes trouvées dans Adobe Creative Cloud : ON `urn:aaid:sc:DEU1:3e884f99-28f3-4d1c-b32d-58236ddfe5a0` (taille Adobe 1 952 156 octets), OFF `urn:aaid:sc:DEU1:a1e7f480-5240-4b29-861b-b31e7eb92b14` (taille Adobe 1 520 779 octets). **Identité octet-à-octet avec les originaux GitHub non établie**, en particulier pour ON ; cette étape est obligatoire avant toute promotion. Les sources intermédiaires ci-jointes font 588×148, **pas la pleine résolution des originaux PO**.

## Traitement et réglages (Adobe)

1. Copies Adobe ON/OFF : rendu de travail 600×200, traitement `image_remove_background` sans couleur de fond afin de détacher le motif métallique. Aucune copie des PNG PO GitHub écrasée.
2. Recadrage géométrique commun normalisé : `left=.01, top=.13, right=.99, bottom=.87` → deux sources RGBA de 588×148.
3. Redimensionnement centré de ces deux sources, `fit=reframe`, cadrage `(0,1,588,147)` → **140×35** et **112×28** sans étirement. Le filtre de rééchantillonnage interne Adobe n'est pas présenté comme algorithme firmware ou reproduction binaire garantie.
4. `qa.py` inspecte transparence alpha, empreintes des candidats, et réalise les prévisualisations **320×240 natif** puis **×4 NEAREST** sur checkout possédant Pillow.

Les images de comparaison (HTML/SVG/PNG générés) peuvent avoir des fonds sombres/clairs, **mais aucun de ces fonds ne fait partie d'un sprite**.

## Sprites candidats (RGBA PNG, faits pour étude PC, PAS pour runtime)

| Fichier | Pixels | Octets | SHA-256 du PNG candidat |
|---|---:|---:|---|
| `attaquer_on_140x35_RGBA.png` | 140×35 | 12071 | `c5cdd3fc788f0b2629cb3d2bac456bfd15e9fcccb8ff4a8979fb0286b82aee05` |
| `attaquer_off_140x35_RGBA.png` | 140×35 | 11848 | `71b1a66c1380d4ab7e8035bc3cea6accf1ee6e77661c738ee4f25f0d20e18b94` |
| `attaquer_on_112x28_RGBA.png` | 112×28 | 8231 | `68fcab7abe17906695f23ec07672098a357340918f5edd1533d0e9b8c6237c27` |
| `attaquer_off_112x28_RGBA.png` | 112×28 | 8089 | `6b504b1673201a89af2f49e95a5874895c97825fef02813355aa1b6dd5e55faf` |
| `source_on_cutout_588x148.png` | 588×148 | 155811 | `26e05999b4f1663f42713d31d76b214094b752ac5d17868e4f1d097bc38454d3` |
| `source_off_cutout_588x148.png` | 588×148 | 151808 | `52dc7fb42aa207c6f7a1f0378fd7d113021d597a84c529d729d9818c862b6759` |

Les six fichiers ont une signature PNG valide et `IHDR color_type=6` (RGBA). **La présence effective d'alpha nul sur les bords doit encore être confirmée par `qa.py`** ; la manipulation de détourage a été réalisée par Adobe. Aucune marge opaque volontaire autour du bouton.

## Différences artistiques ON / OFF

- ON : bouche de canon avec flamme jaune/orange, bouffée de fumée ; rouge/métal et dorures conservés.
- OFF : bouche de canon ouverte sans flamme ni fumée ; base, panneau, texte et relief analogues.
- Ne pas interpréter ces variantes comme `pressed`, `disabled`, animation de tir ou état fonctionnel acquis ; `pressed` relève du feedback logiciel de l'Issue #115.

La proposition compacte **112×28** tient horizontalement dans le panneau droit de 112 px (`x=208..319`) de G04-R2 ; le corps du texte reste identifiable en inspection PC, mais son confort sur CYD physique demeure **NON TESTÉ**. La version 140×35 sert à juger le même visuel dans un espace plus large, sans prétendre qu'il peut entrer dans G04. Aucune géométrie de touche n'est validée par cette étude.

## Arbitrage PO/Pilotage — aucune modification de FEU

La Bible et G04-R2 exigent `FEU` pour verrouiller le tir. Le libellé des sources est `ATTAQUER`. **Ne pas substituer ces sprites au bouton FEU** sans décision produit explicite du PO et cadrage par Pilotage. Aucun élément de `maquettes/attaque/legacy` n'est modifié.

## Vérifications et limites

- Ouvrir `comparatif_320x240.svg` et `comparatif_4x_1280x960.svg` (images intégrées, même échelle ON/OFF) ; `comparatif_references.html` juxtapose également les assets existants JOUER et PALMARÈS **sans les modifier**.
- Dans un clone complet, après installation de Pillow, lancer `python3 draft/etudes/bouton_attaquer/qa.py`. Cela contrôle la présence d'alpha transparent, la taille et le SHA-256 de chaque candidat, calcule le SHA-256 des deux originaux PO, et produit `preuve_320x240.png`, `preuve_x4_1280x960.png` par agrandissement NEAREST. Les comparaisons SVG préexistantes peuvent être vues immédiatement ; les PNG générés ne sont pas encore enregistrés dans cette PR.
- Lancer `python3 scripts/check_graphic_pipeline.py` depuis la racine du clone. **Non exécuté dans cet environnement** (le dépôt binaire complet n'est pas monté) ; aucune CI/validation matérielle revendiquée.
- Provenance : originaux graphiques PO, copies Adobe du même espace utilisateur pour préparation ; ne pas supposer droits d'usage tiers. Décision finale PO requise.
- Les formats, décodage et budgets Flash/RAM seront négociés avec le Développeur après GO PO ; valeurs de fichier PNG **≠** consommation RAM écran.

**Points bloquants à lever pour recette Phase 1 :** contrôle réel des deux fichiers `draft/` en octets via checkout, inspection des PNG générés 320×240 (pas seulement SVG), preuve QA exécutée, retour PO sur la lisibilité et sur le conflit FEU/ATTAQUER. Aucun fichier de `assets/` ou du firmware n'est ajouté.

Refs #139
