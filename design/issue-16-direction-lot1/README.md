# G01 — Direction visuelle LOT 1 / Issue #16

**Responsable :** Graphiste / DA. **Maturité actuelle V2 :** `DESIGN_PREVIEW — ON_DEVICE_UNVERIFIED — EN_ATTENTE_RETOUR_ARTISTIQUE_PO`.

> **Reprise artistique du 30/10/2026 — V2 en attente du PO.**
> La déclinaison initiale (attaque_320x240.png, defense_320x240.png, conventions_320x240.png et render_g01.py) a été **rejetée artistiquement** par le PO. Elle reste dans cette branche comme historique, pas comme cible de production.
> **Proposition actuelle** : [note](ATTAQUE_EXPRESSION_V2.md), [ATTAQUE native](attaque_expression_320x240.png), [zoom ×4](attaque_expression_x4.png), [source Python](attaque_g01_v2.py) et [ORA en calques](attaque_expression_layers.ora). La déclinaison DÉFENSE/conventions est suspendue jusqu'au retour artistique du PO.



## Intention

CYD WARSHIP conserve l'intention de l'accueil retenu dans la PR #8 : horizon maritime, couleurs crépusculaires, cuivre chaud, bleus de mer, silhouettes évocatrices. La continuité concerne l'atmosphère et l'identité, **pas** la reprise ni la retouche de l'illustration déjà validée. En combat, l'océan devient une matière graphique calme et lisible : les cases et impacts priment sur les détails. La direction doit éviter la froideur des tableaux de bord militaires et la démonstration spectaculaire qui masque le geste tactile.

## Livraison

| Fichier | Rôle |
|---|---|
| `render_g01.py` | **Source éditable** : géométrie, composants, couleurs, navires et états définis en code, sans raster encapsulé. Génère tous les PNG. Nécessite Pillow uniquement sur PC. |
| `attaque_320x240.png` | Preview ATTAQUE native : 10×10, choix G5, FEU, ATTAQUE/DÉFENSE, touches/ratés ; aucun navire ennemi révélé. |
| `defense_320x240.png` | Preview DÉFENSE native : cinq navires 5/4/3/3/2, impacts, dernière riposte H5, alerte acquittée. |
| `conventions_320x240.png` | Planche native : sélection, raté, touché, gravité de l'alerte, boutons, titre et verdict. |
| `*_x4.png` | Agrandissements 1280×960, **nearest-neighbor**, directement dérivés de chaque PNG natif. |
| `CONTRAT_GRAPHIQUE.md` | Conventions, IDs, états, demandes d'arbitrage, hypothèses techniques et matrice de couverture G01. |

Commande de génération : `python -m pip install Pillow && python design/issue-16-direction-lot1/render_g01.py design/issue-16-direction-lot1/`.

**Reproductibilité :** les sorties natives sont dessinées **à 320 × 240**, sans réduction depuis une image HD. Les x4 sont strictement calculés par duplication nearest-neighbor et vérifiés par comparaison octet des pixels. Les polices hôtes proviennent de DejaVu (voir ci-dessous) ; elles ne constituent **pas** une police runtime ni un budget Flash prédéterminé. Le poste de production peut changer la rasterisation des fontes : les PNG versionnés demeurent les références de revue.

## Correction de la revue Pilotage du 30/10/2026

1. Sur **ATTAQUE**, aucun dessin de coque/épave adverse, aucun regroupement visuel des impacts. Le nom « Croiseur coulé » figure en message uniquement, sans révélation des cases appartenant au navire (§14.4).
2. L'onglet **DÉFENSE** en vue ATTAQUE peut porter un **repère de gravité non consultée sans compteur**. Une fois DÉFENSE ouverte, la maquette montre « Consultée / Alerte acquittée », distinct de l'information sur la dernière riposte (§14.10.2).
3. La flotte DÉFENSE contient exactement **cinq navires**, de tailles 5, 4, 3, 3, 2, pour 17 cases. Son état illustratif est **2 intacts + 2 endommagés + 1 coulé = 5**.
4. Les previews natives 320×240 et x4 sont des fichiers réels, non des cartouches à l'intérieur d'une planche 1448×1086.
5. Les conventions déploiement, palmarès, verdict, revanche et révélation facultative sont documentées pour G02–G04 sans figer leurs écrans.

## Revue visuelle PC (30/10/2026)

- `PASS` : dimensions 320×240 ; 10 colonnes et 10 lignes ; FEU distinct ; une seule grille visible.
- `PASS` : test de duplication nearest-neighbor ×4 ; marqueurs distincts et aucune coque adverse sur ATTAQUE.
- `PASS` : réconciliation du compte de flotte et de la sémantique d'alerte.
- `À VÉRIFIER` : contraste, lisibilité, précision et taille des cibles **sur la vraie CYD et au stylet** dans S02 ; la surface géométrique de case actuelle est seulement **18×18 px**. Cela peut se révéler trop petit. Il ne faut pas considérer la preview comme une ergonomie approuvée.
- `NON TESTÉ` : flash/heap/décodage, performances, tactile, durée d'animations, polices embarquées.

## Provenance et droits

Illustrations, tracés des navires et composants **dessinés procéduralement pour G01** dans `render_g01.py` (sans réemploi de matériel graphique tiers ni de l'illustration d'accueil). La référence esthétique PR #8 est citée mais non incorporée. Polices de conception DejaVu Sans Condensed (système hôte, licence permissive DejaVu ; pas de police distribuée ni déclarée intégrable). Images génératives présentées dans la conversation initiale : **écartées**, non copiées dans ces livrables.

**Frontière :** maquettes de décision graphique exclusivement. Ne pas utiliser tel quel comme fichiers runtime : l'ESP32 n'a ni PSRAM ni carte microSD garanties ; format, palette, bandes de décodage, transparence, RAM et Flash **restent à contractualiser avec le Développeur et le Pilotage**.

Références : `BIBLE_PRODUIT.md` v0.17 §1, §2, §14.1, §14.4–5, §14.10, §14.13, §15 ; `docs/WORKFLOW_AGENTS.md` ; `docs/TEST_POLICY.md` ; PR #8 ; Issue #16.