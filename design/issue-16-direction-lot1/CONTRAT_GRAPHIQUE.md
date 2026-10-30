# G01 — Contrat graphique provisoire consolidé du LOT 1

**Issue #16 · PR #19 (DRAFT) · 30/10/2026**  
**Statut : IDENTITÉ ARTISTIQUE ACCEPTÉE PAR LE PO COMME SOCLE LOT 1 — CONTRAT PROVISOIRE — DESIGN_PREVIEW — ON_DEVICE_UNVERIFIED.** Décision PO du 30/10/2026 [tracée dans l'Issue #16](https://github.com/Kharn27/ESP32-OTA-Battleship/issues/16#issuecomment-6534947575). Aucune approbation d'écran définitif ni de format firmware.  
**Périmètre :** langage artistique commun et besoins graphiques du LOT 1, **pas** validation pixel à pixel d'ATTAQUE, **pas** contrat d'asset firmware définitif. **Source métier unique :** [BIBLE_PRODUIT.md](../../BIBLE_PRODUIT.md), §14.1/14.4/14.5/14.10/14.13 et §15.

## 1 — Identité à conserver

**« La mer est le plateau »** : CYD WARSHIP doit ressembler à un **jeu vidéo naval illustré en pixel art indépendant contemporain**, chaleureux, expressif, immersif et riche, **pas** à une console militaire austère ou à un tableau de bord accompagné d'une vignette décorative. La scène doit former un tout : ciel crépusculaire, mer turquoise en profondeur, silhouettes nautiques, cuivre, eau/écume/impacts, interface et lumière cohérente. Toujours **lisible et manipulable sur 320 × 240 paysage**, au stylet.

**Principes communs :**
- Un **océan continu**, vagues, reflets et écume organiques ; éviter répétition mécanique de micro-motifs dans les 100 cases. La matière marine **s'efface suffisamment** sous les coordonnées et marqueurs.
- Même perspective, orientation de lumière, échelle et matière pour les navires, le ciel, les sillages, l'interface. Silhouettes lisibles, formes pixel nettes, relief sobre.
- Effets **ancrés** dans le plan d'eau : une gerbe ne flotte pas au-dessus de la case et un projectile ne ressemble pas à un trait de crayon. Pas de collage visuel.
- La scène illustre **le jeu** : grille, historique, cible et **FEU** dominent le décor ; une seule grande grille ATTAQUE **ou** DÉFENSE à la fois (Bible §14.5).
- Interface « marine » : fanions/plaque cuivrée/panneaux courts, sans grands bandeaux de télémétrie, jamais une accumulation de petites boîtes.
- **La forme et le libellé complètent la couleur** pour toute information tactique.

### D'où viennent ces principes

1. **Accueil GRAPH-ACC-001 / [PR #8](https://github.com/Kharn27/ESP32-OTA-Battleship/pull/8) — RETENU PAR LE PO en conception seulement :** horizon maritime, coucher de soleil, navires et action, titre discret et JOUER/PALMARÈS ; **aucune modification de l'accueil**, aucun statut runtime.
2. **Trois premières planches illustrées 1448 × 1086 — RÉFÉRENCE D'INTENTION retenue :** ambition de bataille navale illustrée rétro riche, presque spectaculaire, mer turquoise, crépuscule, coques expressives, impacts et cuivre. **Ni gabarits 320 × 240, ni règles de positionnement, ni assets validés.** Les intentions sont documentées dans l'Issue #16 et la [note ATTAQUE](ATTAQUE_RECOMPOSITION_NOTE.md) ; leurs originaux ne sont pas présents ici comme sources comparables pixel à pixel.
3. **Recomposition actuelle « La mer est le plateau » — PROPOSITION ACTIVE :** [rendu 320 × 240](attaque_recomposition_320x240.png), [zoom x4](attaque_recomposition_x4.png), [source Python](attaque_recomposition.py), [OpenRaster 4 calques](attaque_recomposition_layers.ora) et [note détaillée](ATTAQUE_RECOMPOSITION_NOTE.md). L'horizon et la mer sont continus, le bateau est illustratif **hors grille** et ne renseigne pas sur l'ennemi. Cette image exprime le **langage graphique**, **pas** une implantation d'écran approuvée.

### Séparer références, archives et ouvertures

| Statut | Matériau | Conséquence |
| --- | --- | --- |
| **RETENU : intention** | Pixel art illustré maritime, 3 premières planches | Garder ambition, matière, climat, personnalité ; **pas** leur disposition HD |
| **RETENU : conception accueil** | GRAPH-ACC-001, PR #8 | Continuité d'univers ; accueil inchangé et non testé matériellement |
| **PROPOSÉ : actif** | Recomposition ATTAQUE, sources 320 × 240 | Exemple visuel de G01 à soumettre ; **pas** un écran final |
| **REJETÉ ARTISTIQUEMENT / ARCHIVE** | G01 initiale, V2, V2.1 | Ne pas conserver leur géométrie dashboard/vignette comme direction ; utiles à l'historique et aux états métier |
| **NON RETENU / ARCHIVE GIT** | Anciens habillages A/B/C, agencements D/E/F, scènes narratives et SVG UX obsolètes | Voir [MAQUETTES.md](../../MAQUETTES.md) ; **A/B/C ne sont pas les trois premières planches illustrées** ; ne pas réactiver ces thèmes |
| **OUVERT : missions d'écran** | Haut d'ATTAQUE trop dense, grille chargée, position du bateau/décor, taille des commandes | Corriger dans les missions d'écran **sans refaire G01** |
| **OUVERT : missions d'écran / Développeur / S02** | Composition finale, formats, police, tactile, contrastes et performances | L'acceptation du socle artistique ne vaut ni approbation d'écran ni validation CYD |

La comparaison V2.1/recomposition est dans [ATTAQUE_RECOMPOSITION_NOTE.md](ATTAQUE_RECOMPOSITION_NOTE.md). Les vieilles previews DÉFENSE/conventions du dossier sont **des témoins historiques**, non des maquettes actuelles à généraliser.

## 2 — Couleurs : palette de travail, pas palette firmware

Ancres **RVB hex exactes** de la source actuelle **attaque_recomposition.py**. Le script utilise aussi des variantes locales pour les volumes/ciel ; ce tableau n'est **pas** une LUT exhaustive ni une imposition de RGB565, de quantification ou de nombre de couleurs.

| Fonction | Ancre(s) de conception | Sens proposé |
| --- | --- | --- |
| Contours, encre | **#0A1C32**, **#0A2337** | Lire formes et séparations, contraste froid |
| Mer profonde | **#07354E**, **#0E5069** | Ombres marines |
| Mer de surface | **#126E80**, **#188E93** | Océan vivant, variation et profondeur |
| Crête turquoise | **#30AFAB** | Reflets organiques et ondulation |
| Écume | **#CDF5DE** | Raté et contact eau/coque |
| Texte ivoire | **#F6F9D8** | Informations principales |
| Soleil / laiton | **#F8CB72** | Lumière chaude et accents |
| Cuivre de plaque | **#AA593D** | Matériaux de commandes |
| Action majeure | **#F37733** | FEU et confirmation principale proposée |
| Ambre | **#E99453** | Accent/avertissement en complément de forme/texte |
| Cible réversible | **#79F8F3** | Coins de sélection, distincts des éclats de mer |
| Gravité forte | **#612D3C** et rouge chaud local | Base d'alerte coulé, avec **libellé explicite** |

**Code sémantique de travail :** cible = **coins cyan + coordonnée** ; FEU = **orange/cuivre + verbe** ; raté = **écume claire ancrée dans l'eau** ; touché = **gerbe chaude + assise sombre** ; gravité DÉFENSE = **intensité, contraste et texte**, pas couleur seule. Un naufrage ennemi est annoncé **par le type du navire**, jamais par des segments d'épave dessinés sur ATTAQUE (Bible §14.4).

**Décision PO :** ambiance, richesse et expressivité retenues **comme identité commune**, sans validation définitive des nuances HEX ou des effets sur chaque écran. **S02 :** contrastes et différenciation sur dalle réelle, y compris selon la texture de mer ; valeurs hex non garanties identiques après conversion éventuelle.

## 3 — Typographies et règles de lecture

**Intention :** lettrage franc, compact, pixel net, utile d'abord, sans apparence de terminal. Titre de vue identifiable, coordonnées A–J et 1–10 sans confusion, accents français disponibles (**DÉFENSE, COULÉ, PRÊT**), textes courts avec contraste sur le fond. Ne pas multiplier les niveaux de police sur une même commande.

**Mesures descriptives de la recomposition PC** : FEU **~26 px**, titre de vue **~12–14 px**, cible **~13 px**, dernière riposte **~10 px**, micro-libellé **~9 px**. **Ni tailles ergonomiques validées ni police imposée au firmware.** Famille hôte utilisée : **DejaVu Sans Condensed**, non distribuée et non approuvée pour la CYD ; textes éditables dans le générateur Python, rasterisés dans l'ORA.

**À valider Développeur + S02 :** fonte embarquée, métriques, caractères accentués, stockage Flash, lisibilité réelle (9–14 px, chiffres/lettres proches), espacement des textes et alternative aux abrégés trop serrés.

## 4 — Boutons et états : grammaire, pas coordonnées fixes

| Famille | Langage visuel proposé | États requis / comportement décidé par la Bible |
| --- | --- | --- |
| **FEU** | Commande principale large, cuivre/orange, texte dominant | **sans cible : indisponible ; avec cible : disponible ; pressé ; verrouillé pendant résolution ; non tirable après verdict**. FEU engage irrévocablement la cible |
| **ATTAQUE / DÉFENSE** | Plaques ou pavillons contextualisés, lisibilité avant décor | vue active / autre vue / signal manqué, touché ou coulé / acquitté / verrouillé pendant FEU ; navigation volontaire sans bascule automatique |
| **AUTO / PRÊT** | AUTO secondaire, PRÊT confirmation forte | flotte placée automatiquement, retour accueil toujours possible **avant PRÊT**, y compris en revanche ; pas de placement individuel LOT 1 |
| **QUITTER / ABANDONNER** | Commande secondaire identifiable, confirmation claire | confirmation **CONTINUER / ABANDONNER** après PRÊT avant verdict ; **ignoré sans mise en attente** durant animation FEU |
| **REJOUER / ACCUEIL** | Commandes distinctes de la bannière de verdict | accessibles après verdict ; aucun tir supplémentaire, consultation libre |
| **VOIR SA FLOTTE / AFFICHER / MASQUER** | Option secondaire, jamais confondue avec FEU | après **défaite vs Capitaine** seulement ; demande volontaire sur ATTAQUE ; état conservé entre vues |

**Ordre de lecture proposé** : grille et case → marque de cible / dernier résultat → FEU → résumé DÉFENSE. L'état sélectionné ne doit pas pouvoir être pris pour un tir déjà validé. L'état bloqué doit être visuellement distinct de l'état disponible. **Aucune valeur de hitbox, aucun placement, aucune durée d'animation ne sont figés ici.**

La recomposition **illustre** des cases **18×18 pixels**, FEU proche de x205–319/y187–239 et un bandeau supérieur. **Ces valeurs ne sont pas un contrat tactile.** Le bandeau actuel et la densité de grille sont **des défauts d'écran à traiter plus tard**, et non des principes artistiques retenus.

## 5 — Marqueurs : sémantique et garde-fous

| État / information | Forme proposée | Invariant métier |
| --- | --- | --- |
| Inconnue | Eau/grille lisible, sans marque | Peut être choisie si jamais attaquée |
| Sélection avant FEU | **Quatre coins cyan + coordonnée** répétés hors case | Choix modifiable, désélection possible ; conservé lors d'un aller-retour DÉFENSE |
| Raté historique | **Écume claire** et remous ancré en eau | Non sélectionnable et historique conservé |
| Touché historique | **Gerbe chaude/flamme** sur remous sombre ; silhouette distincte du raté | Non sélectionnable ; ne révèle aucune appartenance d'épave |
| Coulement ennemi | **Texte du type coulé** (« Croiseur coulé »), intensité de message à concevoir | **Jamais de contours ou de cases regroupées en épave** ; navires peuvent se toucher |
| Sa flotte DÉFENSE | Coques du joueur, impacts superposés | **5 navires de 2/3/3/4/5**, 17 cases |
| Dernière riposte | Libellé court, coordonnée touchée visible | **Uniquement dernière manche**, remplacée à la suivante |
| Gravité non consultée | **manqué discret < touché visible < coulé renforcé**, avec libellé/forme, **sans compteur** | **Maximum non consulté** indépendant du dernier tir ; ouverture DÉFENSE acquitte alerte **et résumé**, sans effacer aucun impact |
| Verdict | **VICTOIRE / DÉFAITE / MATCH NUL** bien visible puis compact sur le champ de bataille | Après révélation synchronisée, pas de nouveau tir |
| Inspection post-défaite | Flotte adverse révélée **sous** les impacts déjà visibles | Volontaire et réversible sur ATTAQUE, conserve afficher/masquer entre vues |
| Palmarès | Nombres clairs, plaques sobres | VS CAPITAINE : parties/victoires/défaites/nuls + forfaits globaux uniques |

**Test conceptuel de la commande DÉFENSE :** dernière riposte **E5 TOUCHÉ** et naufrage **COULÉ NON LU** plus ancien. L'information courte dit **E5 TOUCHÉ** alors que l'alerte reste **COULÉ**. Ouvrir DÉFENSE acquitte le message et l'alerte ; les impacts restent. **Jamais de compteur.** Cette distinction vient de la Bible §14.10.2, non d'une invention de G01.

## 6 — Matrice de couverture pour les futures missions d'écran

| Parcours | États à concevoir sans les figer en G01 |
| --- | --- |
| Déploiement | Flotte AUTO préplacée ; AUTO, PRÊT, retour accueil avant PRÊT et après revanche |
| ATTAQUE préparation | Inconnue, raté, touché, case non réutilisable, sélection/désélection, FEU disponible/indisponible |
| Après FEU | Cible verrouillée ; navigation et abandon indisponibles, taps ignorés ; animation courte, résultat des **deux** tirs révélé simultanément |
| DÉFENSE | Sa flotte, dégâts historiques, dernière riposte et gravité non consultée, acquittement à l'ouverture |
| Verdict | Annonce forte puis compacte sur champ de bataille ; inspection libre, tirs bloqués, REJOUER/ACCUEIL |
| Défaite contre Capitaine | VOIR SA FLOTTE sur ATTAQUE, AFFICHER/MASQUER libre, historique toujours superposé |
| Abandon | CONTINUER/ABANDONNER uniquement après PRÊT et avant verdict |
| Palmarès | Statistiques VS CAPITAINE et unique compteur de forfaits |

**Ce sont des exigences de représentation**, pas des écrans supplémentaires produits, des animations prédéfinies ou des changements de gameplay.

## 7 — Hypothèses d'assets : contrat à négocier avec le Développeur

| ID **candidat non contractuel** | Source/usage | États et questions ouvertes |
| --- | --- | --- |
| **scene.sky / scene.sea** | Calque paysage de l'ORA | dessin statique, redraw par bandes ou primitives ? Stockage/décodage Flash et buffers à déterminer |
| **scene.ship.foreground** | Calque du navire décoratif (optionnel) | masque/occlusion, coût, positionnement final non décidé |
| **grid.cell / grid.cursor** | Grille et sélection de la source Python | primitives C ou sprites, zones tactiles, coût de retouche |
| **fx.miss / fx.hit** | Marques de raté/touché dessinées nativement | taille réelle, masque/transparence, éventuelle animation et rafraîchissement |
| **ui.action.fire / ui.tab.attack / ui.tab.defense / ui.alert.defense** | Boutons et alerte | états actifs/bloqués/acquittés, fond opaque ou overlay, longueur dynamique des textes |
| **ui.feedback / ui.verdict / ui.inspect / ui.stats** | **Besoins futurs, non assets G01 livrés** | dimensions et composants à concevoir en G02–G04 |
| **font.ui** | DejaVu **hôte** seulement | police embarquée et glyphes à choisir, coûts à mesurer |

**Sources de conception** : [attaque_recomposition.py](attaque_recomposition.py), [attaque_recomposition_layers.ora](attaque_recomposition_layers.ora), PNG natif, zoom x4 nearest et [SVG vectorisé auxiliaire](attaque_recomposition_trace_editable.svg). Le SVG tracé n'est pas une source sémantique de composants ; l'ORA et le code sont éditables. Dessins pixel originaux ; **aucune image tierce incorporée**. Adobe Firefly seulement pour étude d'ambiance non intégrée ; fonte DejaVu du poste **non livrée**.

**Fiche à fournir lors de la préparation firmware :** ID final, rôle/états, dimensions, transparence et masque, palette/profondeur **si approuvée**, source/export, taille binaire mesurée, licence/provenance, stratégie de dessin/décodage, consommation Flash/RAM/piles/buffers et latence observée. **Ne pas imposer maintenant RGB565, RLE, nombre de couleurs, atlas, bibliothèque ou framebuffer entier**. Aucune PSRAM/microSD garanties ; assets destinés à la Flash.

## 8 — Vérifications à réaliser avec le Développeur et en S02

| Priorité | Responsable | Question / preuve de sortie |
| --- | --- | --- |
| **P1** | **S02 physique, stylet** | Fiabilité **18 px/case de preview**, y compris bords, lignes 1/10, colonnes A/J ; enregistrement des erreurs de sélection, pas simple appréciation PC |
| **P1** | **S02 physique** | Hitboxes FEU/DÉFENSE/retour, appuis accidentels, proximité des commandes, distinction sélection ≠ FEU irrévocable |
| **P1** | **S02 physique** | Contraste réel grille / marques / ciel / mer / alerte, lisibilité des coordonnées, messages **9–14 px**, accents et niveaux manqué/touché/coulé |
| **P1** | **Missions d'écran + S02** | Bandeau supérieur surchargé, coordonnées et avertissement qui se concurrencent ; **grille dense** : corrections de layout ultérieures, **pas** nouveau G01 |
| **P1** | **Développeur + Pilotage** | Dessin par primitives vs sprites, zones redessinées, masques, typographie, stockage en Flash et stratégie d'affichage compatible sans PSRAM |
| **P1** | **Développeur + Pilotage** | **Mesurer** Flash, heap, plus grand bloc, pile, buffers, débit et latence de rafraîchissement sur matériel/firmware identifiés ; pas de budget déduit de la taille PNG |
| **P2** | **Développeur + S02** | Animation brève FEU, blocage des taps sans mise en attente, rafraîchissement sans scintillement, synchro des deux retours |
| **P2** | **PO / futures missions d'écran** | Rôle exact du navire illustratif et équilibre décor/lisibilité des autres vues, détails des transitions et états d'erreur |

**Preuves G01 actuelles :** PNG 320 × 240 et zoom 1280 × 960 nearest inspectés sur PC, script et ORA éditables. **AUCUN essai S02 ni validation physique ; ON_DEVICE_UNVERIFIED.** Les valeurs, tailles et effets demeurent des hypothèses jusqu'aux contrôles.

## 9 — Clôture G01 et frontière de responsabilité

**Décision PO du 30/10/2026 : ACCEPTÉ — SOCLE ARTISTIQUE LOT 1.** [Trace officielle](https://github.com/Kharn27/ESP32-OTA-Battleship/issues/16#issuecomment-6534947575). Le PO retient le pixel art naval contemporain riche et expressif, la mer vivante continue, le crépuscule maritime, les silhouettes crédibles, les impacts expressifs, les commandes cuivrées et la priorité à la lisibilité. Les trois premières planches restent références de l'ambition visuelle. Cette acceptation vaut **base de conception**, sans approbation au pixel de la recomposition ATTAQUE ni finalisation des autres écrans.

**À traiter dans les missions suivantes :** composition, bandeau supérieur, densité de la grille, dimensions, fonts, animations, hitboxes et validations S02. Aucun nouveau cycle artistique n'est requis dans G01. **Pilotage conserve la responsabilité des revues, CI et fusion** ; G01 reste un `DESIGN_PREVIEW / ON_DEVICE_UNVERIFIED`, pas un asset firmware certifié.

Références : [Issue #16](https://github.com/Kharn27/ESP32-OTA-Battleship/issues/16), [PR #19](https://github.com/Kharn27/ESP32-OTA-Battleship/pull/19), [workflow](../../docs/WORKFLOW_AGENTS.md), [politique de test](../../docs/TEST_POLICY.md), [index des archives](../../MAQUETTES.md).
