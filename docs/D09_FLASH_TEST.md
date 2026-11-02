# D09 — Mode de compilation, flash et protocole PO (une CYD)

**Mission** : Issue #43. Lire aussi [docs/D09_UI_DEPLOY_PROBE.md](D09_UI_DEPLOY_PROBE.md) et [docs/TEST_POLICY.md](TEST_POLICY.md).

## Préparation — configuration exacte et non destructive

Matériel cible : **ESP32-2432S028R classique**, CYD 320×240 paysage, ILI9341, écran à stylet résistif XPT2046 candidat. **1 seule CYD** et câble USB données. ESP-IDF **v5.5.6**, cible `esp32`, composants du dépôt courant. Aucune PSRAM ni carte microSD requises. **Ne jamais utiliser `erase-flash` ni effacer la partition NVS du palmarès pour cette mission.**

À faire **seulement après revue et accord du Pilotage** :

```sh
git clone https://github.com/Kharn27/ESP32-OTA-Battleship
cd ESP32-OTA-Battleship
git fetch origin feat/issue-43-d09-deploy-ui-probe
git checkout feat/issue-43-d09-deploy-ui-probe
git rev-parse HEAD              # comparer au HEAD validé de la PR
idf.py --version                # doit indiquer ESP-IDF v5.5.6
idf.py set-target esp32
idf.py menuconfig               # CYD WARSHIP boot mode -> D09 technical deployment
idf.py build
idf.py size
idf.py size-components
sha256sum build/cyd_warship.bin
idf.py -p <PORT_SERIE> flash monitor
```

L'artefact GitHub Actions du HEAD inclut bootloader/partition/firmware et `flasher_args.json` : utiliser **la configuration complète** propre à cette CI si flashage manuel ; ne pas flasher uniquement `cyd_warship.bin` à une adresse supposée. Sauvegarder un palmarès NVS existant avant toute manipulation pouvant écraser la partition et éviter `idf.py erase-flash`. L'outil `flash` normal réécrit l'application/bootloader/partition de l'image, **pas une commande de remise à zéro** ; vérifier cependant que le partitionnement de l'artefact ne détruit pas vos données.

**Alternative pour la mire T02 (contrôle séparé)** : `idf.py menuconfig` → `CYD WARSHIP boot mode` → `T02 legacy diagnostic probe` puis reconstruire et relever **le SHA du nouveau binaire T02**, différent de celui D09, pour éviter de mêler leurs preuves. Ne pas lancer T02 et D09 simultanément.

## Fiche d'essai à retourner au Pilotage

Consigner numéro/type de carte (dont USB-C ou micro-USB), port, OS, outil, `git rev-parse HEAD` complet, SHA-256 réel du .bin flashé, mode Kconfig, logs UART boot complets et photos écran. **Au moins une vidéo courte** peut aider à détecter un appui maintenu ou un rafraîchissement inattendu.

1. **Boot initial / LCD** : confirmer `BOOT_MODE=D09`, `LCD_PROFILE_PASS case=10`, `FRAME mode=1`, écran plein 320×240. Photographie globale de la grille et des 3 boutons, **aucun pixel fantôme**, cadrage, couleurs, légende lisible. Si l'image est incomplète, ne pas poursuivre les coordonnées ; fournir photo/log et STOP.
2. **Tactile étalonnage T02-B à confirmer** : relever `TOUCH_DOWN raw=(x,y)` et `provisional=(x,y)` aux **cinq points** : haut-gauche, **haut-droit**, centre, **bas-gauche**, bas-droit. Vérifier si axes inversés/décalés ; **ne pas conclure que les coordonnées sont calibrées**. Pour tout appui non détecté ou dangereux, ne pas forcer une hitbox en devinant.
3. **AUTO** : stylet au centre de AUTO, répéter au moins 5 fois en **relâchant complètement** entre chaque. Vérifier chaque ligne `ACTION id=2 from=1 to=1 result=0`, chaque `FRAME mode=1`, compte de **17 cases** et 5 navires sur la grille, pas de nouveau combat. Deux placements identiques par hasard sont valides. Maintenir AUTO appuyé plusieurs secondes et vérifier **exactement une** commande (pas de boucle de regeneration).
4. **Zones mortes/bords** : appui à l'extérieur des boutons, sur bordures, grille ; **aucune action**. Faire un glissé commencé hors de la zone AUTO puis terminer sur AUTO : aucune génération. Tester un appui rapide/rebond/relâchement puis répéter et consigner les éventuelles actions non voulues.
5. **RETOUR** : pendant déploiement, appuyer puis relâcher. Vérifier `ACTION id=4 from=1 to=0 result=0`, affichage technique HOME avec START. Aucun forfait ni partie comptabilisée, aucune écriture NVS. START relance un **nouveau déploiement** `ACTION id=1 from=0 to=1 result=0`.
6. **PRÊT** : appuyer une fois sur PRÊT et vérifier `ACTION id=3 from=1 to=2 result=0`, affichage **COMBAT NON INTEGRE**, grille humaine encore visible, aucun FEU ni partie fictive. Toucher AUTO/RETOUR dans cette phase : aucune commande, **aucun abandon forcé**.
7. **Reset / power cycle** : redémarrer ; un nouveau déploiement apparaît, non une reprise de partie ni un forfait. Vérifier logs `MEM phase=startup/lcd_ready/ready/after_action/running`, heap/plus grand bloc, DMA 5120 o, watermark pile, `DRAW ... us`, nombre de bandes et tout `TOUCH_SPI_ERROR`, `TOUCH_MAP_REJECT`, `LCD_REFRESH_STOP`.
8. **Diagnostic T02** si demandé séparément par Pilotage : reconstruire **mode T02**, noter SHA/firmware exact, vérifier mire et raw XPT2046. Le résultat T02 ne remplace pas le jalon D09.

Un résultat `PASS` doit distinguer **observations PO** de vérification indépendante ; `ON_DEVICE_PO_REPORTED_PASS` uniquement sur rapport réel. Si la calibration échoue, classifier précisément `TOUCH_UNCALIBRATED` ou `LCD_FAIL` et conserver le jalon **ON_DEVICE_UNVERIFIED** pour la fonctionnalité tactile. Ne pas attribuer les temps d'animation/rendu ou les budgets mémoire à une estimation hôte.
