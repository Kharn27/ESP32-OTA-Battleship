# GUI-01 #115 — Contrat et recette technique

Mandat : Issue #115, GO Pilotage du 11/11/2026. D15 #70, D14 #69 et
D15-A statique #112 restent indépendants. Aucun routage ou NVS.

## Composant C99

- components/gui_button/include/gui_button.h : GuiButton contient les
  rectangles visuel et tactile, action_id, état NORMAL / PRESSED / DISABLED.
  GuiButtons capture exclusivement un bouton par geste.
- Rectangles demi-ouverts ; DOWN dedans -> PRESSED ; MOVE hors hitbox,
  INVALID ou CANCEL -> NORMAL sans activation et verrouillage du geste
  jusqu'au UP ; UP valide dedans -> NORMAL et une activation. Pas
  d'activation au DOWN ni de transfert au bouton voisin.
- gui_rect_clip utilise des coordonnées et intermédiaires int64 ;
  d15a_compose_region attend une zone déjà clippée, dans 320x240,
  une taille <= 5120 octets et une bande DMA exclusive.
- Quatre assets approuvés depuis assets/manifest.json, sans modification.
  Le RGB565BE du bouton est multiplié par la luminosité 170/255 en
  canaux 5/6/5, avant blend sur fond (alpha8 droit). Alpha=0 laisse
  le fond bit-à-bit. Au relâchement, recomposition NORMAL depuis
  Flash, pas d'image pressed ni framebuffer.
- JOUER visuel [14,154)x[198,230), tactile [8,160)x[190,236) ;
  PALMARÈS visuel [166,306)x[199,229), tactile [160,312)x[190,236).
  Ces hitboxes restent candidates au test PO. IDs internes 1,2.

## Firmware de recette autonome

Option WARSHIP_GUI01_TEST_ONLY=ON. Incompatible avec D15-A statique et
les autres diagnostics. Le normal et D15-A ne lancent aucun tactile
GUI01. La sonde garde SPI3 comme propriétaire unique, lit les médianes
XPT2046 via warship_touch et transforme les coordonnées avec le profil
t02_map_d09_point (calibrage D09, ergonomie bord non prouvée).

Échantillonnage mono-tâche 10 ms, deux niveaux IRQ stables requis
pour DOWN et UP ; MOVE échantillonné pendant la pose ; une mesure
invalide annule le geste. Le relâchement utilise le dernier point
fiable échantillonné : la qualité avec stylet doit être vérifiée sur CYD.
Aucune routine lourde en ISR.

warship_lcd_send attend un sémaphore de fin de transfert DMA avant
réutilisation du tampon de 5120 octets. En cas d'erreur LCD la sonde
s'arrête, rétroéclairage coupé. Geométrie par redessin :
JOUER 140x32=8960 octets en 4 bandes et PALMARÈS 140x30=8400 octets
en 4 bandes ; nombres calculés, non latences mesurées.

## Tests et build

    python3 scripts/check_graphic_pipeline.py
    cmake -S tests -B build-host -G Ninja -DWARSHIP_SANITIZERS=ON
    cmake --build build-host
    ctest --test-dir build-host --output-on-failure

    idf.py -B build set-target esp32
    idf.py -B build build
    idf.py -B build-d15a -D WARSHIP_D15A_G01_TEST_ONLY=ON set-target esp32
    idf.py -B build-d15a -D WARSHIP_D15A_G01_TEST_ONLY=ON build
    idf.py -B build-gui01 -D WARSHIP_GUI01_TEST_ONLY=ON set-target esp32
    idf.py -B build-gui01 -D WARSHIP_GUI01_TEST_ONLY=ON build
    idf.py -B build-gui01 size
    idf.py -B build-gui01 size-components
    sha256sum build-gui01/cyd_warship.bin

CI doit fournir gui01-TEST-ONLY-HEAD (APP BIN, ELF, map, bootloader,
partition table, flasher_args.json) et PASS sur HEAD exact.

## Protocole physique — APRÈS GO DE FLASH DISTINCT DU PILOTAGE

1. Relever PR, HEAD complet, SHA256 du BIN, version de CYD, port et
   outillage. Flasher les offsets contenus dans flasher_args.json,
   sans jamais exécuter erase-flash.
2. Ouvrir l'UART 115200. Archiver WARSHIP_GUI01_TEST_ONLY=ON,
   PANEL_PROFILE, GUI01_READY, GUI01_REDRAW, GUI01_ACTIVATE,
   GUI01_CANCEL, GUI01_COUNTERS et GUI01_MEM.
3. Appuyer et maintenir chaque bouton ; constater l'assombrissement
   du sprite seul, la stabilité du fond, retour normal et durée.
4. Glisser hors zone et jusqu'au voisin, relâcher et vérifier 0 action.
   Valider une pression normale : un compteur par relâchement.
5. Tester la frontière x environ 160, stylet rapide/lent, faux contacts,
   absence de halos, flicker, lisibilité et latence ressentie.
6. Transmettre les logs, une vidéo/photo et observations au Pilotage
   pour revue et arbitrage ergonomique PO. Une seule CYD suffit ici.

Aucun build ou test hôte ne constitue ON_DEVICE_VERIFIED. Aucun
succès de navigation produit D15 n'est revendiqué.
