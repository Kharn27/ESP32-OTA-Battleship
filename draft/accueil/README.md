# Accueil CYD WARSHIP — sources détourees (DRAFT)

**Statut : SOURCE DE TRAVAIL — NON HOMOLOGUÉE**. Ces éléments sont une correction technique préparatoire au premier essai de composition et de rendu CYD, **pas** des exports embarqués validés.

## Origine

Sources remises par le PO et conservées intactes dans `draft/` :
- `draft/Bouton_jouer.jpeg`
- `draft/Bouton_palmares.jpeg`
- `draft/Logo_titre.jpeg`
- `draft/Wallpaper_accueil.jpeg` — fond opaque conservé tel quel.

Les trois premiers fichiers sont des JPEG RGB de 1195 × 896 pixels contenant **un damier gris/blanc peint dans les pixels**. Ils ne comportent pas de canal alpha.

## Détourage livré

Fichiers produits avec Adobe — suppression de fond en PNG RGBA 8 bits, recadrage de la zone utile :

| Fichier | Pixels | Usage |
|---|---:|---|
| `Bouton_jouer_RGBA.png` | 1107 × 252 | Bouton JOUER |
| `Bouton_palmares_RGBA.png` | 1163 × 250 | Bouton PALMARÈS |
| `Logo_titre_RGBA.png` | 1075 × 489 | Logo CYD WARSHIP |

Contrôle réalisé : en-tête PNG, type de couleur 6 (RGBA), dimensions conformes et inspection visuelle sur fond sombre. **Un contrôle final des franges et de l'alpha sur fonds clairs/sombres à l'échelle 320 × 240 reste nécessaire.** Des franges résiduelles au détourage peuvent subsister. Ne pas extrapoler de cette source la qualité de rendu réelle sur TFT.

## Réutilisation

- Composer **un seul accueil 320 × 240 paysage** à partir du wallpaper et des trois éléments en calques indépendants.
- Conserver les libellés exacts **CYD WARSHIP**, **JOUER**, **PALMARÈS** et la direction artistique maritime retenue.
- Pouvoir déplacer/réduire chaque élément sans devoir repeindre le fond.
- Préserver le rapport largeur/hauteur des éléments ; ne pas inverser ou déformer le logo.
- Produire une capture 320 × 240 et un zoom ×4 au pixel, plus les calques/sources et coordonnées des deux zones tactiles.
- Le fond JPEG doit être redimensionné ou recadré en respectant sa composition ; ne pas dénaturer la scène.
- Le rendu alpha sur ESP32 (décodage, composition, RAM/Flash, temps de rendu) est **à qualifier avec le Développeur** ; ne pas traiter la réussite de la maquette comme une preuve firmware.

## Référence d'accueil précédente

`design/ecrans/accueil/README.md` et `design/ecrans/accueil/accueil_cyd_warship_320x240.png` sont la **maquette PO précédemment validée sur PC**, non homologuée matériellement. Cette préparation ne la remplace pas.

Animation GIF patrouilleur PAT-05 : **hors périmètre / reportée**.
