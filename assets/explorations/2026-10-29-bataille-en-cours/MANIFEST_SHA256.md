# Manifeste des empreintes — deux scènes narratives archivées

**ARCHIVEE / EXPLORATION_D_AMBIANCE / NON_VALIDEE_PO / NON_RUNTIME**

**Les sept PNG sont présents sur GitHub** au sein de la PR #2 depuis le commit d'import `686e3e2b27f95860a22272e210b04c729fdf0976`. Leurs tailles et identifiants de blobs Git SHA-1 ont été relevés sur l'arbre GitHub et comparés aux **mêmes octets locaux** ; les SHA-256 locaux sont reproduits ci-dessous. Cette combinaison permet de confirmer une correspondance binaire des fichiers importés. Git SHA-1 de blob et SHA-256 de fichier sont deux notions différentes.

## PNG originaux et exports — présents sur GitHub, vérifiés

| Nom exact | Dimensions (px) | Octets | SHA-256 du fichier | Git blob SHA-1 constaté sur GitHub |
|---|---|---:|---|---|
| `01_bataille_narrative_original_1448x1086.png` | 1448 × 1086 | 2814280 | `1a83c225960929f2c10e4908f9dc71ddcc606f85297c019f236ad6c0ca4a0198` | `057ab0ecf20dfda261307e2bbdba8a40bacedd78` |
| `02_bataille_pixelisee_original_1448x1086.png` | 1448 × 1086 | 2404601 | `657fbef320b12f21d30f346fdb49722be9dafd0dcdf686d7198cf3ec97490f48` | `7344ea8199900b48b688fe6230374656533f6272` |
| `01_bataille_narrative_320x240.png` | 320 × 240 | 170813 | `7825273a1c601f88a20e32493767cf04940f3a61dceb4095099b3b06f7a41313` | `19b104887f2690322cd7468eb67f01eb3723e4bd` |
| `02_bataille_pixelisee_320x240.png` | 320 × 240 | 183601 | `e6d9daa3f41c6b53f0c52d799ef2ec564d2d7fb86ac138a9d19017f18199b757` | `02205646e2703fe7de977d41ae7dcb00591393e4` |
| `01_bataille_narrative_zoom4x.png` | 1280 × 960 | 258663 | `5fd63462332bbd0ad5e17e02368fbb988837a2f1987b4bddba2fc1a80e2f1e4b` | `e72c4fc7b85f70053a6bb7b6b8ceea4b8f35d233` |
| `02_bataille_pixelisee_zoom4x.png` | 1280 × 960 | 273195 | `9ca68f731811f54bddcef0d3cc9dae4e3515812a221ada934624d03bd04959ba` | `59b015956cb0e736ef36a87854100faa2dc365ee` |
| `COMPARATIF_natif_2x320x240.png` | 640 × 240 | 354946 | `e49ccfdf24f0f689f6e0170cfbd0f4e3b86257755b34ff536be15b9a17fa38d7` | `f9f19513c0cf340bf4279775a867fa46bc190f04` |

**Méthode des contrôles :** dimensions contrôlées avec Pillow ; empreintes SHA-256 calculées à partir des octets locaux ; Git blob SHA-1 calculés sur les octets locaux (préfixe `blob <taille>\0` inclus) et comparés avec GitHub. Les deux aperçus PNG sont **identiques pixel à pixel** aux réductions LANCZOS des originaux ; les deux zooms NEAREST sont **identiques pixel à pixel** aux agrandissements 4× des aperçus ; les côtés gauche et droit du comparatif correspondent exactement aux deux PNG 320 × 240. Pas de recadrage.

## Copies JPEG de revue — historiques, également présentes sur GitHub

| Nom exact | Octets | SHA-256 |
|---|---:|---|
| `01_scene_narrative_revue.jpg` | 347440 | `487c900a2ec2b932a34bbf934558f2b83ee667730b57c9165f35dfb489d7d8f4` |
| `02_scene_pixelisee_revue.jpg` | 149783 | `b1ca53dfe1134063ba4b09305597df114e7e2b02b68e637da2bf83e58a8276ae` |
| `01_scene_narrative_preview_320x240.jpg` | 24879 | `73857eb5489ff8fe65a6c85fad6e15c1a7970ff174079cc0dd0c204064cca557` |
| `02_scene_pixelisee_preview_320x240.jpg` | 31033 | `b13330c0e4c8f3480173495bbea0a2cedd7371836e1b10fb45f0b037e38e20cc` |

Les JPEG ont été produits comme **rendus avec pertes pour consultation** lors du transfert initial. Ils ne doivent **jamais** remplacer les sources PNG exactes.

**Provenance des générations :** 01 `a7162bd7-fb6e-49cd-9961-a46bca06b5f5` ; 02 `2674048b-ed4f-45a0-9107-02e4fcc94b57` (**deux générations différentes**).

**Limite des vérifications :** uniquement intégrité documentaire et géométrie d'image ; **aucun test CYD physique, ni compilation firmware, ni validation PO**.
