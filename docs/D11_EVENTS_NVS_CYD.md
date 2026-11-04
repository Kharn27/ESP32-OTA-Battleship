# D11 — Pont des événements de partie D07 vers palmarès NVS D08

**Mandat :** [Issue #63](https://github.com/Kharn27/ESP32-OTA-Battleship/issues/63), Bible §14.4 et §14.8, [D07](D07_SOLO_LIFECYCLE.md), [D08](D08_PALMARES_NVS.md), [D10](D10_COMBAT_TECHNIQUE.md). Base `main` au lancement : `ce775c4b83fbe4ba248acb9b589d73773b1286cb` (D10 fusionné). Branche `feat/issue-63-d11-palmares-events`. Revue et fusion exclusivement par Pilotage.

## Interfaces et séparation

- `components/warship_event_bridge/` : orchestrateur **C99 pur, mono-écrivain**. Entrée : l'enum **public** `WarshipLifecycleEvent` D07 ; stockage : backend injecté D08 + backend de **réservation d'ID**. Aucune flotte, position CPU, image, RTC, FreeRTOS ou allocation dynamique. `NONE` ne réserve ni n'écrit.
- `components/warship_palmares/` D08 : **snapshot V1 44 octets inchangé** (clé `warship/palmares`, cinq compteurs, `last_id`, `last_event` et contrôle). Ajout de `warship_palmares_last_identity()` : lecture minimale du dernier ID/type **confirmé**, refus `RELOAD_REQUIRED` en cas de commit incertain.
- `components/warship_event_id_nvs/` : clé supplémentaire `warship/event_seq`, **réservation monotone** (20 octets, little-endian : magic `WID1`, version=1, ID uint64 non nul, FNV-1a sur 16 premiers octets). Ne stocke **aucun type de résultat**. Erreur version/corruption explicite, aucune migration destructive. Les anciens snapshots D08 ayant `last_id>0` sans cette clé s'ouvrent **sans écriture au boot**, premier nouvel ID = `last_id+1`. Une clé existante avec `event_seq<last_id` est rejetée comme `CORRUPT`.
- `main/d11_runtime.c` : **un seul propriétaire** du ledger, ouverture NVS à l'entrée du firmware, journal des cinq compteurs, ID et code `esp_err_t`. Aucun reset/erase NVS. `main/d09_probe.c` conserve seul `WarshipLifecycle` et remet `emitted` à D11 **seulement après** `warship_combat_presented` ayant ACK la paire après dessin TFT `ESP_OK`. La boucle principale tente une retentative au maximum toutes les 10 secondes si une remise est pendante ; hors SPI/DMA, pas de boucle de retry bloquante. Une écriture NVS synchrone post-ACK peut toutefois introduire une latence, à mesurer sur CYD.
- **Aucun parcours tactile d'abandon** ajouté. Le futur parcours validé `decide_abandon(1)` devra remettre son événement D07 à ce même orchestrateur, sans accès direct à NVS. Le forfait n'est jamais attribué à un `NONE`, incident, extinction, bouton ANNULER, retour pré-PRÊT ou inspection.

Les buffers DMA 5120 o, l'écran 320×240, la main-task stack 4096 o, l'IA et les règles de combat sont inchangés. Les logs D11 ne contiennent pas les flottes.

## Identifiant durable et échecs

Au boot, `reserved_id = max(D08.last_id, NVS.event_seq)`. Aucune nouvelle réservation/écriture au boot normal. Lorsqu'un événement terminal D07 est remis :

1. `PENDING` retient **exactement** `(reserved_id + 1, événement)`. `reserved_id == UINT64_MAX` => `OVERFLOW`, aucune mutation ni écriture.
2. Commit de l'ID en NVS `event_seq` **AVANT** le snapshot résultat. `WRITE_UNCERTAIN` sur échec de retour ; aucun autre événement ne peut s'emparer de l'ID. À la retentative, relecture de la séquence : si égale au pending, réservation considérée réussie ; si encore égale au précédent ID, nouvelle tentative du même pending ; si différente, `CORRUPT`.
3. Application `warship_palmares_apply(ledger,pending_id,event)` : `APPLIED` après commit certain, ou `WRITE_UNCERTAIN` avec cache verrouillé. Une retentative **recharge** D08, puis redonne le **même ID/type** : `ALREADY_APPLIED` (snapshot écrit malgré l'erreur) ou `APPLIED` (snapshot non écrit). Le marqueur durable D08 garde `games=victoires+défaites+nuls`. Aucun double crédit de la dernière paire.
4. Tant que l'événement reste en attente, `receive` d'un nouvel événement retourne `PENDING`, même si l'enum est identique. Il n'existe **pas de queue multi-événements** : cas à tracer comme incident, pas de nouveau crédit implicite. `READ_ERROR`, `RELOAD_REQUIRED`, `CORRUPT`, `INCOMPATIBLE_VERSION`, `CONFLICT` et `OVERFLOW` bloquent l'écrasement risqué de l'état précédent.

**Limite produit :** D07 n'a pas d'outbox ni de reprise après panne. Une coupure entre ACK et écriture de réservation, ou entre réservation et D08, peut **perdre le dernier résultat** après reboot. Une réservation conservée ne porte pas le type du résultat et **ne peut pas le restaurer**. Les deux clés ne sont pas atomiques ensemble. On garantit uniquement le comportement **sans coupure** et la **non-réutilisation des IDs déjà réservés et durables**, **pas du « exactly once » après extinction**. Si le PO exige une garantie supérieure, arbitrer une reprise/outbox via mission distincte avant d'élargir D11.

## Tests C99 / compilation

- `tests/test_warship_event_bridge.c` : vraies transitions D07 jusqu'à **VICTOIRE/DÉFAITE/NUL**, confirmation `CONTINUER/ABANDONNER`, pas de crédit avant dernier ACK, `NONE` sans écriture, retour pré-PRÊT, dédup D08, rechargement après erreur avant/après commit, sticky ID/type, reboot avec ID réservé sans score, ouverture d'un ancien snapshot D08, corruption/overflow.
- `tests/test_warship_event_id_nvs.c` : **vrai** adaptateur ID compilé avec NVS fake, sérialisation versionnée, namespaces de production et de banc, retours `esp_err_t`, absence de formatage, commit incertain, corruption/incompatibilité. `tests/test_warship_palmares_nvs.c` assure la non-régression du vrai adaptateur D08.
- Exécution locale / CI : `cmake -S tests -B build-host -G Ninja -DCMAKE_C_COMPILER=gcc -DWARSHIP_SANITIZERS=ON && cmake --build build-host && ctest --test-dir build-host --output-on-failure -V`, même chose sous clang. CI épinglée sur SHA PR avec ESP-IDF **v5.5.6**, `esp32`, `idf.py set-target esp32 && idf.py build && idf.py size && idf.py size-components`, BIN/ELF/map, `sha256sum`, stack config vérifiée.
- Une CI verte **ne remplace pas** `uxTaskGetStackHighWaterMark(NULL)` **sur CYD** après NVS + plusieurs FEU ; garde contractuelle >=1024 octets. `ON_DEVICE_VERIFIED` : uniquement sur essai réel documenté.

## Diagnostic isolé, strictement volontaire

**Binaire standard** : option CMake `WARSHIP_D11_NVS_BENCH=OFF` (**défaut**), namespace `warship`, **lecture NVS sans crédit au boot** ; un score n'est enregistré qu'après un vrai événement D07.

**Binaire de banc distinct**, à ne flasher **que sur CYD explicitement désignée et avec accord PO/Pilotage** : compiler avec `-D WARSHIP_D11_NVS_BENCH=ON`. Le firmware journalise `D11_BENCH=ARMED namespace=warship_d11lab NOT_REAL_PALMARES`. **Les deux backends** choisissent le namespace séparé `warship_d11lab` ; le palmarès réel `warship/palmares` n'est **ni lu ni modifié** pendant cette expérience. À la première entrée **vraiment vierge** du namespace de banc, une seule `VICTORY` diagnostique ID=1 est créditée **dans le banc seulement**. Aux boots suivants, même `(1,VICTORY)` soumis à la **déduplication D08**, réponse attendue `ALREADY_APPLIED` sans écriture. Si la clé de banc contient déjà un résultat autre que ce cas, ou une réservation orpheline, **pas d'injection** ; aucune remise à zéro automatique.

**Banc** : utiliser un répertoire de build distinct pour ne jamais confondre les deux firmwares (vérifier que la macro est réellement activée dans l'ELF/les logs). Exemples ESP-IDF :

```sh
# Production CI (par défaut)
idf.py set-target esp32
idf.py build
sha256sum build/cyd_warship.bin

# Uniquement sur banc autorisé, avec identité de BIN distincte :
idf.py -B build-d11-bench -D WARSHIP_D11_NVS_BENCH=ON set-target esp32
idf.py -B build-d11-bench -D WARSHIP_D11_NVS_BENCH=ON build
sha256sum build-d11-bench/cyd_warship.bin
# Flasher selon build-d11-bench/flasher_args.json, conserver NVS.
```

Ne **jamais** lancer `erase-flash`, `nvs_flash_erase` ou formatage implicite. Sauvegarder la NVS d'une carte non vierge et convenir de l'expérience avant toute écriture, même isolée.

## Protocole réception CYD — à réaliser par PO sur instruction Pilotage

1. Identifier **une** ESP32-2432S028R classique, sa révision et port série, outils ESP-IDF v5.5.6, firmware HEAD/SHA-256 **effectivement flashé**, namespace et mode de build ON/OFF, NVS sauvegardée si nécessaire. UART 115200 ; **sans erase NVS**.
2. **Production** : démarrer et relever `BOOT_MODE=D09`, `STACK_CONFIG main_task_bytes=4096`, `D11_BENCH=OFF`, `D11_NVS_INIT esp_err=0x0`, puis `D11_COUNTERS games=… victories=… defeats=… draws=… voluntary_forfeits=… last_id=…`. Rebooter : vérifier valeurs inchangées, **aucune écriture automatique**.
3. Jouer D10 : PRÊT, FEU 1, paire affichée+ACK, FEU 2, ACK. `NONE` ne touche pas NVS ; vérifier heap, tactile, LCD, DMA et mémoire. Un vrai verdict terminal fait apparaître **un seul** `D11_EVENT stage=post_d07_ack status=APPLIED` (ou état d'erreur explicite) avec `games +1` et catégorie juste. Aucun score lors d'inspection/retour, et pas de forfait sans confirmation. Ne pas faire semblant de tester le forfait à partir du bouton ANNULER de D10.
4. **Banc diagnostique isolé** : après approbation, flasher BIN **bench**, enregistrer le SHA différent du BIN production, vérifier `D11_BENCH_FIRST_FLASH`, `stage=bench_first_write status=APPLIED`, compteurs banc `games=1 victories=1`, puis reboot et vrai **power-cycle**. Aux deux redémarrages, `D11_BENCH_REBOOT_DEDUP` doit montrer `ALREADY_APPLIED` et exactement les mêmes cinq compteurs. Restaurer le firmware de production sans effacer NVS et confirmer valeurs production **inchangées**.
5. Recueillir `MEM phase=startup_after_nvs_read`, `after_feu`, `after_reveal`, `after_rounds`, `after_nvs_event` (si verdict), `heap8`, `largest8`, `stack_margin_bytes>=1024` **après NVS et FEU** ; aucune faute SPI/tactile/DMA. L'expérience banc ne remplace pas le chemin réel D07→D08.
6. Remettre logs UART horodatés, photos si nécessaire, commandes de flash, SHA exact du BIN, comportement en erreur NVS et verdict PASS/FAIL ; **ne pas qualifier `ON_DEVICE_VERIFIED` sans rapport matériel**. L'Issue D08 #36 reste ouverte tant que la preuve NVS réelle attendue n'est pas reconnue par Pilotage.

**Exclusions :** pas d'écran produit PALMARÈS, d'UX QUITTER/ABANDONNER/REJOUER, de G07/G08 runtime, de sauvegarde de partie, de migration NVS, de réseau/ESP-NOW ou de deuxième CYD. Décisions PO inchangées.
