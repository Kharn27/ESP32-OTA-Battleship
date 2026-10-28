# Galerie des maquettes — I-009 Bataille navale CYD

**État : explorations visuelles, non validées comme spécifications graphiques.**  
**Format :** tous les fichiers de cette galerie sont des SVG **320 × 240**, orientation paysage, destinés à illustrer l'UX sur la vraie CYD classique.

Les visuels sont versionnés dans [assets/maquettes](assets/maquettes/). Ils servent de repères pour les discussions avec le PO et pour une future direction artistique. Aucun ne prouve l'ergonomie tactile réelle ; elle devra être testée au stylet.

## A. Maquettes historiques conservées à l'identique

Ces **trois fichiers** proviennent du [dossier I-009 dans Le Grenier Geek](https://github.com/Kharn27/Le-Grenier-Geek/tree/main/plans/I-009-bataille-navale-cyd/assets) et ont été recopiés sans modification.

### Classique — version historique

![Maquette classique historique — 320×240](assets/maquettes/mode-classique-320x240.svg)

**Attention, règle devenue historique :** l'image suggère le tour par tour traditionnel. Depuis, le **mono-tir simultané est le standard décidé**, et l'alternance n'est plus qu'une variante facultative. Le dessin reste utile pour les deux grilles et l'identité graphique.

### Simultané — concept d'origine

![Maquette historique du duel simultané — 320×240](assets/maquettes/mode-simultane-320x240.svg)

**Intention toujours pertinente :** deux cibles secrètes et résolution commune après validation des deux joueurs. Les commandes, dimensions des cases et couleurs ne sont pas figées.

### Salves — concept d'origine

![Maquette historique des salves — 320×240](assets/maquettes/mode-salves-320x240.svg)

**Attention, chiffres non contractuels :** les indications de puissance de feu figurant sur cette ancienne maquette ne reflètent pas nécessairement la règle tactique désormais retenue (contribution de chaque navire encore à flot). Les grilles tactiques 15 × 15 restent **à arbitrer/tester**.

## B. Esquisses reconstituées depuis nos échanges

Les quatre visuels ci-dessous sont des **reconstitutions statiques de concepts discutés dans le chat**, et non des copies pixel pour pixel des illustrations ou éléments interactifs affichés lors de la conversation. Leur réalisation dans le dépôt permet de ne pas perdre ces idées.

### Accueil — CRÉER / REJOINDRE / VS CPU

![Proposition d'écran d'accueil — 320×240](assets/maquettes/accueil-creer-rejoindre-320x240.svg)

**Décidé côté fonctionnel :** accès volontaire CRÉER / REJOINDRE ; VS CPU dans la cible V1.0. **Proposé côté interface :** disposition exacte des trois choix. La découverte ESP-NOW reste à valider sur deux CYD physiques.

### Placement — AUTO, rotation, validation et erreur

![Esquisse de placement sur grille — 320×240](assets/maquettes/placement-flotte-320x240.svg)

**Décidé côté fonctionnel :** flotte préremplie, AUTO répétable, déplacement par sélection puis destination, rotation même invalide, rouge si invalide, ✓ / ✕ et PRÊT indisponible tant qu'une édition est en cours ou invalide. **Proposé côté interface :** tailles et positionnement des boutons.

### Fiche adverse — palmarès de la CYD

![Esquisse de palmarès adverse — 320×240](assets/maquettes/fiche-adversaire-320x240.svg)

**Décidé côté fonctionnel :** palmarès local propre à la CYD et consultation **à la demande**, jamais imposée. **Fictif :** chiffres et nom affichés ; navigation exacte et titres honorifiques non décidés.

### Victoire — révélation facultative de la flotte

![Esquisse de fin de partie — 320×240](assets/maquettes/fin-partie-secret-320x240.svg)

**Décidé :** la disposition complète de la flotte victorieuse **reste secrète par défaut** ; le vainqueur peut la révéler volontairement après le combat. En cas de destruction réciproque complète, toutes les cases des deux flottes sont déjà connues ; proposer une option analogue ne révélerait donc rien de neuf. L'écran et ses libellés restent proposés.

## C. À ne pas confondre

Certaines **illustrations d'ambiance affichées dans la conversation** provenaient de recherches visuelles temporaires. Les éléments interactifs présentés dans le chat (simulateurs, petits schémas) ne sont pas automatiquement enregistrés comme des images exportables.

Cette galerie conserve les **trois vrais SVG historiques** et **quatre reconstitutions UX**. Elle ne prétend pas archiver à l'identique toutes les images externes ou tous les composants interactifs de la conversation.

## D. Suite de la conception

Les maquettes futures devront être **créées comme fichiers conservables** (SVG/PNG selon le besoin), clairement étiquetées **PROPOSÉ** ou **DÉCIDÉ**, et référencées ici et dans la [bible produit](BIBLE_PRODUIT.md).

**Aucune mission de graphisme ou de développement n'est déclenchée par cette galerie.**
