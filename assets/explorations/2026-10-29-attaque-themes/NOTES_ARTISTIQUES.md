# Notes artistiques historiques A/B/C — NON RETENUES COMME RÉFÉRENCE VISUELLE

> **ARCHIVE — `EXPLORATION_HISTORIQUE — NON_RETENUE_COMME_REFERENCE_VISUELLE — NON_VALIDEE_PO`.** Texte conservé comme trace des intentions et hypothèses créatives antérieures. Les ambiances et dispositions décrites ci-dessous sont **écartées comme référence visuelle de production**. Les mentions d'améliorations/animations et compromis sont des pistes **historiques non décidées**, non des recommandations actives. Aucun test sur CYD ni validation PO. Le cap artistique ultérieur vise un **pixel art indie moderne, riche, expressif et immersif**, sans charte approuvée.

**Situation de référence** : classique 10 × 10 contre Mousse, résolution mono-tir simultané, ATTAQUE affichée, E6 sélectionné (non tiré), C3 touché auparavant, H8 manqué auparavant. Bouton FEU = verrouillage irrévocable du tir. Bouton DÉFENSE = navigation volontaire, avec une alerte de navire touché non consultée. Les illustrations de navires hors grille représentent une vignette d'ambiance « votre escadre », jamais une fuite sur les emplacements secrets ennemis.

## A — Abysses électriques

**Intention.** Un sous-marin bioluminescent surgit dans des eaux profondes. Les panneaux ressemblent à des pièces d'un petit bathyscaphe de fiction, sans chercher l'interface industrielle. Éclairage cyan/turquoise, eau pétrole, éclats de corail. La grille ressemble à une coupe sombre de l'océan ; le réticule clair est un petit instrument de visée.

**Palette illustrative.** Fond `#061724`, eau `#0A4150`, turquoise `#57F1D8`, impact `#EE6952`, sélection `#FFEFAC`, texte ivoire `#E5FFF6`. Typo : sans-serif compacte, titres capitales, micro-libellés en contraste secondaire. Navires : silhouettes pixelisées en profil, phares, hublots, quelques reflets métalliques. Impacts : éclats corail et noyau ivoire ; eau ratée : losange creux blanc.

**Micro-animations envisageables.** (1) Sélection : deux pulsations courtes des coins du réticule, sans modifier la case ; (2) impact : quatre débris corail s'écartant sur 3 à 5 images ; (3) sous-marin décoratif : un hublot ou deux s'allument ponctuellement.

**Compromis.** Très immersif sur fond sombre mais sensible à la qualité de luminosité du TFT. Le cyan vif doit rester distinct des reflets de l'eau. Contrôle de contraste impératif sur matériel.

## B — Archipel

**Intention.** Une aventure maritime au soleil levant : carte de navigation animée et petit navire expressif. Soleil, eau azur, ivoire chaud et boutons orange. Un jeu familial assumé, sans basculer dans une interface enfantine. Composition miroir : la grille est à droite pour que le décor et FEU forment une colonne à gauche.

**Palette illustrative.** Fond mer `#73D3E5`, eau `#2CA2B8`, ivoire `#FFF0BF`, orange `#F8834B`, impact `#EC6E4B`, texte bleu pétrole `#084D74`. Typo : capitales claires avec grands labels d'action, titres plus ludiques. Navires : grandes masses colorées et ombrage en quelques bandes de pixels. Impacts : braises chaudes, sélection ivoire cadrée.

**Micro-animations envisageables.** (1) La vague de sillage parcourt discrètement la vignette du navire ; (2) une gerbe d'écume blanche de 3 images apparaît sur un tir manqué ; (3) le soleil scintille sur 2 pixels d'eau sans gêner les marqueurs.

**Compromis.** Une palette claire donne de l'énergie, mais les grilles pourraient demander un assombrissement pour conserver la lisibilité sur TFT. Le voilier illustratif évoque plus l'aventure que la flotte moderne (sous-marin et porte-avions) : à faire évoluer si cette piste plaît.

## C — Escadre

**Intention.** Tactique navale haut de gamme : destroyer stylisé, métal patiné, mer ardoise et détails acier/sable. Sobriété contrôlée : davantage de personnalité qu'un tableau de bord militaire, avec des textures, une vraie silhouette de navire et une commande FEU chaleureuse.

**Palette illustrative.** Fond `#101C27`, eau `#245160`, acier sauge `#9BD8D0`, bouton cuivre `#DB8A5C`, sélection sable `#FFE5A0`, texte crème `#E6E8CE`. Typo : capitale technique lisible, espacements réguliers. Navires : coque anguleuse et superstructures en profil, plus de micro-détails que les autres pistes. Impacts : explosion corail et marqueur de raté creux.

**Micro-animations envisageables.** (1) Réticule qui se resserre sur E6 en 3 étapes ; (2) coque ennemie touchée : court flash de 2 images sur la case puis impact statique ; (3) l'alerte DÉFENSE oscille très légèrement entre deux intensités jusqu'à consultation.

**Compromis.** La discipline tactique peut sembler plus austère que les deux autres pistes. Préserver l'expressivité de l'eau et des navires lors d'une future déclinaison de tous les écrans.

## Ergonomie et preuve de réalisation

- Résolution de chaque écran exporté : **320 × 240 pixels exactement**. Zoom 4× par plus proche voisin.
- Grille de **10 colonnes × 10 lignes**, cases de **15 × 15 pixels** ; à valider au stylet sur CYD physique.
- FEU est visible et largement plus grand qu'une case ; les cases déjà jouées sont indiquées par des symboles qui diffèrent aussi par **la forme**.
- La flotte ennemie inconnue n'est jamais dessinée.
- Les SVG sont des **sources d'exploration éditables**, pas des ressources firmware optimisées. Tout chiffrage de RAM runtime, Flash embarquée ou fluidité de l'ESP32 serait prématuré.
- Les PNG sont vérifiés informatiquement. **Aucun test sur écran CYD réel, aucune compilation de firmware.**
- **EXPLORATION_HISTORIQUE — NON_RETENUE_COMME_REFERENCE_VISUELLE — NON_VALIDEE_PO.** Les contrôles antérieurs ne prouvent ni rendu firmware ni acceptation.