# DRAFT — Références PO du patrouilleur (I-009)

Ce dossier conserve les **sources graphiques remises par le PO**, distinctes des exports 320 × 240 validés dans [`draft/etudes/patrouilleur/`](etudes/patrouilleur/). Les fonds océaniques de [`draft/etudes/ocean/`](etudes/ocean/) restent inchangés.

## États statiques et décisions PO

| Source dans `draft/` | État | Décision |
| --- | --- | --- |
| `patrouilleur_A.jpeg` | Intact | PAT-01 #79 — accepté |
| `patrouilleur_T1A.jpeg` | Dégâts arrière A | PAT-02 #80 — **T1A retenu** |
| `patrouilleur_T1B.jpeg` | Dégâts arrière B | Alternative PO conservée |
| `patrouilleur_T2A.jpeg` | Dégâts avant A | PAT-03 #81 — **T2A retenu** |
| `patrouilleur_T2B.jpeg` | Dégâts avant B | Alternative PO conservée |
| **`patrouilleur_B_PO_fusion.png`** | **Dégâts complets (avant + arrière)** | PAT-04 #82 — **nouveau master PO 07/11/2026** |
| **`patrouilleur_detruit.gif`** | Animation de destruction | **Nouvelle source PO 07/11/2026 ; PAT-05 #83 non lancé** |

### Remplacement du B historique

Le PO a fourni une **nouvelle composition réunissant les dégâts arrière T1A et avant T2A**. Son **original PNG sans perte** `draft/patrouilleur_B_PO_fusion.png` (1024 × 768 RGB) devient la **seule source canonique pour PAT-04**.

L'ancien `draft/patrouilleur_B.jpeg` (1195 × 896), présent lors du lot initial du 06/11/2026, **est retiré de la branche courante à la demande du PO**. Ancienne empreinte SHA-256 : `de84b5bf265cd290c24dfd84452511bf7892adccd399d9e92ba4c106f8f55450`, ancien blob Git `06c76f842a9d7dbbffb36044d55bdb27c61e8a69`. Il demeure consultable **dans l'historique Git uniquement**.

Un JPEG de compatibilité peut être **reconstruit en mémoire** depuis le PNG maître par le script d'export PAT-04 (Pillow 12.3.0, qualité 98, sans sous-échantillonnage, SHA-256 `0f6eb6a9dd4df3974a1cee5423d304930d4b1643c3612847e7599128fd374830`). **Ce JPEG est avec perte et n'est plus conservé dans `draft/`.** Il permet de reproduire les exports PAT-04 déjà contrôlés sans modifier le rendu approuvé.

### Nouveau GIF — source pour PAT-05 (non intégré)

Le PO a également fourni `patrouilleur_detruit2.gif`, conservé **sans modification octet par octet** sous le nom canonique `draft/patrouilleur_detruit.gif` :

- Dimensions : **478 × 356** ; **9 images × 160 ms = 1 440 ms**.
- SHA-256 du GIF remis : **`64391042c66bb89cce8f04622a213d85fd028ea6bdbefbf59cd60ed611c4f096`**.
- Ancien GIF du lot initial (9 images × environ 130 ms) remplacé ; ancien SHA-256 `bdf1f617e9b9d5f8fb83cea8370c7bf231f57e536206a39321e4a94bafbf4050`, ancien blob Git `264f8d9c8d525016d0f8462999b364844a58596b`.

**PAT-05 reste un spike optionnel distinct**, sans autorisation de lecture GIF embarquée, animation intégrée, export de frames ou validation de correspondance exacte avec le nouveau B statique.

## Empreintes SHA-256 canoniques

```text
631ff1b8f4245599137b6d0d3bd6919b9db3e30844d478434ee864ff885f5df5  draft/patrouilleur_A.jpeg
1e25359998afdedbc93ebf6baebdbeae34b327f4ab75a2f4e9d55e4c7b8a9584  draft/patrouilleur_T1A.jpeg
2bc761b1abcbced2fc760e8ef938571d58b3b4b14e390bad9f219846b32c8666  draft/patrouilleur_T1B.jpeg
f8366482082febc917a812d94efd98a75d03c100a789d5db73ceacc379c41d4d  draft/patrouilleur_T2A.jpeg
bcb36de1ef21c7992af3196d460ce57f68d654d4826302be0fdfa20b5515a9c3  draft/patrouilleur_T2B.jpeg
e004d3b1274133f118ab407e7854d2050121654cca0881b513c7acbc3bd1a808  draft/patrouilleur_B_PO_fusion.png
64391042c66bb89cce8f04622a213d85fd028ea6bdbefbf59cd60ed611c4f096  draft/patrouilleur_detruit.gif
```

La mer est peinte dans les sources : **aucun détourage alpha** ni budget Flash/RAM qualifié. **Statut : `DRAFT_REFERENCES — ON_DEVICE_UNVERIFIED`**.

## Flotte — références PO du 09/11/2026

Nouveau lot de six images de référence, documenté séparément dans [REFERENCES_FLOTTE_PO_2026-11-09.md](REFERENCES_FLOTTE_PO_2026-11-09.md). Les sources JPEG réencodées ne sont ni des sprites RGBA, ni les originaux binaires exacts.
