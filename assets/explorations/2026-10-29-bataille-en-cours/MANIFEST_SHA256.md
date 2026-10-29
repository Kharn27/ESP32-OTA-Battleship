# Empreintes vérifiées — deux générations d'ambiance

**ARCHIVEE / EXPLORATION_D_AMBIANCE / NON_VALIDEE_PO / NON_RUNTIME**

Empreintes **SHA-256 des fichiers réels**, calculées le 29 octobre 2026 sur les octets présents dans l'archive de remise, et sur les copies JPEG re-lues via le dépôt GitHub. **Git blob SHA-1 ≠ SHA-256.** Aucun chiffre ci-dessous n'est une mesure de fonctionnement ESP32.

## PNG sources et exports — octets exacts dans l'archive de remise (NON ENCORE PRÉSENTS DANS GITHUB)

| Nom exact | Dimensions (px) | Taille (octets) | SHA-256 |
|---|---:|---:|---|
| `01_bataille_narrative_original_1448x1086.png` | 1448 × 1086 | 2814280 | `1a83c225960929f2c10e4908f9dc71ddcc606f85297c019f236ad6c0ca4a0198` |
| `02_bataille_pixelisee_original_1448x1086.png` | 1448 × 1086 | 2404601 | `657fbef320b12f21d30f346fdb49722be9dafd0dcdf686d7198cf3ec97490f48` |
| `01_bataille_narrative_320x240.png` | 320 × 240 | 170813 | `7825273a1c601f88a20e32493767cf04940f3a61dceb4095099b3b06f7a41313` |
| `02_bataille_pixelisee_320x240.png` | 320 × 240 | 183601 | `e6d9daa3f41c6b53f0c52d799ef2ec564d2d7fb86ac138a9d19017f18199b757` |
| `01_bataille_narrative_zoom4x.png` | 1280 × 960 | 258663 | `5fd63462332bbd0ad5e17e02368fbb988837a2f1987b4bddba2fc1a80e2f1e4b` |
| `02_bataille_pixelisee_zoom4x.png` | 1280 × 960 | 273195 | `9ca68f731811f54bddcef0d3cc9dae4e3515812a221ada934624d03bd04959ba` |
| `COMPARATIF_natif_2x320x240.png` | 640 × 240 | 354946 | `e49ccfdf24f0f689f6e0170cfbd0f4e3b86257755b34ff536be15b9a17fa38d7` |

**Méthode :** deux images OpenAI initiales PNG exactes ; exports 320 × 240 par Pillow/LANCZOS sans recadrage (rapport 4:3 identique) ; zoom ×4 Pillow/NEAREST sur les exports ; comparatif créé par juxtaposition des deux PNG 320 × 240, sans interpolation supplémentaire. Aucune recompression ou modification des originaux.

## Copies JPEG de revue — PRESENTES DANS GITHUB

| Nom exact | Taille (octets) | SHA-256 |
|---|---:|---|
| `01_scene_narrative_revue.jpg` | 347440 | `487c900a2ec2b932a34bbf934558f2b83ee667730b57c9165f35dfb489d7d8f4` |
| `02_scene_pixelisee_revue.jpg` | 149783 | `b1ca53dfe1134063ba4b09305597df114e7e2b02b68e637da2bf83e58a8276ae` |
| `01_scene_narrative_preview_320x240.jpg` | 24879 | `73857eb5489ff8fe65a6c85fad6e15c1a7970ff174079cc0dd0c204064cca557` |
| `02_scene_pixelisee_preview_320x240.jpg` | 31033 | `b13330c0e4c8f3480173495bbea0a2cedd7371836e1b10fb45f0b037e38e20cc` |

**Provenance des JPEG :** rendus de consultation produits par l'outil d'aperçu Adobe lors du transfert depuis les images originales vers GitHub ; **avec perte**, certaines copies redimensionnées. Il ne faut **jamais** les utiliser comme preuves d'identité des PNG originaux.

### Générations
- Image 01 : `a7162bd7-fb6e-49cd-9961-a46bca06b5f5`.
- Image 02 : `2674048b-ed4f-45a0-9107-02e4fcc94b57`, **nouvelle sortie générative** et non zoom déterministe.

**Contrôles à répéter après import :** taille, SHA-256 et dimensions de chaque PNG sur GitHub à HEAD exact ; aucune affirmation de validation CYD.
