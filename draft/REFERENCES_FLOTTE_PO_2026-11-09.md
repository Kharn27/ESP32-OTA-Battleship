# Références de flotte — remise PO du 09/11/2026

**Statut : DRAFT_REFERENCES — inspiration uniquement, non validée en sprite embarqué.** Ces six visuels ont été fournis par le PO pour orienter le Graphiste. Le dossier `draft/` conserve les références, les livrables officiels restent à produire sous `design/flotte/navires/<navire>/` conformément aux US.

**Intégrité :** les originaux JPEG restent attachés à la conversation PO. Les copies publiées ici ont été transférées via un service de prévisualisation d'images et **réencodées en JPEG qualité 100**, sans redimensionnement demandé ; elles **ne sont pas identiques octet par octet** aux fichiers remis. Ne pas employer leurs SHA Git comme preuve d'identité avec l'original. Les SHA-256 indiqués ci-dessous sont ceux des **originaux remis**.

| Copie dans `draft/` | Fichier PO remis | Interprétation de travail |
| --- | --- | --- |
| [`navire_surface_PO_A.jpeg`](./navire_surface_PO_A.jpeg) | `cont-torpille_A(1).jpeg` | INTACT — affectation Croiseur à confirmer |
| [`navire_surface_PO_B.jpeg`](./navire_surface_PO_B.jpeg) | `cont-torpille_B(1).jpeg` | ENDOMMAGÉ — affectation Croiseur à confirmer |
| [`porte_avions_PO_A.jpeg`](./porte_avions_PO_A.jpeg) | `porteAvion_A(1).jpeg` | INTACT — inspiration FLOT-04 |
| [`porte_avions_PO_B.jpeg`](./porte_avions_PO_B.jpeg) | `porteAvion_B(1).jpeg` | ENDOMMAGÉ — inspiration FLOT-04 |
| [`sous_marin_PO_A.jpeg`](./sous_marin_PO_A.jpeg) | `sousmarinA(1).jpeg` | INTACT — inspiration FLOT-01 |
| [`sous_marin_PO_B.jpeg`](./sous_marin_PO_B.jpeg) | `sousmarinB(1).jpeg` | ENDOMMAGÉ — inspiration FLOT-01 |

## Consignes de qualification

- **Sous-marin** : silhouette et version endommagée utilisables en référence du pilote [FLOT-01 #92](https://github.com/Kharn27/ESP32-OTA-Battleship/issues/92), sans remplacer l'obligation de vraie transparence RGBA, H/V et preuve native.
- **Porte-avions** : références pour [FLOT-04 #95](https://github.com/Kharn27/ESP32-OTA-Battleship/issues/95), production toujours bloquée par le pilote.
- **Navire de surface** : les deux images ont été nommées `cont-torpille_*` par le PO. La coque est jugée visuellement très volumineuse pour un Contre-torpilleur de 3 cases ; **piste Croiseur 4 cases à valider**, pas une décision de réaffectation définitive. Une direction spécifique pour le **Contre-torpilleur** doit être conçue ensuite, sans démarrer [FLOT-02 #93](https://github.com/Kharn27/ESP32-OTA-Battleship/issues/93) avant GO.
- Les images comportent la mer peinte et sont des scènes RGB non détourées. **Aucune** image n'est un sprite RGBA 54×18/90×18/72×18 utilisable directement sur la grille 18 px/case ; l'export runtime, la RAM/Flash et le rendu CYD demeurent `ON_DEVICE_UNVERIFIED`.

## SHA-256 des fichiers PO originaux (avant réencodage)

```text
621cedfb844e0a8dc22eef21df787f067e5fdbf2537d6ba8dc6e8830910dcf5c  cont-torpille_A(1).jpeg
ff6bbfbd51ccf70625816ab15afb0f1aee50f880bb6b5ac1ef6ecfb37bca0eef  cont-torpille_B(1).jpeg
eea1627191dd8e9e5addbfe363cf0f5a8263c86ef79e840736106df7f7856c28  porteAvion_A(1).jpeg
4f162538fe4347e5b76bdf10f7798925672b200b546c1b55b7e9947d6cc7be51  porteAvion_B(1).jpeg
11e4f18a92051edf155ea6f03282332ead0c16bd5118c666410c69253a6e20e1  sousmarinA(1).jpeg
1ea4faedad2be603c0cf03291f4dfa0fe127d9cd5b33fcb476fb2e9765d2514b  sousmarinB(1).jpeg
```
