# I-009 — Politique de validation et de preuves (C / ESP32 CYD / ESP-NOW)

**Objet :** ne jamais confondre une exécution PC, une compilation ESP32, une CI verte et une preuve sur CYD réelle. Cette politique fixe les **niveaux et traces attendues**, sans imposer prématurément le framework, une version ESP-IDF ou des budgets mémoire non caractérisés.

Cible produit : **deux ESP32 CYD classiques ESP32-2432S028R**, écran tactile 320×240 paysage ; LOT 1 solo sur une seule carte. Aucune PSRAM présumée, aucune microSD, assets runtime dans la Flash. L'usage du tactile/stylet, des écrans et de la radio reste à valider sur **le matériel réellement possédé**.

## 1. Niveaux de preuve indépendants

| Niveau | Ce qu'il démontre | Preuves à conserver | Ce qu'il **ne** démontre **pas** |
| --- | --- | --- | --- |
| **HOST_TESTED** | Règles / parsers / machine à états C testés sur PC, éventuellement sous sanitizers | OS/compilateur, commande, jeux de cas, tests PASS/FAIL, commit exact | Compilation ou comportement ESP32 |
| **TARGET_BUILD_VERIFIED** | Build cible ESP32 reproductible dans l'environnement sélectionné | Version outillage réellement utilisée, commande, logs, binaire, commit et cible, avertissements notables | Fonctionnement écran/tactile/radio réelle |
| **CI_PASSED** | Jobs explicitement définis et exécutés par GitHub Actions sur le HEAD indiqué | URL de run, SHA, noms des jobs, résultats | Tests physiques ; couverture au-delà des jobs présents |
| **ON_DEVICE_VERIFIED** | Scénario exécuté sur une CYD physique avec le firmware correspondant | SHA, carte, version matérielle, procédure, photos/logs et observations | Fonctionnement de la liaison à deux CYD |
| **ESP_NOW_2_CYD_VERIFIED** | Échange radio/application testé sur **deux** CYD physiques | Deux cartes, firmwares/SHAs, canal/configuration, étapes, logs émission/réception et scénarios négatifs | Chiffrement/sécurité ou robustesse non testés par ce protocole |
| **DESIGN_PREVIEW** | Aspect graphique et exports examinés sur PC | PNG 320×240, zoom x4 nearest-neighbor, source, revue PO, dimensions | Affichage/tactile ou intégration Flash/RAM sur CYD |

Utiliser `PASS`, `FAIL`, `NON TESTÉ`, `NON APPLICABLE (justification)` : **ne jamais assimiler absence de test à un succès**. Chaque niveau de preuve est attaché à un SHA précis ; après modification pertinente, la preuve peut devoir être réacquise.

## 2. Exigences selon le type d'US

| Travail | Preuves minimales à prescrire dans l'Issue | Porte de fusion |
| --- | --- | --- |
| Documentation | Relecture diff, fidélité à la décision PO, liens et absence de changements de règles intempestifs | Revue Pilotage |
| Graphisme / maquette | Composite natif 320×240, zoom x4, source et dépendances, décision artistique du PO | Peut être accepté comme `DESIGN_PREVIEW / ON_DEVICE_UNVERIFIED` |
| Code de règles C pur | Tests hôte, cas limites et négatifs pertinents, absence d'UB ; build cible si l'US le requiert | Pilotage exige les preuves prévues par l'Issue |
| Intégration écran/tactile/Flash | Build cible, empreinte firmware, usage mémoire mesuré ; validation CYD si la mission affirme un fonctionnement réel | Pas de `ON_DEVICE_VERIFIED` sans test physique |
| ESP-NOW | Tests hôte du protocole, build cible, **essai deux cartes** pour toute affirmation de liaison fonctionnelle | Aucun `ESP_NOW_2_CYD_VERIFIED` par simulation ou essai une carte |

Une PR de code avec une preuve physique encore indisponible ne peut pas être déclarée entièrement conforme à une US qui **exige** cette preuve. Pilotage peut décider d'un jalon technique distinct seulement si le contrat de livraison le dit explicitement et que la validation manquante reste **ouverte et traçable** ; aucune fermeture silencieuse.

## 3. Contrôles C embarqué et mémoire

Lorsqu'ils sont pertinents, vérifier : séparation des règles et du matériel, tailles/bornes de tableaux, pointeurs, absence de comportement indéfini, entrées invalides, pertes de mémoire, piles FreeRTOS, plus grand bloc heap, allocation dynamique, latence des tâches et absence d'attente bloquante dans les callbacks.

Les **budgets** Flash / RAM / piles / buffers et l'environnement de build doivent être **caractérisés et approuvés avant d'être imposés**. Rapporter les mesures avec le SHA, la carte et le scénario ; une valeur estimée ne remplace pas une mesure. Éviter de supposer un framebuffer plein écran en RAM ou la présence de PSRAM. Les assets destinés au runtime doivent tenir en Flash et être accompagnés d'un contrat de décodage/rendu adapté.

## 4. Contrôles ESP-NOW et pair-à-pair

Pour la phase multijoueur (hors LOT 1), couvrir par des tests spécifiques :
- Découverte/pairage, canal radio cohérent et éventuelle coexistence Wi-Fi.
- Version et longueur de trame, validations des champs, ID de partie, numéro de manche et de séquence.
- **ACK applicatif** distinct du succès de livraison radio, timeout et retransmissions **bornées**.
- Déduplication, idempotence, messages perdus, dupliqués, retardés, hors ordre ou non autorisés.
- Déconnexion, reconnexion, redémarrage des deux cartes, désynchronisation et refus des commandes invalides.
- Secret des flottes et des tirs simultanés avant révélation ; protocole de verrouillage/révélation à définir avec le PO si un arbitrage métier est nécessaire.
- Propriétés de chiffrement, authentification et configuration ESP-NOW réellement prouvées ; **ne rien annoncer sécurisé sans preuve**.

Les callbacks radio restent courts ; les traitements métier passent par un chemin borné/déterministe, et leurs ACK ne sont émis qu'à un état approprié.

## 5. Quand solliciter le PO pour un test physique

Transmettre une fiche **reproductible** : Issue/PR, HEAD exact, firmware flashé et hash si possible, nombre de CYD (1 ou 2), branche/cible, câble/port/outillage requis, étapes numérotées, comportement attendu et attendu en cas d'échec, logs à collecter, photos/vidéo utiles.

Étiqueter le rapport selon la provenance : `ON_DEVICE_PO_REPORTED_PASS` si le PO a réalisé et rapporté le test ; ne pas reformuler en essai indépendant du Pilotage. Une compilation correcte n'est pas un test matériel ; un essai sur **une CYD** ne valide jamais une communication entre **deux CYD**.

## 6. Revue et verdict

Pilotage vérifie **le HEAD final**, le diff, les critères et les preuves demandées, puis peut prononcer :
- `CORRECTIONS_REQUISES` : écart constaté.
- `EN_ATTENTE_DE_PREUVES` : validation exigible non fournie.
- `EN_ATTENTE_ARBITRAGE_PO` : choix produit/artistique non tranché.
- `ACCEPTEE_POUR_FUSION` : critères de l'étape définie satisfaits.
- `FUSIONNEE` : **uniquement après confirmation GitHub de la fusion effective**.

La CI et les tests ne déchargent pas Pilotage de la revue contradictoire. Ne pas imposer rétrospectivement un test matériel à une simple maquette ou à un document, mais ne jamais déclarer `ON_DEVICE_VERIFIED` sans le protocole exécuté.
