# D09 — Boot direct et réception tactile ciblée (Issue #43)

**Mandat Pilotage P1 du 02/11/2026.** Ce document remplace l'ancien
protocole avec sélecteur de modes T02/D09. La campagne T02-B (15 mesures)
est terminée et archivées dans [Issue #21](https://github.com/Kharn27/ESP32-OTA-Battleship/issues/21#issuecomment-6578800263).
**Aucun nouvel étalonnage ou photo de l'ancienne sonde n'est demandé.**
Le diagnostic T02 et T02_CAL ne font plus partie du firmware applicatif.
La PR #49 a été fermée sans fusion.

## Firmware et préparation (après accord Pilotage seulement)

- 1 × ESP32-2432S028R classique, LCD ILI9341 320×240 cas 10,
  XPT2046 SPI3, câble données, UART accessible.
- C natif ESP-IDF **v5.5.6**, cible **esp32**.
- **Démarrage D09 direct** : pas de `main/Kconfig.projbuild`,
  pas de `CONFIG_WARSHIP_BOOT_*`, aucun ancien `sdkconfig` requis pour
  choisir le mode. `app_main()` appelle `d09_probe_run()` sans condition.
- Le profil LCD, la DMA 5120 octets, le partitionnement et le dispatcher
  anti-rebond B01 sont inchangés. Il n'y a aucune écriture NVS en D09.

L'artefact de la **CI du HEAD accepté** est l'option privilégiée : vérifier
le SHA-256 réel du `cyd_warship.bin` et employer les binaires bootloader,
partitions, application **avec les offsets contenus dans son
`flasher_args.json`**. Ne pas inventer les offsets. Comparer le
partitionnement avec la carte avant de flasher et préserver les données NVS.
**Ne jamais exécuter `erase-flash`.**

Si une compilation locale est nécessaire :

```sh
git fetch origin
git checkout fix/issue-43-d09-direct-boot-touch-map
git rev-parse HEAD               # doit correspondre au HEAD approuvé
idf.py --version                 # ESP-IDF v5.5.6
idf.py set-target esp32
idf.py build                     # pas de menuconfig ni de mode à choisir
idf.py size
idf.py size-components
sha256sum build/cyd_warship.bin
idf.py -p <PORT_SERIE> flash monitor
```

Attention : un BIN reconstruit localement possède **son propre hash**,
même si le commit source est identique. Consigner le hash du BIN
effectivement flashé. Ne jamais attribuer le hash CI à un autre fichier.

## Projection retenue

Résultat réel PO (5 cibles, 3 gestes par cible), contre-analyse Pilotage #21 :

```text
screen X <- raw Y, bornes 215..3799, pixels 0..319
screen Y <- raw X, bornes 302..3865, pixels 0..239
```

En dehors des bornes, la projection est bornée aux pixels valides.
Un raw ADC saturé (0 ou 4095), une erreur SPI ou un map invalide
continuent d'invalider le geste entier (anti-rebond B01 inchangé).
Sur les cinq **médianes physiques** : écart maximum 2 pixels par axe,
SSE 15 px². Ce n'est pas une garantie sur chaque pression ou bord de
l'écran. Ancien geste PRÊT `raw=(2092,3346)` → **`mapped=(279,120)`**.

## Protocole PO — un seul parcours D09 ciblé

Noter modèle/révision de CYD, port UART, HEAD source complet, SHA-256 du
BIN flashé, date, comportement et extraits de logs. Aucun autre essai
d'étalonnage n'est demandé.

1. **BOOT** : après reset, relever `BOOT_MODE=D09 direct boot`,
   `LCD_PROFILE_PASS case=10`, `FRAME ... mode=1`. Constater
   l'écran de déploiement avec grille 10×10 / 17 cases,
   boutons AUTO / PRET / RETOUR. Si LCD incomplet ou erreur SPI,
   arrêter et transmettre les logs.
2. **PRÊT** : toucher le centre visuel, tenir le stylet suffisamment
   pour franchir le seuil de 40 ms, puis relâcher. Vérifier
   `TOUCH_DOWN raw=... mapped=... valid=1` et une seule
   `ACTION id=3 ... from=1 to=2 result=0`. L'écran affiche
   **COMBAT NON INTEGRE** sans lancer de combat, FEU, NVS ou forfait.
   Vérifier l'absence d'un second PRÊT sur appui maintenu.
3. **START** : redémarrer la carte (PRÊT arrête volontairement la
   démonstration, il n'y a pas de START dans cette phase).
   Depuis DEPLOY, toucher **RETOUR** :
   `ACTION id=4 ... from=1 to=0 result=0`, écran HOME technique.
   Toucher **START** : `ACTION id=1 ... from=0 to=1 result=0`,
   nouveau déploiement à 17 cases, sans résultat/forfait.
4. Joindre les lignes UART `ACTION`, `TOUCH_DOWN`, `TOUCH_UP`,
   `MEM`, `DRAW`, `COUNTERS` et les erreurs éventuelles
   `TOUCH_SPI_ERROR` / `TOUCH_MAP_REJECT` / `LCD_REFRESH_STOP`.
   Toute différence entre le point physique et la zone calculée
   doit être signalée, sans modifier les constantes pendant la campagne.

**Statuts :** tests C99 `HOST_TESTED`, compilation `TARGET_BUILD_VERIFIED`,
CI `CI_PASSED` sont indépendants. Pour cette nouvelle intégration D09,
`ON_DEVICE_UNVERIFIED` jusqu'au retour matériel PO. Pilotage décide de
la réception, de la fusion, et de la suite de #43.
