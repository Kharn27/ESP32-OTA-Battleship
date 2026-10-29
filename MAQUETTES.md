# Index des maquettes et recherches artistiques — CYD WARSHIP / I-009

**ÉTAT DOCUMENTAIRE AU 29/10/2026 — aucune direction artistique de production validée par le PO.**

| Classe | Statut | Contenu / portée |
| --- | --- | --- |
| **Références graphiques approuvées pour production** | **AUCUNE** | Ni charte ni écran final validés ; ne pas convertir directement les archives en assets firmware |
| **Accueil CYD WARSHIP — maquette retenue au stade conception** | **DÉCIDÉ (PO) / DESIGN_PREVIEW accepté, ON_DEVICE_UNVERIFIED** | [PR #8 fusionnée](https://github.com/Kharn27/ESP32-OTA-Battleship/pull/8) : commandes **JOUER / PALMARÈS** ; sélection de Capitaine sous JOUER ; aucune validation comme asset runtime. Voir [Bible §15](BIBLE_PRODUIT.md#15-identité--cyd-warship--et-écran-daccueil--cadrage-po-du-29-octobre-2026-v08) |
| **Scènes narratives du 29 octobre 2026** | **HISTORIQUE_GIT — NON_VALIDEE_PO — NON_RUNTIME** | Retirées de `main` sur décision PO ; [PR #2](https://github.com/Kharn27/ESP32-OTA-Battleship/pull/2) et [dossier historique figé](https://github.com/Kharn27/ESP32-OTA-Battleship/tree/872496d73a150dd5a016c219973e5865576b7e04/assets/explorations/2026-10-29-bataille-en-cours) |
| **Archives visuelles A/B/C** | **HISTORIQUE_GIT — NON RETENUES** | [Habillages A/B/C](https://github.com/Kharn27/ESP32-OTA-Battleship/blob/a768e6cabbdfba1b793f3c77c0d75f0dae001c54/assets/explorations/2026-10-29-attaque-themes/README.md), [PR #1](https://github.com/Kharn27/ESP32-OTA-Battleship/pull/1) |
| **Archives d'agencements D/E/F** | **HISTORIQUE_GIT — NON RETENUES** | [Agencements D/E/F](https://github.com/Kharn27/ESP32-OTA-Battleship/blob/a768e6cabbdfba1b793f3c77c0d75f0dae001c54/assets/explorations/2026-10-29-attaque-layouts/README.md), [PR #1](https://github.com/Kharn27/ESP32-OTA-Battleship/pull/1) |
| **Archives UX antérieures à LOT1-UX-001** | **HISTORIQUE_GIT — NON APPROUVÉES VISUELLEMENT** | Trois anciens SVG de combat + quatre reconstitutions UX [au commit figé](https://github.com/Kharn27/ESP32-OTA-Battleship/tree/a768e6cabbdfba1b793f3c77c0d75f0dae001c54/assets/maquettes) |

**Traçabilité des scènes narratives retirées :** [PR #2 fusionnée](https://github.com/Kharn27/ESP32-OTA-Battleship/pull/2), [dossier au commit `872496d73a150dd5a016c219973e5865576b7e04`](https://github.com/Kharn27/ESP32-OTA-Battleship/tree/872496d73a150dd5a016c219973e5865576b7e04/assets/explorations/2026-10-29-bataille-en-cours) et [README historique, originaux PNG, exports 320 × 240 et manifeste](https://github.com/Kharn27/ESP32-OTA-Battleship/blob/872496d73a150dd5a016c219973e5865576b7e04/assets/explorations/2026-10-29-bataille-en-cours/README.md). Ces illustrations restent consultables dans Git ; elles ne sont ni des pistes actives, ni des écrans de gameplay. Leur retrait ne réécrit aucun historique.

**Décision PO du 29 octobre 2026 :** les 16 fichiers narratifs avaient déjà été retirés de `main` ; le retrait des **7 maquettes UX** et **10 fichiers A–F** est à présent autorisé. Les 17 fichiers restent retrouvables au [commit figé](https://github.com/Kharn27/ESP32-OTA-Battleship/tree/a768e6cabbdfba1b793f3c77c0d75f0dae001c54/assets), et A–F via la [PR #1](https://github.com/Kharn27/ESP32-OTA-Battleship/pull/1). Ce nettoyage ne valide **aucune politique générale future de conservation des assets** et ne modifie aucune règle de jeu.

**Valeur UX historique ≠ approbation visuelle.** Les documents peuvent décrire un principe fonctionnel conservé, mais leurs couleurs, leurs illustrations, leurs composants ou leurs mises en page n'ont pas été acceptés comme direction de production. Les anciens écrans à **deux grilles simultanément visibles** contredisent explicitement la décision **LOT1-UX-001 : une seule grille ATTAQUE ou DÉFENSE à la fois**.

**Cap artistique recherché après revue PO :** jeu indépendant contemporain en pixel art **riche, expressif, immersif**, avec une vraie personnalité maritime. Il s'agit d'une **intention**, pas d'une charte validée. La proposition de thèmes interchangeables **n'est pas une fonctionnalité décidée**.

Les règles du gameplay LOT 1 sont conservées, mais **l'adversaire a changé en v0.12 : VS Capitaine** (Mousse et Amiral sont reportés). Voir [BIBLE_PRODUIT.md](BIBLE_PRODUIT.md) : classique 10 × 10 VS Capitaine, mono-tir simultané, FEU irrévocable, aucune bascule automatique, alerte DÉFENSE persistante jusqu'à consultation, positions adverses cachées.

---

**État de cette galerie : archives de maquettes historiques, non validées comme spécifications graphiques.**  
**Format des maquettes vectorielles historiques :** SVG **320 × 240**, orientation paysage, destinés à illustrer l'UX sur la vraie CYD classique.

Les sept visuels restent [consultables dans l'historique Git](https://github.com/Kharn27/ESP32-OTA-Battleship/tree/a768e6cabbdfba1b793f3c77c0d75f0dae001c54/assets/maquettes), et non dans l'arborescence courante. Ils documentent les échanges UX passés sans servir de référence artistique active. Aucun ne prouve l'ergonomie tactile réelle ; elle devra être testée au stylet.

## A. Maquettes UX historiques — accessibles dans Git

Ces **trois fichiers** proviennent du [dossier I-009 dans Le Grenier Geek](https://github.com/Kharn27/Le-Grenier-Geek/tree/main/plans/I-009-bataille-navale-cyd/assets) et ont été recopiés sans modification.

### Classique — version historique

![Maquette classique historique — 320×240](https://raw.githubusercontent.com/Kharn27/ESP32-OTA-Battleship/a768e6cabbdfba1b793f3c77c0d75f0dae001c54/assets/maquettes/mode-classique-320x240.svg)

**Attention, règle devenue historique :** l'image suggère le tour par tour traditionnel. Depuis, le **mono-tir simultané est le standard décidé**, et l'alternance n'est plus qu'une variante facultative. Le dessin reste une **archive historique UX, et non une référence d'identité graphique**, mais **sa présentation simultanée de deux grilles est désormais écartée pour la V1.0** : une seule grande grille est visible à la fois.

### Simultané — concept d'origine

![Maquette historique du duel simultané — 320×240](https://raw.githubusercontent.com/Kharn27/ESP32-OTA-Battleship/a768e6cabbdfba1b793f3c77c0d75f0dae001c54/assets/maquettes/mode-simultane-320x240.svg)

**Intention toujours pertinente :** deux cibles secrètes et résolution commune après validation des deux joueurs. **Attention : la disposition de deux océans à l'écran n'est plus la règle d'interface V1.0.** Les commandes, dimensions des cases et couleurs ne sont pas figées.

### Salves — concept d'origine

![Maquette historique des salves — 320×240](https://raw.githubusercontent.com/Kharn27/ESP32-OTA-Battleship/a768e6cabbdfba1b793f3c77c0d75f0dae001c54/assets/maquettes/mode-salves-320x240.svg)

**Attention, chiffres non contractuels :** les indications de puissance de feu figurant sur cette ancienne maquette ne reflètent pas nécessairement la règle tactique désormais retenue (contribution de chaque navire encore à flot). Les grilles tactiques 15 × 15 restent **à arbitrer/tester**.

## B. Esquisses reconstituées depuis nos échanges

Les quatre visuels ci-dessous sont des **reconstitutions statiques de concepts discutés dans le chat**, et non des copies pixel pour pixel des illustrations ou éléments interactifs affichés lors de la conversation. Leur conservation dans l'historique Git permet de retrouver ces idées sans maintenir les fichiers dans `main`.

### Accueil — CRÉER / REJOINDRE / VS CPU

![Proposition d'écran d'accueil — 320×240](https://raw.githubusercontent.com/Kharn27/ESP32-OTA-Battleship/a768e6cabbdfba1b793f3c77c0d75f0dae001c54/assets/maquettes/accueil-creer-rejoindre-320x240.svg)

**Décidé côté fonctionnel :** accès volontaire CRÉER / REJOINDRE ; VS CPU dans la cible V1.0. **Proposé côté interface :** disposition exacte des trois choix. La découverte ESP-NOW reste à valider sur deux CYD physiques.

### Placement — AUTO, rotation, validation et erreur

![Esquisse de placement sur grille — 320×240](https://raw.githubusercontent.com/Kharn27/ESP32-OTA-Battleship/a768e6cabbdfba1b793f3c77c0d75f0dae001c54/assets/maquettes/placement-flotte-320x240.svg)

**Décidé côté fonctionnel pour la V1.0 :** flotte préremplie, AUTO répétable, déplacement par sélection puis destination, rotation même invalide, rouge si invalide, ✓ / ✕ et PRÊT indisponible tant qu'une édition est en cours ou invalide. **Attention LOT 1 : seul AUTO + PRÊT est retenu**, sans déplacement ni rotation manuels. **Proposé côté interface :** tailles et positionnement des boutons.

### Fiche adverse — palmarès de la CYD

![Esquisse de palmarès adverse — 320×240](https://raw.githubusercontent.com/Kharn27/ESP32-OTA-Battleship/a768e6cabbdfba1b793f3c77c0d75f0dae001c54/assets/maquettes/fiche-adversaire-320x240.svg)

**Décidé côté fonctionnel :** palmarès local propre à la CYD et consultation **à la demande**, jamais imposée ; **quatre catégories** Humain / Mousse / Capitaine / Amiral et **un compteur global** de forfaits volontaires. **Fictif :** chiffres et nom affichés ; la maquette antérieure ne représente pas encore cette ventilation ; navigation exacte et titres honorifiques non décidés.

### Victoire — révélation facultative de la flotte

![Esquisse de fin de partie — 320×240](https://raw.githubusercontent.com/Kharn27/ESP32-OTA-Battleship/a768e6cabbdfba1b793f3c77c0d75f0dae001c54/assets/maquettes/fin-partie-secret-320x240.svg)

**Décidé :** la disposition complète de la flotte victorieuse **reste secrète par défaut** ; le vainqueur peut la révéler volontairement après le combat. En cas de destruction réciproque complète, toutes les cases des deux flottes sont déjà connues ; proposer une option analogue ne révélerait donc rien de neuf. L'écran et ses libellés restent proposés.

## C. Décisions UX nouvelles à illustrer — 29 octobre 2026

**DÉCIDÉ pour la V1.0 (LOT1-UX-001/002/003) :**
- En combat, **une seule grande grille à la fois** : ATTAQUE (pour cibler) ou DÉFENSE (pour inspecter les navires et impacts). Bascule volontaire, immédiate.
- La résolution simultanée des tirs **ne provoque aucune bascule automatique**.
- Un **indicateur intelligent DÉFENSE** montre le tir manqué de façon discrète, un touché de façon visible et un coulé de façon renforcée. Les dégâts non consultés maintiennent une alerte jusqu'à l'ouverture de DÉFENSE.
- Le **LOT 1** cible désormais une bataille classique contre **Capitaine sur une seule CYD**, avec placement automatique **AUTO + PRÊT**, sans manipulation individuelle des navires.

**Statut graphique des écrans de combat : À CONCEVOIR.** Les anciens SVG conservés dans Git et les petits schémas interactifs de conversation ne constituent **pas** une maquette validée de la nouvelle disposition. La mission artistique distincte de l'**accueil LOT 1** a depuis été livrée et fusionnée ([PR #8](https://github.com/Kharn27/ESP32-OTA-Battleship/pull/8)) au stade **DESIGN_PREVIEW** ; cela ne valide ni ne déclenche la conception des écrans de combat. Voir la [Bible produit actuelle](BIBLE_PRODUIT.md), § 14, pour le périmètre fonctionnel LOT 1.

### Actualisation documentaire du 29 octobre 2026 — LOT 1

**HISTORIQUE — cadrage de la Bible v0.7, remplacé quant à l'adversaire en v0.12 :** fin de partie après résolution des deux tirs, match nul par destruction mutuelle, REJOUER avec conservation par défaut de la formation du joueur, nouvelle flotte aléatoire de Mousse, palmarès VS Mousse et abandon après PRÊT, IA débutante mémorisant ses pistes, navires autorisés à se toucher et épaves non identifiées, inspection libre post-partie et gravité maximale de l'indicateur DÉFENSE. L'accueil minimal du LOT 1 affiche seulement JOUER CONTRE MOUSSE et PALMARÈS.

**Statut visuel des sept SVG historiques inchangé :** ces esquisses ou reconstitutions restent **non validées graphiquement**, consultables dans Git uniquement ; aucune ne prétend représenter toutes les nouvelles règles. L'accueil actif est désormais la **maquette de conception retenue** (PR #8) ; la charte globale et les autres écrans 320 × 240 restent **À CONCEVOIR / À VÉRIFIER**.

## D. À ne pas confondre

Les **deux scènes narratives de la PR #2** ont été retirées de `main` par décision PO et restent accessibles dans [l'historique Git](https://github.com/Kharn27/ESP32-OTA-Battleship/tree/872496d73a150dd5a016c219973e5865576b7e04/assets/explorations/2026-10-29-bataille-en-cours). Les éléments interactifs présentés dans le chat (simulateurs, petits schémas) ne sont pas automatiquement enregistrés comme des images exportables.

Cet index décrit les **trois SVG historiques**, **quatre reconstitutions UX** et **six explorations A–F écartées**, tous retirés de `main` mais retrouvables au [commit figé](https://github.com/Kharn27/ESP32-OTA-Battleship/tree/a768e6cabbdfba1b793f3c77c0d75f0dae001c54) ; il conserve aussi un lien historique vers les **deux scènes narratives retirées**. Il ne prétend pas archiver toutes les images temporaires ou les composants interactifs de la conversation.

## E. Suite de la conception

La politique générale de conservation et de référencement des **futurs visuels** reste **À ARBITRER par le PO**. Le retrait des fichiers historiques n'autorise ni suppression d'un asset actif ni changement du processus de validation graphique.

**Aucune mission de graphisme ou de développement n'est déclenchée par cette galerie.**

## F. Archive historique A/B/C — NON RETENUE COMME RÉFÉRENCE VISUELLE

Le [dossier A/B/C au commit figé](https://github.com/Kharn27/ESP32-OTA-Battleship/tree/a768e6cabbdfba1b793f3c77c0d75f0dae001c54/assets/explorations/2026-10-29-attaque-themes) conserve dans l'historique Git trois anciens habillages de **l'unique grille ATTAQUE** du LOT 1 (classique 10 × 10, VS Mousse, mono-tir simultané), conformes aux décisions `LOT1-UX-001/002/003`.

- **A — Abysses électriques** : océan profond, cyan bioluminescent et corail.
- **B — Archipel** : aventure lumineuse, azur et teintes chaudes.
- **C — Escadre** : stratégie contemporaine, acier et cuivre.

**Classement Pilotage :** trois explorations historiquement appréciées, mais **non retenues comme références visuelles de production**. La possibilité de thèmes interchangeables a été évoquée sans jamais être décidée. Les essais d'agencements D/E/F sont également archivés, et aucune règle n'a changé.

Ces sources SVG retirées de `main` sont **des previews PC 320 × 240, non des assets runtime et non testées sur CYD physique**. L'intégration au produit nécessite une revue du Pilotage, une validation PO et des essais matériels.

## G. Archive historique D/E/F — NON RETENUE COMME RÉFÉRENCE VISUELLE

Le [dossier D/E/F au commit figé](https://github.com/Kharn27/ESP32-OTA-Battleship/tree/a768e6cabbdfba1b793f3c77c0d75f0dae001c54/assets/explorations/2026-10-29-attaque-layouts) documente dans l'historique Git **trois autres dispositions anciennement proposées** indépendantes des thèmes A/B/C : **D grille centrale**, **E navigation latérale**, **F commandes basses**. Les trois SVG **320 × 240** restent visibles dans l'historique GitHub, mais plus sur `main`. Ils conservent les contraintes de `LOT1-UX-001/002/003`, le classique 10 × 10 contre Mousse, la sélection E6, les impacts antérieurs C3 et H8 et l'alerte de DÉFENSE non consultée.

**Statut : EXPLORATION_HISTORIQUE — NON_RETENUE_COMME_REFERENCE_VISUELLE — NON_VALIDEE_PO.** Les PNG/zooms locaux ne sont pas versionnés ; les trois sources SVG et leurs notes le sont. Aucun choix d'agencement, de thème interchangeable ou de rendu technique n'est validé. Aucun test sur CYD.


## H. CYD WARSHIP — maquette d'accueil retenue au stade conception (cadrage initial du 29 octobre 2026)

**DÉCIDÉ — intention PO et actualisation après v0.15 :** titre **CYD WARSHIP** en partie haute, visible mais discret ; scène **fixe** au coucher de soleil, navire du joueur au premier plan, navire ennemi lointain endommagé, tir vers l'horizon et **riposte ratée (gerbe d'eau) près du navire principal**. Les deux commandes de l'accueil sont désormais **JOUER** et **PALMARÈS**, présentées sur deux plaques de même taille dans la maquette sélectionnée. **JOUER** ouvre dès le LOT 1 la sélection d'adversaire, **Capitaine seul jouable**. Le placement exact des zones tactiles reste à tester sur CYD.

**Historique — pistes facultatives du cadrage initial :** avion en feu chutant vers la mer et escadrille en silhouettes près du coucher de soleil. La **composition de l'accueil a ensuite été retenue par le PO** dans la PR #8 et n'est plus « à dessiner » ; cette acceptation ne valide pas le rendu sur CYD physique.

**GRAPH-ACC-001 — LIVRÉE ET FUSIONNÉE :** [maquette d'accueil 320 × 240 retenue par le PO](design/graph-acc-001/accueil_cyd_warship_320x240.png), [PR #8](https://github.com/Kharn27/ESP32-OTA-Battleship/pull/8), fusion `50d9e1dabd71725adba50279929694123fde0fda`. **Statut : DESIGN_PREVIEW accepté / ON_DEVICE_UNVERIFIED** ; les dimensions d'export sont vérifiées, pas la lisibilité tactile sur la vraie CYD, ni l'intégration runtime. Les archives retirées restent consultables dans Git ; aucune exploration rejetée n'est réactivée.

**Référence détaillée :** [BIBLE_PRODUIT.md, § 15](BIBLE_PRODUIT.md). Aucun autre écran, thème ou code n'est demandé par ce cadrage.

**Actualisation v0.12 — 29/10/2026 (historique conservé) :** le PO a choisi **Capitaine pour le LOT 1** et reporté **Mousse et Amiral**. Les descriptions d'anciens SVG explicitement figés « VS Mousse » ci-dessus décrivent **l'historique de ces créations**, pas le périmètre de production actuel. Cette correction v0.12 n'approuvait aucune maquette. Le brief initial de l'issue #6 mentionnait encore Mousse ; après itérations et décision PO, la PR #8 a été fusionnée avec les libellés JOUER / PALMARÈS. La documentation du parcours est suivie par [DOC-ACC-001, issue #9](https://github.com/Kharn27/ESP32-OTA-Battleship/issues/9).
