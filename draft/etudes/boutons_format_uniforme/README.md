# BTN-FMT-01 #163 — harmonisation réelle des cinq boutons 140×35

**CANDIDATE / ART_NOT_REVIEWED / DESIGN_PREVIEW / ON_DEVICE_UNVERIFIED.** GO artistique de cette nouvelle famille **en attente PO**. Les anciens GO restent limités aux anciens fichiers. Rien ne doit être promu dans `assets/` dans cette PR.

## Direction d'art

Cinq plaques maritimes peintes et rivetées, à **gabarit et présence visuelle harmonisés**, ratio **4:1** : JOUER bleu-vert/gouvernail ; PALMARÈS cuivre/trophée ; ATTAQUER ON rouge/canon avec tir ; ATTAQUER OFF rouge/canon au repos ; ABANDON acier sombre/drapeau blanc. Les mots peints et les décors sont ceux des **vraies sources**. Pas d'étirement global des images 140×32 ou 140×30 ; pas de police de substitution.

### Recomposition HD contrôlée

- **JOUER B** : master approuvé 1107×252 inchangé sur `main`, candidat **1108×277** ratio 4:1. Seules des bandes **vides d'ornements** du métal sont étendues (HD y=12–32 de +13 px, y=216–239 de +12 px), puis deux couloirs de métal (x=299–385 et x=884–970) sont élargis ; les deux capsules extérieures, le gouvernail et les rivets sont translatés de 24 px HD. Lettrage B gardé à sa taille réelle, puis réduction 4:1 proportionnelle.
- **PALMARÈS B** : master approuvé 1163×250 inchangé, candidat **1164×291**. Bandes métalliques y=12–32 de +21 px, y=213–238 de +20 px ; couloirs latéraux de métal x=290–342 et 1018–1062 étendus et capsules déplacées de 24 px. Trophée, rivets et mot avec accent **È** ne sont pas étirés. Le métal reconstitué est interpolé localement ; cette texture n'est pas strictement identique à celle cachée dans l'original.
- Les deux nouveaux masters sont éditables via `maitres/` **et** `generate.py`, puis réduits en mode alpha prémultiplié (`RGBa`, LANCZOS) à **140×35 RGBA**. Plaque visible des deux : boîte alpha>64 **[1,1,139,34]** ; pas seulement un canvas plus grand.
- **ATTAQUER ON/OFF** : copies **byte-for-byte** des PNG déjà approuvés `assets/boutons/attaquer_{on,off}_140x35_RGBA.png`, aucun changement artistique ; les deux masters 2000×500 de `assets/` sont contrôlés en provenance.
- **ABANDON** : création depuis **`draft/Bouton_Abandon.png` Git exact 2172×724**, SHA-1 blob `3818cb092456e1ff222d8b2dacf9f9aa55266135`, SHA256 `1b7c026a5ebcf7283de9ce93bf740fab350aa4aaf22da80641c34fe421c8719f`. Recadrage `(24,95)-(2148,626)` → master **2124×531** puis 140×35 proportionnel, même méthode/empreinte que [étude #149](https://github.com/Kharn27/ESP32-OTA-Battleship/pull/150) ; **pas** la copie de 2048×682 uploadée dans ChatGPT. ABANDON **sans GO PO**.

Les neuf sources sont vérifiées à leur **SHA-1 Git**, dimensions et mode RGBA ; leurs SHA-256 réels et ceux des 20 sorties figurent dans `rapport.json`.

## Livrables / preuves

- `maitres/` : **2 masters HD recomposés** et reproductibles.
- `exports/` : **5 PNG RGBA 140×35** ; les 2 ATTAQUER sont des copies exactes des sources approuvées. Statut des 5 = candidat de famille.
- `preuves/comparatif_{sombre,clair}_320x240.png` : **5 lignes existant / nouveau**, à échelle 1:1 sur deux fonds. JOUER B existant 140×32 et PALMARÈS B existant 140×30 sont centrés verticalement ; les nouveaux sont 140×35.
- `preuves/famille_f_sombre_clair_320x240.png` : cinq nouveaux dans les deux palettes de fond.
- `preuves/gabarit_contexte_320x240.png` : **test d'encombrement sur vrai canvas CYD 320×240**, deux colonnes et cinquième centré, marges et séparations réalistes. **PAS un nouvel écran d'accueil/combat**, aucune spécification de zones tactiles.
- Les quatre vues natives disposent de leur **1280×960 ×4 NEAREST** ; les cinq sprites ont leur zoom individuel ×4. Les prévisualisations restent uniquement sous `draft/`, **pas des assets plein écran**.
- `rapport.json` : provenance exacte, empreintes, tailles, boîte alpha visible, limites de validation.
- `generate.py` : source reproductible ; `requirements.txt` : versions d'outils épinglées.

## Vérification

Depuis un checkout propre, à la racine du repo :

```bash
python3 -m pip install -r draft/etudes/boutons_format_uniforme/requirements.txt
python3 draft/etudes/boutons_format_uniforme/generate.py --repo . --check
python3 scripts/check_graphic_pipeline.py
```

Pour régénérer les sorties dans un checkout de travail, retirer `--check`. `--check` vérifie **octet pour octet** les 20 PNG et le rapport sans aucune écriture ; les images sources sont auditées à leur blob Git original. Les étiquettes des planches seules utilisent les fontes DejaVuSans-Bold / LiberationSans-Bold du système ; **aucune police externe au graphisme source n'est utilisée dans les sprites**.

## Décisions et préservation

- [GO PO B historique #157](https://github.com/Kharn27/ESP32-OTA-Battleship/issues/157#issuecomment-6983850624) : ne valide **pas** les formats refaits.
- [GO PO ATTAQUER #141](https://github.com/Kharn27/ESP32-OTA-Battleship/pull/141#issuecomment-6924527916) : 140×35 inchangés.
- [PR Draft #150 ABANDON](https://github.com/Kharn27/ESP32-OTA-Battleship/pull/150) : référence **non approuvée artistiquement**.
- **Décision attendue du PO** sur la famille F avant toute promotion ; aucun test CYD physique, `ON_DEVICE_UNVERIFIED`.
- Diff limité à `draft/etudes/boutons_format_uniforme/`. **Aucun `assets/`, `maquettes/`, `draft/` original PO, firmware ou Bible touché**. Pilotage est seul habilité à fusionner.

Refs #163.
