# Inventaire de travail des portraits A–E — Issue #132

**Statut : `DRAFT / INVENTAIRE POUR ARBITRAGE PO` — 13 novembre 2026.** Index documentaire uniquement. **68 propositions visuelles localisables** (8 + 8 + 16 + 20 + 16) ; **68 n'est PAS un nombre de personnages distincts**. Aucun personnage ni rôle n'est assigné, aucune extraction en série ni validation artistique implicite.

## 1. Originaux et intégrité

Provenance : cinq PNG RGBA remis par le PO le 12/11/2026, archivés par [Issue #124](https://github.com/Kharn27/ESP32-OTA-Battleship/issues/124) / [PR #125](https://github.com/Kharn27/ESP32-OTA-Battleship/pull/125) dans `draft/portraits/`. Empreintes **SHA-256 publiées** dans le [README des originaux](README.md) :

| Original PO | Dimensions | Octets | SHA-256 déclaré lors de l'archivage | SHA Git blob, confirmé identique archive / main |
| --- | --- | ---: | --- | --- |
| [A](portrait_A.png) | 1448×1086 RGBA | 2623103 | `30e92fa1eed23733d812a521c6910390860620f906a0f3529e8410abe0bd9acc` | `6bef962b211eb69caf3c5d725f344c5907b9a6a9` |
| [B](portrait_B.png) | 1448×1086 RGBA | 2907041 | `77a81a05009dd8ee7f5af6cda2d13ce7d9282ef0aebbf39db10cad9c1ff6b9ea` | `49070120aae4fe5e2366e51ab084d4eb17f7d362` |
| [C](portrait_C.png) | 1448×1086 RGBA | 3505756 | `b24d39572a54d464e1692cb76967e9d98463fa9a9e8b1589693ef88795ef7ad8` | `7db3c78f0b04242ada55904be880045143f5a0e5` |
| [D](portrait_D.png) | 1536×1024 RGBA | 3439478 | `68f1e1811a3370fa7249d70fd3bd3e9d21481511c1401ed994ecb6ef6cee111c` | `c3e66de0eae3edfbff39ef50e9a5125bde9fa374` |
| [E](portrait_E.png) | 1448×1086 RGBA | 3561289 | `c9620a0a1fad7c15e82c4ab1f7ac1debf07c2ed19b75f498c40e0597bd95f975` | `19c1e1c5df185e779a3dc78f2f24c2f5336cdd42` |

**Contrôle réellement effectué :** chacun des cinq SHA Git blob et des cinq nombres d'octets de `main` est **strictement identique** à ceux du commit d'archivage `99989f94018e20834198a87a78b02f72e2e4cd14` (PR #125). L'identité du SHA Git blob prouve l'identité octet pour octet avec la version archivée. Les **SHA-256 ci-dessus sont repris du README**, qui décrivait la vérification d'archivage ; la présente passe n'a pas recalculé les SHA-256 localement, faute d'un transfert binaire brut depuis GitHub dans cet environnement. Le contrôle de SHA Git ne doit pas être faussement présenté comme une nouvelle mesure SHA-256. Aperçus des cinq originaux consultés en lecture seule dans Adobe, **sans réencodage** ; correspondance de noms et tailles Adobe/GitHub, sans preuve SHA-256 interstockage.

## 2. Convention et zones de repérage

- Identifiant **provisoire stable de position** : `<planche>-L<ligne>-C<colonne>`, balayage de haut en bas, gauche à droite ; n'exprime **ni identité ni nom de personnage**.
- La zone `[x0,y0,x1,y1)` est la **cellule de localisation** en pixels de la planche originale (origine haut-gauche, borne finale exclue). **Ce n'est ni un détourage, ni un recadrage d'export validé.** Les zones se touchent aux limites, et les ornements peuvent frôler ou déborder d'une cellule.
- Grilles de repérage : A/B `4 colonnes × 2 lignes` (x = 0/362/724/1086/1448 ; y = 0/543/1086) ; C/E `4 × 4` (mêmes x ; y = 0/272/543/815/1086) ; D `5 × 4` (x = 0/307/614/922/1229/1536 ; y = 0/256/512/768/1024).
- **Pilote déjà traité : `A-L1-C1`.** Recadrage antérieur réel : `[0,49,362,411)` de `portrait_A.png` ([dossier pilote](pilote_A_L1_C1/README.md)). Cette région de **production pilote** est distincte de la cellule de repérage plus large `[0,0,362,543)` du tableau ci-dessous ; ne pas l'extraire à nouveau automatiquement.
- Les colonnes de regroupement **Gxx = ressemblances structurées pour revue**, **Qxx ? = rapprochement faible à investiguer**, `—` = pas de regroupement proposé à ce stade ; **aucune colonne n'est une décision d'équivalence**.

## 3. Registre exhaustif — 68 positions

| ID provisoire | Cellule source `[x0,y0,x1,y1)` | Signes distinctifs visibles | Regroupement à étudier |
| --- | --- | --- | --- |
| `A-L1-C1` | `[0,0,362,543)` | Barbe blanche, casquette claire à ancre, uniforme rouge et or | PILOTE |
| `A-L1-C2` | `[362,0,724,543)` | Chevelure brune, casquette bleu nuit à ancre, boucles d’oreilles | — |
| `A-L1-C3` | `[724,0,1086,543)` | Barbe sombre, bandeau rouge, tresses et tatouage de cou | — |
| `A-L1-C4` | `[1086,0,1448,543)` | Barbe courte, peau sombre, uniforme bleu et épaulettes or | — |
| `A-L2-C1` | `[0,543,362,1086)` | Rousse à tresse, lunettes de pilote et col de fourrure | Q01 ? |
| `A-L2-C2` | `[362,543,724,1086)` | Moustache rousse, lunettes de pilote, gilet et cuir | — |
| `A-L2-C3` | `[724,543,1086,1086)` | Cheveux gris relevés, uniforme bleu, écharpe rouge | Q02 ? |
| `A-L2-C4` | `[1086,543,1448,1086)` | Jeune homme brun, uniforme bleu, épaulettes, foulard blanc | — |
| `B-L1-C1` | `[0,0,362,543)` | Cheveux argentés relevés, bijoux, épaulettes et drapé violet | Q02 ? |
| `B-L1-C2` | `[362,0,724,543)` | Rousse aux cheveux courts, lunettes de pilote et bretelles de cuir | Q01 ? |
| `B-L1-C3` | `[724,0,1086,543)` | Femme au bandeau bordeaux, tresses, grands anneaux or | — |
| `B-L1-C4` | `[1086,0,1448,543)` | Homme à barbe grise, lunettes dorées, pendentif de navigation | — |
| `B-L2-C1` | `[0,543,362,1086)` | Femme tatouée au bras/épaule, cheveux relevés, grand anneau | — |
| `B-L2-C2` | `[362,543,724,1086)` | Femme sous voile clair, casquette marine, uniforme bleu/or | — |
| `B-L2-C3` | `[724,543,1086,1086)` | Vieil homme hilare, barbe blanche, bonnet bleu et foulard rouge | Q03 ? |
| `B-L2-C4` | `[1086,543,1448,1086)` | Jeune homme à peau sombre, cheveux courts, insigne ancre sur uniforme | — |
| `C-L1-C1` | `[0,0,362,272)` | Vieux pirate barbu blanc, tricorne noir à crâne, manteau rouge | G01 |
| `C-L1-C2` | `[362,0,724,272)` | Pirate rousse, tricorne noir à galon or, regard latéral | G02 |
| `C-L1-C3` | `[724,0,1086,272)` | Pirate à bandeau rouge, barbe noire, cache-œil et sourire | Q04 ? |
| `C-L1-C4` | `[1086,0,1448,272)` | Pirate corpulent souriant, tricorne noir, moustache blanche | Q05 ? |
| `C-L2-C1` | `[0,272,362,543)` | Pirate femme, bandeau turquoise, tresses et bijoux dorés | G03 |
| `C-L2-C2` | `[362,272,724,543)` | Homme brun barbu, veste bleu sombre à grand col doré | — |
| `C-L2-C3` | `[724,272,1086,543)` | Vieux pirate blanc, lunettes, tricorne, barbe et pipe | G06 |
| `C-L2-C4` | `[1086,272,1448,543)` | Barbe rousse, grand tricorne à crâne, bouche ouverte | Q06 ? |
| `C-L3-C1` | `[0,543,362,815)` | Pirate blonde, tricorne à plume rouge, veste sombre | G04 |
| `C-L3-C2` | `[362,543,724,815)` | Homme barbu, tricorne bleu à bordure or, col haut | — |
| `C-L3-C3` | `[724,543,1086,815)` | Pirate à barbe grise, bandeau rouge, yeux fermés | — |
| `C-L3-C4` | `[1086,543,1448,815)` | Jeune rousse riante, bandeau rouge, cheveux ébouriffés | Q07 ? |
| `C-L4-C1` | `[0,815,362,1086)` | Squelette pirate luminescent turquoise, tricorne | G05 |
| `C-L4-C2` | `[362,815,724,1086)` | Pirate femme brune, cache-œil, manteau sombre et or | — |
| `C-L4-C3` | `[724,815,1086,1086)` | Homme chauve tatoué à l’ancre, barbe/moustache noires | G08 |
| `C-L4-C4` | `[1086,815,1448,1086)` | Pirate blond à lunettes de pilote, barbe claire et cuir | Q08 ? |
| `D-L1-C1` | `[0,0,307,256)` | Vieux pirate barbu blanc, tricorne à ancre et manteau rouge | G01 |
| `D-L1-C2` | `[307,0,614,256)` | Pirate à longue barbe rousse, bandeau rouge, sourire | G07 |
| `D-L1-C3` | `[614,0,922,256)` | Femme rousse au bandeau rouge et anneaux dorés | — |
| `D-L1-C4` | `[922,0,1229,256)` | Pirate brun à tricorne, longues mèches/tresses ornées | Q09 ? |
| `D-L1-C5` | `[1229,0,1536,256)` | Marin âgé, bonnet bleu, barbe grise et pipe | G09 |
| `D-L2-C1` | `[0,256,307,512)` | Pirate femme à bandeau vert, bijoux et peau sombre | G03 |
| `D-L2-C2` | `[307,256,614,512)` | Homme barbu, tatouages sombres sur visage et épaule | G10 |
| `D-L2-C3` | `[614,256,922,512)` | Squelette pirate aux reflets cyan, tricorne à tête de mort | G05 |
| `D-L2-C4` | `[922,256,1229,512)` | Pirate brun au chapeau sombre, bandeau, barbe, manteau rouge | — |
| `D-L2-C5` | `[1229,256,1536,512)` | Pirate blonde, tricorne à plume rouge et veste sombre | G04 |
| `D-L3-C1` | `[0,512,307,768)` | Vieux pirate barbu blanc, monocle/lunettes, grand tricorne | G06 |
| `D-L3-C2` | `[307,512,614,768)` | Pirate roux à bandeau rouge, rire bouche ouverte | G07 |
| `D-L3-C3` | `[614,512,922,768)` | Femme cheveux noirs relevés, manteau bleu, col rouge | — |
| `D-L3-C4` | `[922,512,1229,768)` | Pirate à barbe sombre, bandeau rouge, regard sérieux | Q04 ? |
| `D-L3-C5` | `[1229,512,1536,768)` | Marin âgé au bonnet bleu et barbe blanche, veste bleue | G09 |
| `D-L4-C1` | `[0,768,307,1024)` | Pirate roux, bandeau rouge et sourire, portrait plus bas | Q06 ? |
| `D-L4-C2` | `[307,768,614,1024)` | Femme brune au tricorne à crâne, veste rouge sombre | Q10 ? |
| `D-L4-C3` | `[614,768,922,1024)` | Vieux pirate barbu blanc, lunettes de pilote sur tricorne | G06 |
| `D-L4-C4` | `[922,768,1229,1024)` | Homme corpulent, moustache blanche, bandeau rouge et sourire | Q05 ? |
| `D-L4-C5` | `[1229,768,1536,1024)` | Pirate femme, tricorne sombre à liseré or, drapé violet | Q10 ? |
| `E-L1-C1` | `[0,0,362,272)` | Vieux pirate à barbe blanche, tricorne noir, plume rouge | G01 |
| `E-L1-C2` | `[362,0,724,272)` | Jeune femme rousse, cheveux bouclés, grands anneaux | — |
| `E-L1-C3` | `[724,0,1086,272)` | Pirate blond moustachu, lunettes dorées sur le front, grand rire | Q08 ? |
| `E-L1-C4` | `[1086,0,1448,272)` | Homme barbu sous capuche sombre, épaulette à ancre | — |
| `E-L2-C1` | `[0,272,362,543)` | Marin barbu blanc, bonnet bleu, pipe fumante, vêtement jaune | G09 |
| `E-L2-C2` | `[362,272,724,543)` | Pirate rousse au tricorne noir galonné, regard latéral | G02 |
| `E-L2-C3` | `[724,272,1086,543)` | Homme brun à moustache, uniforme bleu à broderie or | — |
| `E-L2-C4` | `[1086,272,1448,543)` | Homme rond en toque blanche, moustache et rire ouvert | — |
| `E-L3-C1` | `[0,543,362,815)` | Homme à peau sombre, tatouages au visage, collier clair | G10 |
| `E-L3-C2` | `[362,543,724,815)` | Femme blonde, lunettes de pilote et col de fourrure | Q01 ? |
| `E-L3-C3` | `[724,543,1086,815)` | Pirate à tresses sombres, tricorne et sourire, bijoux or | Q09 ? |
| `E-L3-C4` | `[1086,543,1448,815)` | Vieux marin aux cheveux blancs, lunettes dorées, veste bleue | Q08 ? |
| `E-L4-C1` | `[0,815,362,1086)` | Jeune femme souriante, couvre-chef sombre, joue marquée | Q07 ? |
| `E-L4-C2` | `[362,815,724,1086)` | Femme aux cheveux blancs, tricorne bleu, col rouge | — |
| `E-L4-C3` | `[724,815,1086,1086)` | Homme chauve tatoué d’une ancre, barbe et moustache noires | G08 |
| `E-L4-C4` | `[1086,815,1448,1086)` | Pirate femme, bandeau turquoise, perroquet coloré, bijoux | G03 |

## 4. Regroupements proposés, non fusionnés

**Hypothèses à comparer visuellement en planches natives :** les positions ci-dessous **restent toutes individuellement sélectionnables**, même lorsqu'un rapprochement est très plausible. Les motifs communs peuvent relever du même personnage avec expression ou costume différent, **ou** d'un simple archétype artistique.

| Groupe | Positions conservées | Raison observable | Décision attendue |
| --- | --- | --- | --- |
| G01 | C-L1-C1 ; D-L1-C1 ; E-L1-C1 | Pirate âgé à longue barbe blanche, tricorne, uniforme rouge ; emblèmes/coiffe différents | même personnage ou archétype ? |
| G02 | C-L1-C2 ; E-L2-C2 | Pirate rousse en tricorne sombre, attitude semblable | expression/cadrage à retenir ? |
| G03 | C-L2-C1 ; D-L2-C1 ; E-L4-C4 | Femme au bandeau turquoise, bijoux ; perroquet ajouté en E | trois variantes ou plusieurs identités ? |
| G04 | C-L3-C1 ; D-L2-C5 | Pirate blonde au tricorne et plume rouge | retenir une seule vue ? |
| G05 | C-L4-C1 ; D-L2-C3 | Squelette pirate à reflets cyan, tricorne | déclinaison du même personnage ? |
| G06 | C-L2-C3 ; D-L3-C1 ; D-L4-C3 | Vieux pirate blanc barbu et lunettes, coiffes apparentées | accessoires/cadrages, homogénéité ? |
| G07 | D-L1-C2 ; D-L3-C2 | Pirate roux à bandeau, barbe et rire marqué | variantes d'expression ? |
| G08 | C-L4-C3 ; E-L4-C3 | Homme chauve/barbu, tatouage d'ancre | vérifier variantes de costume/cadrage |
| G09 | D-L1-C5 ; D-L3-C5 ; E-L2-C1 | Marin âgé bonnet bleu, barbe grise/blanche ; pipe sur deux vues | versions du même marin ? |
| G10 | D-L2-C2 ; E-L3-C1 | Homme à tatouages faciaux sombres, épaules découvertes | même individu ou proximité de motif ? |

**Questions ouvertes, ressemblance plus faible — ne pas dédupliquer :**

| Comparaison | Positions | Ambiguïté précise |
| --- | --- | --- |
| Q01 | A-L2-C1 ; B-L1-C2 ; E-L3-C2 | Trois femmes à lunettes de pilote : rousse tressée, rousse coiffée court, blonde ; identité ou archétype ? |
| Q02 | A-L2-C3 ; B-L1-C1 | Deux femmes aux cheveux gris, uniformes et coiffures différents |
| Q03 | B-L2-C3 ↔ G09 | Marin âgé au bonnet bleu et barbe blanche, mais expression et vêtements distincts |
| Q04 | C-L1-C3 ; D-L3-C4 | Barbus à bandeau rouge : cache-œil chez C, absent/apparent autrement chez D |
| Q05 | C-L1-C4 ; D-L4-C4 | Pirates corpulents, moustaches blanches, couvre-chefs différents |
| Q06 | C-L2-C4 ; D-L4-C1 ↔ G07 | Pirates roux exubérants : tricornes/bandeaux et expressions divergents |
| Q07 | C-L3-C4 ; E-L4-C1 | Jeunes femmes rousses souriantes, coiffes différentes |
| Q08 | C-L4-C4 ; E-L1-C3 ; E-L3-C4 | Hommes âgés ou blonds avec lunettes d'aviateur : accessoires communs, visages différents |
| Q09 | D-L1-C4 ; E-L3-C3 | Pirates à tresses et bijoux, morphologies différentes |
| Q10 | D-L4-C2 ; D-L4-C5 | Deux femmes en tricorne sombre : possible thème commun sans identité certaine |

**Lecture du registre :** 68 **positions** sont confirmées ; les Gxx et Qxx sont des **pistes de travail**, **pas** un décompte des personnages canoniques. Les lignes sans G/Q restent des **propositions à part entière**. Le PO doit pouvoir choisir « conserver séparées », « traiter en variantes », « écarter une position », ou « demander un comparatif » pour chaque rapprochement.

## 5. Liste réduite de premiers exports à soumettre au PO (aucun export lancé)

Ordre proposé après le **pilote déjà existant A-L1-C1** :

1. **B-L2-C4** — uniforme et visage sans couvre-chef débordant : vérifier finesse des yeux / détails dorés en 48.
2. **C-L2-C1** — bandeau et bijoux turquoise : vérifier contours colorés et lisibilité ; référence potentielle du groupe G03.
3. **C-L4-C1** — squelette luminescent : cas difficile pour les petites surfaces et les effets de bord sur deux fonds, groupe G05.
4. **E-L1-C4** — silhouette encapuchonnée sombre : mesurer la lisibilité des valeurs dans un portrait 48.
5. **D-L3-C2** — expression très ouverte, barbe rousse : évaluer silhouette et distinction de G07.

**Choix de ce lot = proposition artistique, pas approbation.** Après accord explicite du PO sur ces identifiants, préparer séparément les exportations pertinentes selon l'Issue d'extraction et leur contrat technique. Le PO peut en remplacer ou supprimer certains. Priorité à l'examen des groupes G01/G03/G05/G09 et des Q01/Q08 avant de lancer des variantes en série.

## 6. Arbitrages PO demandés

1. Le PO reconnaît-il **G01 à G10** comme des variantes du même personnage, ou faut-il garder certaines positions totalement indépendantes ? Quelles **positions précises** conserver pour les groupes confirmés ?
2. Les comparaisons **Q01 à Q10** correspondent-elles à des identités communes, à une même famille de design ou à des personnages distincts ? En cas de doute : **ne rien fusionner**.
3. Le premier lot proposé (5 IDs, **sans A-L1-C1**) convient-il dans cet ordre ou souhaite-t-il des remplacements ? Quel visage/quelle expression utiliser dans les groupes multi-vues ?
4. Pour les détails visuellement délicats (perroquet E-L4-C4, éclats cyan du squelette, cheveux, accessoires), faut-il demander une revue de cadrage **avant** toute mise au format ?
5. Confirmation que **les identifiants ne désignent aucun rôle** et qu'aucun portrait n'est nommé « Capitaine » à ce stade.

## 7. Statuts, dépendances et preuves

- **Sources PO :** inchangées sur `main`, empreintes Git blob identiques à l'archive ; fichiers source **jamais écrasés/recompressés** dans cette mission. Pas de nouveaux PNG.
- **Choix PO #128 du 13/11/2026 :** **48×48 en jeu ; 64×64 sur futur écran de sélection ; 32×32 non retenu pour ces deux usages** ([décision PO](https://github.com/Kharn27/ESP32-OTA-Battleship/issues/128#issuecomment-6856598977)). Cette décision porte sur le contexte de taille, **pas** sur le choix d'identités, le catalogue ou l'UX des écrans. Inscription dans la Bible confiée séparément à [#131](https://github.com/Kharn27/ESP32-OTA-Battleship/issues/131).
- **Matériel :** le seul pilote `A-L1-C1` a fait l'objet d'une sonde physique TEST-ONLY #128/#129 ; les 67 autres positions ne sont **pas** testées sur CYD. **`ON_DEVICE_UNVERIFIED`** pour toute nouvelle proposition.
- **Périmètre :** aucun ajout/modification des images originales, aucune extraction 48/64, aucun ×4, aucune promotion `assets/`, aucune édition de `assets/manifest.json`, aucune maquette, aucun changement firmware ou Bible.
- **Contrôle de pipeline :** `python3 scripts/check_graphic_pipeline.py` à exécuter par CI/contrôleur du dépôt ; non exécuté localement dans cette passe, le dépôt et les originaux binaires n'étant pas matérialisés dans le conteneur. Ne pas confondre les éventuels checks CI avec validation graphique.
- **Suivi :** [Issue #132](https://github.com/Kharn27/ESP32-OTA-Battleship/issues/132). **Revue Pilotage + arbitrage PO avant tout export massif ou fusion.** `Refs #132`.
