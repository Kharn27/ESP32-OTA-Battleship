# DRAFT — Patrouilleur, dégâts et animation (I-009)

Les **sept références originales remises par le PO** ont vocation à remplacer les anciens fichiers de préparation de l'océan dans ce dossier de travail. Les fonds océaniques approuvés restent dans [`design/ocean/`](../design/ocean/), intacts.

| Asset | État | Recommandation provisoire |
|---|---|---|
| `patrouilleur_A.jpeg` | Intact | Retenir |
| `patrouilleur_T1A.jpeg` | Dégâts arrière A | Alternative |
| `patrouilleur_T1B.jpeg` | Dégâts arrière B | Préféré |
| `patrouilleur_T2A.jpeg` | Dégâts avant A | Préféré |
| `patrouilleur_T2B.jpeg` | Dégâts avant B | Alternative |
| `patrouilleur_B.jpeg` | Dégâts complets | Retenir |
| `patrouilleur_detruit.gif` | Destruction animée (9 frames, environ 1,17 s) | Référence animation, intégration non décidée |

**Décision artistique finale à confirmer par PO** ; ne pas jeter les alternatives. Ces images contiennent déjà la mer peinte ; ce ne sont **pas** des sprites alpha indépendants. N'imposer aucun format runtime ni animation GIF sur ESP32 sans qualification Flash/RAM/TFT.

**Provenance :** fichiers fournis en conversation par le PO le 06/11/2026. Aucun redessin, redimensionnement, recolorisation ou substitution. Contrôle par empreintes SHA-256 attendues :

```text
631ff1b8f4245599137b6d0d3bd6919b9db3e30844d478434ee864ff885f5df5  draft/patrouilleur_A.jpeg
de84b5bf265cd290c24dfd84452511bf7892adccd399d9e92ba4c106f8f55450  draft/patrouilleur_B.jpeg
1e25359998afdedbc93ebf6baebdbeae34b327f4ab75a2f4e9d55e4c7b8a9584  draft/patrouilleur_T1A.jpeg
2bc761b1abcbced2fc760e8ef938571d58b3b4b14e390bad9f219846b32c8666  draft/patrouilleur_T1B.jpeg
f8366482082febc917a812d94efd98a75d03c100a789d5db73ceacc379c41d4d  draft/patrouilleur_T2A.jpeg
bcb36de1ef21c7992af3196d460ce57f68d654d4826302be0fdfa20b5515a9c3  draft/patrouilleur_T2B.jpeg
bdf1f617e9b9d5f8fb83cea8370c7bf231f57e536206a39321e4a94bafbf4050  draft/patrouilleur_detruit.gif
```

**Statut :** `DRAFT_REFERENCES — ON_DEVICE_UNVERIFIED`. Ce lot ne constitue pas une validation firmware, tactile ou matérielle.