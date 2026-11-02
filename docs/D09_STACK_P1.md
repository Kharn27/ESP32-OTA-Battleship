# D09 / PR #52 — Correction P1 : pile à PRÊT (Issue #43)

## Preuve et cause technique

**Retour matériel antérieur du PO :** HEAD
`3db97a5ab972ca28f73d965f5fb5eac04c9438bd`, binaire construit
**localement** SHA-256 `af06da0b661ade2a4e2ae24497f40d2e940099e5daf2056fd7bd1f3cde4db11e`.
La fonction `memory_log()` relève
`uxTaskGetStackHighWaterMark(NULL)` sur la tâche ESP-IDF `app_main` :
**1696 octets à `MEM phase=ready`**, **96 octets à
`MEM phase=after_action` suite à PRÊT**. En ESP-IDF, cette métrique
est la *marge minimale historique en octets*, pas la pile libre
instantanée. Les deux valeurs sont des mesures PO sur l'ancien
firmware ; elles ne valent **pas** pour le HEAD corrigé.

L'analyse source explique la pointe de pile :

```text
D09: warship_deploy_apply(READY)
 -> D07: warship_lifecycle_ready
       WarshipSoloSession session (auto, taille complète)
     -> D06: warship_solo_init
          WarshipSoloSession next (auto, taille complète)
        -> D02: warship_battle_init
             WarshipBattle state (auto, taille complète)
```

Les trois buffers appartiennent à des appels imbriqués, donc leurs
durées de vie peuvent se chevaucher. La taille effective des types
et `CONFIG_ESP_MAIN_TASK_STACK_SIZE` sont **journalisées sur le
firmware corrigé** via `STACK_CONFIG main_task_bytes=... sizeof_solo=...
sizeof_battle=...`, et cette option est aussi imprimée dans la CI
à partir de son propre `sdkconfig`. Ces tailles ne sont pas en elles-mêmes
une mesure du minimum de pile : seules les traces sur la CYD
permettront de vérifier le critère.

## Correctif minimal : zéro changement de taille de pile ou de heap

- **D07** : `warship_lifecycle_ready()` remplace l'allocation locale
  `WarshipSoloSession session` par `warship_solo_init(&game->solo,...)`.
  `warship_solo_init()` garantit contractuellement destination inchangée
  sur échec. Le contrôleur possède déjà l'espace `game->solo` et les
  flottes `human`/`captain` sont disjointes de cet espace. La phase
  n'avance vers COMBAT qu'après succès.
- **D02** : `warship_battle_init()` valide d'abord tous les arguments
  et les deux flottes. Les seules opérations restantes sont
  `memset`, copies de flottes, affectations constantes : aucune ne peut
  échouer. L'état est alors construit en place dans `*battle`, ce
  qui supprime `WarshipBattle state` local **tout en garantissant**
  aucune écriture sur erreur. Le contrat interdit l'aliasage
  flotte-destination.
- **D06** : garde `WarshipSoloSession next` (transaction). Aucune
  API, règle métier, structure, télémétrie de jeu, B01, mapping, pilote,
  partition ou paramètre de tâche n'a été changé.
- **Libellé UART** : `ACTION ... mapped=` remplace `provisional=`
  afin de refléter le mapping désormais calibré.

La réduction réelle du pic de pile dépend du compilateur Xtensa et des
autres frames du parcours (notamment les projections publiques après
PRÊT) ; il serait incorrect d'annoncer un gain `sizeof(solo)+
sizeof(battle)` exact avant test. Les tests C99 valident l'atomicité
et la déterminisme de l'initialisation D02/D07, les anciennes suites
vérifient les règles/CPU/confidentialité et B01. L'essai de réception
est **strictement** le mini-parcours physique ci-dessous.

## Mini-test PO D09 — seulement après contre-revue Pilotage

Sur **une seule CYD** ESP32-2432S028R, utilisant le **nouvel HEAD et le
BIN validés par Pilotage**, vérifier le SHA-256 de l'application réellement
flashée, les offsets officiels `flasher_args.json` et le partitionnement
avant flash. **Pas de `erase-flash`, préserver NVS.** Si construction
locale, conserver SON empreinte différente de celle du BIN CI.

1. BOOT : collecter `App version`, `BOOT_MODE=D09 direct boot`,
   `STACK_CONFIG main_task_bytes=... sizeof_solo=... sizeof_battle=...`,
   `LCD_PROFILE_PASS case=10`, puis `MEM phase=ready ... stack_margin_bytes=...`.
2. Toucher **PRÊT une seule fois** au centre, attendre
   `TOUCH_DOWN raw=... mapped=... valid=1`,
   `ACTION id=3 ... from=1 to=2 result=0`,
   `FRAME phase=2 mode=2` et `DEMO_STOP COMBAT NON INTEGRE`.
3. Collecter **`MEM phase=after_action stack_margin_bytes=...`** et les
   relevés `MEM phase=running`, `COUNTERS` et les erreurs possibles
   (`TOUCH_SPI_ERROR`, `LCD_REFRESH_STOP`, timeout/DMA).
   Le critère est **au moins 1024 octets sur `after_action`**
   après passage PRÊT, sans erreur ni régression fonctionnelle.
   Si valeur inférieure, **FAIL** de la réception mémoire et aucun merge.

Noter carte, firmware/SHA, port UART, version IDF, résultat et lignes
complètes. **Ne pas** refaire les 15 points T02-B, le parcours
RETOUR/START déjà fonctionnel, ni demander des photos T02.

### Niveaux de preuve attendus

- `HOST_TESTED` : tests GCC et Clang C99 ASan/UBSan sur le **nouveau HEAD**.
- `TARGET_BUILD_VERIFIED` : build Xtensa ESP-IDF v5.5.6 réel avec
  `size`, `size-components`, lecture de
  `CONFIG_ESP_MAIN_TASK_STACK_SIZE` dans `sdkconfig`.
- `CI_PASSED` : trois jobs verts sur ce **même HEAD** et SHA-256 du BIN.
- `ON_DEVICE_PO_REPORTED_PASS` : historique fonctionnel sur l'ancien
  HEAD ; **le critère mémoire corrigé n'est PAS TESTÉ** jusqu'au
  retour du mini-test sur la nouvelle image.

**Pilotage** est seul responsable de la contre-revue, du verdict et
de la fusion. L'Issue #43 reste ouverte.
