# G01 — Contrat graphique initial (PROPOSÉ, non normatif)

**Statut :** `DESIGN_PREVIEW / ON_DEVICE_UNVERIFIED / EN_ATTENTE_ARBITRAGE_PO`.

## Principes communs

- **Atmosphère :** pixel art indépendant contemporain, coucher de soleil chaud contre bleu de mer profond ; cuivre pour les contours, écume cyan pâle pour les points de contact avec l'eau. L'accueil PR #8 reste inchangé.
- **Ordre de lecture :** grille → cible/résultat → commande principale → indicateur DÉFENSE. Aucune double grille.
- **Typographie de preview :** titres fonctionnels 11–14 px, valeurs d'action 24 px, légendes 9–10 px. Les valeurs finales restent à **essayer réellement sur 320×240**. Dépendance hôte : DejaVu Sans Condensed ; **aucune police embarquée validée**.
- **Hiérarchie des commandes :** action principale chaude (FEU, PRÊT) ; navigation et options en ardoise ; avertissement ambre ; coulé/dégât rouge. Jamais couleur seule : libellé ou forme additionnelle.
- **Cases d'essai :** grille de 10×10, cases de **18×18 pixels dans ces previews** (coordonnées 22,48 à 202,228). Ce sont des valeurs de **concept**, pas une cible tactile minimale imposée. Précision, tolérance et calibrage S02 restent ouverts.
- **Mer :** matière animable avec parcimonie ; sur les cases elle est volontairement plus silencieuse que le décor d'accueil. Trajectoire non dessinée sur la grille, gerbe liée au point d'impact.

### Palette de travail (hex RVB, à arbitrer PO)

| Usage | Couleur | HEX |
|---|---|---|
| Nuit/contour | Navy | `#081E31` |
| Panneau bleu | Navy 2 | `#0C2D45` |
| Eau | Sea | `#0C5070` |
| Eau claire | Sea 2 | `#156686` |
| Accent de sélection | Cyan | `#5BDCF0` |
| Écume | Foam | `#C3E8EF` |
| Laiton | Brass | `#D59C50` |
| Soleil | Sun | `#F6BE68` |
| Action | Orange | `#E97935` |
| Impact / gravité haute | Red | `#ED6650` |
| Alerte intermédiaire | Amber | `#FFCA65` |
| Texte | White | `#F3F1DC` |

## Étapes et états UI — couverture G01, non écrans finalisés

| Parcours / état | Convention proposée | Invariant produit |
|---|---|---|
| Déploiement | Panneau flottille sur eau calme ; `AUTO` secondaire, `PRÊT` principal, retour accueil identifiable | Flotte préplacée ; sortie avant PRÊT sans forfait, y compris revanche ; pas de placement individuel LOT 1 |
| ATTAQUE — inconnue | Fond de mer neutre ; coordonnées non surchargées | Case encore ciblable |
| ATTAQUE — sélection | Quatre coins cyan + repère central clair ; FEU devient l'action saillante | Désélection ou changement avant FEU ; sélection conservée en aller-retour DÉFENSE |
| FEU / verrouillage | Plaque FEU orange, animation transitoire discrète à définir | Verrouillage irrévocable ; pas de nouvelle sélection, bascule ou abandon pendant animation ; révélation synchronisée |
| Tir manqué | Petite écume blanche intégrée à la case ; aucune trajectoire dessinée | Historique conservé, case non sélectionnable |
| Tir touché | Marque rouge croisée et point chaud ; distincte de l'écume | Historique conservé, case non sélectionnable |
| Adversaire coulé sur ATTAQUE | Message texte « [TYPE] coulé » ; **aucune coque, aucun contour de groupe ou ombre d'épave** | Type annoncé, appartenance des cases secrète ; navires adjacents possibles |
| DÉFENSE — flotte | Silhouettes/bandeaux d'acier lisibles **uniquement sur sa propre grille** ; marqueur de dégâts superposé | Cinq navires de 2/3/3/4/5 cases visibles |
| DÉFENSE — commande non consultée | Repère de gravité : discret pour manqué, ambre pour touché, rouge pour coulé ; **sans compteur** | Gravité maximale non consultée ; résumé limité à dernière riposte |
| DÉFENSE — ouverture | Repère d'alerte retiré/acquitté ; résumé consulté distinct des impacts permanents | Acquitte l'alerte et le résumé de la commande sans effacer l'historique |
| Verdict | Cartouche superposé transitoire « VICTOIRE / DÉFAITE / MATCH NUL », puis cartouche compact | Sur champ de bataille après les deux impacts, pas d'écran obligatoire |
| Après verdict | `REJOUER` / `ACCUEIL`, vues consultables ; FEU non disponible | Aucun nouveau tir ; verdict et palmarès inchangés |
| Défaite VS Capitaine | `VOIR SA FLOTTE` discret et persistant, puis commande `AFFICHER / MASQUER` | Révélation facultative sur **ATTAQUE existante** ; cinq navires adverses visibles uniquement après demande ; impacts devant ; état conservé entre vues |
| Palmarès | Plaques/rangs doux, chiffres lisibles, bilan « VS CAPITAINE » | Parties, victoires, défaites, nuls et unique compteur global de forfaits |
| Abandon | Confirmation simple `CONTINUER / ABANDONNER` | Seulement en partie commencée avant verdict ; indisponible pendant animation |

