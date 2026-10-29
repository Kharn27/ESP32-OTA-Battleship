# I-009 — Scènes narratives « Bataille en cours » (archives)

**ARCHIVEE / EXPLORATION_D_AMBIANCE / NON_VALIDEE_PO / NON_RUNTIME**

> **FRONTIÈRE PRODUIT :** ces scènes montrent, à titre **strictement narratif**, un navire ennemi **visible, touché et en feu** avec des tirs vers l'horizon. **Ce ne sont pas des écrans de gameplay** : elles n'autorisent en aucun cas à révéler les positions inconnues de la flotte adverse en combat. Le LOT 1 et la confidentialité des flottes restent fixés par [BIBLE_PRODUIT.md](../../../BIBLE_PRODUIT.md).

Deux générations OpenAI datées du 29/10/2026, sans interface : contre-torpilleur au premier plan, tir lointain, océan au couchant et cible en feu.

| 01 — image narrative | 02 — pixelisation plus marquée |
|:---:|:---:|
| ![Vue 01 : copie JPEG de revue 320x240](01_scene_narrative_preview_320x240.jpg) | ![Vue 02 : copie JPEG de revue 320x240](02_scene_pixelisee_preview_320x240.jpg) |
| [Copie JPEG de revue](01_scene_narrative_revue.jpg) | [Copie JPEG de revue](02_scene_pixelisee_revue.jpg) |

Ces aperçus de revue **sont des JPEG avec pertes, non des PNG originaux**. Le comparatif des **deux véritables exports natifs PNG 320 × 240**, juxtaposés dans un fichier 640 × 240 sans réduction supplémentaire, est livré dans l'archive de remise de la conversation : `COMPARATIF_natif_2x320x240.png`, **pas encore archivé sur GitHub**.

## Origine exacte des deux générations

| Génération | Identifiant OpenAI | PNG original exact | Export PNG natif |
|---|---|---|---|
| 01 — narrative | `a7162bd7-fb6e-49cd-9961-a46bca06b5f5` | `01_bataille_narrative_original_1448x1086.png` | `01_bataille_narrative_320x240.png` |
| 02 — plus pixelisée | `2674048b-ed4f-45a0-9107-02e4fcc94b57` | `02_bataille_pixelisee_original_1448x1086.png` | `02_bataille_pixelisee_320x240.png` |

**La génération 02 est une image régénérée**, pas une transformation pixel-perfect de la génération 01. **Aucune source multicouche** n'est disponible. Les PNG originaux (1448 × 1086) sont gardés **strictement octet pour octet**, les exports sont des réductions à 320 × 240 **LANCZOS** (Pillow), sans recadrage (rapport 4:3), puis zooms optionnels 1280 × 960 au **plus proche voisin**, sans interpolation.

**IMPORTANT — LIVRAISON PARTIELLE DU CORRECTIF GITHUB :** faute de chemin de transfert binaire fiable via le connecteur disponible, **les deux originaux PNG et les deux exports PNG natifs ne sont PAS encore versés dans cette branche**. Ils sont conservés et contrôlés dans l'archive de remise, mais celle-ci **ne constitue pas le support GitHub durable exigé par le Pilotage**. Ne pas considérer cette correction comme complète tant qu'ils ne sont pas importés et contrôlés sur le HEAD GitHub.

Consulter l'[inventaire SHA-256](MANIFEST_SHA256.md) (nom, dimensions, taille, empreinte de chaque fichier), et les [consignes d'import binaire](IMPORT_ORIGINAUX.md) pour achever la mise au dépôt sur cette même PR.

## Intention artistique et limites
Océan bleu pétrole et turquoise, écume froide contrastant avec les reflets or/corail, fumée anthracite. La narration naît de la séparation claire entre **navire principal proche** et **navire ennemi lointain touché**, reliés par les traits de tir.

Il s'agit de **recherche de langage graphique**, non d'une charte, d'un sprite exploitable, d'une grille, ni d'une recommandation sur les règles d'affichage. Aucune direction artistique validée, aucun calcul de RAM/Flash, aucune compilation ni aucun test sur écran CYD.

**La PR #1 assure séparément le classement historique de A–F dans MAQUETTES.md.** Aucun changement simultané de ce fichier dans la PR #2 ; sa synchronisation éventuelle attend l'autorisation du Pilotage après revue de la #1.

**Statut de remise : CORRECTIONS DOCUMENTAIRES FOURNIES / BLOQUANT BINAIRE OUVERT — NON_VALIDEE_PO — ON_DEVICE_UNVERIFIED.** Fusion interdite sans contre-revue du Pilotage.
