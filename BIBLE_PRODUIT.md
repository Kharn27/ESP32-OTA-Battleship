# I-009 — Bible produit : CYD WARSHIP (bataille navale ESP32 CYD)

**Version documentaire :** 0.17 — consolidation des arbitrages pré-backlog du LOT 1 (navigation directe VS Capitaine, CAP-TEST-001, compléments CAP-CPU-003/004/008, LOT1-UX-011) ; décisions et errata antérieurs conservés, 30 octobre 2026  
**Maturité :** expression de besoin en construction ; **ce document entier n'est pas encore validé**  
**Projet :** [ESP32-OTA-Battleship](https://github.com/Kharn27/ESP32-OTA-Battleship)  
**Origine :** [I-009 / IDEE.md — Le Grenier Geek](https://github.com/Kharn27/Le-Grenier-Geek/blob/main/plans/I-009-bataille-navale-cyd/IDEE.md)

> Gouvernance : **DÉCIDÉ** = approuvé par le PO ; **PROPOSÉ** = suggestion non validée ; **À ARBITRER** = règle encore ouverte ; **À VÉRIFIER** = faisabilité, mesure ou essai nécessaire ; **REPORTÉ** = hors du besoin immédiat. Une cible V1.0 ne signifie pas automatiquement « MVP fixé ».

## 1. Promesse et priorité de conception

Créer **CYD WARSHIP**, un jeu de bataille navale familial, tactile et tactique sur ESP32 CYD classique. Le **LOT 1** commence contre **Capitaine** sur une seule CYD ; la cible multijoueur humain utilise **deux ESP32 CYD**, un par joueur, avec une flotte secrète par console. **Intention artistique décidée par le PO :** jeu indépendant contemporain en pixel art riche, expressif et immersif, avec priorité à la lisibilité sur écran natif 320 × 240. **Charte visuelle définitive non validée.**

**DÉCIDÉ — principes d'expérience :**
- Le **gameplay passe avant le spectacle** : fluide, compréhensible sans se poser de questions, avec des retours immédiats.
- Associer une stratégie familiale facile d'accès à une ambiance maritime vivante et expressive, sans imposer l'austérité d'un terminal militaire ou d'un tableau de bord technique. Les choix précis de palette, sprites et composition restent à arbitrer.
- Les animations, le sonar et les éventuels sons doivent renforcer le suspense **sans ralentir** les actions.
- Le jeu doit rester agréable à deux enfants comme à des joueurs qui souhaitent approfondir la stratégie.

**PROPOSÉ :** retour visuel immédiat à la sélection, à la validation et à la révélation ; animations courtes et discrètes plutôt que séquences imposées.

**Références et explorations graphiques :** [index des maquettes et recherches artistiques](MAQUETTES.md), comprenant sept SVG UX historiques et six explorations A–F non retenues, retirés de `main` sur décision PO du 29 octobre 2026 et [conservés dans l'historique Git](https://github.com/Kharn27/ESP32-OTA-Battleship/tree/a768e6cabbdfba1b793f3c77c0d75f0dae001c54/assets) (voir aussi la [PR #1](https://github.com/Kharn27/ESP32-OTA-Battleship/pull/1)). Les deux scènes narratives de la [PR #2](https://github.com/Kharn27/ESP32-OTA-Battleship/pull/2) ont été retirées de l'arborescence courante par décision PO, mais leurs [originaux, exports et provenance restent consultables dans l'historique Git](https://github.com/Kharn27/ESP32-OTA-Battleship/tree/872496d73a150dd5a016c219973e5865576b7e04/assets/explorations/2026-10-29-bataille-en-cours). **Aucune charte ni interface visuelle de production n'a été approuvée par le PO.** Les images ne sont ni des captures de firmware ni des représentations normatives des informations visibles en jeu.

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

![Maquette du mode classique historique](https://raw.githubusercontent.com/Kharn27/ESP32-OTA-Battleship/a768e6cabbdfba1b793f3c77c0d75f0dae001c54/assets/maquettes/mode-classique-320x240.svg)

![Maquette du duel simultané](https://raw.githubusercontent.com/Kharn27/ESP32-OTA-Battleship/a768e6cabbdfba1b793f3c77c0d75f0dae001c54/assets/maquettes/mode-simultane-320x240.svg)

![Maquette des salves historique](https://raw.githubusercontent.com/Kharn27/ESP32-OTA-Battleship/a768e6cabbdfba1b793f3c77c0d75f0dae001c54/assets/maquettes/mode-salves-320x240.svg)

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

![Esquisse de placement tactile, état invalide](https://raw.githubusercontent.com/Kharn27/ESP32-OTA-Battleship/a768e6cabbdfba1b793f3c77c0d75f0dae001c54/assets/maquettes/placement-flotte-320x240.svg)

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

**DÉCIDÉ pour le LOT 1 (§ 14.10) :** une courte animation précède la révélation **visuellement simultanée** du résultat sur ATTAQUE et de la riposte dans DÉFENSE, sans minuterie imposée entre les manches. **À ARBITRER :** détails graphiques et durées de l'animation ; modalités des variantes autres que le classique mono-tir simultané ; comportement de l'écran quand la liaison est interrompue après FEU.

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

![Esquisse de fin de partie : révélation facultative de la flotte](https://raw.githubusercontent.com/Kharn27/ESP32-OTA-Battleship/a768e6cabbdfba1b793f3c77c0d75f0dae001c54/assets/maquettes/fin-partie-secret-320x240.svg)

**DÉCIDÉ — FIN-001 :** résultat connu des deux joueurs ; les tirs et impacts déjà révélés restent visibles. **La disposition complète de la flotte victorieuse demeure secrète par défaut**, et **le vainqueur choisit s'il souhaite la révéler** à son adversaire. Refuser ou ne pas choisir ne déclenche aucune révélation.

**Cas de l'égalité par destruction réciproque :** les deux flottes sont alors entièrement touchées et leurs positions déjà connues : un bouton de « révélation » symétrique reste envisageable **par analogie**, mais ne fournirait aucun nouveau renseignement. La présentation précise de cette option est **PROPOSÉE** et non encore fixée.

**DÉCIDÉ pour le LOT 1 VS Capitaine (cf. § 14) :** résolution des deux tirs avant le résultat ; victoire, défaite et match nul par destruction réciproque ; inspection libre après bataille. **Capitaine vainqueur autorise toujours** le perdant à révéler facultativement sa flotte, sans révélation automatique.  **À ARBITRER :** conditions des autres variantes et détails graphiques des écrans de résultat. L'**abandon volontaire** est traité sans résultat comptabilisé (§ 7).

## 7. Identité et palmarès de chaque CYD

![Esquisse de la fiche adverse consultable à la demande](https://raw.githubusercontent.com/Kharn27/ESP32-OTA-Battleship/a768e6cabbdfba1b793f3c77c0d75f0dae001c54/assets/maquettes/fiche-adversaire-320x240.svg)

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

![Esquisse de l'écran d'accueil avec création, rejoindre et VS CPU](https://raw.githubusercontent.com/Kharn27/ESP32-OTA-Battleship/a768e6cabbdfba1b793f3c77c0d75f0dae001c54/assets/maquettes/accueil-creer-rejoindre-320x240.svg)

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

**DÉCIDÉ — démarche de cadrage CPU (29/10/2026, v0.11) :** Capitaine sert d'**adversaire de référence fonctionnelle** pour étalonner ensuite les difficultés. Ses dix principes de comportement déjà arbitrés sont consignés au **§ 16**. **DÉCIDÉ ensuite en v0.12 : le LOT 1 se joue contre Capitaine ; Mousse et Amiral sont REPORTÉS.** Ses dix principes servent maintenant au premier lot.

**DÉCIDÉ — équité des CPU :** pas d'accès aux placements secrets, aux identités cachées des touches ni aux cases exactes des épaves ; les décisions reposent uniquement sur les tirs, résultats, annonces et règles accessibles à un humain.

**PROPOSÉ / À ARBITRER — différenciation non finalisée :**
- **Mousse** : débutant, exploration plutôt instinctive, maladresses crédibles ; ses décisions historiques LOT1-CPU-001 à 006 sont conservées pour un lot futur (§ 17).
- **Capitaine** : joueur compétent mais imparfait, CPU du LOT 1, dix principes déjà cadrés au § 16 ; équilibrage réel restant à vérifier.
- **Amiral** : exploitation plus poussée des indices et des placements possibles ; comportement précis non décidé.
- Les compositions de flottes tactiques selon les niveaux restent à définir dans les lots concernés.

**À ARBITRER :**
- Logique de décision exacte, différenciation observable des trois niveaux et équilibre des flottes tactiques.
- Présentation des adversaires, éventuels délais d'animation, tutoriel ou aide.
- La présentation et la vérification de la difficulté réelle des trois CPU ; le **palmarès séparé VS humain et par difficulté CPU est décidé** (§ 7).
- Conditions de victoire et comportement d'un CPU dans les différentes combinaisons de combat.

**Attention :** le solo facilite l'étude et les essais du gameplay, mais **ne remplace pas** les vérifications propres au multijoueur ESP-NOW.

## 10. Expérience à finaliser / questions transverses

- **Fin de manche et de partie :** règles de victoire/défaite/égalité décidées pour le classique mono-tir simultané VS Capitaine (§ 14) ; les autres variantes, les détails d'affichage et les transitions restent à préciser. Le secret du vainqueur (FIN-001) et la revanche pilotée par l'hôte en VS humain sont conservés.
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
| LOT1-001 | Bataille classique complète VS Capitaine sur une CYD ; remplace VS Mousse (v0.12) | **DÉCIDÉ (PO, v0.12)** |
| LOT1-PL-001 | Déploiement par AUTO répétable puis PRÊT ; pas de retouche manuelle dans le LOT 1 | **DÉCIDÉ (lot 1)** |
| LOT1-CPU-001 | Règle Mousse, ancien identifiant LOT 1 conservé au § 17 | **DÉCIDÉ historiquement / REPORTÉ** |
| LOT1-UX-001 | Une seule grande grille visible à la fois, bascule immédiate ATTAQUE/DÉFENSE | **DÉCIDÉ (V1.0)** |
| LOT1-UX-002 | Après FEU, aucune bascule automatique ; résultat joueur sur ATTAQUE, défense consultable à volonté | **DÉCIDÉ (V1.0)** |
| LOT1-UX-003 | Indicateur DÉFENSE : raté discret, touché visible, coulé renforcé, alerte persistante jusqu'à consultation | **DÉCIDÉ (principe V1.0)** |
| LOT1-FIN-001 | Résultat après résolution des deux tirs ; destruction mutuelle = match nul | **DÉCIDÉ** |
| LOT1-FIN-002 | Capitaine vainqueur autorise toujours la révélation facultative sur ATTAQUE ; affichage/masquage réversible, tirs historiques conservés | **DÉCIDÉ (PO, v0.13)** |
| LOT1-FIN-003 | Inspection libre ATTAQUE/DÉFENSE après résultat, sans nouveaux tirs | **DÉCIDÉ** |
| LOT1-FIN-005 | Le vainqueur humain choisit ; Mousse (lot futur) et Capitaine (LOT 1) autorisent toujours leur flotte après une victoire | **DÉCIDÉ (PO, v0.13)** |
| LOT1-REV-001 | REJOUER contre Capitaine dans le LOT 1 | **DÉCIDÉ (PO, v0.12)** |
| LOT1-REV-002 | Revanche : ancien placement du joueur affiché par défaut, AUTO ou PRÊT | **DÉCIDÉ** |
| LOT1-REV-003 | Capitaine reçoit une nouvelle flotte aléatoire valide à chaque nouvelle partie et revanche, sans reconduction volontaire de la disposition précédente | **DÉCIDÉ (PO, v0.14)** |
| LOT1-STAT-001 | Palmarès VS Capitaine et compteur global dès le LOT 1 | **DÉCIDÉ (PO, v0.12)** |
| LOT1-CPU-002 | Règle Mousse, ancien identifiant LOT 1 conservé au § 17 | **DÉCIDÉ historiquement / REPORTÉ** |
| LOT1-CPU-003 | Règle Mousse, ancien identifiant LOT 1 conservé au § 17 | **DÉCIDÉ historiquement / REPORTÉ** |
| LOT1-CPU-004 | Règle Mousse, ancien identifiant LOT 1 conservé au § 17 | **DÉCIDÉ historiquement / REPORTÉ** |
| LOT1-CPU-005 | Règle Mousse, ancien identifiant LOT 1 conservé au § 17 | **DÉCIDÉ historiquement / REPORTÉ** |
| LOT1-CPU-006 | Règle Mousse, ancien identifiant LOT 1 conservé au § 17 | **DÉCIDÉ historiquement / REPORTÉ** |
| CAP-CPU-001 à 010 | Capitaine : dix principes décidés au § 16, désormais CPU du LOT 1 | **DÉCIDÉ (PO, v0.12)** |
| CAP-TEST-001 | Capitaine évalué par ses comportements tactiques observables et des essais de difficulté ressentie, sans taux de victoire imposé | **DÉCIDÉ (PO, v0.17)** |
| CAP-CPU-003/004/008 (précisions) | Après une touche : prochaine cible orthogonale inconnue plausible ; après deux touches voisines alignées : priorité aux prolongements plausibles, sans certitude d'un même navire | **DÉCIDÉ (PO, v0.17)** |
| LOT1-FLT-001 | Navires au contact latéral ou diagonal autorisés, sans chevauchement | **DÉCIDÉ** |
| LOT1-CBT-001 | Type du navire coulé annoncé, épave non identifiée case par case | **DÉCIDÉ** |
| LOT1-ABD-001 | PRÊT ouvre la période d'abandon comptant comme forfait volontaire | **DÉCIDÉ** |
| LOT1-UX-004 | Indicateur DÉFENSE par gravité maximale non consultée, acquis à l'ouverture | **DÉCIDÉ** |
| LOT1-UX-005 | Enchaînement libre des manches sans timer ni CONTINUER ; nouvelle cible immédiatement disponible après révélation | **DÉCIDÉ (PO)** |
| LOT1-UX-006 | Animation de tir brève après FEU ; préparation de la cible suivante ignorée pendant l'animation, non mémorisée | **DÉCIDÉ (PO) / durée À VÉRIFIER** |
| LOT1-UX-007 | Résultat sur ATTAQUE et riposte dans DÉFENSE révélés visuellement au même instant | **DÉCIDÉ (PO)** |
| LOT1-UX-008 | DÉFENSE : coordonnées de la seule dernière riposte ; gravité maximale non consultée conservée ; résumé et alerte acquittés à l'ouverture | **DÉCIDÉ (PO)** |
| LOT1-UX-009 | Sélection de cible conservée lors d'un aller-retour volontaire ATTAQUE/DÉFENSE avant FEU | **DÉCIDÉ (PO)** |
| LOT1-UX-010 | Navigation ATTAQUE/DÉFENSE verrouillée pendant l'animation après FEU, demandes ignorées sans mise en attente ; réactivée après révélation simultanée | **DÉCIDÉ (PO, v0.15)** |
| LOT1-UX-011 | QUITTER/ABANDONNER également indisponibles pendant l'animation après FEU ; appuis ignorés, réactivation à la révélation | **DÉCIDÉ (PO, v0.17)** |
| LOT1-FIN-004 | Verdict sur le champ de bataille puis format compact, sans écran intermédiaire obligatoire | **DÉCIDÉ (PO) / rendu À VÉRIFIER** |
| V1-UX-010 | Futures salves : commande DÉFENSE de taille fixe ; liste des touches de la dernière salve, naufrage près de sa coordonnée et débordement « +N impacts » | **DÉCIDÉ (PO) / hors LOT 1** |
| LOT1-ACC-001 | Accueil : exactement JOUER et PALMARÈS, sans accès factice | **DÉCIDÉ (PO, après v0.15)** |
| LOT1-ACC-008 | Ancien arbitrage : écran de sélection dès le LOT 1 avec Capitaine seul jouable | **DÉCIDÉ historiquement (v0.16) / REMPLACÉ par LOT1-ACC-009** |
| LOT1-ACC-009 | LOT 1 : JOUER mène directement au déploiement VS Capitaine ; sélection d'adversaire reportée ; RETOUR éventuel du déploiement vers l'accueil sans forfait avant PRÊT | **DÉCIDÉ (PO, v0.17)** |
| LOT1-ACC-002 | Titre officiel affiché : CYD WARSHIP ; nom du dépôt GitHub inchangé | **DÉCIDÉ (PO)** |
| LOT1-ACC-003 | Accueil V1.0 fondé sur une scène maritime **fixe**, sans animation obligatoire | **DÉCIDÉ (PO)** |
| LOT1-ACC-004 | Titre en haut : identifiable, lisible mais discret, sans masquer la scène | **DÉCIDÉ (intention PO) / implantation À CONCEVOIR** |
| LOT1-ACC-005 | Commandes **superposées à l'illustration**, préférentiellement dans la zone calme en bas à gauche | **DÉCIDÉ (principe PO) / position exacte À VÉRIFIER** |
| LOT1-ACC-006 | Conserver coucher de soleil, navire principal et ennemi distant ; représenter une **riposte adverse manquée** par une gerbe d'eau proche du navire principal | **DÉCIDÉ (intention de scène PO) / rendu À CONCEVOIR** |
| LOT1-ACC-007 | Avion en feu en chute près du navire et escadrille en silhouettes devant le soleil | **PROPOSÉ par le PO / À VÉRIFIER en 320 × 240** |

## 12. Traçabilité et sources

**Visuels historiques :** [MAQUETTES.md](MAQUETTES.md) : 3 SVG historiques recopiés à l'identique depuis le Grenier Geek et 4 esquisses UX reconstituées, [consultables dans l'historique Git](https://github.com/Kharn27/ESP32-OTA-Battleship/tree/a768e6cabbdfba1b793f3c77c0d75f0dae001c54/assets/maquettes) après retrait de `main`. Les images d'ambiance externes et anciens composants interactifs du chat ne sont pas archivés à l'identique.

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
- **Conservation visuelle :** les trois maquettes d'origine et quatre reconstitutions UX sont retrouvables dans l'historique Git du dépôt et référencées depuis la bible, sans être maintenues sur `main`.

**Aucune modification n'est apportée au dépôt historique Le Grenier Geek.** Ce document est une référence fonctionnelle vivante, destinée à être amendée avec les décisions du PO et le retour de l'architecte. Il n'autorise aucun développement.


**Consolidation du parcours de combat LOT 1 (29 octobre 2026, version 0.9) — arbitrages PO du présent échange :**
- **Rythme du combat :** aucune temporisation imposée entre les manches, aucun CONTINUER ; animation de tir brève, nouvelles sélections ignorées (non mises en file) pendant l'animation ; révélation visuelle simultanée du tir joueur et de la riposte (LOT1-UX-005 à 007).
- **Navigation :** une cible sélectionnée avant FEU est conservée après un aller-retour volontaire ATTAQUE/DÉFENSE (LOT1-UX-009).
- **DÉFENSE :** résumé intégré des seules coordonnées de la dernière riposte, avec naufrage nommé le cas échéant ; pas d'accumulation des coordonnées ; alerte maximale des événements non consultés conservée jusqu'à ouverture, qui acquitte aussi le résumé (LOT1-UX-008).
- **Fin de combat :** verdict intégré au champ de bataille après les deux tirs, mis en avant puis compact ; inspection des deux grilles sans tir, REJOUER/ACCUEIL accessibles (LOT1-FIN-004).
- **Salves futures :** commande DÉFENSE à taille fixe, pas de défilement, annonce du naufrage rattachée à sa coordonnée, résumé **« +N impacts »** pour les touches non affichables (V1-UX-010) ; pas de réalisation des salves dans le LOT 1.
- **Question conservée ouverte :** association de « coulé » à une coordonnée lorsque plusieurs nouveaux tirs de la même salve achèvent ensemble un navire ; gestion graphique de l'espace limitée ; pas de règle d'ordre de résolution inventée.
- **Maturité :** décisions fonctionnelles validées, mais effets, durées et implantation révisables après essais matériels sous arbitrage explicite du PO. Aucun changement de charte, aucune maquette approuvée, aucun travail technique lancé.
- **Traçabilité :** v0.7 (règles LOT 1), v0.8 (nom CYD WARSHIP et accueil) et décisions antérieures intégralement conservées ; le fichier [MAQUETTES.md](MAQUETTES.md) et les assets restent inchangés.

**Consolidation de la révélation pédagogique LOT 1 (29 octobre 2026, version 0.10) — arbitrages PO :**
- **Permission du vainqueur :** VOIR SA FLOTTE n'est possible qu'après une autorisation MONTRER MA FLOTTE ; en VS humain, cette autorisation dépend du choix du vainqueur, et son parcours multijoueur est hors LOT 1.
- **Mousse vainqueur :** autorisation toujours accordée, proposition discrète non modale après la défaite ; révélation uniquement sur demande du perdant, sur ATTAQUE, sans nouvel écran.
- **Inspection :** tirs et marques conservés, navires révélés affichables/masquables à volonté, état mémorisé lors des bascules ATTAQUE/DÉFENSE ; observation visuelle seule, sans fiche ni aide analytique.
- **Cas sans bénéficiaire :** aucun MONTRER MA FLOTTE après une victoire humaine VS Mousse ; pas de commande spéciale en cas de nul. Révélation sans effet statistique.
- **Prochaine discussion :** cas particuliers de l'IA Mousse lorsque des navires se touchent ou que plusieurs pistes semblent alignées ; les règles existantes restent inchangées.
- **Portée :** complément strictement fonctionnel aux décisions v0.7 à v0.9 ; aucune maquette ni travail technique créé ; [MAQUETTES.md](MAQUETTES.md) inchangé.

**Consolidation de l'étalon CPU Capitaine (29 octobre 2026, version 0.11) — arbitrages PO dans la conversation de cadrage :**
- **DÉCIDÉ — méthode :** définir un joueur de référence **Capitaine** pour apprécier ensuite les comportements de Mousse et d'Amiral, sans transformer cette référence en développement du LOT 1.
- **DÉCIDÉ — CAP-CPU-001 à 010 (§ 16) :** recherche initiale organisée mais souple, exploration orthogonale soutenue après touche, révision des hypothèses contradictoires, gestion prioritaire d'une piste à la fois, mémoire après naufrage, exploitation des touches inexpliquées, persévérance raisonnée, prise en compte simple des tailles restantes, sans optimisation probabiliste systématique.
- **DÉCIDÉ — LOT1-CPU-006 (§ 14.3) :** après deux touches alignées et un prolongement manqué, Mousse privilégie l'autre extrémité encore inconnue.
- **NON DÉCIDÉ :** comportement de Mousse après échec des **deux** prolongements d'une fausse piste ; deux alternatives ont été proposées sans choix du PO. Ne pas transférer automatiquement les déductions de Capitaine à Mousse.
- **REPORTÉ :** implémentation des difficultés Capitaine et Amiral, et approfondissement de leurs spécifications, à des lots ultérieurs. LOT 1 inchangé : **VS Mousse seul**, une CYD, classique mono-tir simultané.
- **Prochaine étape demandée par le PO :** inventaire des manques fonctionnels et critères d'acceptation nécessaires **avant création d'un backlog**. La présente modification documentaire ne crée ni backlog, ni mission, ni planification, ni validation globale de la Bible.

**Changement explicite du LOT 1 — v0.12, décision du PO (29 octobre 2026) :**
- **DÉCIDÉ :** Capitaine remplace Mousse comme adversaire du LOT 1. **Mousse et Amiral sont REPORTÉS** à des lots ultérieurs, sans annulation des principes déjà validés.
- **Conservé :** partie classique 10 × 10 / cinq navires / 17 cases, mono-tir simultané, parcours combat, revanche, palmarès et règles UX ; les dix principes Capitaine du § 16 sont ceux du premier lot.
- **Conséquences historiques de la v0.12 :** accueil alors libellé **JOUER CONTRE CAPITAINE** (remplacé ultérieurement par **JOUER**, décision LOT1-ACC-001/008) ; palmarès actif **VS CAPITAINE**, combats et revanche contre Capitaine.
- **Conservé pour lot futur :** Mousse et ses décisions historiques LOT1-CPU-001 à 006 (§ 17). Le préfixe ancien n'implique plus une réalisation dans le LOT 1.
- **Historique des questions ouvertes en v0.12, désormais résolues :** l'autorisation systématique de révélation a été décidée en v0.13 et le renouvellement aléatoire de la flotte de Capitaine à chaque partie/revanche en v0.14.
- **Impact documentaire externe :** l'issue graphique #6 et les maquettes anciennes peuvent mentionner Mousse ; leur statut historique est conservé. Le Pilotage devra actualiser le brief d'accueil avant réalisation/validation, sans mission silencieuse.
- **Aucune issue, nouvelle mission, implémentation ou planification lancée par cette correction.**

**Arbitrage LOT1-FIN-002/005 — DÉCIDÉ par le PO le 29 octobre 2026 (v0.13) :**
- **Capitaine vainqueur autorise systématiquement la révélation de sa flotte**, comme Mousse (reporté) ; le joueur perdant reste libre de ne pas l'afficher.
- Après le verdict DÉFAITE, **VOIR SA FLOTTE** est proposé discrètement, sans dialogue ni affichage obligatoire ; les cinq navires sont affichables et masquables à volonté sur ATTAQUE, avec marques des tirs conservées et état mémorisé après navigation vers DÉFENSE.
- Aucune fiche, analyse automatique ou modification de résultat et statistiques. Aucun MONTRER MA FLOTTE après victoire du joueur contre CPU et aucune commande de révélation nécessaire en cas de match nul.
- L'autorisation humaine en multijoueur reste volontaire ; la nouvelle décision ne la remplace pas.
- **Question encore ouverte lors de la v0.13, désormais résolue en v0.14 :** renouvellement de la flotte de Capitaine à chaque partie et revanche (voir ci-dessous).

**Arbitrage LOT1-REV-003 — DÉCIDÉ par le PO le 29 octobre 2026 (v0.14) :**
- Pour **chaque nouvelle partie et chaque revanche** contre Capitaine, sa flotte classique de cinq navires est **aléatoirement repositionnée** dans une disposition secrète et valide.
- Capitaine **ne choisit pas de conserver volontairement sa formation précédente**. Une égalité fortuite entre deux dispositions reste possible, sans être assimilée à une réutilisation délibérée.
- En revanche, **le joueur humain conserve par défaut sa propre formation précédente** et peut choisir AUTO ou PRÊT, comme déjà décidé (LOT1-REV-002). Aucun changement des règles de contact, du mode de combat ou du palmarès.
- Les arbitrages spécifiques Mousse restent conservés pour un lot ultérieur ; la présente décision n'ajoute pas Mousse ni Amiral au LOT 1.
- **Aucun backlog, nouvelle mission, développement ou planning lancé.**

**Arbitrage LOT1-UX-010 — DÉCIDÉ par le PO le 29 octobre 2026 (v0.15) :**
- Après **FEU**, pendant la courte animation de tir, **ATTAQUE/DÉFENSE est verrouillée**. Le joueur reste sur ATTAQUE jusqu'à la révélation synchronisée des deux attaques.
- Un appui sur DÉFENSE durant l'animation est ignoré, jamais mémorisé et ne vaut ni consultation ni acquittement de l'alerte DÉFENSE.
- Navigation de nouveau libre immédiatement après la révélation, sans bascule automatique ni action différée.
- Cette règle précise LOT1-UX-006/007/008/009 ; la durée et le confort tactile restent à vérifier sur matériel réel.
- Aucun backlog, mission, développement ou planning engagé par cette mise à jour documentaire.

**Complément documentaire à la v0.15 — DOC-ACC-001 / issue #9 (décisions PO du 29 octobre 2026) :**
- **DÉCIDÉ — LOT1-ACC-001/008 :** l'accueil comporte **JOUER** et **PALMARÈS** (plaques de même taille dans la maquette retenue). JOUER ouvre dès le LOT 1 un **écran de sélection d'adversaire** où **Capitaine est le seul adversaire jouable** ; Mousse, Amiral et VS CYD sont reportés. La composition et le retour du sous-menu restent **À CONCEVOIR / À ARBITRER** sans accès factice.
- **Traçabilité graphique :** PR #8 GRAPH-ACC-001 [fusionnée](https://github.com/Kharn27/ESP32-OTA-Battleship/pull/8) au commit `50d9e1dabd71725adba50279929694123fde0fda` ; maquette **DESIGN_PREVIEW**, sans validation tactile ni firmware embarqué.
- **ERR-DOC-001 (§ 14.1), ERR-DOC-002 (§ 14.3), ERR-DOC-003 (§ 14.10.3) : CORRECTIONS RÉDACTIONNELLES DE COHÉRENCE DE LA v0.15**, et non nouvelles décisions PO. Respectivement : seuls Mousse et Amiral sont reportés ; les principes de Capitaine sont au § 16 et les anciennes règles de Mousse au § 17 ; Capitaine vainqueur autorise déjà systématiquement une révélation *facultative* depuis la v0.13 (§ 14.13).
- **Invariants :** LOT 1 VS Capitaine seul ; dix principes CAP-CPU-001 à 010, confidentialité des flottes, résultats, abandon, palmarès et décisions V1.0 historiques intégralement conservés. La présente actualisation n'approuve ni réalisation technique ni nouveau mode jouable.

**Arbitrages complémentaires pré-backlog (30 octobre 2026, version 0.17) — décisions PO ; conservation de la traçabilité :**
- **LOT1-ACC-009 — DÉCIDÉ, remplace LOT1-ACC-008 pour le LOT 1 :** depuis l'accueil **JOUER / PALMARÈS**, JOUER conduit **directement au déploiement contre Capitaine** (flotte préplacée, AUTO, PRÊT). Aucun écran de sélection d'adversaire au LOT 1 ; ce niveau de menu est **REPORTÉ** aux lots où il deviendra utile. Le combat n'est engagé qu'à **PRÊT**. **Si** une commande RETOUR est proposée pendant le déploiement, elle ramène à l'accueil sans forfait avant PRÊT ; sa présence et sa représentation restent **À CONCEVOIR**. L'ancien choix LOT1-ACC-008 (sélection immédiate avec Capitaine seul) reste identifiable comme **REMPLACÉ**, non effacé.
- **CAP-TEST-001 — DÉCIDÉ :** réception de Capitaine sur des **comportements tactiques observables** et des **essais de difficulté ressentie**, **sans objectif imposé de taux de victoire** ; scénarios de plateau reproductibles et équité/no-triche à formaliser sans imposer d'algorithme.
- **CAP-CPU-003/004/008 — DÉCIDÉ (précisions) :** après une touche, le **prochain tir** vise une case orthogonale inconnue et plausible, si elle existe. Après deux touches adjacentes alignées (ex. E5/F5), priorité aux prolongements plausibles de l'axe (ex. D5/G5), même si un premier prolongement rate ; les deux touches **ne prouvent jamais** l'identité d'un même navire. Une hypothèse devenue impossible est révisée ; les autres pistes sont conservées selon CAP-CPU-005/008.
- **LOT1-UX-011 — DÉCIDÉ :** de FEU à la révélation synchronisée, **QUITTER/ABANDONNER** sont aussi momentanément indisponibles ; leurs appuis sont ignorés, sans action différée, puis la commande appropriée redevient accessible dès la révélation. La confirmation d'abandon après PRÊT, le palmarès et le forfait restent régis par LOT1-ABD-001 ; aucun nouveau mode d'abandon.
- **À FORMALISER / À VÉRIFIER avant transmission finale :** scénarios d'acceptation objectifs des dix CAP-CPU, retours matériels stylet/affichage, éventuels détails de navigation et erreurs. **Aucun backlog ni travail technique engagé** par ces décisions.

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

Le **LOT 1 est choisi dans son intention** ; avant de créer un backlog, identifier puis fermer les derniers **arbitrages fonctionnels bloquants**, particulièrement les cas limites de Capitaine et les règles propres à l'adversaire, et consolider les **parcours et critères d'acceptation observables**. Les travaux artistiques déjà lancés conservent leur statut sans validation implicite. **Aucune nouvelle mission technique, graphique, issue ou planification n'est créée par cette mise à jour.**

## 14. Périmètre fonctionnel du LOT 1 — Première bataille contre Capitaine (changement v0.12, socle v0.7 conservé)

**Statut : DÉCIDÉ pour les éléments explicitement marqués ci-dessous ; cadrage fonctionnel non terminé.** La présente section reprend les décisions du PO validées dans la conversation du 29 octobre 2026. La maquette d'accueil a été retenue au stade de conception (PR #8) ; les autres maquettes et les critères d'acceptation définitifs restent à préparer. La Bible complète n'est pas validée en bloc.

### 14.1 Objectif et découpage — LOT1-001, LOT1-ACC-001/008

**DÉCIDÉ — LOT1-001 :** sur **une seule ESP32 CYD classique** (tactile au stylet, écran réel **320 × 240 en paysage**, sans PSRAM présumée), jouer **une bataille classique complète contre Capitaine**, de l'accueil jusqu'au résultat. Ce lot **ne dépend pas d'ESP-NOW**.

- **DÉCIDÉ :** grille **10 × 10** ; cinq navires classiques (2, 3, 3, 4 et 5 cases), **17 cases** occupées au total.
- **DÉCIDÉ :** **mono-tir simultané** : une cible par camp, tirs secrets, résolution commune à chaque manche ; **FEU** verrouille irrévocablement le tir du joueur. Les cases déjà attaquées restent visibles et ne peuvent plus être ciblées.
- **DÉCIDÉ — LOT1-ACC-001/009 (révision PO v0.17) :** accueil volontairement minimal avec exactement deux actions **JOUER** et **PALMARÈS**, de même dimension visuelle sur la maquette d'accueil retenue. **JOUER mène directement au déploiement VS CAPITAINE** (AUTO / PRÊT), sans sélecteur intermédiaire dans le LOT 1. **PALMARÈS** reste accessible depuis l'accueil. Aucun bouton factice vers une fonction indisponible. Le choix antérieur LOT1-ACC-008 (sélection d'adversaire immédiate) est **REMPLACÉ** pour le LOT 1.
- **REPORTÉ à des lots ultérieurs, sans abandon V1.0 :** CRÉER/REJOINDRE et ESP-NOW, adversaires CPU Mousse et Amiral, familles tactiques, variante classique alternée, grilles 15 × 15 et placement manuel. L'accueil V1.0 s'enrichira lorsque ces expériences seront réellement disponibles.

**DÉCIDÉ — début du parcours LOT 1 :** ACCUEIL → **JOUER** → **DÉPLOIEMENT VS CAPITAINE** → **PRÊT** → COMBAT. **PROPOSÉ — forme détaillée des transitions de fin :** RÉSULTAT → INSPECTION / REJOUER / ACCUEIL, conformément aux décisions déjà actées. Avant PRÊT, un éventuel **RETOUR** depuis le déploiement ramène à l'accueil sans forfait ; la présence et l'habillage de cette commande sont **À CONCEVOIR**. Aucun menu de sélection d'adversaire n'est nécessaire dans le LOT 1 ; il est **REPORTÉ**.

### 14.2 Déploiement et contact des navires — LOT1-PL-001, LOT1-FLT-001

- **DÉCIDÉ — LOT1-PL-001 :** dès l'entrée en déploiement, les cinq navires du joueur sont **intégralement prépositionnés au hasard** sur une grille valide. **AUTO** renouvelle toute la disposition autant de fois que voulu ; **PRÊT** valide la disposition affichée et engage le combat. **AUTO ne vaut jamais PRÊT.** Le placement individuel, la rotation et ✓/✕ sont **hors LOT 1**, conservés pour la V1.0.
- **DÉCIDÉ — LOT1-FLT-001 :** les bâtiments peuvent se toucher **par un côté horizontal, vertical ou en diagonale**. Ils **ne se superposent pas**, restent dans la grille et aucune case d'eau n'est imposée entre eux. Cette règle s'applique **aux flottes du joueur et de Capitaine** et doit rester cohérente avec le futur placement manuel.

**Critères d'acceptation proposés à consolider :** AUTO et le déploiement initial respectent toujours la composition imposée, les limites de grille et l'absence de superposition ; PRÊT est nécessaire pour démarrer le combat ; un contact entre deux navires ne rend pas leur placement invalide.

### 14.3 IA de Capitaine — CAP-CPU-001 à 010

**DÉCIDÉ : Capitaine est le CPU du LOT 1** depuis la réorientation v0.12. Ses dix principes sont spécifiés au § 16 : recherche organisée et souple, exploitation locale des touches, révision des hypothèses contradictoires, pistes mémorisées et explorées prioritairement une à une, naufrages ambigus, persévérance et exploitation simple des tailles restantes. Pas d'omniscience, de tir répété ou d'optimisation probabiliste systématique.

**DÉCIDÉ — CAP-TEST-001 :** les dix principes CAP-CPU sont évalués par des **comportements tactiques observables** et des **essais de difficulté ressentie**, sans taux de victoire imposé. **DÉCIDÉ (précisions CAP-CPU-003/004/008) :** prochaine cible orthogonale inconnue plausible après première touche ; après deux touches adjacentes alignées, prolongements plausibles prioritaires sans certitude sur leur appartenance à un même navire (détails au § 16). **À FORMALISER / À VÉRIFIER avant backlog :** scénarios représentatifs, équité objectivement contrôlable et difficulté ressentie à tester en parties réelles, sans prescrire l'algorithme. Les règles historiques de l'IA Mousse sont conservées au § 17 et leur réalisation est REPORTÉE hors LOT 1. L'IA Capitaine, définie au § 16, est retenue pour le LOT 1.

### 14.4 Combat, naufrages et résultat — LOT1-CBT-001, LOT1-FIN-001

- **DÉCIDÉ — FEU / ciblage :** le joueur peut modifier sa cible **avant FEU** ; après FEU, elle est **définitivement verrouillée**. Les tirs du joueur et de Capitaine sont résolus pour la même manche, sans avantage de priorité. Un historique affiche raté/touché ; aucune case déjà attaquée ne redevient sélectionnable.
- **DÉCIDÉ — LOT1-CBT-001 :** quand un navire coule, le **type du navire** est annoncé (« Croiseur coulé ! » par exemple), mais les cases appartenant précisément à son épave **ne sont pas identifiées visuellement parmi les touches**. Cela préserve l'intérêt tactique des navires collés. Le joueur **et Capitaine** reçoivent cette même information limitée.
- **DÉCIDÉ — LOT1-FIN-001 :** le résultat est évalué **après résolution des deux tirs de la manche**, même si l'un des tirs achève la flotte adverse :
  - Tous les navires de Capitaine coulés, flotte humaine encore à flot : **VICTOIRE**.
  - Tous les navires humains coulés, flotte de Capitaine encore à flot : **DÉFAITE**.
  - Deux dernières flottes détruites au cours de cette même manche : **MATCH NUL**.
  - Les deux flottes ont encore un navire à flot : **manche suivante**.
  - Après un résultat définitif, **aucun nouveau tir** ; un résultat ne compte qu'**une seule partie**.

**DÉCIDÉ (expérience LOT 1, cf. § 14.10) :** enchaînement sans minuterie obligatoire ni CONTINUER, animation courte après FEU et révélation visuelle synchronisée des deux attaques. **À ARBITRER / À VÉRIFIER :** séquence interne de choix de Capitaine, style et durée exacts de l'animation, présentation graphique des annonces « coulé » ; aucun de ces détails ne modifie la résolution simultanée.

### 14.5 Présentation du combat et indicateur — LOT1-UX-001 à 004

- **DÉCIDÉ — LOT1-UX-001 :** une seule **grande grille** affichée à la fois ; **ATTAQUE** pour cibler et consulter l'historique offensif, **DÉFENSE** pour voir sa flotte et les dégâts. Bascule immédiate, volontaire, sans menu intermédiaire, **sauf pendant l'animation après FEU** (LOT1-UX-010).
- **DÉCIDÉ — LOT1-UX-002 :** **aucune bascule automatique** après résolution ; le résultat du joueur apparaît sur ATTAQUE, les tirs de Capitaine sont signalés par l'indicateur DÉFENSE. Le joueur reste libre de consulter ses dégâts plus tard.
- **DÉCIDÉ — LOT1-UX-003/004 :** gravité croissante **manqué (discret) < touché (visible) < coulé (alerte renforcée)**. En cas de plusieurs événements non consultés, **l'indicateur conserve la gravité maximale**, même après un tir manqué. **L'ouverture de DÉFENSE acquitte** l'alerte accumulée. Pas de compteur d'événements dans le LOT 1.

**Critère d'acceptation proposé :** touché → coulé → manqué sans ouverture de DÉFENSE laisse l'alerte « coulé » active jusqu'à cette ouverture ; aucun écran DÉFENSE ne s'ouvre sans action du joueur.

**À VÉRIFIER sur CYD réelle :** précision des cases et boutons au stylet, lisibilité des impacts et niveaux d'alerte, emplacement des commandes et confort de bascule. **À CONCEVOIR :** codes couleurs, pictogrammes, libellés, animations et charte graphique définitifs. Les anciens SVG sont des intentions non normatives.

### 14.6 Fin de partie, confidentialité et inspection — LOT1-FIN-002/003

- **DÉCIDÉ :** résultat clairement annoncé, tir et impacts déjà révélés conservés ; aucun tir après le résultat.
- **DÉCIDÉ — LOT1-FIN-003 :** après **victoire, défaite ou nul**, le joueur peut basculer librement entre **ATTAQUE** et **DÉFENSE** et inspecter leurs historiques sans les modifier. **REJOUER** et **ACCUEIL** restent accessibles.
- **DÉCIDÉ — FIN-001 conservée :** la flotte complète du **vainqueur** reste **secrète par défaut** ; le vainqueur humain choisit de l'autoriser ou non à la révélation. Le perdant ne voit **VOIR SA FLOTTE** que si le vainqueur a choisi **MONTRER MA FLOTTE**. Ce mécanisme de consentement humain est conservé pour le multijoueur, hors LOT 1.
- **DÉCIDÉ pour Capitaine — LOT1-FIN-002/005 (v0.13) :** Capitaine vainqueur autorise **toujours** le joueur perdant à utiliser **VOIR SA FLOTTE** ; cette révélation reste facultative pour le joueur et n'est jamais automatique. La consultation réversible sur ATTAQUE reste inchangée.
- **DÉCIDÉ — victoire du joueur VS Capitaine :** aucune commande **MONTRER MA FLOTTE**, puisqu'aucun adversaire humain ne peut consulter cette révélation.
- **DÉCIDÉ / cohérence :** en cas d'égalité par destruction réciproque, les deux flottes sont déjà intégralement touchées ; aucune commande spéciale de révélation n'est nécessaire dans le LOT 1.

**DÉCIDÉ (présentation du verdict, cf. § 14.10.3) :** verdict sur le champ de bataille, d'abord clairement mis en avant puis affiché en format compact, sans écran de résultat intermédiaire obligatoire. **À VÉRIFIER :** style et durée de cette transition, lisibilité et implantation de la commande facultative de révélation, inspection réelle sur CYD 320 × 240.

### 14.7 Rejouer et déploiements successifs — LOT1-REV-001 à 003

- **DÉCIDÉ — LOT1-REV-001 (v0.12) :** après chaque résultat, **REJOUER** lance une nouvelle bataille contre **Capitaine**, en **classique mono-tir simultané**, sans repasser par la sélection des adversaires ; retour obligatoire au déploiement.
- **DÉCIDÉ — LOT1-REV-002 :** lors d'une revanche, le déploiement affiche **par défaut la disposition précédente du joueur**. **PRÊT** la conserve ; **AUTO** permet de générer une autre disposition autant de fois que voulu. Aucun placement manuel dans le LOT 1.
- **DÉCIDÉ — LOT1-REV-003 (v0.14) :** pour **chaque nouvelle partie et chaque revanche**, Capitaine reçoit une **nouvelle disposition aléatoire valide et secrète** de ses cinq navires. Il ne conserve pas volontairement sa formation précédente ; une coïncidence aléatoire entre deux placements reste possible. La règle historiquement décidée pour Mousse est conservée pour son lot futur.
- **DÉCIDÉ :** nouveaux historiques de tirs vierges et tous les navires intacts pour les deux camps ; le palmarès de la partie précédente demeure conservé.

**Critère d'acceptation proposé :** après résultat, REJOUER revient au déploiement avec l'ancienne formation du joueur et AUTO/PRÊT ; Capitaine reçoit une nouvelle flotte aléatoire valide et secrète. Il ne reprend pas volontairement sa disposition précédente ; les historiques de tirs sont remis à zéro, les navires redeviennent intacts et les statistiques déjà acquises sont conservées.

### 14.8 Palmarès, abandon et seuil d'engagement — LOT1-STAT-001, LOT1-ABD-001

- **DÉCIDÉ — LOT1-STAT-001 :** depuis l'accueil, **PALMARÈS** permet de voir, sans consultation obligatoire, le bilan **VS CAPITAINE** : **parties jouées, victoires, défaites, matchs nuls**, et le **compteur global unique de forfaits volontaires**. Ces compteurs sont **persistants sur la CYD**, pas sur un compte personnel.
- **DÉCIDÉ :** une victoire, défaite ou égalité définitive incrémente **exactement une** partie dans la catégorie VS CAPITAINE et **exactement une** catégorie de résultat ; les forfaits ne sont pas des parties jouées. Les quatre catégories V1.0 **VS HUMAIN / MOUSSE / CAPITAINE / AMIRAL** restent décidées, mais seul le combat VS CAPITAINE est disponible dans ce lot.
- **DÉCIDÉ — LOT1-ABD-001 :** **PRÊT est le seuil d'engagement.** Avant PRÊT, quitter le déploiement ne comptabilise **ni forfait ni partie**. Après PRÊT et avant le résultat définitif, une demande d'abandon affiche **CONTINUER / ABANDONNER**. CONTINUER préserve l'état et les compteurs ; ABANDONNER confirmé ajoute **1** au **compteur global de forfaits**, sans inscrire la bataille au palmarès VS CAPITAINE. Après résultat définitif, retour à l'accueil sans forfait supplémentaire.
- **DÉCIDÉ :** un incident technique, une extinction ou une coupure imprévue ne valent pas automatiquement abandon volontaire (BN-STAT-003). La reprise après redémarrage reste **REPORTÉE** dans le cadre V1.0.

**Critères d'acceptation proposés :** abandon confirmé en cours de bataille = forfait +1 uniquement ; quitter au déploiement ou après résultat = +0 ; annuler la confirmation = aucun changement ; résultat final = partie et bilan incrémentés une fois et persistants.

### 14.9 Exclusions, points à instruire et prochaine discussion

**REPORTÉ hors LOT 1 :** multijoueur ESP-NOW et ses salons/connexions, les niveaux Mousse et Amiral, variante alternée, modes et compositions tactiques, grille 15 × 15, placement manuel, interfaces avancées et éventuelle console commerciale. **Ces décisions V1.0 ne sont pas annulées.**

**DÉCIDÉ — parcours nominal détaillé au § 14.10 et révélation pédagogique au § 14.13 :** sélection, FEU irrévocable, animation courte, révélation synchronisée, information des dégâts dans DÉFENSE, passage libre à la manche suivante, verdict intégré et révélation facultative cadrée. **À ARBITRER / À CONCEVOIR :** cas particuliers restants de l'IA de Capitaine, modalités visuelles et ergonomie du palmarès, détails graphiques et transitions.

**À VÉRIFIER :** ergonomie réelle sur CYD 320 × 240 et comportement observable en parties de test. Les ajustements visuels issus des tests ne doivent pas devenir silencieusement de nouvelles règles.

**DÉCIDÉ — v0.12 à v0.14 :** Capitaine est l'adversaire du LOT 1 ; Mousse et Amiral sont reportés. L'autorisation systématique de révélation par Capitaine vainqueur (v0.13) et la nouvelle flotte aléatoire de Capitaine à chaque partie et revanche (v0.14) sont décidées. **PROPOSÉ — prochaine étape :** consolider les critères d'acceptation de Capitaine et les derniers cas limites tactiles avant backlog. Conserver un arbitrage produit à la fois. **Aucune mission technique/graphique supplémentaire, aucun backlog ni planning n'est lancé par ce document.**

### 14.10 Parcours tactile et visuel d'une manche contre Capitaine — décisions PO du 29/10/2026

**Statut : DÉCIDÉ pour les principes ci-dessous ; durées, implantation et style graphique À VÉRIFIER sur CYD réelle.** Les règles de résolution simultanée et la confidentialité des flottes ne changent pas.

#### 14.10.1 Choix, FEU, animation et nouvelle manche — LOT1-UX-005/006/007/009

1. **Avant FEU :** sur ATTAQUE, le joueur sélectionne une case encore inconnue, peut la désélectionner ou choisir une autre cible ; les cases déjà attaquées ne sont pas sélectionnables. Une bascule volontaire vers DÉFENSE puis un retour sur ATTAQUE **conservent la sélection en cours**, sans validation ni annulation implicite.
2. **FEU :** la cible choisie devient **irrévocablement verrouillée** pour cette manche. Capitaine prépare également sa cible sans accès aux informations secrètes ; l'ordre interne exact de son choix n'est pas prescrit. Le joueur ne peut plus modifier le tir déjà engagé.
3. **Animation brève :** un petit effet visuel de tir/impact accompagne la résolution. **Aucun timer de 5 ou 10 secondes, aucun délai artificiel entre les manches, aucun bouton CONTINUER** n'est imposé. Pendant cette animation, les **appuis sur la grille ATTAQUE visant à préparer une nouvelle cible sont ignorés et non mis en attente** ; le prochain tir ne peut être préparé qu'après la révélation. **DÉCIDÉ — LOT1-UX-010 :** la navigation **ATTAQUE/DÉFENSE est temporairement verrouillée** de FEU jusqu'à la révélation. Un appui sur DÉFENSE durant cet intervalle est **ignoré et non mémorisé** ; l'écran reste sur ATTAQUE et aucune bascule différée n'intervient.
4. **Révélation visuelle synchronisée :** au même instant, le résultat du tir du joueur s'inscrit sur **ATTAQUE** et la commande **DÉFENSE** présente la riposte de Capitaine. Il ne s'agit pas d'une résolution successivement avantageuse : les **deux attaques sont résolues ensemble**, et les deux résultats sont révélés ensemble. Un naufrage annonce le type de navire, sans identifier les cases de l'épave adverse. **Après la révélation**, la navigation redevient immédiatement libre : DÉFENSE n'est consultable qu'à la suite d'un nouvel appui volontaire, sans bascule automatique.
5. **Suite du combat :** si les deux flottes ont encore un bâtiment à flot, le joueur peut **immédiatement** préparer la manche suivante, ou consulter DÉFENSE aussi longtemps qu'il le souhaite. **Aucune bascule automatique** ne lui est imposée.

**DÉCIDÉ — LOT1-UX-010 :** pendant l'animation de FEU, navigation verrouillée, appuis sur DÉFENSE ignorés et non différés ; libre consultation après révélation simultanée. **Critère d'acceptation proposé :** un appui DÉFENSE pendant l'animation ne quitte pas ATTAQUE, n'acquitte pas les alertes DÉFENSE et ne provoque aucune bascule a posteriori ; un nouvel appui après révélation ouvre DÉFENSE conformément à LOT1-UX-008. **À VÉRIFIER :** durée et fluidité de l'animation, confort tactile et compréhension sur CYD réelle.

**DÉCIDÉ — LOT1-UX-011 (v0.17) :** de l'appui sur **FEU** jusqu'à la révélation synchronisée des deux attaques, les commandes **QUITTER / ABANDONNER** sont également indisponibles. Tout appui pendant cet intervalle est **ignoré et non mémorisé** ; aucun dialogue ne s'ouvre et aucune demande d'abandon différée n'est exécutée. Dès la révélation, la commande pertinente est de nouveau accessible : avant résultat définitif, l'abandon suit **CONTINUER / ABANDONNER** et les règles du forfait après PRÊT ; après résultat, le retour à l'accueil n'ajoute pas de forfait.

**Critère d'acceptation proposé (LOT1-UX-011) :** une tentative d'abandon pendant l'animation ne modifie ni la manche, ni les tirs, ni les compteurs, et n'ouvre aucun dialogue après la révélation ; une nouvelle demande effectuée après révélation déclenche la confirmation seulement si la partie est encore en cours.

#### 14.10.2 Commande DÉFENSE : dernière riposte et alerte non consultée — LOT1-UX-008

**DÉCIDÉ :** le bouton/zone tactile **DÉFENSE** remplit deux fonctions indépendantes et complémentaires :

- **Information de la dernière manche seulement :** si Capitaine touche une case, le message intégré associe son impact à sa coordonnée, par exemple **« Capitaine vous a touché en E5 »**, ou **« E5 (Croiseur coulé) »** si ce point achève un navire. Une riposte manquée reste indiquée discrètement. Le texte exact et l'habillage restent à concevoir.
- **Alerte persistante :** la commande conserve la **gravité maximale des événements non consultés** (manqué < touché < coulé), même si la dernière manche est moins grave. Elle ne conserve pas une liste cumulative de toutes les coordonnées.
- **Manche suivante sans consultation :** le contenu « dernière riposte » est **remplacé** par celui de la nouvelle manche ; les coordonnées antérieures ne sont pas répétées dans la commande. L'alerte maximale non consultée continue, elle, à s'appliquer.
- **Ouverture de DÉFENSE :** la consultation **acquitte l'alerte et efface les coordonnées du résumé du bouton**. De retour sur ATTAQUE, la commande n'affiche plus ce message ancien. **Cela n'efface aucun impact ni aucun historique sur la grille DÉFENSE.**
- **Navigation :** le joueur décide seul d'ouvrir DÉFENSE ; cette consultation ne modifie pas la cible déjà préparée sur ATTAQUE avant FEU.

**Exemple observable :** manche N−1, Capitaine touche B4 ; le joueur ne consulte pas DÉFENSE. Manche N, Capitaine manque son tir : la commande donne l'information de la **dernière manche manquée**, mais conserve l'alerte **TOUCHÉ** de B4 non consulté. À l'ouverture de DÉFENSE, l'alerte et le résumé sont acquittés ; B4 reste marqué sur la grille.

#### 14.10.3 Dernière manche, verdict et inspection — LOT1-FIN-004

**DÉCIDÉ :** le verdict **VICTOIRE / DÉFAITE / MATCH NUL** n'intervient **qu'après** la révélation synchronisée des deux tirs de la dernière manche. Il apparaît **sur le champ de bataille**, en conservant la grille et ses impacts, **sans écran de résultat intermédiaire obligatoire**.

- Le verdict est d'abord **bien visible**, puis se **réduit à une présentation compacte** qui libère de la place sur la grille, sans jamais perdre l'identification du résultat.
- Cette présentation ne nécessite **aucune confirmation ni attente obligatoire** ; ses transitions et durées exactes restent à vérifier.
- Les vues **ATTAQUE** et **DÉFENSE** restent librement consultables après le résultat, sans nouveau tir possible ; **REJOUER** et **ACCUEIL** restent accessibles.
- La révélation facultative de la flotte de Capitaine, **autorisée systématiquement lorsque Capitaine gagne (DÉCIDÉ en v0.13, LOT1-FIN-002/005)**, reste déclenchée au choix du joueur perdant via **VOIR SA FLOTTE** ; la confidentialité (**FIN-001**) et l'inspection sont détaillées au § 14.13.

**PROPOSÉ (non encore décidé) :** ne pas ajouter de bouton INSPECTER si la bascule ATTAQUE/DÉFENSE permet déjà la consultation post-partie.

### 14.11 Prolongement UX des salves — décisions V1.0 conservées, réalisation hors LOT 1

**DÉCIDÉ sur le principe pour les futurs modes salves ; REPORTÉ hors réalisation LOT 1.** Le LOT 1 ne présente qu'une seule coordonnée par manche, mais les règles d'information suivantes sont actées pour ne pas perdre les arbitrages du PO :

- La commande **DÉFENSE reste de taille fixe**, quelle que soit la quantité d'impacts de la dernière salve : ni agrandissement automatique, ni texte défilant.
- Le message expose, dans l'espace disponible, les **coordonnées touchées lors de la dernière salve uniquement** ; les impacts des manches précédentes ne s'empilent pas. Le joueur peut retrouver tous les dégâts sur sa grille DÉFENSE.
- L'annonce du type de bâtiment coulé se place **auprès de la coordonnée ayant achevé ce navire**, et non dans un bandeau indépendant ni nécessairement à la fin de la liste totale : **« E4, E5 (Croiseur coulé), B4 »**. Dans cet exemple, E5 est le point qui complète le croiseur et B4 appartient aussi à la salve.
- Si les coordonnées dépassent l'espace disponible, la commande privilégie les **annonces de naufrage**, puis affiche les coordonnées qu'elle peut contenir et termine par **« +N impacts »**, où **N compte uniquement les cases touchées omises du résumé**, et non les tirs manqués. Aucune donnée n'est supprimée de la grille.
- L'ouverture de DÉFENSE acquitte **à la fois** le résumé de la dernière salve et l'alerte de gravité maximale non consultée, conformément au § 14.10.2.
- Cette association d'un naufrage à une coordonnée concerne **les dégâts sur sa propre flotte**. Elle ne lève **jamais** l'interdiction de révéler à l'attaquant l'appartenance précise des cases touchées à une épave ennemie (LOT1-CBT-001).

**À ARBITRER pour les salves :** lorsqu'au cours d'une même résolution simultanée plusieurs nouveaux impacts contribuent ensemble à couler un même navire, la convention exacte d'association du libellé « coulé » à l'une des coordonnées. Une attribution à la dernière coordonnée présentée a été **PROPOSÉE**, mais **pas validée**. L'ordre d'énumération des tirs ne doit pas créer de priorité de résolution fictive.

**À VÉRIFIER sur écran réel 320 × 240 :** lisibilité des libellés de naufrage, nombre de coordonnées affichables, abréviations, tailles tactiles et maintien de la place destinée à la grille. Aucun détail graphique ni nombre maximal de caractères n'est validé.

### 14.12 Maturité de ces arbitrages et essais de jeu

**DÉCIDÉ :** l'expérience recherchée privilégie le **rythme libre**, la consultation volontaire des dégâts, des retours de tir courts et un verdict lisible sans interrompre l'inspection. **Les choix UX pourront être revus explicitement par le PO au vu des parties de test du LOT 1** ; ce caractère révisable ne constitue pas une autorisation de les modifier sans nouvel arbitrage.

**À VÉRIFIER / À CONCEVOIR :** durées d'animation et de mise en avant du verdict, emplacement du résumé DÉFENSE, codes couleur, icônes, transitions, lisibilité au stylet et comportement observable sur le matériel réel. Les anciennes maquettes restent non normatives. Aucune mission graphique ni développement n'est engagé par cette section.

### 14.13 Révélation volontaire et inspection pédagogique — v0.10 complétée pour Capitaine en v0.13 (LOT1-FIN-002/005)

**Statut : DÉCIDÉ pour le parcours de consultation et l'autorisation systématique de Capitaine vainqueur. Graphisme À CONCEVOIR / À VÉRIFIER sur CYD 320 × 240.** Cette section complète FIN-001, LOT1-FIN-002/003/004 et ne modifie ni le résultat ni le palmarès.

**DÉCIDÉ — autorisation préalable par le vainqueur (FIN-001, LOT1-FIN-005) :**
- La commande **VOIR SA FLOTTE** n'est disponible pour le perdant **que si le vainqueur a autorisé la révélation de sa propre flotte** par **MONTRER MA FLOTTE**.
- Dans le futur multijoueur humain, **MONTRER MA FLOTTE** est une action volontaire du vainqueur ; sans autorisation, aucune révélation adverse ni commande VOIR SA FLOTTE. Le détail du parcours d'échange entre deux CYD reste **REPORTÉ hors LOT 1**.
- **DÉCIDÉ pour les CPU :** Mousse (lot futur) **et Capitaine (LOT 1)** autorisent toujours la révélation de leur flotte lorsqu'ils gagnent. Cette autorisation ne révèle aucun navire automatiquement et ne nécessite aucun bouton fictif actionné par le CPU.
- Si le joueur gagne contre Capitaine, **MONTRER MA FLOTTE n'est pas affiché**, faute d'adversaire humain bénéficiaire. En cas de **match nul**, aucune commande de révélation particulière n'est nécessaire dans le LOT 1 : toutes les positions sont déjà connues.

**DÉCIDÉ — consultation volontaire après une défaite VS Capitaine (LOT1-FIN-002) :**
- Après le verdict **DÉFAITE** puis son affichage compact, **VOIR SA FLOTTE** est accessible directement sur le **champ de bataille**, de manière discrète et persistante jusqu'au départ de la fin de partie ; **aucune boîte OUI/NON, aucune interruption ni écran d'analyse obligatoire**.
- Un appui volontaire révèle la **totalité des cinq navires de Capitaine**, y compris leurs cases encore inconnues, **sur la grille ATTAQUE existante**.
- Les **tirs réussis, tirs manqués et résultats historiques** restent visibles au-dessus / distinctement des positions révélées. Le joueur compare ainsi les tirs réellement tentés aux navires découverts **sur une même grille**.
- Une fois la révélation demandée, le joueur peut **AFFICHER ou MASQUER la flotte à volonté** ; masquer restitue la lecture initiale de la grille ATTAQUE, **sans supprimer les tirs ni révoquer l'autorisation**.
- L'état courant **flotte affichée / flotte masquée** est **conservé** lorsqu'il passe volontairement sur DÉFENSE puis revient sur ATTAQUE. La navigation ATTAQUE/DÉFENSE, l'inspection libre et l'accès à REJOUER / ACCUEIL demeurent disponibles.
- L'inspection pédagogique est **strictement visuelle** : pas de sélection de navire, fiche, nom contextuel, détail par bâtiment, comptage automatisé des cases découvertes ou analyse guidée. Le joueur tire ses propres conclusions. Une éventuelle inspection enrichie est **REPORTÉE**.
- Après le verdict, **aucun nouveau tir** ; révéler, masquer ou inspecter ne change **ni le résultat, ni le nombre de parties, ni le palmarès, ni le compteur de forfaits**.

**À CONCEVOIR / À VÉRIFIER :** lisibilité des silhouettes de navires sous les marques de tir et des cases encore inconnues, taille et emplacement des commandes sur **CYD tactile 320 × 240**, formulation précise du basculement AFFICHER/MASQUER, coexistence du verdict compact et des commandes REJOUER / ACCUEIL. Aucun code couleur, dessin, durée ou maquette de production n'est validé par la présente décision.

**Traçabilité historique v0.10 (VS Mousse) :** révélation sur ATTAQUE, affichage/masquage réversible, état mémorisé entre les grilles, proposition discrète non modale, autorisation obligatoire et inspection visuelle simple. Mousse autorise toujours s'il gagne (décision maintenue, réalisation reportée) ; absence de commande MONTRER MA FLOTTE après victoire humaine solo. Le LOT 1 v0.12 retient désormais Capitaine (§ 14.3 et § 16) ; **sa politique d'autorisation est désormais décidée en v0.13**, tandis que les règles propres à Mousse restent conservées au § 17.

## 15. Identité « CYD WARSHIP » et écran d'accueil — cadrage PO du 29 octobre 2026 (v0.8)

Cette section complète la Bible **sans modifier les décisions de gameplay ni le découpage du LOT 1 définis au § 14**. Elle retrace le cadrage initial de la maquette d'accueil (v0.8) et ses suites documentées ci-dessous. La validation initiale du concept **ne valait pas validation de l'illustration** ; celle-ci a ensuite été retenue au stade de conception (PR #8), sans validation firmware ni matérielle.

### 15.1 Nom du jeu — LOT1-ACC-002

**DÉCIDÉ (PO) :** le titre officiel du jeu affiché au joueur est **CYD WARSHIP**. « Bataille navale » reste une description du jeu, non un second titre imposé. L'identifiant du projet **I-009** et le nom du dépôt **ESP32-OTA-Battleship** ne sont pas renommés automatiquement.

**À VÉRIFIER si diffusion publique :** disponibilité et usage commercial du nom ; cette question ne bloque pas le cadrage artistique.

### 15.2 Écran d'accueil : scène, composition et navigation — LOT1-ACC-003 à 008

**DÉCIDÉ (PO) — intentions à représenter :**
- Pour la **V1.0**, l'accueil montre une **scène illustrée fixe** : pas d'animation requise sur cet écran.
- **Coucher de soleil préservé**, avec atmosphère maritime expressive ; un navire principal au premier plan et un navire ennemi à l'horizon, dans leur relation de composition actuelle.
- Le navire ennemi reste identifiable et endommagé ; notre bâtiment tire vers lui. La scène doit aussi montrer **la riposte ennemie ratée**, par exemple une **gerbe d'eau près du navire principal**, sans cacher ce dernier.
- Le titre **CYD WARSHIP** apparaît **dans la partie haute**, clairement identifiable mais **visuellement discret** : il ne doit pas écraser le paysage.
- Les commandes du LOT 1 sont **directement superposées à l'illustration**, dans une partie moins chargée, **de préférence en bas à gauche**. Éviter un panneau massif. Leur visibilité et leur facilité de sélection au stylet restent prioritaires.

**DÉCIDÉ (LOT1-ACC-001/008, complément PO après v0.15) — exactement deux actions sur l'accueil du LOT 1 :**
- **JOUER**
- **PALMARÈS**

**DÉCIDÉ — navigation (LOT1-ACC-009, révision PO v0.17) :** les deux plaques de commande ont la même dimension visuelle dans la maquette d'accueil retenue. **JOUER** mène **directement au déploiement classique 10 × 10 VS CAPITAINE**, prépositionné et modifiable par AUTO ; seule la validation PRÊT lance le combat. **PALMARÈS** ouvre la consultation depuis l'accueil. Un éventuel **RETOUR** pendant le déploiement ramène à l'accueil sans forfait avant PRÊT ; l'existence et le dessin de cette commande ne sont pas imposés par cet arbitrage.

**DÉCIDÉ historiquement / REMPLACÉ :** LOT1-ACC-008 prévoyait dès le LOT 1 un écran de sélection d'adversaire avec Capitaine seul jouable. Ce choix est **remplacé** par LOT1-ACC-009 pour éviter une étape intermédiaire à choix unique. **REPORTÉ :** la sélection d'adversaire lorsque d'autres modes seront réellement disponibles, y compris un futur « VS autre CYD » ; **aucun ESP-NOW, Mousse, Amiral, CRÉER / REJOINDRE, variantes ou bouton factice actif** au LOT 1.

**PROPOSÉ lors du cadrage initial v0.8 — options facultatives, non imposées à la composition finalement retenue :**
- Un **avion en feu**, en chute vers la mer à proximité du navire principal.
- Une **escadrille en silhouettes** dans le ciel lointain, idéalement devant les couleurs du coucher de soleil.
- Réduire la quantité et le détail des avions si leur lisibilité ou la sobriété de la composition l'exigent.

**DESIGN_PREVIEW retenu / À VÉRIFIER pour l'intégration :** composition, titre, boutons et rendu à **320 × 240 natifs** sont fixés dans la maquette approuvée par le PO (PR #8), **sans constituer un contrat firmware**. Lisibilité et contraste sur l'écran CYD réel, géométrie des zones tactiles au stylet résistif et format d'asset embarqué restent **À VÉRIFIER**. Les éléments aériens du cadrage initial demeurent facultatifs.

### 15.3 Frontière entre illustration et jeu

Les images de bataille précédemment produites sont des **études narratives historiques**, non des maquettes d'accueil validées et non des assets firmware. Leur retrait de l'arborescence actuelle est déjà tracé dans [MAQUETTES.md](MAQUETTES.md) et dans l'historique Git. **Ne pas les réintroduire automatiquement comme fichiers actifs.** La mission GRAPH-ACC-001 a livré une **maquette d'accueil complète comprenant titre et deux actions**, retenue par le PO au **stade de conception** via la [PR #8](https://github.com/Kharn27/ESP32-OTA-Battleship/pull/8) (fusion `50d9e1dabd71725adba50279929694123fde0fda`). Cette acceptation artistique ne constitue ni une validation sur CYD réelle ni une intégration firmware.

La présence d'un ennemi visible en feu dans une **illustration d'accueil** n'autorise pas la révélation des positions secrètes adverses **pendant le combat**.

### 15.4 Transmission au Pilotage et maturité

**DÉCIDÉ :** le PO a demandé de documenter ces intentions avant de confier au **Pilotage** la coordination de la prochaine maquette avec le graphiste. Le Pilotage garde la responsabilité des missions graphiques, des revues et des fusions.

**PROPOSÉ à la v0.8 / réalisé ensuite pour revue de conception :** une première composition unique, complète, **320 × 240**, avec agrandissement ×4 sans interpolation et source SVG intégrant un fond raster. La maquette retenue est versionnée dans `design/graph-acc-001/` après fusion de la PR #8. Pas de validation matérielle des zones tactiles, d'autres écrans, de sprites runtime ni de développement firmware.

**À ARBITRER par le PO :** politique générale de conservation des ressources graphiques pour les futurs travaux. Le retrait ponctuel des **17 fichiers historiques**, autorisé le 29 octobre 2026, n'approuve pas automatiquement cette politique et ne vaut autorisation de supprimer aucun asset actif ; la mission d'accueil LOT 1 a été livrée au stade maquette de conception.

**Statut graphique général :** **maquette d'accueil retenue par le PO au stade DESIGN_PREVIEW**, dans la PR #8 fusionnée ; **ON_DEVICE_UNVERIFIED**. Aucune charte visuelle globale, aucun format runtime/firmware et aucune ergonomie tactile réelle n'ont été validés par cette livraison. Les autres écrans restent à concevoir et à vérifier.

### Traçabilité de la v0.8

**Source :** arbitrages explicites du PO dans la conversation du 29 octobre 2026 : nom **CYD WARSHIP** ; illustration d'accueil fixe ; titre dans le ciel, visible mais discret ; boutons superposés dans une zone calme, idéalement en bas à gauche ; maintien du coucher de soleil et des navires ; ajout d'une riposte adverse manquée ; aéronefs à explorer sans les déclarer obligatoires. **Tous les autres arbitrages fonctionnels de la v0.7 sont conservés.**



## 16. Référentiel fonctionnel CPU Capitaine — décisions PO du 29 octobre 2026 (v0.11)

**Statut : DÉCIDÉ pour les dix principes et l'inclusion de Capitaine au LOT 1 (v0.12).** Le PO a retenu Capitaine comme premier adversaire jouable après l'avoir défini comme référence intermédiaire. **Mousse et Amiral sont REPORTÉS** ; cette décision n'engage pas de développement.

**Règle commune d'équité — DÉCIDÉ :** Capitaine raisonne sur les tirs, les résultats, les annonces de naufrage et les règles connues par les joueurs. Il n'obtient **aucune information secrète** sur la disposition humaine, l'appartenance des impacts ou les cases exactes des épaves.

| Référence | Principe DÉCIDÉ par le PO | Portée / limite |
| --- | --- | --- |
| **CAP-CPU-001** | **Recherche organisée dès le premier tir** | Capitaine répartit ses recherches sur les zones inconnues au lieu d'attendre une touche en tirant purement au hasard. |
| **CAP-CPU-002** | **Recherche organisée mais souple** | Aucune obligation de balayage en damier ou de motif rigide ; l'exploration conserve de l'imprévisibilité. |
| **CAP-CPU-003** | **Exploitation méthodique d'une première touche** | Il privilégie les cases orthogonales inconnues autour d'une touche jusqu'à découvrir un nouvel indice ou épuiser les voisins possibles, sans renoncer après un simple raté. |
| **CAP-CPU-004** | **Révision d'une hypothèse contradictoire** | Avec D4/E4 touchés, C4/F4 ratés et aucun naufrage, il comprend que les deux touches ne forment pas un unique navire horizontal et les traite comme des pistes distinctes, sans identifier les bateaux. |
| **CAP-CPU-005** | **Une piste prioritaire à la fois** | Il explore une piste exploitable jusqu'à épuisement ou révision justifiée, mémorise les autres et y revient ; pas de comparaison permanente des meilleures chances. |
| **CAP-CPU-006** | **Prudence après naufrage** | Après « coulé », il considère sa piste probablement terminée, passe à une autre piste connue si disponible, mais conserve les touches ambiguës et peut y revenir. |
| **CAP-CPU-007** | **Exploitation d'une zone ambiguë sans autre piste connue** | Si une zone rassemble quatre touches qu'une épave annoncée de trois cases ne suffit pas à expliquer, il soupçonne au moins une touche appartenant à un autre navire et explore la zone en priorité, **sans savoir laquelle**. |
| **CAP-CPU-008** | **Persévérance raisonnée** | Il poursuit une piste tant qu'il reste des prolongements crédibles, élimine les hypothèses impossibles à la lumière des ratés, puis passe à une autre piste ou à la recherche générale. |
| **CAP-CPU-009** | **Prise en compte des tailles restantes** | Grâce aux types déjà coulés, il évite une zone dont les cases inconnues isolées ne peuvent contenir aucun navire encore à flot ; déduction géométrique simple et justifiable. |
| **CAP-CPU-010** | **Pas d'optimisation systématique des possibilités** | Il élimine les zones impossibles mais explore souplement les zones plausibles, sans classer systématiquement celles-ci par nombre de placements ou probabilité de touche. |

### Précisions d'acceptation Capitaine décidées en v0.17 — CAP-TEST-001, CAP-CPU-003/004/008

**DÉCIDÉ — première touche (CAP-CPU-003) :** après avoir touché une case (exemple E5), **le tir suivant vise une case orthogonale inconnue et plausible** parmi D5, F5, E4, E6 s'il en existe. L'ordre de ces possibilités n'est pas imposé. Un simple raté ne termine pas l'exploration d'une piste encore crédible. Lorsque tous les voisins directs ne sont plus exploitables, les règles existantes de prolongement, de révision et de mémorisation des pistes (CAP-CPU-005/008) demeurent applicables.

**DÉCIDÉ — deux touches voisines alignées (CAP-CPU-003/004/008) :** exemple E5 et F5 touchés, navire non coulé : Capitaine **privilégie D5 ou G5**, lorsqu'un prolongement de l'axe est encore inconnu et plausible. Si l'un rate, l'autre demeure prioritaire s'il reste plausible ; si les prolongements deviennent impossibles, il révise son hypothèse et revient aux autres pistes crédibles. **Ces deux touches peuvent provenir de navires différents qui se touchent : l'alignement est une hypothèse, jamais une preuve ni une information secrète.**

**DÉCIDÉ — méthode de validation (CAP-TEST-001) :** conformité des choix sur des situations de plateau observables, contrôle de l'absence de triche (seules les informations publiques peuvent orienter les choix), aucune attaque répétée, puis vérification qualitative de la difficulté ressentie lors de parties réelles ; **aucun taux de victoire contractuel**.

**Critères d'acceptation PROPOSÉS à finaliser avant backlog :**
- Étant donné une première touche isolée E5 et au moins une voisine orthogonale inconnue plausible, quand Capitaine rejoue, alors sa cible est l'une de ces voisines.
- Étant donné E5/F5 touchées et D5/G5 encore plausibles, le tir suivant privilégie l'un des deux prolongements ; après un raté sur l'un, l'autre reste prioritaire, sans certitude sur l'identité du navire.
- Étant donné deux touches alignées encadrées par des ratés et sans annonce de naufrage (ex. C4 raté, D4/E4 touchées, F4 raté), Capitaine doit réviser l'hypothèse d'un unique navire horizontal, sans exploiter d'information secrète.
- Capitaine mémorise les autres pistes, peut revenir sur une zone ambiguë après naufrage, et n'identifie jamais avec certitude les cases d'une épave simplement annoncée comme coulée.
- Une situation publique identique ne doit pas conduire à un choix favorisé par une **position secrète inaccessible** ; les modalités de vérification sont à instruire par le Pilotage.

Les scénarios sont **PROPOSÉS** comme formulation de réception et non comme décisions supplémentaires ni prescription d'algorithme.

**Nuance fonctionnelle essentielle :** les navires peuvent se toucher. Une annonce de naufrage ne désigne **jamais les cases exactes de l'épave** ; l'analyse des touches non expliquées doit donc rester hypothétique, et aucun CPU ne peut « marquer » secrètement comme certaines les cases d'un navire coulé. En particulier, CAP-CPU-007 décrit le cas où **les touches observées ne peuvent toutes être expliquées par le navire annoncé**, sans supposer que Capitaine reçoit l'identité de ces touches.

**DÉCIDÉ pour LOT 1 :** Capitaine sera l'adversaire du premier jeu complet. **REPORTÉ :** Mousse, Amiral, compositions tactiques et équilibrage comparatif. **PROPOSÉ / non décidé :** attribuer à Amiral un balayage en damier, des comparaisons de configurations ou une recherche probabiliste.

**DÉCIDÉ — CAP-TEST-001 (v0.17) :** la conformité de Capitaine repose sur des scénarios de **comportement tactique observable** et des **essais de difficulté ressentie**, sans objectif statistique de victoire. **À FORMALISER** : ensemble représentatif de cas objectifs à vérifier (priorités locales, pistes multiples, naufrages ambigus, déductions géométriques sans secret), sans imposer un algorithme ni un ordre unique de tirs lorsque la règle laisse un choix. La difficulté ressentie reste **À VÉRIFIER en parties réelles** et ne conditionne pas la réception à un pourcentage arbitraire. **Aucun nouveau comportement de Mousse n'est validé par analogie** avec les principes CAP-CPU-001 à 010.

**Traçabilité :** les dix choix CAP-CPU ont été approuvés successivement par le PO le 29 octobre 2026. Le PO a ensuite explicitement corrigé le périmètre du LOT 1 pour y retenir Capitaine et reporter Mousse et Amiral. Les règles Mousse sont conservées au § 17.

## 17. Réserve historique Mousse — REPORTÉ hors LOT 1 (v0.12)

**DÉCIDÉ historiquement ; réalisation REPORTÉE.** Ce texte provient de l'ancien § 14.3 et conserve ses identifiants d'origine LOT1-CPU-001 à 006 pour la traçabilité : ils ne constituent plus des exigences du LOT 1, désormais VS Capitaine.

### 17.1 Règles de Mousse conservées, à rattacher au lot futur

**DÉCIDÉ — adversaire débutant crédible, imparfait et sans triche.** Mousse ne reçoit jamais la carte secrète, l'identité secrète d'une case touchée ni la composition d'une épave ; il raisonne sur ses tirs passés et les annonces qu'un adversaire humain recevrait.

- **LOT1-CPU-001 / 002 — Exploration et chasse :** sans piste exploitable, Mousse tire au hasard parmi les cases **jamais attaquées**. Après une touche, il privilégie les cases inconnues voisines. Il peut choisir une mauvaise direction, mais cherche à exploiter sa découverte plutôt que repartir systématiquement ailleurs.
- **LOT1-CPU-003 — Hypothèse d'alignement :** après **deux impacts voisins alignés**, Mousse **suppose** qu'ils appartiennent à un même navire et privilégie l'axe et ses prolongements encore inconnus. Il peut se tromper de sens. **Les navires pouvant se toucher**, cet alignement n'est pas une preuve d'appartenance commune et ne doit pas être traité comme une connaissance cachée.
- **LOT1-CPU-004 — Pistes multiples :** Mousse mémorise toutes les découvertes et privilégie **une piste exploitable à la fois** ; il peut reprendre une autre piste déjà identifiée sans perdre les impacts précédents.
- **LOT1-CPU-005 — Après un coulé :** la piste actuelle est considérée comme **probablement terminée** ; Mousse privilégie une autre piste disponible, tout en conservant les impacts antérieurs et la possibilité d'y revenir. Il ne connaît pas les cases exactes de l'épave.
- **LOT1-CPU-006 — DÉCIDÉ (29/10/2026) :** si Mousse a deux touches adjacentes alignées et rate le premier prolongement testé, il **privilégie l'autre extrémité encore inconnue** avant de remettre en cause cet axe. Cela décrit une priorité, non une garantie de toucher.
- **DÉCIDÉ :** aucune case déjà attaquée ne peut être rejouée par Mousse ; ses mauvaises décisions découlent des **hypothèses** et choix de direction, pas d'une consultation des positions secrètes.

**À ARBITRER / préciser sans prescrire d'algorithme :** après deux prolongements ratés encadrant deux touches sans annonce de naufrage (ex. C4 raté, D4/E4 touchés, F4 raté), Mousse perd-il temporairement sa piste ou continue-t-il à explorer localement ? **Aucune option n'a été validée** pour ce cas. Restent également ouverts : sélection entre voisins, pistes qui se recoupent, absence de piste exploitable, réactions aux naufrages ambigus et mesure observable du niveau « débutant ». Ne pas transformer automatiquement les capacités de Capitaine (§ 16) en capacités de Mousse. Les principes ci-dessus ne constituent pas une implémentation technique.

**Critères d'acceptation proposés à consolider :** préférence observable pour une zone touchée puis son axe possible ; capacité à retrouver une ancienne piste ; absence de tir répété ou de déduction utilisant une information secrète.

