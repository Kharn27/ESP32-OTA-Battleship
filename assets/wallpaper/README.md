# Wallpaper d’accueil retenu — CYD WARSHIP

Ce répertoire conserve **uniquement le fond de scène exploitable séparément par le développeur** pour l’accueil actuel : aucune inscription, aucun bouton ni logo n’est peint dans cette image.

## Original — 1195 × 896 px

![Fond d’accueil original — JPEG](accueil_retenu_original_1195x896.jpeg)

## Version CYD — 320 × 240 px

![Fond d’accueil CYD](accueil_retenu_CYD_320x240.png)

Le JPEG original est repris octet par octet de [`draft/Wallpaper_accueil.jpeg`](../../draft/Wallpaper_accueil.jpeg), et la version native CYD en est une réduction LANCZOS reproductible. Les éléments interactifs sont des sprites individuels dans [`assets/logo/`](../logo/) et [`assets/boutons/`](../boutons/).

**L’ancienne composition complète**, avec titre et boutons incrustés, n’est **pas** un wallpaper : elle se trouve exclusivement dans [`maquettes/accueil-historique/`](../../maquettes/accueil-historique/legacy/README.md), conformément à la [décision PO #107](https://github.com/Kharn27/ESP32-OTA-Battleship/issues/107).

Voir le [manifeste SHA-256](../manifest.json), le [script de vérification](../../scripts/export_wallpapers.py) et le [GO PC de l’accueil actuel](https://github.com/Kharn27/ESP32-OTA-Battleship/pull/90#issuecomment-6740684237). **Rendu et intégration sur CYD physique non vérifiés** ; les deux exports restent `ON_DEVICE_UNVERIFIED`.