### Trois niveaux DÉFENSE (pas un compteur)

1. **Discret :** dernière riposte manquée, sans autre événement non consulté plus grave ; contraste bleu/ardoise.
2. **Visible :** au moins un touché non consulté, pas de coulé non consulté ; ambre, sans chiffre.
3. **Renforcé :** un coulé non consulté ; rouge, sans chiffre.

L'onglet peut conserver la gravité d'un événement plus ancien même si **la dernière riposte** affichée est manquée. Ouvrir DÉFENSE acquitte les deux informations, puis le bouton revient à son aspect normal. Les exemples de couleurs sont des **propositions**, les règles sont celles de la Bible.

## Contrat d'assets **provisoire** — à négocier Pilotage + Développeur

| Identifiant candidat | Source/états proposés | Dimension conceptuelle | Fond / transparence | Export / mémoire |
|---|---|---|---|---|
| `ui.tab.attack` / `ui.tab.defense` | actif, inactif, attention | Boîtes 100×28 px (preview) | Opaque ; 1–2 petits overlays éventuels | Non figé |
| `ui.action.fire` | disponible, verrouillé/inactif, pressé | 110×51 px (preview) | Opaque / fond local | Non figé |
| `grid.10x10.cell` | eau, sélection, raté, touché | 18×18 px (preview) | Possiblement couches distinctes | Non figé |
| `grid.10x10.ship.player` | segment/bandeau, endommagé, coulé | 18×18 par case (preview) | Transparence potentielle, à valider | Non figé |
| `ui.defense.alert` | discret, touché, coulé, acquitté | Coin ~12×12 px (preview) | Superposé, transparence à préciser | Non figé |
| `ui.panel.feedback` | texte dernier bilan, riposte | 108×60–70 px (preview) | Fond opaque | Non figé |
| `ui.verdict` / `ui.inspect` / `ui.stats` | états à illustrer en G02–G04 | À déterminer | À déterminer | Non figé |

**Formats possibles à discuter, non imposés :** rendu par primitives C pour grille/contours/textes, sprites dédiés pour détails, petites régions statiques en Flash, éventuelle transparence par masque ; profondeur, encodage, compression, décodeur, table de glyphes et budget Flash/RAM inconnus. Les PNG RGB de prévisualisation **ne sont pas des assets firmware validés**. Ne pas déduire le coût RAM des tailles de PNG. L'absence de PSRAM et de microSD est prise en compte, sans inventer de plafond.

## Décisions attendues

**PO :** palette chaud/froid, style des coques du joueur, force des impacts, visibilité relative des trois niveaux d'alerte, rythme des transitions et hiérarchie de commande. Rien n'est déclaré approuvé pour le combat.

**S02 / matériel :** lisibilité des 9–10 px, touchabilité des 18×18, activation sûre de FEU, précision des bords, contraste de dalle, retours de sélection, scintillement et animation brève. L'objectif est de **falsifier** ce dimensionnement, pas de le décréter ergonomique.

**Pilotage + Développeur :** IDs finaux, formats, transparence, stratégie d'affichage, budgets mesurés, mémoire, partition et interfaces. G02–G04 feront les écrans de production après arbitrage.