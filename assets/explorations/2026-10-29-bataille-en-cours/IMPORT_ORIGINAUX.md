# Blocage et procédure d'import des PNG exacts dans PR #2

**À l'attention du Pilotage / Graphiste — correction technique encore nécessaire.**

Le connecteur GitHub peut modifier les fichiers texte et créer des objets Git mais **ne dispose d'aucun accès direct aux octets des PNG de l'environnement local**. Les copies JPEG du premier dépôt sont des rendus avec pertes ; elles ne remplacent **ni** les PNG originaux exacts, **ni** les aperçus natifs.

Les fichiers sont disponibles sous leurs noms exacts dans l'archive de remise `I009-exploration-bataille-en-cours-revue-pilotage.zip`. Pour les déposer de façon **durable et versionnée** sur **la même branche**, utiliser un environnement ayant un accès GitHub authentifié et l'archive :

```bash
git clone https://github.com/Kharn27/ESP32-OTA-Battleship.git
cd ESP32-OTA-Battleship
git switch art/etude-bataille-en-cours-20261029

DEST=assets/explorations/2026-10-29-bataille-en-cours
SOURCE=/chemin/vers/I009-ART-exploration-bataille-en-cours
cp "$SOURCE"/{01,02}_*original_1448x1086.png "$DEST/"
cp "$SOURCE"/{01,02}_*320x240.png "$DEST/"
cp "$SOURCE"/{01,02}_*zoom4x.png "$DEST/"
cp "$SOURCE"/COMPARATIF_natif_2x320x240.png "$DEST/"

# Vérifier chaque empreinte dans MANIFEST_SHA256.md avant commit.
sha256sum "$DEST"/*.png
git add "$DEST"
git commit -m "art: archive exact OpenAI PNG sources and 320x240 exports"
git push origin art/etude-bataille-en-cours-20261029
```

Le dépôt ne doit **pas** recevoir une conversion JPEG renommée en `.png`. Les octets et le SHA-256 des originaux doivent coïncider avec [MANIFEST_SHA256.md](MANIFEST_SHA256.md).

Après import : contrôler la liste `git ls-tree -r HEAD`, l'intégrité des sept PNG et le HEAD GitHub ; ajuster README et description de PR pour retirer la mention de blocage. Demander **une contre-revue Pilotage**, et **aucune fusion** sans sa décision.

Les règles de **secret des positions ennemies** ne changent pas : l'illustration narrative ne définit aucune information légitime de l'écran ATTAQUE.
