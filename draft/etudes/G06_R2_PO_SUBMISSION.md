# G06-R2 — proposition visuelle du PO, reçue le 12/11/2026

**ÉTAT : PROPOSITION / CANDIDATE**, pas encore référence active ; [Issue de production G06-R2 #119](https://github.com/Kharn27/ESP32-OTA-Battleship/issues/119). Ce dossier **n'est ni un asset `assets/` ni un écran `maquettes/`**. L'actuelle référence G06 #47 / PR #51 reste canonique jusqu'à validation et remplacement contrôlé.

## Fichier de transmission
- [Image G06-R2 1024 × 768 — copie JPEG qualité haute](g06_r2_proposition_po_1024x768.jpg). **Source :** image **PNG RGBA originale 1024 × 768** jointe directement par le PO dans la discussion du 12/11/2026, numérisée/transférée via Adobe. Cette copie **n'est pas le PNG d'origine bit-identique**, mais une référence visuelle pour les agents GitHub ; elle ne doit pas être utilisée comme source maître d'exports définitifs sans l'original.
- Le même contenu d'origine est accessible dans l'espace Creative Cloud connecté sous le fichier `image(20261112-114539).png` ; cette information constitue une provenance, pas une URL GitHub stable. Les agents ne doivent pas présumer y avoir accès.

## Contenu et règles
Composition pixel art : **PALMARÈS**, panneau **VS CAPITAINE**, quatre nombres **PARTIES JOUÉES / VICTOIRES / DÉFAITES / MATCHS NULS**, bande **FORFAITS VOLONTAIRES — GLOBAL**, bouton **ACCUEIL / RETOUR**, paysage phare/voilier sur mer au soleil couchant. La bande **EXEMPLE** et les chiffres **12/07/03/02/01** ne sont que du remplissage graphique et n'entrent pas dans le jeu réel.

**Ne pas intégrer une capture aplatie comme fond du firmware.** Le Graphiste doit soumettre au PO une réduction/adaptation **320×240**, valider la lisibilité et livrer les sprites distincts réutilisables ; C/ESP32 rend les cinq statistiques de NVS en lecture seule. Voir [D15 #70](https://github.com/Kharn27/ESP32-OTA-Battleship/issues/70) et [SPIKE DEV #118](https://github.com/Kharn27/ESP32-OTA-Battleship/issues/118).

Aucune décision artistique sur des exports séparés n'est créée par ce stockage. **ON_DEVICE_UNVERIFIED.**
