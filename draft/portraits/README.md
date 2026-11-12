# Planches portraits — originaux PO (12 novembre 2026)

**Provenance :** cinq images PNG RGBA fournies directement par le PO dans la conversation de pilotage I-009 du 12/11/2026. Ce répertoire est un **stockage d'originaux PO**, non un catalogue d'assets runtime.

| Planche | Dimensions | Taille octets | SHA-256 du fichier d'origine |
|---|---|---:|---|
| `portrait_A.png` | 1448 × 1086 RGBA | 2623103 | `30e92fa1eed23733d812a521c6910390860620f906a0f3529e8410abe0bd9acc` |
| `portrait_B.png` | 1448 × 1086 RGBA | 2907041 | `77a81a05009dd8ee7f5af6cda2d13ce7d9282ef0aebbf39db10cad9c1ff6b9ea` |
| `portrait_C.png` | 1448 × 1086 RGBA | 3505756 | `b24d39572a54d464e1692cb76967e9d98463fa9a9e8b1589693ef88795ef7ad8` |
| `portrait_D.png` | 1536 × 1024 RGBA | 3439478 | `68f1e1811a3370fa7249d70fd3bd3e9d21481511c1401ed994ecb6ef6cee111c` |
| `portrait_E.png` | 1448 × 1086 RGBA | 3561289 | `c9620a0a1fad7c15e82c4ab1f7ac1debf07c2ed19b75f498c40e0597bd95f975` |

Chaque planche contient plusieurs **propositions visuelles**, pas nécessairement autant de personnages uniques. Éviter de déduire leurs identités/rôles ou le nombre final d'assets. Les PNG sources ne doivent être **ni modifiés, ni recadrés, ni recompressés** à cet emplacement.

## Pipeline imposé

- Sources originales : `draft/portraits/portrait_{A,B,C,D,E}.png` (ce dossier).
- Extraction/recadrages du portrait **pilote uniquement**, en branche/PR Graphiste ; candidats d'export `32×32`, `48×48`, `64×64`, sans choix arrêté.
- `assets/portraits/` : uniquement les sprites individuels **acceptés explicitement par le PO**, déclarés dans `assets/manifest.json` avec ID stable, SHA-256 et statut matériel.
- `maquettes/` : uniquement des compositions d'écrans utilisant les IDs des assets existants ; ne pas y dupliquer de planches ni de sprites.
- Le DEV effectue ensuite une sonde `TEST-ONLY` sur **vraie CYD 320×240**. Aucun export en série avant verdict PO.

Voir [Issue #124](https://github.com/Kharn27/ESP32-OTA-Battleship/issues/124), [AGENTS.md](../../AGENTS.md) et [docs/GRAPHIC_PIPELINE.md](../../docs/GRAPHIC_PIPELINE.md).

## Vérification des sources

```sh
sha256sum draft/portraits/portrait_?.png
```

Les cinq empreintes doivent correspondre **octet pour octet** au tableau avant toute fusion de l'archivage.
