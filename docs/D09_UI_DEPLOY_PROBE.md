# D09 — Première interface technique de déploiement CYD (Issue #43)

**Statut : prototype technique**, pas interface produit finale. Mandat [Issue #43](https://github.com/Kharn27/ESP32-OTA-Battleship/issues/43), [passation Pilotage](https://github.com/Kharn27/ESP32-OTA-Battleship/issues/43#issuecomment-6574648073), Bible v0.17 §14.1–2, §14.8 et §15.2, contrats LOT1-PL-001 / FLT-001 / ACC-010 / ABD-001. Base : `main` `43eee7b86e728d1aa8c5ca79cc7c9a7e99d3af86` (D08 fusionnée).

## Limite fonctionnelle volontaire

L'écran **D09** démarre par un déploiement D07 réel : cinq navires humains valides 2+3+3+4+5, exactement 17 cases sur 10×10. Il n'invente ni flotte manuelle ni règle métier, ne lit jamais directement `WarshipLifecycle` pour dessiner et ne montre jamais la flotte de Capitaine.

- **AUTO** : nouvel appel `warship_lifecycle_auto()`, en conservant PRÊT non engagé et la flotte Capitaine secrète. Toute coïncidence entre deux tirages est acceptée, tout échec RNG/D07 est journalisé et ne change pas le modèle.
- **PRÊT** : unique appel `warship_lifecycle_ready()`. Après succès, la CYD affiche explicitement **DEMO / COMBAT / NON / INTEGRE**, sans FEU, abandon implicite, palmarès ni NVS. **Le mode est arrêté après PRÊT : redémarrer la carte pour une nouvelle démo.**
- **RETOUR** : appelle `warship_lifecycle_leave_deploy()` **avant PRÊT uniquement** et affiche un écran technique **D09 TECH / START**. START relance `warship_lifecycle_start()` (nouvelle partie non engagée), sans écran HOME produit final. Ni forfait ni résultat D07 ne sont émis.
- Les hitboxes et couleurs sont **provisoires**, indépendantes des maquettes G03 #37 / PR #41, qui ne sont pas intégrées ni modifiées.

La future UI de combat, les finales, le NVS D08, l'ESP-NOW, les effets et le menu PALMARÈS sont **hors périmètre**.

## Frontières et mémoire

| Responsabilité | Implémentation | Contrat |
|---|---|---|
| Moteur/déploiement | D07 `WarshipLifecycle` | **Privé à la tâche D09** (contient les secrets CPU) |
| Projection publique | `warship_lifecycle_observe()` | `WarshipLifecyclePublic`, flotte humaine uniquement |
| C99 sans matériel | `components/warship_deploy_ui/` | hit-test paramétré, debounce, décisions D07 autorisées, raster déterministe |
| LCD | `components/warship_lcd/` de T02 | **Cas 10**, séquence `MV=0, MX=0, MY=1`, **320×240**, une bande **5120 o DMA** |
| Tactile | `components/warship_touch/` | **SPI3 exclusif**, PENIRQ36, XPT2046 médiane de trois lectures X/Y ; projection issue des mesures PO #21 |
| Adaptateur matériel D09 | `main/d09_probe.c` | une seule tâche, propriétaire D07, LCD et toucher ; journalise temps, erreurs, mémoire |

Aucun framebuffer plein écran : un `WarshipDeployFrame` de **100 octets de cases + un octet mode**, dérivé de l'observation publique D07, est alimenté par la flotte humaine. La fonction `warship_deploy_pixel()` fournit les couleurs RGB565 à la demande ; `d09_probe.c` remplit la bande **8 lignes × largeur × 2** du pilote LCD puis appelle `warship_lcd_send()` **synchrone / DMA terminé** avant de modifier le buffer. Le bloc reste alloué et **n'est jamais réutilisé après erreur ou timeout LCD**. Il n'existe aucune allocation graphique proportionnelle à 320×240 ; aucune PSRAM/LVGL/microSD. Les deux images complètes (boot et RETOUR/START) nécessitent 30 bandes DMA, et AUTO ne redessine que la région de grille `(0,0,220,240)` ; PRÊT seulement le panneau `(220,0,100,240)`. Les temps réels sont consignés en `DRAW ... us=...`, **non inventés**.

Les structures `WarshipLifecycle` et `WarshipLifecyclePublic` sont statiques (BSS) au niveau de la tâche de sonde et non dupliquées sur sa pile. `MEM phase=...` journalise heap libre/plus grand bloc/minimum, bloc DMA, stack watermark FreeRTOS, nombre de bandes/erreurs, tailles de structures. **Le watermark pile doit être mesuré réellement sur l'appareil**, et `idf.py size` ne le prouve pas. L'ordonnanceur est utilisé seulement par la boucle de polling `vTaskDelay(20ms)` ; aucun traitement graphique dans une IRQ.

## Projection tactile mesurée et dispatcher B01 inchangé

La campagne T02-B physique approuvée par Pilotage (Issue #21) a produit
15 gestes indépendants sur cinq cibles, avec permutation des axes :
**écran X ← raw Y 215..3799 ; écran Y ← raw X 302..3865**, écran 320×240,
bornage sur les pixels [0..319]×[0..239]. Les cinq médianes donnent au
plus **2 px d'écart** par axe, somme des carrés 15 px². Ce résultat ne
garantit pas la répétabilité de chaque geste ni les bords : l'UX D09
reste à confirmer physiquement. Le point PRÊT PO `raw=(2092,3346)`
est désormais projeté vers **(279,120)**, dans la zone PRÊT.

La conversion reste pure C99 dans `t02_touch_map` (fonction
`t02_map_d09_point()`), le transport XPT2046 conserve
`SPI3_HOST MOSI32/MISO39/SCLK25/CS33/IRQ36`, 1 MHz, mode 0,
trois requêtes `0xD0 X` et `0x90 Y`, médiane par axe.
Une valeur saturée ADC (0 ou 4095), une erreur SPI ou une
projection invalide annulent le geste entier. Aucune modification du
dispatcher **B01** n'a été faite dans ce correctif.

Le dispatcher C99 `WarshipDeployInput` réalise :
- Appui stable pendant **40 ms** avant émission ; relâchement stable **60 ms** pour réarmer, garde supplémentaire **120 ms** entre actions acceptées. La fréquence de lecture ciblée est 20 ms, hors temps de dessin.
- Un geste, y compris maintenu, glissé, rebondissant ou observé après rafraîchissement, produit **au plus une action**. Le premier point échantillonné est conservé ; un appui débutant hors d'une commande ne peut pas glisser vers AUTO/PRÊT/RETOUR pour les déclencher. **Correctif B01 (revue Pilotage PR #44)** : le premier DOWN réserve le geste **même si `valid=0`** ; une erreur SPI/mapping ultérieure invalide définitivement ce geste. Un relâchement bref (même 59 ms) **consomme** toute action en attente et le réappui ne réinitialise ni le premier point ni le latch d'invalidation. Seul un relâchement continu **>=60 ms** arme une **nouvelle** pression, qui devra ensuite durer **>=40 ms** ; aucune transition D07 n'est autorisée pendant un geste invalidé. Contrat de sécurité vérifié également sur premiers appuis valides interrompus et horloge non monotone au relâchement.
- Trois zones `AUTO (231,39,82,40)`, `PRÊT (231,99,82,40)`, `RETOUR (231,159,82,40)`, et `START (231,99,82,40)` **en HOME seulement**, avec une bordure non interactive de 2 px. La grille `(23,24,190,190)` contient 10×10 cases de 19×19. Les dimensions/recouvrements sont validés ; `warship_deploy_hit()` refuse les phases non attendues.
- `warship_deploy_apply()` est limité aux cinq transitions autorisées de D07 : START, AUTO, READY et RETURN (aucun FEU ni abandon). Aucun événement D07 terminal/forfait n'est consommé ni stocké par D09.

Les logs de D09 comprennent : `TOUCH_DOWN raw=... mapped=... valid=... triplet`, `TOUCH_UP`, `TOUCH_SPI_ERROR`, `TOUCH_MAP_REJECT`, `ACTION id=... raw=... mapped=... from=... to=... result=...`, `FRAME ... public_human`, `DRAW ... us`, `MEM phase`, et les compteurs. Cette instrumentation doit permettre au PO et Pilotage de constater les inversions et les fausses zones sans attribuer à ce mapping une précision non testée.

### Démarrage D09 direct (décision Pilotage du 02/11/2026)

Le sélecteur `main/Kconfig.projbuild` et l'ancienne sonde
`main/t02_probe.c/.h` ont été supprimés. La sonde temporaire T02_CAL
de la PR #49 n'est jamais entrée dans `main` (PR fermée sans fusion).
L'application compile seulement `app_main.c` et `d09_probe.c`
dans le composant principal ; `app_main()` appelle
**inconditionnellement `d09_probe_run()`**, sans aucune référence
`CONFIG_WARSHIP_BOOT_*`. Un ancien `sdkconfig` ne sélectionne
donc plus de sonde au démarrage. Les pilotes LCD/XPT2046 restent
initialisés une seule fois. `BOOT_MODE=D09 direct boot` permet
d'identifier le chemin dans l'UART.

## Validation

Le nouveau CTest `warship_deploy_ui_technical` teste sur PC : géométrie et hitboxes centres/coins/limites, refus des zones externes, debounce, tenue prolongée, glissé depuis le premier point, rebonds, appui/release, erreur SPI/map simulée (valid=0), **séquence B01 100 ms invalide → UP 120 ms → DOWN valide 140 ms → maintien 180 ms sans READY**, rebonds UP 59 ms, reprise après UP 60 ms et nouveau DOWN 40 ms, invalidation après premier sample valide, rebond initial valide sans déclenchement, D07 inchangé tant que NO_ACTION, timestamps incohérents, phases D07 légales et illégales, AUTO répété avec 17 cases, arrêt PRÊT, sortie RETOUR sans forfait, retour START, rejet RNG et intégrité du modèle byte-for-byte, absence de fuite CPU via deux flottes CPU différentes donnant une projection/frame identiques. Les onze suites historiques T01–D08 restent inchangées.

```sh
for cc in gcc clang; do
  cmake -S tests -B "build-host-$cc" -G Ninja -DCMAKE_C_COMPILER="$cc" -DWARSHIP_SANITIZERS=ON
  cmake --build "build-host-$cc"
  ctest --test-dir "build-host-$cc" --output-on-failure -V
done

# ESP-IDF v5.5.6 / cible esp32 Xtensa (depuis checkout du HEAD)
idf.py set-target esp32
idf.py build
idf.py size
idf.py size-components
sha256sum build/cyd_warship.bin
```

Voir également `docs/D09_FLASH_TEST.md` pour commandes et scénario de validation physique reproductible. Les mesures statiques Flash/IRAM/DRAM et hash de l'application sont associés à la **CI du HEAD final**, distincte d'un test physique. Toute compilation cible est `TARGET_BUILD_VERIFIED` et **n'est pas** `ON_DEVICE_VERIFIED`. La calibration T02-B est acquise séparément sur les cinq médianes, mais la **réception des commandes D09 PRÊT/RETOUR/START sur le nouveau firmware** reste `ON_DEVICE_UNVERIFIED` jusqu’à un nouveau retour PO.

**Caveat ergonomique :** sur CYD 320×240, le rendu et les hitboxes sont encore *techniques*. Le texte affiché est `PRET` ASCII (faute d'accent bitmap dans cette police technique), sans décider l'orthographe/design final validé par le PO. Les couleurs/rectangles/tailles sont des paramètres de banc technique. Le passage au graphisme G03 nécessitera une validation artistique distincte.

**NE PAS UTILISER CETTE PR comme preuve d'intégration D08 NVS ou de fonctionnalité du jeu complet.** D08 #36 et T02-B #21 conservent leur propre suivi matériel.
