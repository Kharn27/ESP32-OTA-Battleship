# D08 — Palmarès VS Capitaine / forfaits : C99 et NVS (Issue #36)

**Mandat :** Issue #36 et passation Pilotage [#issuecomment-6573116639](https://github.com/Kharn27/ESP32-OTA-Battleship/issues/36#issuecomment-6573116639), Bible produit v0.17 §14.8, §14.14 : LOT1-STAT-001, LOT1-ABD-001, BN-STAT-003. Base `main` `c7d37dc94ea2467591f607654edaabe14cdc71e0`, D07 fusionnée. Cette documentation précise le contrat **technique de persistance**, sans introduire de règle produit, d'UX ni de remise à zéro utilisateur.

## Architecture

Deux composants indépendants :

- `components/warship_palmares/` : **réducteur C99** pur, stockage injecté par `WarshipPalmaresBackend.load/store`. Aucun accès NVS, ESP-IDF, RTC, heap, écran, réseau, ni flotte secrète. Il reçoit exclusivement un `WarshipLifecycleEvent` D07 et **un identifiant externe de transaction**. Les callbacks sont invoqués de façon synchrone. Son API `warship_palmares_read()` fournit à la future présentation seulement cinq compteurs `WarshipPalmaresView`, sans modifier le stockage.
- `components/warship_palmares_nvs/` : adaptateur spécifique **ESP-IDF v5.5.6**, `nvs_flash_init()`, `nvs_open()`, `nvs_get_blob()`, `nvs_set_blob()`, `nvs_commit()`, `nvs_close()`. Namespace `warship`, clé `palmares` dans la partition NVS par défaut. Aucun effacement, migration, création de compte ou réinitialisation silencieuse. La sonde de liaison T02 référence le backend, **sans même appeler `nvs_flash_init()`** : cela ne teste aucune écriture physique.

La logique C99 exige un modèle **un seul écrivain** (contrôleur applicatif) et un backend/context stable pendant la durée de vie de `WarshipPalmares`. Pas d'accès concurrent à la même clé, pas de verrou partagé FreeRTOS ou arbitrage multithread dans D08. D08 ne sérialise ni positions de navires, ni état de bataille, ni cible CPU, ni historique de tirs.

## Snapshot unique V1 — 44 octets

Un enregistrement autonome compact, explicitement encodé **little-endian** (sans dépendre du padding C), est écrit en **une clé NVS** :

| Décalage | Type | Champ |
|---:|---|---|
| 0 | u32 | magic `0x57535031` (`WSP1` logique) |
| 4 | u32 | version `1` |
| 8 | u32 | `games` : parties VS CAPITAINE |
| 12 | u32 | `victories` |
| 16 | u32 | `defeats` |
| 20 | u32 | `draws` |
| 24 | u32 | `voluntary_forfeits` : compteur GLOBAL |
| 28 | u64 | `last_id` : dernier événement appliqué |
| 36 | u32 | `last_event` : type D07 1/2/3/4, ou 0 si vierge |
| 40 | u32 | checksum **FNV-1a 32** des 40 premiers octets |

Les valeurs sont **bornées à `UINT32_MAX`**, l'ID à `UINT64_MAX`. `games == victories + defeats + draws` est vérifié avec somme 64 bits **avant toute modification** et à chaque décodage. Les énumérations et le dernier événement sont validés ; un registre avec `last_id = 0` ne peut porter aucun résultat. **FNV-1a détecte les corruptions accidentelles**, mais **n'est pas une preuve cryptographique d'intégrité**, ni un système antitamper.

Si une relecture `reload()` voit un ID **inférieur** à un ID déjà confirmé en mémoire, y compris une clé disparue alors qu'un résultat avait été enregistré, D08 renvoie `CORRUPT` et préserve le cache au lieu de réinitialiser silencieusement les compteurs. Une nouvelle instance après redémarrage ne peut en revanche pas distinguer une NVS entièrement effacée d'une carte véritablement vierge : cette limite impose des sauvegardes et un protocole matériel non destructif.\n\nUne NVS véritablement **absente** (namespace/clé manquante) à la première ouverture est un palmarès **vierge à zéro**, sans écriture NVS automatique. Une clé existante de longueur autre que 44, magic erroné, checksum faux, invariant cassé ou type illégal => **`CORRUPT`** ; version différente => **`INCOMPATIBLE_VERSION`**. Les opérations échouées conservent la sortie et n'effacent **jamais** le palmarès en Flash. Une migration V2 n'est **pas implémentée** (sera à spécifier avant tout changement de schéma).

## Source des événements et contrat d'identité

D07 fournit `WarshipLifecycleEvent`, avec exactement **un enum terminal** à `ack_reveal()` (VICTORY / DEFEAT / DRAW) ou à `decide_abandon(1)` (VOLUNTARY_FORFEIT). Les lectures de PALMARÈS, CONTINUER, EXIT avant PRÊT, INSPECTER, REJOUER non terminé, coupure avant verdict et `EVENT_NONE` **ne comptabilisent rien**.

