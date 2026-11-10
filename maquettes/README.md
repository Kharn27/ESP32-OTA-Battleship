# Bibliothèque de maquettes d'écrans

Ce répertoire stocke les **compositions d'écrans**, pas les boutons, navires, fonds ni autres images réutilisables.

Chaque `maquettes/<nom-ecran>/` doit contenir un `screen.json` conforme au [contrat graphique](../docs/GRAPHIC_PIPELINE.md), un aperçu **320 × 240**, des informations de placement et les sources éditables. Toutes les images constitutives se référencent par leurs **IDs dans `assets/manifest.json`**. Les PNG aplatis de maquette sont exclusivement des **prévisualisations PC**, non des images runtime par défaut.

La validation artistique de l'écran est séparée de la validation de chaque asset et des preuves physiques CYD. Aucun écran existant n'a encore été déplacé par cette PR de gouvernance.
