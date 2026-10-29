# I-009 — Bible produit : Bataille navale ESP32 CYD

**Version documentaire :** 0.7 — consolidation des décisions fonctionnelles LOT 1, 29 octobre 2026  
**Maturité :** expression de besoin en construction ; **ce document entier n'est pas encore validé**  
**Projet :** [ESP32-OTA-Battleship](https://github.com/Kharn27/ESP32-OTA-Battleship)  
**Origine :** [I-009 / IDEE.md — Le Grenier Geek](https://github.com/Kharn27/Le-Grenier-Geek/blob/main/plans/I-009-bataille-navale-cyd/IDEE.md)

> Gouvernance : **DÉCIDÉ** = approuvé par le PO ; **PROPOSÉ** = suggestion non validée ; **À ARBITRER** = règle encore ouverte ; **À VÉRIFIER** = faisabilité, mesure ou essai nécessaire ; **REPORTÉ** = hors du besoin immédiat. Une cible V1.0 ne signifie pas automatiquement « MVP fixé ».

## 1. Promesse et priorité de conception

Créer un jeu de bataille navale familial, tactile et tactique sur ESP32 CYD classique. Le **LOT 1** commence contre Mousse sur une seule CYD ; la cible multijoueur humain utilise **deux ESP32 CYD**, un par joueur, avec une flotte secrète par console. **Intention artistique décidée par le PO :** jeu indépendant contemporain en pixel art riche, expressif et immersif, avec priorité à la lisibilité sur écran natif 320 × 240. **Charte visuelle définitive non validée.**

**DÉCIDÉ — principes d'expérience :**
- Le **gameplay passe avant le spectacle** : fluide, compréhensible sans se poser de questions, avec des retours immédiats.
- Associer une stratégie familiale facile d'accès à une ambiance maritime vivante et expressive, sans imposer l'austérité d'un terminal militaire ou d'un tableau de bord technique. Les choix précis de palette, sprites et composition restent à arbitrer.
- Les animations, le sonar et les éventuels sons doivent renforcer le suspense **sans ralentir** les actions.
- Le jeu doit rester agréable à deux enfants comme à des joueurs qui souhaitent approfondir la stratégie.

**PROPOSÉ :** retour visuel immédiat à la sélection, à la validation et à la révélation ; animations courtes et discrètes plutôt que séquences imposées.

**Références et explorations graphiques :** [index des maquettes et recherches artistiques](MAQUETTES.md), comprenant sept SVG UX historiques et six explorations A–F non retenues, toujours présents sur `main`. Les deux scènes narratives de la [PR #2](https://github.com/Kharn27/ESP32-OTA-Battleship/pull/2) ont été retirées de l'arborescence courante par décision PO, mais leurs [originaux, exports et provenance restent consultables dans l'historique Git](https://github.com/Kharn27/ESP32-OTA-Battleship/tree/872496d73a150dd5a016c219973e5865576b7e04/assets/explorations/2026-10-29-bataille-en-cours). **Aucune charte ni interface visuelle de production n'a été approuvée par le PO.** Les images ne sont ni des captures de firmware ni des représentations normatives des informations visibles en jeu.

## 2. Cadre matériel et frontières

**DÉCIDÉ / contraintes de référence :**
- Prototype sur ESP32-2432S028R classique, tactile résistif, **320 × 240 en paysage**, stylet ; pas de PSRAM présumée.
- Deux consoles pour une partie humaine à deux ; liaison directe **ESP-NOW**, sans Internet, box ni serveur.
- Pas de carte microSD. Images, polices et ressources sonores embarquées en Flash.
- Utilisation économe de la RAM ; ne pas imposer un grand framebuffer plein écran.
- Batterie externe ou alimentation externe acceptable pour le prototype ; aucun joystick, bouton ABXY ou grand écran supposé.
- Les trois maquettes historiques 320 × 240 dans le Grenier Geek sont des **intentions UX**, non des interfaces validées.

**À VÉRIFIER :** précision tactile, notamment les petites cases des premières maquettes (~12 × 12 px) ; solution d'affichage des grilles 15 × 15 ; ressources mémoire ; matériel audio réellement présent ; comportement et portée radio.

**REPORTÉ / horizon distinct :** console multijeux, boîtier/batterie intégrés, commercialisation, mode « amiral » à pouvoirs spéciaux.

## 3. Expériences de jeu visées pour la V1.0

**DÉCIDÉ sur le principe :** trois familles d'expérience constituent une bonne base pour la V1.0 ; leur nom définitif, leur présentation et le découpage éventuel en MVP restent ouverts.

| Famille / nom provisoire | Flotte et grille | Combat |
| --- | --- | --- |
| **Classique** | Grille **10 × 10**, flotte traditionnelle fixe de 5 navires, 17 cases | **Mono-tir simultané par défaut** ; **mono-tir alterné** comme variante facultative |
| **Escadre tactique** | Composition personnalisable, **5 navires maximum** ET **17 cases maximum** | Salves préparées et simultanées |
| **Flotte libre** | Composition personnalisable, **17 cases maximum**, sans plafond supplémentaire de nombre de navires | Salves préparées et simultanées |

### Trois maquettes historiques des expériences de combat

**Références d'intention, non normatives** : la maquette « classique » ci-dessous représente le tour par tour d'origine ; le **simultané est désormais le mode standard**. Le nombre de tirs visible dans l'ancienne maquette des salves peut également différer de nos règles tactiques actuelles.

![Maquette du mode classique historique](assets/maquettes/mode-classique-320x240.svg)

![Maquette du duel simultané](assets/maquettes/mode-simultane-320x240.svg)

![Maquette des salves historique](assets/maquettes/mode-salves-320x240.svg)

**À ARBITRER :** grille **15 × 15 envisagée**, non encore actée pour les deux familles tactiques ; arbitrage ergonomique réel sur CYD ; noms commerciaux ; place exacte des trois familles dans le premier MVP.

### Deux paramètres indépendants pour le combat — DÉCIDÉ

1. **Volume de tir :** mono-tir ou salve.
2. **Ordre de résolution :** alterné ou simultané.

Les **quatre combinaisons sont conceptuellement possibles**, mais cela **ne valide pas** leur exposition libre dans les menus dès la V1.0. **PROPOSÉ :** un accès simple aux trois expériences et, plus tard seulement, une personnalisation avancée des paramètres.

## 4. Flottes et budget

### Flotte classique — DÉCIDÉ

| Navire | Taille en cases | Contribution aux tirs en mode salve* |
| --- | ---: | ---: |
| Patrouilleur | 2 | 1 |
| Sous-marin | 3 | 2 |
| Contre-torpilleur | 3 | 2 |
| Croiseur | 4 | 3 |
| Porte-avions | 5 | 5 |

*Les contributions ci-dessus correspondent à la mécanique tactique proposée par le PO et retenue pour la réflexion sur les salves. En **mono-tir**, chaque joueur tire une seule fois, indépendamment de sa flotte.*

- **Classique :** un exemplaire de chaque navire, soit 5 bâtiments et 17 cases.
- **Tactique : DÉCIDÉ** — plafond **absolu de 17 cases occupées**, sans obligation de les remplir ; les cases inutilisées ne confèrent pas de bonus.
- **Escadre : DÉCIDÉ** — 5 navires maximum et 17 cases maximum.
- **Flotte libre : DÉCIDÉ** — nombre de navires non plafonné séparément ; seul le budget de cases fixe une borne matérielle.
- **BN-FT-006 — DÉCIDÉ :** chaque flotte tactique doit comporter **au moins un navire** ; une flotte vide est interdite. Une flotte d'un seul patrouilleur de deux cases reste autorisée.
- **DÉCIDÉ pour les compositions tactiques :** plusieurs exemplaires d'un même type sont autorisés.
- **À ARBITRER :** éventuelles restrictions d'équilibrage si une composition domine systématiquement les autres. Aucun « coût de construction » distinct de la taille n'est retenu pour l'instant.

**Exemples théoriques (pas des compositions recommandées) :** 8 patrouilleurs = 16 cases et 8 tirs de salve ; 2 porte-avions + 1 sous-marin + 2 patrouilleurs = 17 cases et 14 tirs de salve. Ces stratégies n'ont pas encore été équilibrées.

### Puissance de feu et dégâts — DÉCIDÉ

- Dans les variantes à salves, chaque navire **encore à flot** apporte sa contribution aux tirs.
- Un navire coulé **retire sa contribution des manches suivantes**.
- En résolution simultanée, les tirs déjà verrouillés sont exécutés même si leur navire d'origine est coulé au cours de la manche ; aucune suppression rétroactive.

**DÉCIDÉ :** une cible ne peut recevoir qu'un tir au cours d'une salve et une case attaquée lors d'une manche précédente ne peut plus être ciblée (§ 6, BN-FT-008 et BN-FT-009).

**À ARBITRER :** règles détaillées de fin d'affrontement, présentation de l'état des navires partiellement touchés et équilibrage réel des compositions tactiques.

## 5. Déploiement tactile — DÉCIDÉ

![Esquisse de placement tactile, état invalide](assets/maquettes/placement-flotte-320x240.svg)

*Reconstitution conceptuelle 320 × 240 : commandes et positions à évaluer sur matériel, non validées graphiquement.*

**Objectif :** une flotte immédiatement jouable, avec la liberté de peaufiner son placement sur l'écran résistif.

1. L'écran de placement s'ouvre avec **toute la flotte déjà placée aléatoirement**.
2. Chaque pression sur **AUTO** **recompose aléatoirement toute la flotte**, autant de fois que souhaité.
3. Un navire peut être déplacé par **sélection du navire puis toucher d'une case de destination** ; pas de glisser-déposer nécessaire.
4. **ROTATION** change l'orientation du navire sélectionné, **même si la nouvelle position est invalide**.
5. Une position invalide reste visible **en rouge** ; le joueur peut poursuivre ses corrections.
6. La modification d'un navire est **provisoire** jusqu'à **✓** (confirmer, seulement si position valide) ou **✕** (annuler et retrouver la position/orientation précédentes).
7. Le bouton général **PRÊT** est indisponible **si une position est invalide ou si un navire attend encore ✓ / ✕**.
8. AUTO n'équivaut pas à PRÊT : seul le joueur confirme que son déploiement est terminé.
9. **BN-FT-006 — DÉCIDÉ :** en mode tactique, **PRÊT** reste indisponible si la flotte est vide.

### Déploiement lors d'une revanche — BN-REV-001, DÉCIDÉ

- Chaque joueur choisit librement, pour lui-même, de **conserver les positions précédentes**, de **relancer AUTO** pour obtenir un nouveau placement ou de **personnaliser** son déploiement.
- Conserver une disposition risquée est un choix stratégique autorisé : le jeu ne force pas un placement inédit lors d'une revanche.
- Les mêmes contrôles de validité et la confirmation **PRÊT** s'appliquent.
- **PROPOSÉ :** ne pas révéler à l'adversaire si la disposition précédente a été conservée.
- **À ARBITRER :** traitement de la composition d'une flotte tactique lorsqu'on change de mode, et possibilité de conserver aussi sa composition à l'identique.

**À ARBITRER :** comportement exact d'AUTO pendant une modification en cours, point d'ancrage du navire à la sélection de destination, messages tactiles et placement sur grille 15 × 15.

## 6. Combat — règles décidées

### Mono-tir alterné (variante classique)

- Un tir par joueur, à tour de rôle.
- Un tir **touché, raté ou coulé** termine le tour ; **aucun tir bonus** pour un touché.

### Mono-tir simultané (standard)

- Chaque joueur sélectionne secrètement **une cible**.
- Avant FEU, le joueur peut corriger sa cible.
- **FEU verrouille définitivement** la cible, même si l'autre joueur n'a pas encore validé.
- Une fois les deux tirs verrouillés, leurs effets sont **résolus ensemble**, sans révéler à l'avance la cible adverse.
- Une destruction réciproque et une égalité sont possibles.

### Salves tactiques simultanées

- Les joueurs préparent secrètement **toutes les cibles** de leur salve, sans retour sur les impacts intermédiaires.
- Ils peuvent modifier leurs sélections tant que FEU n'a pas été pressé.
- **Un seul FEU** verrouille définitivement toute la salve.
- Les salves des deux joueurs sont résolues simultanément après les deux validations.
- La perte d'un bâtiment réduit la puissance de feu des manches **ultérieures**.

### Sélection, salves partielles et historique des tirs — DÉCIDÉ pour la V1.0

- **BN-FT-007 — Salves partielles autorisées :** un joueur peut déclencher **FEU** avec **au moins une cible** et **au plus** le nombre de tirs disponibles. Les tirs non utilisés sont perdus pour la manche, sans cumul ultérieur. Le plafond disponible reste calculé à partir des navires encore à flot.
- **BN-FT-008 — Un tir par cible et par salve :** premier appui tactile = sélection ; deuxième appui = désélection. Une case ne peut figurer qu'une fois dans la salve. Le compteur de sélection est actualisé ; une fois le plafond atteint, il faut retirer une sélection avant d'en ajouter une autre.
- **BN-FT-009 — Une case déjà attaquée est verrouillée :** toute case tirée lors d'une manche précédente reste affichée comme **touchée** ou **manquée**, mais devient définitivement **non sélectionnable** pour cette partie. Appuyer dessus ne consomme aucun tir et ne provoque pas de nouvelle attaque.
- Ces protections de ciblage s'appliquent **au classique et aux modes tactiques**, en **VS humain et VS CPU**. Elles n'annulent pas le caractère irrévocable du bouton **FEU** : seul le pointage *avant* FEU est modifiable.

**PROPOSÉ :** compteur « X/Y tirs » bien lisible, indicateur sobre « Ordres transmis — en attente de l'adversaire », animations courtes et non bloquantes.

**À VÉRIFIER sur CYD réel :** lisibilité des cases déjà attaquées et ergonomie de la sélection/désélection au stylet.

**À ARBITRER :** ordre et animation de révélation des impacts ; définition exhaustive de victoire/égalité pour les variantes autres que le classique mono-tir simultané (qui est **DÉCIDÉ** pour le LOT 1, cf. § 14) ; comportement de l'écran quand la liaison est interrompue après FEU.

### Présentation du combat — DÉCIDÉ pour la V1.0

**LOT1-UX-001 — Une grande grille à la fois :**
- Le joueur consulte la grille **ATTAQUE** ou la grille **DÉFENSE**, jamais deux grilles réduites simultanément.
- La bascule entre les deux vues doit être **immédiate**, sans navigation complexe ; les informations et impacts déjà acquis restent conservés.
- **ATTAQUE** est interactive pour le choix des tirs ; **DÉFENSE** présente les navires du joueur et les impacts reçus. Il n'y a pas de sélection de tir sur la grille DÉFENSE pendant le combat.
- Pas de réglage utilisateur proposant plusieurs dispositions de grilles en V1.0.

**LOT1-UX-002 — Pas de bascule automatique après un tir :**
- Les tirs du joueur et du CPU sont résolus selon la règle de simultanéité retenue.
- Le résultat du tir du joueur apparaît sur **ATTAQUE**, tandis qu'un indicateur informe des tirs ennemis.
- Le jeu **ne force jamais le passage sur DÉFENSE** : le joueur choisit quand consulter ses propres dégâts.

**LOT1-UX-003 — Indicateur intelligent sur la commande DÉFENSE :**
- **Tir ennemi manqué :** information discrète permettant de poursuivre immédiatement sur ATTAQUE.
- **Navire touché :** alerte bien visible, sans changement automatique d'écran.
- **Navire coulé :** alerte **plus marquée** qu'un simple touché.
- Si le joueur n'a pas consulté DÉFENSE après un impact important, **l'alerte reste visible jusqu'à cette consultation**.
- **DÉCIDÉ (LOT1-UX-004) :** si plusieurs événements restent non consultés, l'indicateur conserve le **niveau de gravité le plus élevé** (manqué < touché < coulé). Un événement moins grave ultérieur ne diminue pas l'alerte ; ouvrir DÉFENSE acquitte les alertes accumulées. Aucun compteur d'événements dans le LOT 1. Les libellés et codes visuels restent **À CONCEVOIR / À VÉRIFIER**.
- **À VÉRIFIER :** confort réel des cibles au stylet sur grille 10 × 10, lisibilité des indicateurs sur 320 × 240 ; une grille tactique 15 × 15 reste non validée.

Ces règles d'affichage sont décidées **pour l'interface V1.0**. Leur implantation visuelle, la charte artistique et les maquettes définitives ne sont pas encore validées.

### Fin de partie — confidentialité des flottes

![Esquisse de fin de partie : révélation facultative de la flotte](assets/maquettes/fin-partie-secret-320x240.svg)

**DÉCIDÉ — FIN-001 :** résultat connu des deux joueurs ; les tirs et impacts déjà révélés restent visibles. **La disposition complète de la flotte victorieuse demeure secrète par défaut**, et **le vainqueur choisit s'il souhaite la révéler** à son adversaire. Refuser ou ne pas choisir ne déclenche aucune révélation.

**Cas de l'égalité par destruction réciproque :** les deux flottes sont alors entièrement touchées et leurs positions déjà connues : un bouton de « révélation » symétrique reste envisageable **par analogie**, mais ne fournirait aucun nouveau renseignement. La présentation précise de cette option est **PROPOSÉE** et non encore fixée.

**DÉCIDÉ pour le LOT 1 VS Mousse (cf. § 14) :** résolution des deux tirs avant le résultat ; victoire, défaite et match nul par destruction réciproque ; révélation facultative de la flotte de Mousse vainqueur à visée pédagogique ; inspection libre après bataille. **À ARBITRER :** conditions des autres variantes et détails graphiques des écrans de résultat. L'**abandon volontaire** est traité sans résultat comptabilisé (§ 7).

## 7. Identité et palmarès de chaque CYD

![Esquisse de la fiche adverse consultable à la demande](assets/maquettes/fiche-adversaire-320x240.svg)

*Données fictives — esquisse UX, pas une interface validée.*

**DÉCIDÉ — BN-STAT-001 :**
- Chaque console conserve un **palmarès persistant propre à la CYD**, et non un compte joueur.
- Quatre catégories **indépendantes** sont conservées : **VS HUMAIN**, **VS MOUSSE** (CPU débutant), **VS CAPITAINE** (CPU intermédiaire) et **VS AMIRAL** (CPU expérimenté).
- Pour **chacune** de ces catégories : **parties jouées, victoires, défaites et matchs nuls**.
- L'adversaire peut consulter la fiche d'une CYD **à la demande** ; le palmarès n'est jamais un écran obligatoire avant de jouer.

**DÉCIDÉ — BN-STAT-002 : abandon volontaire et partie non comptabilisée**
- Une bataille abandonnée **avant un résultat définitif** est **ignorée** dans le palmarès des **deux participants** : ni victoire, ni défaite, ni nul, ni partie jouée. En solo VS CPU, les compteurs de la CYD restent également inchangés.
- Aucun malus de victoire/défaite n'est attribué à celui qui abandonne. Une coupure radio temporaire **suspend** d'abord la bataille (§ 8) et ne constitue pas un forfait.

**DÉCIDÉ — BN-UX-003 : confirmation d'un abandon volontaire**
- Une commande **QUITTER / ABANDONNER** lancée pendant une partie affiche une confirmation explicite : le joueur peut **CONTINUER** ou **ABANDONNER**.
- Annuler la confirmation ne modifie ni la partie ni les statistiques.
- La formulation et la disposition des boutons sur 320 × 240 restent à concevoir et tester.

**DÉCIDÉ — BN-STAT-003 : un seul compteur global de forfaits**
- Chaque CYD mémorise **un unique compteur global** de forfaits volontaires, sans ventilation par adversaire, difficulté ou mode.
- Ce compteur augmente de **1 uniquement lorsque le joueur confirme lui-même son abandon** ; les compteurs de parties/victoires/défaites/nuls restent inchangés.
- **Aucune déconnexion radio, extinction imprévue ou erreur technique n'augmente automatiquement ce compteur.** Le forfait est une information humoristique, non un classement.
- **Libellé humoristique PROPOSÉ :** « Canots de sauvetage empruntés ».

**PROPOSÉ :** accès « Dossier adverse » dans le salon de jeu, éventuel nom de console/commandant, taux de victoire, grades ou séries ; consultation des quatre catégories par onglets et éventuel total de parties.

**À ARBITRER :** éventuelles statistiques supplémentaires **par mode**, partage du palmarès lorsque plusieurs humains utilisent la même CYD, cas d'une session interrompue définitivement sans abandon confirmé, présentation exacte du compteur et de la fiche.

## 8. Créer / rejoindre : multijoueur humain

![Esquisse de l'écran d'accueil avec création, rejoindre et VS CPU](assets/maquettes/accueil-creer-rejoindre-320x240.svg)

*Disposition graphique encore PROPOSÉE ; les trois accès fonctionnels sont retenus pour la cible V1.0.*

**DÉCIDÉ — expérience fonctionnelle :**
- L'accueil propose une action volontaire **CRÉER** ou **REJOINDRE** plutôt qu'un appairage automatique imposé.
- Chaque partie humaine oppose **deux CYD**, même si plusieurs consoles et plusieurs duos sont présents à proximité.
- Les joueurs doivent pouvoir changer d'adversaire sans opération technique pénible.
- Positions des navires et cibles secrètes restent confidentielles jusqu'aux révélations prévues par les règles.

**PROPOSÉ sur la base du retour architecte (non encore arbitré par le PO) :**
- L'hôte rend sa partie visible ; l'invité la choisit parmi les annonces disponibles.
- L'hôte **accepte ou refuse** explicitement l'invitation ; confirmation claire sur les deux écrans.
- Une partie occupée n'admet pas de troisième joueur ; invitation expirée ou ancienne rejetée ; changement d'adversaire après fermeture propre de la session.
- Un signe visuel commun peut aider deux enfants à vérifier qu'ils rejoignent le bon adversaire.
- Aucun canal radio, adresse MAC ou autre paramètre technique ne doit être saisi par le joueur.

### Interruption de liaison — DÉCIDÉ pour la V1.0

- Une **coupure radio temporaire suspend** la partie et doit être indiquée clairement aux joueurs.
- La partie **reprend lorsqu'une reconnexion cohérente est obtenue** ; aucun tir ne doit être perdu ou appliqué deux fois à cause de la coupure.
- Une simple coupure radio **ne constitue pas automatiquement une défaite**.
- **REPORTÉ après V1.0 :** sauvegarde/restauration complète de partie après extinction ou redémarrage de la CYD. Cela pourra être reconsidéré ultérieurement.
- **DÉCIDÉ :** l'abandon volontaire confirmé et son effet statistique sont traités au § 7.
- **À ARBITRER :** comportement si l'adversaire ne revient pas, délai d'attente, retour au menu et sort d'une partie interrompue définitivement sans abandon explicite.

### Résultats de la consultation de l'architecte (28/10/2026)

**Avis reçu : FAISABILITÉ TECHNIQUE FAVORABLE**, sans vérification sur les deux CYD physiques. Le parcours CRÉER / REJOINDRE et la coexistence de duos indépendants sont considérés réalisables sur ESP32 classique via ESP-NOW. La découverte des annonces, la synchronisation des associations et la reprise après rupture restent les sujets délicats.

**À VÉRIFIER sur matériel :** fiabilité de la découverte, présence de plusieurs parties, radio en environnement chargé, cohérence des sessions et reprises, consommation de mémoire, confidentialité des échanges. Aucun nombre maximal de parties simultanées n'est garanti.

**Recommandations d'architecture, NON validées comme décisions PO :** canal Wi-Fi commun prédéfini (sous réserve d'essais) ; annonces de découverte avec contrôle d'obsolescence ; confirmations applicatives et protection contre doublons ; distinction entre consentement visuel et sécurité cryptographique. Les annonces broadcast ne sont pas automatiquement chiffrées ; la sécurité des échanges privés requiert une étude adaptée.

**DÉCIDÉ — BN-REV-002 / BN-REV-003 / BN-REV-004 : parcours de revanche V1.0**
- Le joueur qui a choisi **CRÉER** est l'**hôte** de la session (pas un serveur central).
- Après le résultat, l'hôte est celui qui **propose la REVANCHE** ; **CHANGER DE MODE** n'est pas une option de même niveau que REVANCHE, mais un choix *dans* ce parcours.
- L'hôte choisit soit **REJOUER** (mêmes règles), soit **CHANGER DE MODE** (nouvelles règles et paramètres sélectionnés par l'hôte).
- L'invité voit la proposition et peut **ACCEPTER** ou **QUITTER** ; il ne peut pas modifier les paramètres choisis par l'hôte. **Aucune nouvelle partie n'est imposée.**
- Si la proposition est acceptée, les deux consoles **conservent leur association** ; chaque joueur revient au déploiement et dispose de la liberté prévue par BN-REV-001.
- La logique de revanche vise à enchaîner les batailles sans refaire le parcours CRÉER / REJOINDRE.

**À ARBITRER :** détails d'affichage et de fermeture de session, abandon ou absence durable de l'adversaire, refus initial d'une invitation, expiration et départ propre. L'accès QUITTER depuis la fin de partie reste à mettre en forme ; la règle d'accord de l'invité pour la revanche est décidée.

## 9. Mode solo VS CPU — DÉCIDÉ pour la V1.0

**DÉCIDÉ :**
- Un joueur peut affronter le **CPU sur une seule CYD**, sans deuxième console.
- Le **VS CPU est inclus dans la cible V1.0** et disponible dans **toutes les familles de jeu V1.0** : classique simultané, classique alterné, escadre tactique, flotte libre.
- **Trois niveaux de difficulté** sont prévus.
- Dans les variantes tactiques, la difficulté influence **aussi le choix stratégique de la composition de flotte du CPU**, pas seulement le choix de ses tirs.
- Le CPU doit respecter les mêmes contraintes de flotte et de résolution que l'adversaire humain.

**DÉCIDÉ comme intitulés des trois niveaux utilisés dans le palmarès : Mousse, Capitaine, Amiral.**

**PROPOSÉ — comportement précis des niveaux, non encore arbitré :**
- **Mousse** : débutant, comportement simple, compositions plutôt aléatoires.
- **Capitaine** : intermédiaire, recherche méthodique, flotte raisonnablement équilibrée.
- **Amiral** : expérimenté, exploitation plus poussée des indices et composition stratégiquement optimisée.
- **Principe d'équité souhaitable :** le CPU ne doit jamais accéder aux positions secrètes des navires humains ; sa difficulté provient de ses décisions, pas de tricherie.

**À ARBITRER :**
- Logique de décision exacte, différenciation observable des trois niveaux et équilibre des flottes tactiques.
- Présentation des adversaires, éventuels délais d'animation, tutoriel ou aide.
- La présentation et la vérification de la difficulté réelle des trois CPU ; le **palmarès séparé VS humain et par difficulté CPU est décidé** (§ 7).
- Conditions de victoire et comportement d'un CPU dans les différentes combinaisons de combat.

**Attention :** le solo facilite l'étude et les essais du gameplay, mais **ne remplace pas** les vérifications propres au multijoueur ESP-NOW.

## 10. Expérience à finaliser / questions transverses

- **Fin de manche et de partie :** règles de victoire/défaite/égalité décidées pour le classique mono-tir simultané VS Mousse (§ 14) ; les autres variantes, les détails d'affichage et les transitions restent à préciser. Le secret du vainqueur (FIN-001) et la revanche pilotée par l'hôte en VS humain sont conservés.
- **Cas d'erreur :** la double sélection d'une cible et les tirs sur case déjà jouée sont traités ; restent notamment les écrans en désaccord, expiration des sessions et absence prolongée d'un adversaire. L'abandon **volontaire confirmé** est décidé, mais une interruption définitive non confirmée reste à définir.
- **Lisibilité :** propre flotte versus eaux adverses ; possibilité d'une grande grille unique et d'une bascule immédiate, surtout en 15 × 15.
- **Audio :** vérifier le matériel réel avant d'exiger des effets sonores.
- **Ergonomie :** évaluer chaque maquette sur une CYD physique, sans déduire la faisabilité d'une image.
- **MVP :** distinguer à terme première expérience jouable et cible complète V1.0 ; aucun planning, backlog ou chantier n'est lancé.

## 11. Registre succinct des décisions

| Référence | Décision ou sujet | État |
| --- | --- | --- |
| UX-001 | Gameplay fluide en priorité, immersion rétro sobre | **DÉCIDÉ** |
| UX-002 | Palmarès persistant par CYD, consultable à la demande | **DÉCIDÉ** |
| PL-001 | Flotte prépositionnée ; AUTO à volonté et retouche manuelle | **DÉCIDÉ** |
| PL-002 | Sélection + destination ; rotation autorisée même en position invalide | **DÉCIDÉ** |
| PL-003 | Rouge si invalide ; ✓ confirmer / ✕ annuler ; PRÊT bloqué en cas d'erreur ou de modification pendante | **DÉCIDÉ** |
| CL-001 | Classique : flotte fixe 5 bâtiments / 17 cases sur 10 × 10 | **DÉCIDÉ** |
| CB-001 | Simultané = standard ; alterné = variante facultative ; verrouillage irrévocable à FEU | **DÉCIDÉ** |
| CB-002 | Paramètres conceptuels mono-tir/salve et alterné/simultané | **DÉCIDÉ** |
| FT-001 | Flottes tactiques : 17 cases au maximum, non obligatoires, doublons autorisés | **DÉCIDÉ** |
| FT-002 | Escadre : max 5 navires ; flotte libre : sans plafond de nombre distinct | **DÉCIDÉ** |
| FT-003 | Salves secrètes simultanées ; puissance de tir liée aux navires encore à flot | **DÉCIDÉ** |
| BN-FT-006 | Au moins 1 navire en flotte tactique ; flotte vide refusée, PRÊT bloqué | **DÉCIDÉ** |
| BN-FT-007 | Salves partielles dès 1 tir ; tirs non employés perdus, sans cumul | **DÉCIDÉ** |
| BN-FT-008 | Sélection/désélection par appuis successifs ; une cible au plus par salve | **DÉCIDÉ** |
| BN-FT-009 | Case déjà attaquée non sélectionnable dans tous les modes, résultat conservé | **DÉCIDÉ / ergonomie À VÉRIFIER** |
| FIN-001 | Flotte du vainqueur secrète par défaut ; révélation volontaire après le résultat | **DÉCIDÉ** |
| BN-REV-001 | À la revanche, chaque joueur peut garder, randomiser ou personnaliser le placement | **DÉCIDÉ** |
| BN-REV-002 | REVANCHE : rejouer les mêmes règles ou choisir « Changer de mode » en sous-étape | **DÉCIDÉ** |
| BN-REV-003 | L'hôte CRÉER décide les règles de la revanche ; association conservée | **DÉCIDÉ** |
| BN-REV-004 | Invité : accepter ou quitter la revanche, sans négocier les paramètres | **DÉCIDÉ** |
| BN-STAT-001 | 4 palmarès : VS Humain, VS Mousse, VS Capitaine, VS Amiral | **DÉCIDÉ** |
| BN-STAT-002 | Partie abandonnée ignorée pour les deux joueurs ; aucune victoire/défaite/nul/partie jouée | **DÉCIDÉ** |
| BN-STAT-003 | Un unique compteur global de forfaits volontaires sur chaque CYD | **DÉCIDÉ** |
| BN-UX-003 | Confirmation obligatoire CONTINUER / ABANDONNER | **DÉCIDÉ** |
| CN-001 | Choix volontaire CRÉER / REJOINDRE | **DÉCIDÉ** |
| CN-002 | Étude ESP-NOW favorable sur le papier ; découverte, sécurité et multipaires à vérifier physiquement | **À VÉRIFIER** |
| CN-003 | Reconnexion d'une partie après coupure radio temporaire en V1.0 ; reprise après reboot reportée | **DÉCIDÉ / REPORTÉ** |
| FT-004 | Grille tactique 15 × 15 | **À ARBITRER / tester** |
| SOLO-001 | VS CPU sur une seule CYD, dans tous les modes V1.0 | **DÉCIDÉ** |
| SOLO-002 | Trois difficultés CPU, dont la composition de flotte tactique dépend du niveau | **DÉCIDÉ sur le principe** |
| SOLO-003 | Noms Mousse / Capitaine / Amiral pour les niveaux et palmarès ; comportements et équité CPU à préciser | **DÉCIDÉ (noms) / PROPOSÉ (comportements et équité)** |
| V1-001 | Trois familles de jeu comme cible de V1.0 ; MVP à préciser | **DÉCIDÉ sur le principe** |
| V1-002 | Catalogue V1.0 jugé assez riche ; priorité désormais à détailler règles et parcours | **DÉCIDÉ comme orientation de travail** |
| LOT1-001 | Première expérience à livrer : une partie classique complète VS Mousse sur une seule CYD | **DÉCIDÉ (orientation du lot)** |
| LOT1-PL-001 | Déploiement par AUTO répétable puis PRÊT ; pas de retouche manuelle dans le LOT 1 | **DÉCIDÉ (lot 1)** |
| LOT1-CPU-001 | Mousse explore au hasard et recherche autour des impacts, tout en commettant des erreurs | **DÉCIDÉ (principe LOT 1)** |
| LOT1-UX-001 | Une seule grande grille visible à la fois, bascule immédiate ATTAQUE/DÉFENSE | **DÉCIDÉ (V1.0)** |
| LOT1-UX-002 | Après FEU, aucune bascule automatique ; résultat joueur sur ATTAQUE, défense consultable à volonté | **DÉCIDÉ (V1.0)** |
| LOT1-UX-003 | Indicateur DÉFENSE : raté discret, touché visible, coulé renforcé, alerte persistante jusqu'à consultation | **DÉCIDÉ (principe V1.0)** |
| LOT1-FIN-001 | Résultat après résolution des deux tirs ; destruction mutuelle = match nul | **DÉCIDÉ** |
| LOT1-FIN-002 | Mousse vainqueur propose une révélation facultative de sa flotte pour apprendre | **DÉCIDÉ** |
| LOT1-FIN-003 | Inspection libre ATTAQUE/DÉFENSE après résultat, sans nouveaux tirs | **DÉCIDÉ** |
| LOT1-REV-001 | REJOUER contre Mousse dans le LOT 1 sans repasser par l'accueil | **DÉCIDÉ** |
| LOT1-REV-002 | Revanche : ancien placement du joueur affiché par défaut, AUTO ou PRÊT | **DÉCIDÉ** |
| LOT1-REV-003 | Nouvelle flotte aléatoire générée pour Mousse à chaque partie/revanche | **DÉCIDÉ** |
| LOT1-STAT-001 | Palmarès VS Mousse et compteur global consultables dès le LOT 1 | **DÉCIDÉ** |
| LOT1-CPU-002 | Après impact, Mousse privilégie une recherche autour de la cible | **DÉCIDÉ** |
| LOT1-CPU-003 | Après deux impacts adjacents alignés, priorité à l'axe supposé | **DÉCIDÉ** |
| LOT1-CPU-004 | Plusieurs pistes mémorisées, poursuivies prioritairement une à une | **DÉCIDÉ** |
| LOT1-CPU-005 | Après coulé, piste supposée terminée ; priorité à une autre piste | **DÉCIDÉ** |
| LOT1-FLT-001 | Navires au contact latéral ou diagonal autorisés, sans chevauchement | **DÉCIDÉ** |
| LOT1-CBT-001 | Type du navire coulé annoncé, épave non identifiée case par case | **DÉCIDÉ** |
| LOT1-ABD-001 | PRÊT ouvre la période d'abandon comptant comme forfait volontaire | **DÉCIDÉ** |
| LOT1-UX-004 | Indicateur DÉFENSE par gravité maximale non consultée, acquis à l'ouverture | **DÉCIDÉ** |
| LOT1-ACC-001 | Accueil minimal : JOUER CONTRE MOUSSE et PALMARÈS ; autres accès dans les lots ultérieurs | **DÉCIDÉ (découpage délégué)** |

## 12. Traçabilité et sources

**Visuels sauvegardés :** [MAQUETTES.md](MAQUETTES.md) : 3 SVG historiques recopiés à l'identique depuis le Grenier Geek et 4 esquisses UX reconstituées, en assets locaux. Les images d'ambiance externes et anciens composants interactifs du chat ne sont pas archivés à l'identique.

**Source de départ (27 octobre 2026) :** [IDEE.md du Grenier Geek](https://github.com/Kharn27/Le-Grenier-Geek/blob/main/plans/I-009-bataille-navale-cyd/IDEE.md). Les anciennes illustrations sont disponibles dans le dossier [assets](https://github.com/Kharn27/Le-Grenier-Geek/tree/main/plans/I-009-bataille-navale-cyd/assets).

**Consolidation des règles métier (28 octobre 2026, version 0.5) :**
- **Revanche :** conservation libre de la formation par chaque joueur ; REVANCHE comprend REJOUER ou CHANGER DE MODE ; l'hôte choisit les règles et l'invité accepte ou quitte ; association conservée si accepté.
- **Palmarès :** quatre catégories séparées (humain, Mousse, Capitaine, Amiral), quatre indicateurs par catégorie ; un compteur global de forfaits volontaires.
- **Abandon :** confirmation CONTINUER / ABANDONNER ; aucun résultat ni partie jouée comptabilisé pour les deux participants ; seule la CYD ayant volontairement confirmé incrémente son forfait ; coupures radio et incidents exclus.
- **Flottes et salves :** au moins un navire tactique ; salves partielles autorisées à partir d'un tir ; tirs non utilisés perdus ; sélection/désélection au toucher et pas de double cible.
- **Historique :** case déjà attaquée définitivement non ciblable dans la partie, résultat visible ; applicable à tous les modes.
- **Non décidé :** révéler ou non à l'adversaire qu'une formation a été conservée ; comportement précis des trois difficultés CPU ; grille 15 × 15 ; détails des victoires/égalités, des sessions interrompues définitivement et de l'UX de plusieurs écrans.
- **Traçabilité :** modifications apportées directement à la bible existante ; aucune modification du dépôt d'idées historique, pas de création de backlog ou de mission technique.

**Consolidation LOT 1 et UX V1.0 (29 octobre 2026, version 0.6) :**
- Premier lot ciblé sur une **partie classique jouable jusqu'au résultat contre Mousse sur une seule CYD**.
- Pour le LOT 1, placement **AUTO uniquement**, répétable, suivi de **PRÊT** ; le placement manuel reste une exigence V1.0 pour un lot suivant.
- Mousse débutant explore au hasard, exploite un impact pour tenter les cases voisines, et peut commettre des erreurs. Règles détaillées du CPU à formaliser.
- Pour la **V1.0**, le combat montre **une seule grande grille à la fois** (ATTAQUE ou DÉFENSE), sans disposition au choix.
- **Aucune bascule automatique** après les tirs ; un **indicateur intelligent** sur DÉFENSE distingue manqué, touché, coulé et conserve une alerte de dégât tant que la défense n'a pas été consultée.
- Les maquettes historiques du combat peuvent contredire cette disposition ; la décision fonctionnelle de la bible prévaut sur les anciens visuels non normatifs.
- Les éléments de charte graphique, maquettes nouvelles, détails d'affichage des alertes et décomposition complète en lots **ne sont pas encore validés**.

**Consolidation LOT 1 (29 octobre 2026, version 0.7) — décisions issues des arbitrages du PO dans la conversation de cadrage :**
- Résultat après deux tirs ; victoire, défaite ou nul en cas de destruction mutuelle (LOT1-FIN-001).
- REJOUER avec ancien placement du joueur affiché par défaut, AUTO/PRÊT ; Mousse reçoit une nouvelle flotte aléatoire (LOT1-REV-001 à 003).
- Palmarès VS Mousse consultable et compteur global des forfaits (LOT1-STAT-001) ; début du risque de forfait à PRÊT (LOT1-ABD-001).
- IA Mousse : recherche locale, hypothèse d'alignement, conservation des pistes et changement de piste après coulé (LOT1-CPU-002 à 005), sans accès aux secrets.
- Contact libre des navires sans superposition ; annonce du type coulé sans identification de l'épave (LOT1-FLT-001, LOT1-CBT-001).
- Mousse vainqueur propose une révélation facultative de sa flotte ; inspection libre des deux grilles après la bataille (LOT1-FIN-002/003).
- Indicateur DÉFENSE : gravité maximale non consultée, acquittée en ouvrant DÉFENSE (LOT1-UX-004).
- **Découpage délégué à l'assistant PO par le PO :** accueil minimal du LOT 1 uniquement avec accès jouables (LOT1-ACC-001) ; les entrées V1.0 futures ne sont pas abandonnées.
- **Portée :** décisions de besoin et de découpage, pas approbation intégrale de la Bible, pas de backlog, aucune mission graphique/technique lancée. Les maquettes ne sont pas modifiées et leur statut demeure conceptuel.

**Synthèse des échanges PO (28 octobre 2026) :**
- Le **mono-tir simultané** a remplacé le tour par tour comme **mode standard** ; l'alternance traditionnelle est **conservée comme variante** (elle n'a pas été supprimée).
- Le déploiement est devenu **prérempli avec AUTO**, retouche au stylet et validation explicite.
- La « flotte tactique » initiale a été distinguée en **escadre à nombre limité** et **flotte libre**, toutes deux sous plafond de 17 cases.
- CRÉER / REJOINDRE est le choix UX retenu ; l'architecte a rendu un **avis favorable de faisabilité**, mais la **validation sur matériel** et les détails de la découverte restent ouverts.
- Le **VS CPU est validé pour la V1.0 et tous ses modes**, avec trois niveaux de difficulté et des compositions stratégiques adaptées au niveau.
- La **reconnexion après perte radio temporaire** est un objectif V1.0 ; la reprise d'une partie après extinction/redémarrage est **reportée**.
- Le PO veut désormais **détailler les règles et les parcours** plutôt qu'ajouter des modes à la cible V1.0, tout en conservant la trace des bonnes idées.
- **Fin de partie :** le vainqueur décide seul de révéler ou non sa flotte ; les dispositions non découvertes restent secrètes. En cas de destruction réciproque, toutes les cases sont déjà connues.
- **Conservation visuelle :** les trois maquettes d'origine et quatre reconstitutions UX sont désormais archivées dans le dépôt et référencées depuis la bible.

**Aucune modification n'est apportée au dépôt historique Le Grenier Geek.** Ce document est une référence fonctionnelle vivante, destinée à être amendée avec les décisions du PO et le retour de l'architecte. Il n'autorise aucun développement.


## 13. Réserve d'idées — ne rien perdre sans gonfler la V1.0

Cette réserve n'est **ni un backlog ni un engagement de livraison**. Toute idée pourra être réexaminée sans devenir une exigence tacite.

| Piste | État | Commentaire |
| --- | --- | --- |
| **Mode « Amiral » enrichi** avec ordres ou capacités tactiques spéciales | **REPORTÉ** | Évoqué dès l'idée I-009 ; aucun pouvoir défini |
| **Combinaisons libres de paramètres de combat et de flotte** | **PROPOSÉ** | Les deux axes mono-tir/salve et alterné/simultané sont décidés, mais pas un éditeur de parties personnalisées V1.0 |
| **Mécanique de coût de construction distinct du nombre de cases** | **PROPOSÉ / en réserve** | Alternative envisagée au budget de 17 cases ; pas retenue pour les règles actuelles |
| **Statistiques supplémentaires** (par mode de jeu, taux de victoire, série, grades, nom de commandant) | **PROPOSÉ** | Les quatre catégories humain/difficulté CPU et le compteur global de forfaits sont déjà décidés ; seules ces extensions restent ouvertes |
| **Tournois entre plusieurs CYD** | **PROPOSÉ / en réserve** | Idée induite par les rencontres entre plusieurs duos ; non exigée en V1.0 |
| **Sauvegarde/reprise après extinction** | **REPORTÉ** | Envisageable après V1.0 ; complexe du fait de la cohérence des deux états |
| **Améliorations graphiques et audiovisuelles** (radar, explosions, son) | **PROPOSÉ / à vérifier** | Toujours au service de la fluidité et des contraintes CYD |
| **Console multijeux** (Puissance 4, etc.) et boîtier commercial | **REPORTÉ / horizon distinct** | Ne pas mélanger avec le prototype bataille navale |

### Prochain chantier de réflexion produit (sans mission de développement)

Le **LOT 1 est choisi dans son intention** ; il reste à produire sa **fiche de périmètre fonctionnel et ses critères d'acceptation**, sans confondre ce travail PO avec un planning technique ou un backlog. En parallèle, explorer une **charte graphique cohérente**, des parcours et les besoins exprimables en **user stories pour le graphiste** (sans engager ni assigner une mission).

## 14. Périmètre fonctionnel du LOT 1 — Première bataille contre Mousse (consolidation v0.7)

**Statut : DÉCIDÉ pour les éléments explicitement marqués ci-dessous ; cadrage fonctionnel non terminé.** La présente section reprend les décisions du PO validées dans la conversation du 29 octobre 2026. Les maquettes et critères d'acceptation définitifs restent à préparer ; la Bible complète n'est pas validée en bloc.

### 14.1 Objectif et découpage — LOT1-001, LOT1-ACC-001

**DÉCIDÉ — LOT1-001 :** sur **une seule ESP32 CYD classique** (tactile au stylet, écran réel **320 × 240 en paysage**, sans PSRAM présumée), jouer **une bataille classique complète contre Mousse**, de l'accueil jusqu'au résultat. Ce lot **ne dépend pas d'ESP-NOW**.

- **DÉCIDÉ :** grille **10 × 10** ; cinq navires classiques (2, 3, 3, 4 et 5 cases), **17 cases** occupées au total.
- **DÉCIDÉ :** **mono-tir simultané** : une cible par camp, tirs secrets, résolution commune à chaque manche ; **FEU** verrouille irrévocablement le tir du joueur. Les cases déjà attaquées restent visibles et ne peuvent plus être ciblées.
- **DÉCIDÉ (découpage fonctionnel délégué à l'assistant PO) — LOT1-ACC-001 :** accueil volontairement minimal avec les accès fonctionnels **JOUER CONTRE MOUSSE** et **PALMARÈS** ; aucun bouton factice vers des fonctions indisponibles.
- **REPORTÉ à des lots ultérieurs, sans abandon V1.0 :** CRÉER/REJOINDRE et ESP-NOW, difficultés Capitaine/Amiral, familles tactiques, variante classique alternée, grilles 15 × 15 et placement manuel. L'accueil V1.0 s'enrichira lorsque ces expériences seront réellement disponibles.

**PROPOSÉ — parcours utilisateur à détailler :** ACCUEIL → DÉPLOIEMENT → COMBAT → RÉSULTAT → INSPECTION / REJOUER / ACCUEIL. L'enchaînement des grandes étapes est cohérent avec les décisions ci-dessous ; les transitions et textes des écrans ne sont pas validés.

### 14.2 Déploiement et contact des navires — LOT1-PL-001, LOT1-FLT-001

- **DÉCIDÉ — LOT1-PL-001 :** dès l'entrée en déploiement, les cinq navires du joueur sont **intégralement prépositionnés au hasard** sur une grille valide. **AUTO** renouvelle toute la disposition autant de fois que voulu ; **PRÊT** valide la disposition affichée et engage le combat. **AUTO ne vaut jamais PRÊT.** Le placement individuel, la rotation et ✓/✕ sont **hors LOT 1**, conservés pour la V1.0.
- **DÉCIDÉ — LOT1-FLT-001 :** les bâtiments peuvent se toucher **par un côté horizontal, vertical ou en diagonale**. Ils **ne se superposent pas**, restent dans la grille et aucune case d'eau n'est imposée entre eux. Cette règle s'applique **aux flottes du joueur et de Mousse** et doit rester cohérente avec le futur placement manuel.

**Critères d'acceptation proposés à consolider :** AUTO et le déploiement initial respectent toujours la composition imposée, les limites de grille et l'absence de superposition ; PRÊT est nécessaire pour démarrer le combat ; un contact entre deux navires ne rend pas leur placement invalide.

### 14.3 IA de Mousse — LOT1-CPU-001 à 005

**DÉCIDÉ — adversaire débutant crédible, imparfait et sans triche.** Mousse ne reçoit jamais la carte secrète, l'identité secrète d'une case touchée ni la composition d'une épave ; il raisonne sur ses tirs passés et les annonces qu'un adversaire humain recevrait.

- **LOT1-CPU-001 / 002 — Exploration et chasse :** sans piste exploitable, Mousse tire au hasard parmi les cases **jamais attaquées**. Après une touche, il privilégie les cases inconnues voisines. Il peut choisir une mauvaise direction, mais cherche à exploiter sa découverte plutôt que repartir systématiquement ailleurs.
- **LOT1-CPU-003 — Hypothèse d'alignement :** après **deux impacts voisins alignés**, Mousse **suppose** qu'ils appartiennent à un même navire et privilégie l'axe et ses prolongements encore inconnus. Il peut se tromper de sens. **Les navires pouvant se toucher**, cet alignement n'est pas une preuve d'appartenance commune et ne doit pas être traité comme une connaissance cachée.
- **LOT1-CPU-004 — Pistes multiples :** Mousse mémorise toutes les découvertes et privilégie **une piste exploitable à la fois** ; il peut reprendre une autre piste déjà identifiée sans perdre les impacts précédents.
- **LOT1-CPU-005 — Après un coulé :** la piste actuelle est considérée comme **probablement terminée** ; Mousse privilégie une autre piste disponible, tout en conservant les impacts antérieurs et la possibilité d'y revenir. Il ne connaît pas les cases exactes de l'épave.
- **DÉCIDÉ :** aucune case déjà attaquée ne peut être rejouée par Mousse ; ses mauvaises décisions découlent des **hypothèses** et choix de direction, pas d'une consultation des positions secrètes.

**À ARBITRER / préciser sans prescrire d'algorithme :** sélection entre plusieurs voisins, comportement lorsque plusieurs pistes se recoupent, absence de piste restante et cas limites, mesure observable de la difficulté « débutant ». Les principes ci-dessus ne constituent pas une implémentation technique.

**Critères d'acceptation proposés à consolider :** préférence observable pour une zone touchée puis son axe possible ; capacité à retrouver une ancienne piste ; absence de tir répété ou de déduction utilisant une information secrète.

### 14.4 Combat, naufrages et résultat — LOT1-CBT-001, LOT1-FIN-001

- **DÉCIDÉ — FEU / ciblage :** le joueur peut modifier sa cible **avant FEU** ; après FEU, elle est **définitivement verrouillée**. Les tirs du joueur et de Mousse sont résolus pour la même manche, sans avantage de priorité. Un historique affiche raté/touché ; aucune case déjà attaquée ne redevient sélectionnable.
- **DÉCIDÉ — LOT1-CBT-001 :** quand un navire coule, le **type du navire** est annoncé (« Croiseur coulé ! » par exemple), mais les cases appartenant précisément à son épave **ne sont pas identifiées visuellement parmi les touches**. Cela préserve l'intérêt tactique des navires collés. Le joueur **et Mousse** reçoivent cette même information limitée.
- **DÉCIDÉ — LOT1-FIN-001 :** le résultat est évalué **après résolution des deux tirs de la manche**, même si l'un des tirs achève la flotte adverse :
  - Tous les navires de Mousse coulés, flotte humaine encore à flot : **VICTOIRE**.
  - Tous les navires humains coulés, flotte de Mousse encore à flot : **DÉFAITE**.
  - Deux dernières flottes détruites au cours de cette même manche : **MATCH NUL**.
  - Les deux flottes ont encore un navire à flot : **manche suivante**.
  - Après un résultat définitif, **aucun nouveau tir** ; un résultat ne compte qu'**une seule partie**.

**À ARBITRER :** séquence de choix du CPU et mise en scène/rythme de résolution, présentation d'une annonce « coulé », animations/retours ; aucun de ces détails ne modifie la simultanéité.

### 14.5 Présentation du combat et indicateur — LOT1-UX-001 à 004

- **DÉCIDÉ — LOT1-UX-001 :** une seule **grande grille** affichée à la fois ; **ATTAQUE** pour cibler et consulter l'historique offensif, **DÉFENSE** pour voir sa flotte et les dégâts. Bascule immédiate, volontaire, sans menu intermédiaire.
- **DÉCIDÉ — LOT1-UX-002 :** **aucune bascule automatique** après résolution ; le résultat du joueur apparaît sur ATTAQUE, les tirs de Mousse sont signalés par l'indicateur DÉFENSE. Le joueur reste libre de consulter ses dégâts plus tard.
- **DÉCIDÉ — LOT1-UX-003/004 :** gravité croissante **manqué (discret) < touché (visible) < coulé (alerte renforcée)**. En cas de plusieurs événements non consultés, **l'indicateur conserve la gravité maximale**, même après un tir manqué. **L'ouverture de DÉFENSE acquitte** l'alerte accumulée. Pas de compteur d'événements dans le LOT 1.

**Critère d'acceptation proposé :** touché → coulé → manqué sans ouverture de DÉFENSE laisse l'alerte « coulé » active jusqu'à cette ouverture ; aucun écran DÉFENSE ne s'ouvre sans action du joueur.

**À VÉRIFIER sur CYD réelle :** précision des cases et boutons au stylet, lisibilité des impacts et niveaux d'alerte, emplacement des commandes et confort de bascule. **À CONCEVOIR :** codes couleurs, pictogrammes, libellés, animations et charte graphique définitifs. Les anciens SVG sont des intentions non normatives.

### 14.6 Fin de partie, confidentialité et inspection — LOT1-FIN-002/003

- **DÉCIDÉ :** résultat clairement annoncé, tir et impacts déjà révélés conservés ; aucun tir après le résultat.
- **DÉCIDÉ — LOT1-FIN-003 :** après **victoire, défaite ou nul**, le joueur peut basculer librement entre **ATTAQUE** et **DÉFENSE** et inspecter leurs historiques sans les modifier. **REJOUER** et **ACCUEIL** restent accessibles.
- **DÉCIDÉ — FIN-001 conservée :** la flotte complète du **vainqueur** reste **secrète par défaut** ; sa révélation est facultative, jamais automatique. Le joueur humain victorieux conserve le choix prévu par FIN-001, dont la mise en scène solo reste à préciser.
- **DÉCIDÉ — LOT1-FIN-002, exception pédagogique VS Mousse :** lorsque **Mousse gagne**, il **propose** au joueur une commande de type **VOIR SA FLOTTE**. Le joueur choisit de révéler ou non la disposition complète de Mousse, navires non découverts inclus. Les tirs précédents restent visibles. Cela ne change pas le résultat ni le palmarès.
- **DÉCIDÉ / cohérence :** en cas d'égalité par destruction réciproque, les deux flottes sont déjà intégralement touchées ; un bouton spécial de révélation n'est **pas imposé** dans le LOT 1. Son affichage éventuel reste **PROPOSÉ** dans la Bible.

**À ARBITRER :** mise en scène de la fin, accès à la révélation, retour visuel à une grille déjà inspectée et options pertinentes quand le joueur gagne.

### 14.7 Rejouer et déploiements successifs — LOT1-REV-001 à 003

- **DÉCIDÉ — LOT1-REV-001 :** après chaque résultat, **REJOUER** lance une nouvelle bataille contre **Mousse**, en **classique mono-tir simultané**, sans repasser par la sélection des adversaires ; retour obligatoire au déploiement.
- **DÉCIDÉ — LOT1-REV-002 :** lors d'une revanche, le déploiement affiche **par défaut la disposition précédente du joueur**. **PRÊT** la conserve ; **AUTO** permet de générer une autre disposition autant de fois que voulu. Aucun placement manuel dans le LOT 1.
- **DÉCIDÉ — LOT1-REV-003 :** pour **chaque nouvelle partie et chaque revanche**, Mousse obtient une **nouvelle génération aléatoire de flotte valide**. Il ne conserve pas volontairement son ancien placement ; une coïncidence aléatoire est possible.
- **DÉCIDÉ :** nouveaux historiques de tirs vierges et tous les navires intacts pour les deux camps ; le palmarès de la partie précédente demeure conservé.

**Critère d'acceptation proposé :** après n'importe quel résultat, REJOUER revient à un déploiement avec l'ancienne formation du joueur, AUTO/PRÊT, et un adversaire préparant un nouvel agencement secret.

### 14.8 Palmarès, abandon et seuil d'engagement — LOT1-STAT-001, LOT1-ABD-001

- **DÉCIDÉ — LOT1-STAT-001 :** depuis l'accueil, **PALMARÈS** permet de voir, sans consultation obligatoire, le bilan **VS MOUSSE** : **parties jouées, victoires, défaites, matchs nuls**, et le **compteur global unique de forfaits volontaires**. Ces compteurs sont **persistants sur la CYD**, pas sur un compte personnel.
- **DÉCIDÉ :** une victoire, défaite ou égalité définitive incrémente **exactement une** partie dans la catégorie VS MOUSSE et **exactement une** catégorie de résultat ; les forfaits ne sont pas des parties jouées. Les quatre catégories V1.0 **VS HUMAIN / MOUSSE / CAPITAINE / AMIRAL** restent décidées, mais seul le combat VS MOUSSE est disponible dans ce lot.
- **DÉCIDÉ — LOT1-ABD-001 :** **PRÊT est le seuil d'engagement.** Avant PRÊT, quitter le déploiement ne comptabilise **ni forfait ni partie**. Après PRÊT et avant le résultat définitif, une demande d'abandon affiche **CONTINUER / ABANDONNER**. CONTINUER préserve l'état et les compteurs ; ABANDONNER confirmé ajoute **1** au **compteur global de forfaits**, sans inscrire la bataille au palmarès VS MOUSSE. Après résultat définitif, retour à l'accueil sans forfait supplémentaire.
- **DÉCIDÉ :** un incident technique, une extinction ou une coupure imprévue ne valent pas automatiquement abandon volontaire (BN-STAT-003). La reprise après redémarrage reste **REPORTÉE** dans le cadre V1.0.

**Critères d'acceptation proposés :** abandon confirmé en cours de bataille = forfait +1 uniquement ; quitter au déploiement ou après résultat = +0 ; annuler la confirmation = aucun changement ; résultat final = partie et bilan incrémentés une fois et persistants.

### 14.9 Exclusions, points à instruire et prochaine discussion

**REPORTÉ hors LOT 1 :** multijoueur ESP-NOW et ses salons/connexions, autres niveaux CPU, variante alternée, modes et compositions tactiques, grille 15 × 15, placement manuel, interfaces avancées et éventuelle console commerciale. **Ces décisions V1.0 ne sont pas annulées.**

**À ARBITRER :** parcours tactile et visuel **d'une manche complète contre Mousse**, y compris sélection, verrouillage FEU, résolution simultanée, information des deux impacts et retour à la manche suivante ; transitions entre les écrans ; cas particuliers restants de l'IA ; détails des vues de résultat et palmarès.

**À VÉRIFIER :** ergonomie réelle sur CYD 320 × 240 et comportement observable en parties de test. Les ajustements visuels issus des tests ne doivent pas devenir silencieusement de nouvelles règles.

**PROPOSÉ — prochaine étape de cadrage :** définir précisément le déroulement joueur d'une manche, avec **une question produit importante à la fois**, puis consolider les critères d'acceptation. **Aucune mission technique/graphique, aucun backlog ni planning n'est lancé par ce document.**
