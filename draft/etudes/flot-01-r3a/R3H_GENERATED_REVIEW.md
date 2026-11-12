# FLOT-01 R3-H — image générée à examiner (Issue #113 / PR #116)

**Statut : CANDIDATE / DESIGN_PREVIEW — ON_DEVICE_UNVERIFIED — STOP R3-A PO.**

Livraison additionnelle à la première proposition R3-A de cette PR. Aucun remplacement du candidat R3-A initial ni des ressources R2 approuvées sous assets/. Pas de R3-B vertical, TOUCHÉ ou COULÉ.

## Fichiers livrés et dimensions réelles

| Fichier | Pixels | Rôle |
| --- | ---: | --- |
| r3h_source_generated_2172x724_RGBA.png | 2172×724 | Original HD issu de génération ChatGPT ; raster aplati, éditable au pixel, sans calques indépendants |
| r3h_generated_intact_h_54x18.png | 54×18 RGBA | Coque H isolée, proue droite, **candidat** non runtime ; alpha 0/255 |
| r3h_generated_intact_h_x4.png | 216×72 RGBA | Zoom NEAREST exact de la coque |
| r3h_generated_sillage_h_66x22.png | 66×22 RGBA | Filet d'écume séparé de la coque |
| r3h_generated_sillage_h_x4.png | 264×88 RGBA (opaque)A | Zoom NEAREST exact de l'effet |
| r3h_generated_mer_sombre_320x240.png et r3h_generated_mer_claire_320x240.png | 320×240 RGBA (opaque) | Preuves PC sur deux mers synthétiques contrastées et grille 10×10 |
| r3h_generated_mer_sombre_x4_1280x960.png et r3h_generated_mer_claire_x4_1280x960.png | 1280×960 RGBA (opaque) | Zooms ×4 NEAREST des preuves plein écran |
| r3h_generated_detail_sombre_x4.png et r3h_generated_detail_claire_x4.png | 264×88 RGBA (opaque) | Vues de détail H + eau |

## Provenance et méthode

La source est l'image générée dans ChatGPT le 12 novembre 2026 à partir de la direction conceptuelle R3 H et des recadrages déjà présents dans la conversation. Elle **n'est pas** un détourage authentique de la maquette historique G05. Les pixels HD ont été transférés via Adobe Creative Cloud, avec réencodage PNG sans promesse de conservation des octets compressés ; la source visuelle est ainsi versionnée dans cette PR.

Le script [build_r3h_generated.py](build_r3h_generated.py), exécuté avec Pillow 12.3.0, utilise un masque séparateur manuel et reproductible : région métallique (coque) / pixels de mousse. Réduction LANCZOS avant seuillage de l'alpha en 0 ou 255. **L'écume est exclue du fichier de coque.** Pour la composition, la coque s'ancre en (x,y), la couche de sillage en (x-10,y) dans un cadre 66×22 ; convention **proposée** seulement, à arbitrer par Pilotage/Dev.

La source HD est aplatie : le métal et l'écume se chevauchent près de la dérive. Le masque ne recrée pas les calques inexistants ; des détails de reflets et de mâts se simplifient sensiblement à 54×18. **La qualité à 1× est l'objet de la revue PO**, pas un résultat considéré acquis. Deux fonds illustratifs fabriqués pour la preuve (pas les mers G05 officielles) évaluent le contraste. La grille de contexte suit l'origine (21,53), pas 18, coque en C6–E6 à (57,143).

Comparaisons indépendantes obligatoires : [maquette G05 legacy 320×240](../../../maquettes/defense/legacy/g05_r2_defense_320x240.png), [R2 actuellement approuvé](../../../assets/flotte/sous-marin/sous_marin_intact_h.png), et [premier candidat R3-A](sous_marin_intact_r3a_h_54x18.png). Ne remplacer aucun de ces trois fichiers.

## Contrôles

Exécuter depuis ce répertoire : python3 build_r3h_generated.py --check

Le mode --check ne modifie aucun fichier : dimensions, canal RGBA/alpha binaire et **identité pixel à pixel des sorties décodées** sont vérifiés contre la source. Le script affiche le SHA-256 des fichiers effectivement présents. Une comparaison en pixels (et non en octets PNG) est nécessaire car le transfert Adobe peut réencoder le PNG sans changer l'image.

Ce dépôt ne reçoit à cette étape que des candidats dans draft/ : aucun ajout à assets/manifest.json, aucune modification des maquettes, du firmware, de la Bible ou des originaux PO. Les poids des PNG ne sont pas des mesures de mémoire CYD. **Tests sur écran réel, budget Flash/RAM et rendu de l'alpha : NON VÉRIFIÉS.**

**Décision demandée :** PO choisit, rejette ou demande correction du R3-H à 1× et du filet d'eau ; Pilotage vérifie le HEAD, décide de la suite et seul Pilotage peut fusionner.
