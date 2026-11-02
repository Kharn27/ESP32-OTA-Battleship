# G04 — ATTAQUE VS CAPITAINE · Issue #45

**Responsable :** Graphiste / DA · **Preuve :** `DESIGN_PREVIEW — ON_DEVICE_UNVERIFIED` · **Décision artistique PO G04 :** EN ATTENTE.  
**Branche :** `design/issue-45-attaque` · **Base :** `main` au lancement. **Ne pas fusionner hors Pilotage.**  
**Source de vérité :** [Bible produit](../../BIBLE_PRODUIT.md) §§14.1, 14.4–5, 14.8, 14.10.1–2, 14.13 ; [mandat #45](https://github.com/Kharn27/ESP32-OTA-Battleship/issues/45) ; [identité artistique G01](../issue-16-direction-lot1/CONTRAT_GRAPHIQUE.md). Conception **sans firmware**.

## Une composition principale, plus trois vignettes d'états

| Fichier | Usage |
|---|---|
| [g04_attaque_320x240.svg](g04_attaque_320x240.svg) | **Source SVG réellement éditable** : groupes nommés (mer continue, grille, marqueurs, DÉFENSE, FEU, sortie) |
| [g04_attaque_320x240.png](g04_attaque_320x240.png) | Aperçu principal **320 × 240 pixels natifs** |
| [g04_attaque_x4.png](g04_attaque_x4.png) | Preuve **1280 × 960 par NEAREST sur le PNG natif** |
| [g04_etats_annexes.svg](g04_etats_annexes.svg) / [PNG](g04_etats_annexes.png) | Trois **vignettes ciblées** de transitions / contrôles, **pas trois écrans supplémentaires** |
| [export_previews.py](export_previews.py) | Reproduction et contrôles géométriques/sémantiques/PNG |

L'état principal représente **ATTAQUE, cible G5 préparée avant FEU**. L'axe vertical porte **A–J (lignes)** et l'horizontal **1–10 (colonnes)**, dans le même sens que G03. Les zones déjà attaquées sont **C2 RATÉ** et **E7 TOUCHÉ** ; G5 est encore **inconnue**. Aucun segment de coque de Capitaine ni silhouette adverse : les cases touchées n'identifient **jamais** l'appartenance à un navire précis.

### Composition & intention artistique

**« La mer est le plateau » (G01)** : un seul plan d'eau turquoise en profondeur, quelques sillages et reflets irréguliers sous la grille. Le cuivre/ambre et l'encre marine relient cadre, commandes et éclats d'impact ; l'eau reste libre sous la sélection. L'océan n'est ni une vignette isolée ni un décor collé derrière une interface de télémétrie. **Une unique grille offensive 10×10** occupe le côté gauche, sans grille DÉFENSE simultanée. Les touches, le raté et le curseur ont des **contours différents** : gerbe chaude à assise d'eau sombre, écume blanche ancrée, **quatre coins cyan** sur case encore inconnue. Aucun tir dessiné comme un trait flottant, aucune épave groupée, aucune flotte ennemie révélée.

La zone droite privilégie **DÉFENSE** puis **FEU**, le bouton orange/ambre dominant. Les deux informations **séparées** dans DÉFENSE montrent explicitement le scénario de test : **dernière riposte = RATÉ**, **alerte maximale non consultée = COULÉ** (événement antérieur). Pas de compteur, pas de reprise de coordonnées antérieures dans le résumé. L'ouverture reste une **action volontaire**.

### Implantation visuelle de preview (non hitboxes validées)

| Élément | Région approximative x,y,w,h | Sens / précautions |
|---|---|---|
| Grille offensive | `18,42,180,180`, 10×10 cases de **18 px** | Sélection de **case inconnue** uniquement ; entrée tactile réelle à étudier en T02-B/S02 |
| Vue ATTAQUE | Bandeau haut `20,1,106,25` | Vue courante identifiable, pas une commande de bascule automatique |
| **QUITTER** (libellé provisoire) | `251,4,65,20` | **Demande d'abandon** pendant bataille : confirmation **CONTINUER / ABANDONNER** avant verdict, **jamais** retour immédiat sans forfait |
| **DÉFENSE** | `206,35,111,102` | Navigation volontaire ; deux régions sémantiques **dernier tir** / **alerte non lue** ; réinitialisation du résumé et alerte lors de l'ouverture |
| Cible préparée | `205,140,113,25` | `CIBLE G5` visible hors case ; **pas** un impact déjà validé |
| **FEU** | `208,167,109,58` | Action principale, verrouille irrévocablement le tir préparé ; état principal = disponible |
| Historique / mini-légende | `18,226,180,14` | RATÉ (écume) / TOUCHÉ (gerbe). Ces marques restent visibles, ne sont plus sélectionnables |

**Tailles indicatives du SVG PC :** titre ATTAQUE 15 px, DÉFENSE 13 px, résultats et alerte 12–13 px, **FEU 29 px**, cible G5 14 px, légende 8 px, coordonnées 8.5–8.8 px, résumé 8.1–8.6 px, QUITTER 9 px. **Points fragiles :** les textes d'information à 8–9 px, la distinction des chiffres et la surface de sortie (20 px de haut) sont à contrôler au stylet/TFT ; il ne s'agit PAS de tailles ergonomiques approuvées.

## Mini-contrat d'états UI à destination du Développeur (proposition, non API firmware)

| Contexte | FEU / sélection | DÉFENSE et historique | Sortie |
|---|---|---|---|
| **Sans cible** | FEU **indisponible** ; historique conservé, seules cases inconnues sélectionnables | DÉFENSE volontaire et disponible | Commande secondaire disponible |
| **Avant FEU, G5 choisie** (maquette principale) | **Coins cyan** G5 + coordonnée hors grille ; touche de G5 une deuxième fois = désélection, autre inconnue = nouveau choix ; FEU actif | DÉFENSE volontaire ; aller-retour **conserve G5** | Quitter demande confirmation |
| **FEU engagé / courte animation** | Tir irrévocable ; grille/FEU temporairement indisponibles ; **appuis ignorés et non mémorisés** | DÉFENSE indisponible ; aucune bascule/acquittement ni action différée | QUITTER indisponible, aucun dialogue différé |
| **Révélation synchronisée** | Nouveau résultat RATÉ ou TOUCHÉ sur ATTAQUE **au même instant** que la nouvelle riposte sur DÉFENSE ; aucune sélection de la manche suivante avant révélation | Résumé = **dernière riposte seulement**, alerte = **gravité maximale non lue** ; pas de bascule automatique | Commande réactivée selon état de partie |
| **Après révélation, partie en cours** | Nouvelle cible inconnue sélectionnable immédiatement, FEU selon sélection | Consultation libre sur demande, pas de bouton CONTINUER | Confirmation CONTINUER/ABANDONNER |
| **Après ouverture volontaire de DÉFENSE** | Sur retour à ATTAQUE, cible préparée (si existante) inchangée | Résumé **et alerte acquittés** ; les impacts restent sur la grille DÉFENSE | Commande selon étape du combat |
| **Navire ennemi coulé** | Annonce textuelle du **type** (« Croiseur coulé ! ») ; **aucune case de coque adverse regroupée/identifiée** | Les règles de dernière riposte/gravité sont indépendantes | Inchangée |
| **Après verdict** | Aucun tir nouveau ; bascule ATTAQUE/DÉFENSE libre (autres écrans hors G04) | Historiques conservés | Accueil sans forfait supplémentaire |

**États illustrés en petites vignettes :** FEU sans cible, FEU engagé (verrous), révélation simultanée (bascule de nouveau libre). Les variantes sont *une proposition d'habillage* ; aucune durée, animation, bouton supplémentaire ni nouvelle règle. Une future riposte ayant coulé un navire du joueur doit montrer **la coordonnée et le type de navire coulé** dans le résumé de la **dernière manche** ; si cela déborde en 111 px, **composer sur deux lignes ou écourter un texte d'introduction**, sans éliminer l'information, sans inventer de compteur ni un mode défilant automatique.

### Validation et provenance

Depuis la racine du dépôt :

```sh
python3 -m pip install 'CairoSVG==2.7.1' 'Pillow==11.3.0'
python3 design/issue-45-attaque/export_previews.py
```

Le script vérifie : dimensions SVG, **9 lignes intérieures sur chaque axe = 10×10**, 20 coordonnées, couches sémantiques indépendantes, RATÉ et TOUCHÉ différenciés, cible distincte, FEU/DÉFENSE/QUITTER présents, dernière riposte ET alerte explicites, **aucune balise <use> ni symbole de flotte ennemie**, rendu natif, agrandissement 1280×960 **RG­BA exactement NEAREST** et contrôle négatif d'une altération RGB. Il exporte également l'annexe 720×160.

**Provenance :** dessin SVG original pour G04, sans photo, image tierce ni contenu généré à la place des éléments tactiques ; palette et principes G01 repris, sans retoucher G01/G02/G03. Police `DejaVu Sans Condensed` du poste de rendu **non incluse** : substitut de maquette non contractuel. PNG / SVG sont des **sources et previews PC**, pas des assets Flash/PSRAM validés ; aucun RGB565, RLE, atlas, API écran, budget mémoire ou hitbox ne sont prescrits.

**Limites et arbitrages PO :** composition finale, densité de décor, marqueurs, taille/type de police, accent « COULÉ » et libellé de sortie restent à accepter. **NON TESTÉ** : contraste réel 320×240 sur CYD, stylet/rotation T02-B, intégration D09, vitesse de rafraîchissement, allocations, consommation Flash/RAM. La fusion de D09 n'est pas une preuve d'affichage de cette maquette. **Pilotage revoit le HEAD actuel et fusionne seul** après avis artistique explicite du PO.

Liens : [Issue #45](https://github.com/Kharn27/ESP32-OTA-Battleship/issues/45), [G01 #16](https://github.com/Kharn27/ESP32-OTA-Battleship/issues/16), [G02 #30](https://github.com/Kharn27/ESP32-OTA-Battleship/issues/30), [G03 #37](https://github.com/Kharn27/ESP32-OTA-Battleship/issues/37), [D09 #43](https://github.com/Kharn27/ESP32-OTA-Battleship/issues/43), [Test Policy](../../docs/TEST_POLICY.md).
