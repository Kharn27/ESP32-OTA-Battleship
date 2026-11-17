# BTN-ATK-01 — extraction ON / OFF — étude locale sur les pièces jointes

**Issue #139 · PR Draft #141 · CANDIDATE / ON_DEVICE_UNVERIFIED · aucun GO artistique PO**

> **Attention provenance : ces fichiers ne sont PAS les binaires originaux du `main` GitHub.**
> Les deux fichiers joints dans le chat sont des rendus 2048×682, tandis que les références officielles du dépôt sont de dimension 2172×724. La revue de Pilotage exige la régénération à partir de leurs octets exacts. Ce dossier propose des **candidats de comparaison**, mais **ne lève pas** cette réserve. Aucune mise à jour de la PR #141 ne peut être annoncée sur la base de ces fichiers.

## Provenance vérifiée

| Élément | Taille | Format | SHA-256 |
|---|---:|---|---|
| Pièce jointe ON, `source_upload_on_2048x682.png` | 2048×682 | PNG RGBA | `49646da9d0e6f941e448a53f13cba95f8b486aeeaa5b5487c013e8cc3de9392c` |
| Pièce jointe OFF, `source_upload_off_2048x682.png` | 2048×682 | PNG RGBA, alpha entièrement 255 | `74806a3576363382e9fd891c212439b82b8bb81e17bd66baf349dc04b927cf68` |
| GitHub main ON, `draft/Bouton_Attaquer_on.png` | 2172×724 | RGBA | `f2474a5ac04e65844008a9b4a7697ca92a20484c66b77c896f331a67a9e8617c` |
| GitHub main OFF, `draft/Bouton_Attaquer_off.png` | 2172×724 | RGB | `f803b3e460124c14ce24099748d597c7c996c3a1e127d4d06e483b9cc773cfd7` |

Les originaux PO de `main` n'ont **pas** été modifiés. Aucun export ici n'est annoncé comme dérivé des binaires `main`.

## Technique de détourage et d'échelle

- Recadrage commun en haute résolution, rectangle fixe `(24,90,2024,590)`, soit **2000×500**, ratio 4:1 ; aucune déformation en 140×35 ou 112×28.
- **ON :** conserve l'alpha existant (normalisation de l'opacité plafonnée à 253 et suppression des pixels alpha ≤2), et la flamme/fumée originale. Aucune retouche de peinture ni d'icône.
- **OFF :** le fond de la pièce jointe est opaque/noir. Utilisation du contour alpha géométriquement correspondant du ON, aligné au contour OFF (décalage/échelle subpixel), puis retrait de la prémultiplication contre le fond noir aux bords. Le masque réellement utilisé est livré en `mask_off_2000x500_L.png` (niveaux de gris). L'intérieur (canon au repos, typographie, relief, dorures) provient du OFF fourni.
- Redimensionnement LANCZOS **avec traitement en alpha prémultiplié**, puis sortie RGBA ; nettoyage des pixels parfaitement transparents. Même cadrage dans les deux états ; arrondis minimisés.
- Sources HD régénérables à partir des deux originaux **joints** au présent dossier : `source_on_cutout_2000x500_RGBA.png` et `source_off_cutout_2000x500_RGBA.png`.

## Exports candidats

| Nom | Résolution | Poids PNG | SHA-256 |
|---|---:|---:|---|
| `attaquer_on_140x35_RGBA.png` | 140×35 | 11 750 o | `99d494ceb508f75b8da755fc6ff4016039417b74aba284f56520a09decd73152` |
| `attaquer_off_140x35_RGBA.png` | 140×35 | 11 280 o | `5e501830edfe8fcaa0e10d5191ca1a69d0b4dbb71f890dd8d84454f31000413e` |
| `attaquer_on_112x28_RGBA.png` | 112×28 | 7 964 o | `0b6a0330d158487349313b652d4bdaac7e8fbf524190075a1b76768a1accea30` |
| `attaquer_off_112x28_RGBA.png` | 112×28 | 7 683 o | `49ff99ebe3974aec3d9762fc8af7732dddb6b420cb3e57b89c688baafdce1213` |

Comparatifs en **320×240 réels** et ×4 **NEAREST** : `preuve_140x35_*`, `preuve_112x28_*`, plus `comparatif_*` pour deux tailles vues ensemble. Les fonds de comparaison ne sont pas incorporés aux PNG autonomes.

## QA locale réellement exécutée

Requiert Python 3, `pillow`, `numpy` :

```sh
python3 generate.py --check
```

`--check` est **strictement non destructif** : contrôle les SHA des pièces jointes, reconstruit *en mémoire* le masque/les sources/les quatre PNG/les six previews, compare **chaque pixel** avec les PNG remis, vérifie le canal alpha et les coins transparents, puis vérifie pixel par pixel l'agrandissement ×4. `--write` est réservé à une régénération voulue et explicite.

**Résultat local :** `PIXEL_IDENTITY_OK; x4_NEAREST_OK`, consigné dans `QA_check.log`. Les coins des quatre sprites ont alpha=0 ; chaque sprite comporte des pixels alpha=0 et alpha=255. Les sources livrées et le masque sont également inspectables.

Le contrôle `scripts/check_graphic_pipeline.py` du dépôt complet et la CI **n'ont pas été réexécutés sur cette étude locale**, qui n'est pas encore en PR. Les anciennes CI vertes de #141 concernent le HEAD précédent et ne prouvent pas ces exports.

## Réserves et décisions

- **Filiation GitHub** : toujours à régénérer et attester directement des deux originaux de `main` (2172×724), pas de ces copies de pièces jointes. Pour éviter une altération par téléchargement ou par insertion d'image dans le chat, transférer les binaires exacts dans une archive ZIP et contrôler leur SHA-256.
- **Sens ON/OFF** : seulement deux états picturaux PO ; OFF = canon au repos, ON = flamme/fumée. Aucun état logiciel « pressed », « disabled » ou animation validé.
- **Libellé** : la Bible et G04-R2 utilisent **FEU** ; **ATTAQUER** ne peut le remplacer sans arbitrage PO et Pilotage.
- **112×28** : largeur compatible avec le panneau droit G04 (112px), mais ni l'intégration dans la maquette ni la lisibilité réelle au stylet ne sont validées.
- **PO** : pas de validation finale ; **matériel** : `ON_DEVICE_UNVERIFIED`. Les poids PNG ci-dessus ne sont pas des mesures de RAM/Flash embarquée.
