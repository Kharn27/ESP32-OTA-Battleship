# D15-PREP — Faisabilité G06 PALMARÈS : graphismes, primitives, NVS lecture seule

**Issue** : [#118](https://github.com/Kharn27/ESP32-OTA-Battleship/issues/118) — SPIKE DEV autorisé le 12/11/2026, LOT 1.  
**Base auditée** : `main` `55ee8b72a098d3da5e28bac45bb1a2dde7129f4a`.  
**Branche documentaire** : `spike/issue-118-g06-feasibility`.  
**Statuts** : G06 **`VALIDATED_PO_PC` / `LEGACY_UNDECOMPOSED` / `ON_DEVICE_UNVERIFIED`** ; **pas** un asset firmware.  
**Limite** : cette étude ne code ni HOME D15 #70, ni PALMARÈS produit, ni graphisme, ni migration NVS ; elle ne constitue pas un GO artistique, matériel ou D15.

## 1. Sources vérifiées et hiérarchie

- Règles permanentes [AGENTS.md](../AGENTS.md), [pipeline graphique](GRAPHIC_PIPELINE.md), [workflow](WORKFLOW_AGENTS.md), [tests](TEST_POLICY.md).
- [Bible](../BIBLE_PRODUIT.md) **§14.8** (LOT1-STAT-001/ABD-001, résultats vs forfaits) et **§15** (accueil et navigation). Les cinq statistiques et la non-comptabilisation des forfaits comme parties sont **DÉCIDÉES** ; le dessin des cellules et sa capacité variable n'est pas une règle métier implicite.
- [screen.json](../maquettes/palmares/screen.json) : `size:[320,240]`, `assets:[]`, `VALIDATED_PO_PC`, `LEGACY_UNDECOMPOSED`, `ON_DEVICE_UNVERIFIED` ; GO de la **maquette entière** [PR #51](https://github.com/Kharn27/ESP32-OTA-Battleship/pull/51), pas des sprites.
- [README G06](../maquettes/palmares/legacy/README.md), [SVG éditable](../maquettes/palmares/legacy/g06_palmares_320x240.svg), [aperçu 320×240](../maquettes/palmares/legacy/g06_palmares_320x240.png), [vignette zéro 252×108](../maquettes/palmares/legacy/g06_zero_partie.svg), [générateur de préviews](../maquettes/palmares/legacy/export_previews.py). Ces fichiers restent **intacts**, sous `legacy/` et ne peuvent pas être importés tels quels dans `assets/`.
- [Catalogue](../assets/manifest.json), [contrat G01 D15-A](D15A_G01_PREVIEW.md), [GUI-01 #115](GUI01_BUTTON.md), [D08 NVS](D08_PALMARES_NVS.md), [D11 événements](D11_EVENTS_NVS_CYD.md), [code du réducteur](../components/warship_palmares/warship_palmares.c), [backend NVS](../components/warship_palmares_nvs/warship_palmares_nvs.c) et [pont D11](../components/warship_event_bridge/warship_event_bridge.c).
- États au lancement : GUI-01 [PR #117](https://github.com/Kharn27/ESP32-OTA-Battleship/pull/117) intégrée à `main`; D14 [PR #91](https://github.com/Kharn27/ESP32-OTA-Battleship/pull/91) reste **Draft/bloquée** par G03 [#111](https://github.com/Kharn27/ESP32-OTA-Battleship/issues/111) ; D15 [#70](https://github.com/Kharn27/ESP32-OTA-Battleship/issues/70) **non démarrée**.

## 2. Inventaire de la composition PC (faits constatés)

Le SVG G06 fait **320×240** et contient **7 groupes nommés principaux**, cinq sous-groupes `data-stat`, **263 rectangles**, **32 chemins**, **9 polygones** et **16 textes**. Les chiffres `12/07/03/02/01` sont **fictifs** ; `EXEMPLE` est une étiquette réservée à la démonstration PC. La vignette zéro (252×108) est une **preuve de composition secondaire**, pas un nouvel écran de runtime. La police SVG `DejaVu Sans Condensed` du PC **n'est pas** un asset de fonte embarqué homologué.

Répartition visuelle issue du SVG et du README (boîtes **approximatives**, non hitboxes contractuelles) :

| Couche G06 / groupe SVG | Zone nominale (px) | Traitement candidat | Verdict pour D15, et justification |
|---|---|---|---|
| `scene-maritime-originale-g01-ocean-continu` : ciel, soleil, océan, reflets | Toile `[0,320)×[0,240)` | Image de scène *nue* séparée, sans journal/texte/bouton ; éventuellement primitives simples pour larges aplats, mais **199 rectangles, 8 chemins, 3 polygones** et transparences dans la scène | **`ASSETS_GRAPHISTE_REQUIS`** pour reproduire le décor approuvé sans réinterprétation arbitraire ; aucune scène G06 approuvée individuellement |
| `navire-guerre-illustre-ancre-dans-la-mer` : bâtiment narratif avec coque, canons, sillage | Droite, voisinage de x≈230–315, autour de la ligne d'eau ; borne exacte à exporter | Sprite RGBA transparent **séparé** si réutilisable, ou élément composité **dans le seul fond nu** avec décision PO/DA documentée | **`ASSETS_GRAPHISTE_REQUIS` / `ARBITRAGE_PO_REQUIS`** sur découpage. Ce n'est **pas** un navire tactique |
| `entete-palmares-et-avertissement-fictif` : barre, filet, titre | `[0,320)×[0,29)` | Barres + lignes par primitives C99 ; titre au moyen de glyphes Flash. **Supprimer EXEMPLE uniquement de la présentation réelle**, sans retoucher la maquette oracle | **`PRIMITIVES_POSSIBLES`**, mais fonte et rendu du titre **`ARBITRAGE_PO_REQUIS`** |
| `journal-de-bord-stats-capitaine` : cadre cuivre, intérieur, rivets, texture | Enveloppe `[12,216)×[43,166)` soit 204×123 | Lignes, à-plats, séparateurs par C ; bord biseauté/ornements colorés soit par export individuel transparent **sans aucune inscription/chiffre**, soit primitives après comparaison fidèle | **`ARBITRAGE_PO_REQUIS`** : privilégier asset décoratif approuvé pour fidélité ; **`PRIMITIVES_POSSIBLES`** pour la géométrie simple |
| `VS CAPITAINE` et libellés des quatre cellules | Bloc intérieur environ `[18,210)×[67,160)` | Champs de texte fixe, glyphes Flash, grille et aplats dessinés en C ; aucune donnée exemple | **`PRIMITIVES_POSSIBLES`**, sous réserve d'une police/licence/validation visuelle et accents FR |
| Quatre `data-stat` (`parties`, `victoires`, `defaites`, `nuls`) | Baselines (29,107), (127,107), (29,155), (127,155), taille maquette 24 px | **Cinq valeurs réelles uniquement** à partir de `WarshipPalmaresView`, format décimal non borné à 99, glyphes générés, couleurs et champs distincts | **`PRIMITIVES_POSSIBLES`** pour chiffres, **`ARBITRAGE_PO_REQUIS`** pour débordement ≥ 6 chiffres (voir §4) |
| `forfaits-volontaires-globaux-distincts` : plaque, texte GLOBAL, case numérique | `[12,216)×[173,217)` ; case `[166,207)×[182,210)` | Plaque cuivre décorative candidate à exporter sans mots ni chiffres, ou primitives validées ; valeur `voluntary_forfeits` distincte et dynamique | **`ARBITRAGE_PO_REQUIS`** pour la plaque et son format numérique ; **`PRIMITIVES_POSSIBLES`** pour cadre/lignes/texte |
| `retour-volontaire-vers-accueil` : plaque cuivre, flèche, ACCUEIL, RETOUR | ≈ `[221,316)×[173,217)` ; visuel du README ≈94×41 | Sprite individuel approuvé **ou** primitive C (polygones, flèche, texte) si PO accepte conformité. `GuiButton` GUI-01 pour hitbox distincte et événements ; renderer G06 à produire plus tard | **`ARBITRAGE_PO_REQUIS`**, sinon **`ASSETS_GRAPHISTE_REQUIS`** pour conserver exactement le bouton dessiné. Aucune hitbox approuvée sur CYD |
| `ecume-et-profondeur-premier-plan` | Bas de l'écran, y≈225–239 | Filet et écume dessinables en petites primitives, ou composés au fond marin | **`PRIMITIVES_POSSIBLES`** si fidélité testée ; toute retouche de décor demande validation |

**Arbre de composition candidat** (non implémenté) : fond G06 nu → navire décoratif (s'il est séparé) → encadrement du journal / plaque globale / bouton (assets ou primitives approuvées) → titres/libellés statiques (font approuvée) → **5 chiffres dynamiques** → éventuels effets GUI-01 pendant l'appui. L'ordre effectif d'écume et de navire doit respecter le SVG ; la répartition exacte des calques reste au Graphiste/PO.

### Catalogue existant : réutilisation réelle ≠ similarité visuelle

`assets/manifest.json` ne référence **aucun asset PALMARÈS G06**. Il contient notamment ces quatre exports CYD **approuvés individuellement** pour G01 :

| ID stable | SHA-256 PNG | Usage G06 |
|---|---|---|
| `wallpaper.accueil.retenu.cyd` | `afb167ac27d091ccde416c5644faff4e5ac191df19753cab6a24db5a0c91a1c6` | **Non substituable** au décor propre à G06 sans nouvel arbitrage artistique |
| `logo.titre.cyd` | `24d31c91af583fe794871c8f1e3452f93b394fddb68649c0c780ffa09a7b2df7` | Logo « CYD WARSHIP », **pas** titre « PALMARÈS » |
| `boutons.jouer.cyd` | `e01df0c989bfd7f15e6fa649b245d7f3d702f1f51c33e756786c568cc199b9fc` | Mauvaise commande |
| `boutons.palmares.cyd` | `a90b0b581253365b128367196c7e5fc1e25ec811db579b856f41e73c5a8c69ca` | Ouvre PALMARÈS depuis HOME, **pas** ACCUEIL depuis G06 |

Les PNG `maquettes/palmares/legacy/*`, les sources SVG, les faux nombres, `EXEMPLE`, les visuels de `draft/` et le sous-marin de flotte ne peuvent **pas** devenir des actifs G06 par simple déplacement/copie. Une validation PC globale n'est pas l'homologation individuelle du fond, des plaques, des sprites ou de la fonte.

## 3. Typographie : faisabilité et licence, sans adoption implicite

Le SVG décrit des textes en **DejaVu Sans Condensed** (poids 700/800), tailles maquette : titre 16 px, VS CAPITAINE 13, étiquettes 9,1–10,6, chiffres VS 24, chiffres GLOBAL 21, ACCUEIL 13, RETOUR 9. Il **n'embarque aucune fonte TTF ni atlas binaire**.

**Piste technique** : génération **hors cible** d'un atlas de glyphes en Flash (latin FR + ponctuation, `0–9`) à partir d'une fonte dont la licence et la forme ont reçu l'accord approprié ; blitter des masques d'alpha 1/4/8 bits dans la bande RGB565, sans parser TTF/SVG ni FreeType à l'exécution. Les rectangles/lignes peuvent utiliser des primitives C99 bornées. Il n'existe **pas** dans les composants actuels de moteur de texte général G06 à réutiliser tel quel.

**Licence à instruire** : DejaVu utilise la licence des polices Bitstream Vera et des apports distincts (selon les glyphes) ; cette licence autorise généralement la redistribution sous conditions de maintien des notices, et de nommage pour les modifications. Référence officielle : [projet DejaVu](https://dejavu-fonts.github.io/), [LICENSE amont](https://github.com/dejavu-fonts/dejavu-fonts/blob/master/LICENSE). Avant incorporation, vérifier précisément variante, attribution, sous-ensemble de glyphes, licence dans le firmware/distribution et GO typographique PO. **Aucun fichier de police n'est ajouté par ce SPIKE.**

**Option typographique : `ARBITRAGE_PO_REQUIS`** (DejaVu rasterisé fidèlement vs police bitmap alternative approuvée ; ne pas substituer silencieusement une « police système »). Le texte français doit garder accents et lisibilité sur vraie CYD.

## 4. Preuve de capacité des nombres : limite concrète du dessin

Le stockage D08 est **uint32_t** pour chacun des cinq compteurs (plage `0..4294967295`), sans plafond métier 99. Le dessin de G06 et la vignette zéro n'illustrent que **deux chiffres**, sans établir un maximum.

**Sonde PC jetable exécutée localement, sans fichier ajouté** : Pillow **12.3.0**, `/usr/share/fonts/truetype/dejavu/DejaVuSansCondensed-Bold.ttf` sur l'hôte Linux (SHA-256 locale du fichier : `21f02ebcea75529d3ed9d26e089cd1b9c59a4b784119ba3a96e9bc83cf755ec7`), méthode `ImageFont.truetype(...).getlength()`. **Mesure hôte**, pas rendu SVG garanti identique et pas preuve LCD.

| Texte décimal | 24 px, VS (px) | Fenêtre utile VS estimée ≈79 px | 21 px, GLOBAL (px) | Fenêtre GLOBAL estimée ≈39 px |
|---|---:|---|---:|---|
| `00` | 30,1 | passe | 26,3 | passe |
| `99` | 30,1 | passe | 26,3 | passe |
| `100` | 45,1 | passe | 39,4 | **dépasse légèrement** |
| `99999` | 75,2 | passe de justesse | 65,7 | **dépasse** |
| `100000` | 90,2 | **dépasse** | 78,8 | **dépasse** |
| `4294967295` | 150,3 | **dépasse** | 131,4 | **dépasse** |

Contrat candidat : formater tous les champs en décimal `uint32_t`, buffer **au moins 11 caractères** par valeur (10 chiffres + NUL), un `snprintf` borné avec `PRIu32`, zéro-padded sur **2 caractères minimum** uniquement si le PO confirme cette présentation. En aucun cas : `%02u` comme hypothèse de plage, saturation à 99, troncature, superposition incontrôlée, ellipses masquant un chiffre, remise à zéro fictive. La vue doit accepter les cinq valeurs exactes.

**Décision bloquante avant G06 runtime** : conserver les cellules 2×2, définir une réduction de fonte par tranche, une extension contrôlée des champs, un réagencement ou un affichage alternatif **validé par PO** pour 3–10 chiffres (particulièrement le global dès trois chiffres). Cette étude **ne choisit pas** le comportement d'affichage au nom du PO.

## 5. Contrat NVS → vue, lecture seule et erreurs

### API constatée, sans modification

- [`warship_palmares_nvs_init()`](../components/warship_palmares_nvs/warship_palmares_nvs.c) appelle **seulement** `nvs_flash_init()`. Son erreur est propagée (dont `ESP_ERR_NVS_NO_FREE_PAGES` / `ESP_ERR_NVS_NEW_VERSION_FOUND`), **sans** `nvs_flash_erase()`.
- [`warship_palmares_nvs_backend()`](../components/warship_palmares_nvs/warship_palmares_nvs.c) retourne `load` + `store`. `load()` fait `nvs_open("warship", NVS_READONLY)`, puis `nvs_get_blob(...,"palmares")`. Namespace ou clé réellement absents → `load=1`. Erreur NVS → `load=-1` et `context.last_error`. BLOB existant de longueur ≠44 et checksum/version/valeurs invalides → erreur au cœur.
- [`warship_palmares_open(&ledger,backend,&context)`](../components/warship_palmares/warship_palmares.c) exige **les deux callbacks `load` ET `store` non NULL**, même si l'appel ne fait qu'une lecture. **Ne pas** proposer `backend.store=NULL` sans refonte/contrat approuvé. `open()` lit un snapshot 44 o, vérifie magic/version/checksum/identité/invariant, et ne modifie pas `out` si erreur.
- `warship_palmares_read(&ledger,&view)` copie le cache validé **sans accès NVS**, sans mutation, et retourne `RELOAD_REQUIRED` sur ledger incertain. `WarshipPalmaresView` contient `uint32_t games,victories,defeats,draws,voluntary_forfeits` (données utiles **20 o par cinq uint32_t**, sans présumer du padding des enveloppes).
- **Intégration actuelle D11** : [`d11_runtime.c`](../main/d11_runtime.c) possède un `static WarshipEventBridge bridge` et un seul `WarshipPalmaresNvsContext`. `d11_runtime_init()` initialise NVS et ouvre le bridge au boot. [`warship_event_bridge_read()`](../components/warship_event_bridge/warship_event_bridge.c) fournit déjà les cinq chiffres du dernier état **durablement reconnu** en mémoire. **Attention :** `d11_runtime.h` n'exporte **pas encore** d'accesseur UI vers ce bridge ; aucune interface d'écran n'est implémentée.

### Proposition d'interface D15, À VALIDER PAR PILOTAGE (pas de code dans ce SPIKE)

**Préférence** : rendre disponible côté futur contrôleur D15 un accesseur borné `d11_runtime_palmares_snapshot(WarshipPalmaresView *out)` (nom **PROPOSÉ**, pas fonction existante), retournant `WarshipEventStatus`, qui appelle **uniquement** `warship_event_bridge_read(&bridge,out)` lorsque D11 est actif. Le renderer reçoit une **copie** de `WarshipPalmaresView` et un statut, **jamais** de ledger, backend ni accès `warship_palmares_apply`. Pas de nouveau `nvs_flash_init` / accès concurrents / lecture NVS dans la couche UI ; le writer D11 reste propriétaire.

Alternative si une **relecture physique à chaque entrée** est exigée : appel contrôlé à l'API D08 depuis une façade lecture seule dans l'orchestrateur, à séquencer avec le propriétaire D11 et ses commits ; cette option nécessite accord Pilotage pour éviter deux états de vérité/caches. Ne jamais rouvrir une NVS indépendante dans le renderer ni interpréter un snapshot ancien comme récent.

**Étapes proposées (non exécutées)** : (1) lire l'état de D11 ; (2) si résultat `APPLIED`, copier la vue locale ; (3) vérifier défensivement `(uint64_t)victories+defeats+draws == games` ; (4) préparer cinq décimaux bornés et uniquement alors composer ; (5) sur échec, **interdire l'affichage de valeurs nulles fictives** et remonter cause/status au contrôleur/diagnostic, sans écrire NVS. Aucun `store`, `apply`, `reload` arbitraire, `nvs_set_blob`, `nvs_commit`, `nvs_erase` ni reset dans le chemin de consultation. Ne pas toucher à la clé de réservation d'ID D11.

### Table de décision de consultation — obligatoire pour D15

| Situation NVS / D11 | Code observé/attendu | Ce que doit faire la vue |
|---|---|---|
| Namespace ou clé palmarès **réellement absents**, aucune incohérence de bridge | `load=1`, ouverture `APPLIED` | Afficher **5×00** (zéros réels d'un stockage vierge), **aucune écriture/initialisation de blob** |
| Blob V1 **valide** | `APPLIED` puis `read=APPLIED` | Afficher 4 valeurs VS Capitaine + 1 forfait GLOBAL **exactes**, après invariant ; ex. non fictif |
| Un commit récent est en attente/incertain | bridge `PENDING` / ledger `RELOAD_REQUIRED` selon étape | Ne jamais inventer ni créditer le résultat. Seul dernier état durable validé peut être montré comme tel ; politique d'attente visible à soumettre au PO |
| NVS non initialisable, clé inaccessible, faute SPI Flash / `nvs_get_blob` | `READ_ERROR` / cause `esp_err_t` | **État indisponible, aucun 0 de secours**, journal diagnostic, pas d'effacement |
| Blob incomplet/surdimensionné, magic ou FNV erronés, somme ou type incohérents | `CORRUPT` | **État indisponible**, aucune réinitialisation, aucune mutation |
| Version NVS incompatible | `INCOMPATIBLE_VERSION` | **État indisponible**, pas de migration automatique |
| Ledger non initialisé, pointeur invalide | `BAD_ARGUMENT` | Faute d'intégration : ne pas dessiner de chiffres, log utile |
| Forfaits uniquement (ex. 0 partie, 2 forfaits) | V1 valide, `games=0`, `voluntary_forfeits=2` | Montrer `00/00/00/00` VS Capitaine **et 02 GLOBAL**, ne pas intégrer forfait dans `games` |
| Compteurs supérieurs à 99 ou proches de `UINT32_MAX` | Snapshot V1 cohérent | Garder valeurs exactes ; résoudre la **présentation sans perte** selon arbitrage §4 |

Le cœur D08 applique `games = victories+defeats+draws` en **arithmétique 64 bits** et conserve l'ancien `out` lors d'une lecture échouée. Invariant additionnel de la couche UI : **ne pas publier le buffer temporaire tant que la lecture n'a pas réussi**, même si la sortie précédente n'a pas été modifiée. Ne pas afficher `EXEMPLE` ni les valeurs de démo.

**Arbitrage PO requis** : état visible sur erreur réelle NVS / transaction en attente (« indisponible », panneau, retour, etc.). Ce document ne crée aucun écran d'erreur ni texte joueur normatif.

## 6. Budget (ordres de grandeur, PAS mesures d'intégration)

La CYD cible 320×240, RGB565 (2 o/px), **sans PSRAM présumée**, sans microSD. Le LCD dispose d'un seul tampon DMA de **5120 o**, `warship_lcd_send()` attend la fin DMA avant réutilisation (API du profil LCD sur `main`, voir [header](../components/warship_lcd/include/warship_lcd.h) et [GUI-01](GUI01_BUTTON.md)). Aucun framebuffer complet à allouer.

| Hypothèse de représentation **non décidée** | Calcul géométrique théorique | Statut |
|---|---:|---|
| Fond propre G06 RAW RGB565, 320×240 | **153600 o Flash** | **ESTIMATION**, pas de fond G06 exporté |
| Journal statique sans texte, boîte 204×123, RGB565BE+alpha8 | 204×123×3 = **75276 o Flash** | **MAJORANT/ESTIMATION** de cette boîte |
| Plaque globale statique 204×43 RGBA équivalent RGB565+alpha8 | **26316 o Flash** | **ESTIMATION** |
| Bouton retour 94×42 RGB565+alpha8 | **11844 o Flash** | **ESTIMATION** |
| En-tête décoratif 320×29 RGB565+alpha8 | **27840 o Flash** | **ESTIMATION** (peut être primitives C) |
| **Somme des seules boîtes ci-dessus** | **294876 o** | Exemple de budget **non optimisé, hors navire séparé, fontes, code, padding, compression, métadonnées et chevauchements** |
| Une bande complète LCD | 320×8×2 = **5120 o RAM DMA empruntés** | **Budget actuel du composant** ; pas une allocation G06 supplémentaire |
| Redessin visuel bouton 94×42 | **7896 o transférés** en 6 bandes de hauteur ≤8 | **ESTIMATION géométrique** |
| Redessin 4 cellules de nombres d'environ 80×30 et case GLOBAL 41×28 | (4×80×30 + 41×28)×2 = **21496 o** | **ESTIMATION illustrative**, à mesurer après contrat de boîtes |
| Copie de cinq compteurs `uint32_t` | **20 o utiles** | **Taille des champs C**, pas coût complet ledger/backend/pile |
| Buffer texte maximal par valeur `uint32_t` | **11 octets** (10 chiffres + NUL), réutilisable | **Minimum à provisionner**, hors mémoire glyphes |
| Glyphes décimaux 10×(24×24) | **720 o à 1 bit/pixel** ou **5760 o à 8 bits/pixel** | **Estimation de masque plein par glyphe**, pas atlas approuvé ni dimension mesurée |

**Contexte mesuré antérieurement, pas budget G06** : la CI GUI-01 sur le HEAD `845beaf47f4c388f36e6e1a63e94a59accfcd873` a rapporté image ESP-IDF 433369 o, DRAM statique 14024 o, IRAM 64143 o pour son firmware **isolé** ([workflow](https://github.com/Kharn27/ESP32-OTA-Battleship/actions/runs/43015915939)). Cela ne mesure **ni** le binaire NORMAL futur D15 **ni** G06. Les chiffres `heap8`, `largest8`, piles Xtensa, latence affichage et latence stylet G06 sont **NON MESURÉS**. Aucune prédiction de « zéro scintillement ».

**Choix technique proposé** : données et glyphes en Flash, compositions par bandes ou zones sales, un seul buffer DMA avec contrôle de sérialisation, redessin des chiffres **à l'entrée ou en cas de nouvelle valeur** et de la seule zone bouton sur geste ; pas de PNG G06 complet, pas de TTF/SVG décodé sur ESP32, pas de double buffer plein écran. **Pilotage doit valider l'interface renderer, les couches/ressources et l'enveloppe avant D15.**

## 7. Dépendances et découpage D15 recommandé (PAS un GO D15)

1. **G03 P0 #111 → D14 #69/PR #91** : toujours bloquants pour le véritable transport/runtime écran normal et le déploiement. Ne pas intégrer G06 dans un firmware normal qui contournerait D14.
2. **G06 graphique** : décision Pilotage sur Issue Graphiste ultérieure (sans interrompre G03), livraison d'une **scène nue** et, selon validation, d'un **navire décoratif** et/ou **plaques + bouton**, avec IDs propres et SHA256/GO PO dans `assets/manifest.json` ; refonte de la composition non legacy par les IDS selon pipeline. Le SPIKE ne déplace/modifie aucun fichier de `legacy/`, `draft/`, `maquettes/`, `assets/`.
3. **Typographie + débordement** : fournir un contrat de glyphes et variantes chiffrées réels `0, 9, 99, 100, 999, 1000, 99999, 100000, UINT32_MAX` ; accord PO pour la préservation de tous les chiffres, seuils de bascule de taille, contrastes et mention d'erreur.
4. **Cœur lecture seule** : conserver D08/D11 comme source, valider l'accesseur minimal du propriétaire D11 (sans permettre apply depuis la UI), tableau de statuts fail-closed ci-dessus, absence de seconde ouverture contradictoire. Tests hôte de cas absents, corrompus, incompatibles, erreurs I/O, retry incertain et séparations des forfaits.
5. **Intégration D15 future**, **uniquement après GO distinct Pilotage** : composition 320×240 à partir d'assets individuels, textes dynamiques, retour ACCUEIL, réutilisation de l'état/gestes C99 GUI-01. **Le `d15a_compose_region` actuel est spécifique aux quatre calques G01** : son réemploi tel quel ne suffit pas pour le fond/plaque/bouton G06 ; le compositor doit accepter les nouveaux descripteurs sans effet sur les quatre assets validés. Traitement « pressed » de bouton dessiné en primitives = contrat renderer à compléter, pas compétence déjà prouvée.
6. **Preuves futures** : hôte golden pixels/champs larges/sentinelles/no NVS writes ; build NORMAL ESP-IDF v5.5.6/CI HEAD exact ; mesures Flash/DRAM/IRAM/heap8/largest8/stack/redraw ; recette sur **une vraie CYD** (stylet, font, échecs NVS et NVS préservée), puis revue/merge exclusivement Pilotage.

## 8. Brief de besoin Graphiste G06 — PRÊT À ATTRIBUER, NON ATTRIBUÉ

**Niveau requis** : ressources **candidates** indépendantes de G06, à découper depuis le **SVG original approuvé** de [G06 #47 / PR #51](https://github.com/Kharn27/ESP32-OTA-Battleship/pull/51) sous contrôle PO. Ce n'est pas une demande de produire avant fin P0 G03 #111.

- **Indispensable** : décor maritime natif **320×240** sans aucun texte, chiffres fictifs, journal, plaques ou boutons ; préserver le navire militaire narratif de G06. Décider avec PO si navire est intégré au fond *en tant que scène décorative* ou sprite RGBA séparé avec rectangle alpha/bounds/ordre explicités.
- **À trancher** : journal de bord décoratif **zone ~204×123 à (12,43)** et plaque globale **~204×43 à (12,173)** sans aucun mot/chiffre ; livrer seulement si reconstruction par primitives fidèle **n'est pas validée par PO**. Sources éditables, canal alpha de vrais composants, pas de découpe d'une capture composant texte inclus.
- **À trancher** : bouton **ACCUEIL/RETOUR zone ≈94×41 à (221,173)** avec contour, flèche, éventuelle typographie approuvée et alpha. Un sprite de bouton complet statique ou plaque sans texte avec glyphes dynamiques selon décision PO ; GUI-01 génère le pressed logiciel, aucun nouveau pressed PNG par défaut.
- **Pour chaque export** : original source conservé, usage et ancrage `x,y,w,h` natifs, ordre des couches, format RGBA ou RGB avec alpha si nécessaire, zones transparentes/anti-alias, mémoire cible estimée, ID manifeste stable, SHA-256 et lien du **GO explicite PO individuel** ; statut matériel `ON_DEVICE_UNVERIFIED` tant que non reçu.
- **Ne pas inclure** `EXEMPLE`, les valeurs `12/07/03/02/01` et un fond avec textes/boutons déjà dessinés ; aucune copie des sprites du G01, ni du G03, ni du navire tactique FLOT-01.

## 9. Preuves du SPIKE et points de décision à transmettre

| Contrôle | État au moment du document | Portée / limite |
|---|---|---|
| Lecture Issue/AGENTS/Bible/pipeline/manifest/G06 SVG & README/socles D08-D11 GUI01 | **PASS — audit de sources sur `main 55ee8b72…`** | Aucun fichier graphique modifié |
| Inventaire SVG groupé et éléments | **PASS** : 7 groupes principaux, 263 rect, 32 path, 9 polygon, 16 text | Caractérisation de structure, **pas** validation artistique |
| Vérification dimensions de chiffres à la fonte hôte | **PASS (mesure PC)** : Pillow 12.3.0 et police locale SHA indiqués §4 | Font hôte, **pas** rendu réel TFT |
| `python3 scripts/check_graphic_pipeline.py` | **NON EXÉCUTÉ dans cet environnement** : pas de checkout intégral accessible (Git réseau indisponible) | Aucun asset ou screen.json modifié ; reste à exécuter en environnement dépôt/CI |
| Tests C99 G06 / compilation ESP32 G06 | **NON APPLICABLE** : pas de code firmware G06 dans le SPIKE | Les tests D08/GUI-01 antérieurs ne valident pas G06 |
| `HOST_TESTED` G06, `TARGET_BUILD_VERIFIED` G06, `ON_DEVICE_VERIFIED` G06 | **NON TESTÉ / NON APPLICABLE** selon niveau | La PR documentaire ne constitue aucune validation cible |

**Décisions attendues du Pilotage et du PO** : (A) quels blocs G06 exigent des assets distincts plutôt que les seules primitives et dans quel ordre les demander au Graphiste après G03 ; (B) police licenciée et approuvée, marges et chiffres jusqu'à 10 positions ; (C) comportement visible d'une erreur NVS, d'une lecture incertaine et du bouton retour ; (D) interface lecture D11 accessible par D15 et contrat de rendu avant toute implémentation.

**Conclusion de faisabilité** : **faisable techniquement en C99/ESP-IDF sous conditions**, sans inventer de jeu ni écrire NVS, **mais pas intégrable fidèlement avec le catalogue graphique actuel**. Bloqueurs : découpage/validation individuelle de G06 + police/dépassement chiffré + dépendance D14/G03. Le présent rapport est **une demande de décisions et de ressources, pas une réception de D15**.