Un `warship_palmares_apply(ledger, event_id, event)` réussi applique :

| Événement | Parties | Victoires | Défaites | Nuls | Forfaits globaux |
|---|---:|---:|---:|---:|---:|
| D07 VICTORY | +1 | +1 | 0 | 0 | 0 |
| D07 DEFEAT | +1 | 0 | +1 | 0 | 0 |
| D07 DRAW | +1 | 0 | 0 | +1 | 0 |
| D07 VOLUNTARY_FORFEIT | 0 | 0 | 0 | 0 | +1 |
| D07 NONE | 0 | 0 | 0 | 0 | 0 |

### Exigence cruciale : l'ID n'existe pas dans D07

**D07 ne fournit NI ID d'événement NI outbox persistante.** Le paramètre `event_id` de D08 doit venir de **l'orchestrateur futur**, stable et strictement croissant **sur cette CYD**, à travers **toutes catégories et tous redémarrages**. `0` est réservé et refusé pour les événements réels. Le même événement re-livré après panne/timeout **doit conserver exactement le même ID ET son type**. L'ID ne peut pas être un compteur RAM réinitialisé à 1 à chaque démarrage, ni un numéro de manche, ni un simple `enum`. Les ID peuvent avoir des lacunes, mais doivent toujours être plus grands que le dernier ID définitivement appliqué. Le futur intégrateur doit garantir qu'un ID retenu pour une transition D07 n'est **pas réattribué** à une autre transition (y compris après reboot).

**Déduplication minimale O(1), sans journal infini :**

- `id > last_id`, événement valide => construction des compteurs suivants, nouveau marqueur `(id, type)`, **un seul `store` snapshot**, `APPLIED` **seulement si commit a répondu succès**.
- `id == last_id`, même type => **`ALREADY_APPLIED`**, aucune écriture.
- `id == last_id`, type différent => **`CONFLICTING_ID`**, aucune écriture.
- `0 < id < last_id` => **`STALE_ID`**, aucune écriture ; les anciennes identités ne sont plus présentes dans la clé : un conflit ancien ne peut pas être qualifié plus précisément, mais **aucun double incrément** ne se produit.
- `NONE` => **`NO_CHANGE`** sans écriture, quel que soit l'ID.
- Catégorie ou compteur dépassant `UINT32_MAX` => **`OVERFLOW`** avant commit, sans mutation.

**Statuts nommés du résultat :** `APPLIED`, `ALREADY_APPLIED`, `NO_CHANGE`, `BAD_EVENT`, `STALE_ID`, `CONFLICTING_ID`, `OVERFLOW`, `READ_ERROR`, `WRITE_UNCERTAIN`, `RELOAD_REQUIRED`, `CORRUPT`, `INCOMPATIBLE_VERSION`, `BAD_ARGUMENT`.

## Commit incertain, redémarrage et garanties bornées

La réduction est transactionnelle au niveau du **snapshot candidat en mémoire** et du **seul blob** : elle encode **les cinq compteurs, l'ID et le type dans le même appel `nvs_set_blob` suivi de `nvs_commit`**. NVS gère son propre stockage, mais **l'atomicité physique sous coupure n'a pas été validée sur CYD**. Aucun double enregistrement réparti sur des clés séparées n'est utilisé.

Si `store` retourne un échec (ou `nvs_commit` signale une erreur), D08 retourne **`WRITE_UNCERTAIN`**, sans avancer les compteurs en cache : l'écriture peut **avoir réussi** malgré l'erreur. Le ledger se verrouille en état `uncertain` : les lectures de cache et les prochains `apply` retournent **`RELOAD_REQUIRED`**, afin de ne pas masquer un commit antérieur ni écraser un état neuf. `warship_palmares_reload()` relit la source de vérité, vérifie version/invariant/checksum et déverrouille seulement en cas de succès. Ensuite, rejouer **le même ID/type** retourne soit `ALREADY_APPLIED` si l'écriture était durable, soit `APPLIED` après une nouvelle tentative si l'ancien snapshot était encore présent. Une lecture impossible/corrompue ne réarme **pas** les écritures.

**Pas de garantie exactly-once de bout en bout :** une coupure entre la transition terminale D07 et l'enregistrement du snapshot, ou avant que l'orchestrateur ait durablement retenu l'ID/type en attente, peut **perdre un événement**. D08 n'implémente ni outbox, ni recovery de session, ni synchronisation externe : l'orchestrateur devra définir le maintien durable des IDs/événements ou accepter/documenter explicitement cette fenêtre avec Pilotage/PO. La garantie fournie est seulement la **non-répétition d'un même événement identifié et vérifié** lorsque son ID reste le dernier ou est obsolète. Ne pas interpréter la disparition d'un événement D07 en forfait, ni attribuer `APPLIED` après une erreur `nvs_commit`.

## Reproduction sur hôte et cible

`tests/test_warship_palmares.c` utilise un NVS factice : stockage absent, valeurs successives V/D/N/forfaits, lecture sur nouvelle instance après reboot simulé, invariant, NONE sans écriture, doublon/conflict/stale et reboot, échec d'écriture **avant** stockage, erreur **après** copie durable (commit incertain), reload/réessai, erreur de lecture et conservation byte-for-byte, tronquage/oversize, magic/checksum/version/invariant/type invalides, overflow 32 bits et ID 64 bits, raccordement à un véritable D07 `decide_abandon()` avec CONTINUER puis ABANDONNER. Une seconde suite `warship_palmares_nvs_adapter` compile **le vrai fichier adaptateur NVS** sur PC avec faux `esp_err.h/nvs.h/nvs_flash.h` : échec de `nvs_flash_init` sans erase, clé/namespace absents, `nvs_set_blob` refusé sans commit, `nvs_commit` à effet durable mais retour erreur, réouverture/déduplication, erreurs de `nvs_open/nvs_get_blob`, longueur invalide et corruption. Les neuf suites T01–D07 restent intactes. Compilation hôte stricte et ASan/UBSan.

```sh
cmake -S tests -B build-gcc -G Ninja -DCMAKE_C_COMPILER=gcc -DWARSHIP_SANITIZERS=ON
cmake --build build-gcc
ctest --test-dir build-gcc --output-on-failure -V
cmake -S tests -B build-clang -G Ninja -DCMAKE_C_COMPILER=clang -DWARSHIP_SANITIZERS=ON
cmake --build build-clang
ctest --test-dir build-clang --output-on-failure -V
# CI ESP-IDF v5.5.6 : esp32 Xtensa
idf.py set-target esp32
idf.py build
idf.py size
idf.py size-components
sha256sum build/cyd_warship.bin
```

**Espaces mémoire :** snapshot NVS **44 octets utiles**, un buffer temporaire de 44 octets dans le réducteur et la lecture ; `sizeof(WarshipPalmares)` et `sizeof(WarshipPalmaresView)` sont reportés par les tests hôte. Le coût réel de NVS en Flash/IRAM/DRAM, ses métadonnées/piles, l'overhead par enregistrement et l'endurance ne sont **pas déduits de ces tailles**. Les métriques `idf.py size` sont à relever sur HEAD final dans la PR ; le runtime T02 ne fait qu'un test de **liaison sans lecture/écriture**. Les heap/stacks Xtensa restent **NON TESTÉS** avant intégration. La version ESP-IDF doit rester v5.5.6 comme la CI.

### Protocole matériel ultérieur (à piloter par Pilotage)

**Prérequis non disponibles dans D08 isolé :** firmware d'intégration proposant un scénario reproductible **non destructif** de lecture/écriture du palmarès et logs des codes `esp_err_t`, carte CYD ESP32-2432S028R identifiée, SHA commit, SHA firmware, séquence de flash connue et partition NVS conservée. **Ne pas flasher une partition vierge sur un palmarès réel sans sauvegarde/autorisation** ; ne pas appeler `nvs_flash_erase` pour « réparer » un échec d'initialisation.

1. Enregistrer la carte, port, versions ESP-IDF/outillage, commits SHA et binaire SHA-256, avec configuration de partition.
2. Démarrer sans modifier un palmarès existant ; lire cinq compteurs et identifiant de dernier événement. Sur banc vierge dédié, comparer le zéro attendu.
3. Enregistrer sur le banc deux événements distincts avec IDs monotones, vérifier les compteurs et logs NVS, puis **reset logiciel et power-cycle**, relire mêmes valeurs.
4. Soumettre le même ID/type après reboot et confirmer `ALREADY_APPLIED` sans incrément ; soumettre même ID/type contradictoire et confirmer `CONFLICTING_ID`, vérifier contenu préservé.
5. Injecter des erreurs contrôlées **dans le backend d'essai**, sans corrompre la NVS opérationnelle du PO ; vérifier `WRITE_UNCERTAIN`/reload et absence de reset automatique. Pour toute véritable injection de coupure durant commit, prévoir **un support/banc non critique** et une politique de sauvegarde avant essai.
6. Fournir logs, photos écran/port série si pertinents, mesures RAM et piles FreeRTOS à charge réelle, constat explicite PASS/FAIL.

**Statut initial de D08 :** `HOST_TESTED`/ `TARGET_BUILD_VERIFIED` seulement après preuve sur HEAD ; `ON_DEVICE_UNVERIFIED`. Une PR de socle peut être fusionnée sous décision Pilotage, mais **Issue #36 reste ouverte jusqu'à validation matérielle exigée**, comme stipulé dans son mandat.
