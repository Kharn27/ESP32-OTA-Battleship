# G07-R2 — FIN DE PARTIE VS Capitaine · livraison partielle (Issue #53)

> **PROPOSITION DA NON VALIDÉE PAR LE PO — PR DRAFT.**
> `DESIGN_PREVIEW — ON_DEVICE_UNVERIFIED`.
> Une **seule scène DÉFAITE** est fournie. Ce dossier ne prouve ni VICTOIRE ni MATCH NUL.

## Maquette native — 320 × 240

![G07-R2 DÉFAITE — preview 320 × 240](g07_r2_defaite_320x240.png)

<details>
<summary>Zoom obligatoire ×4 — 1280 × 960 strict NEAREST</summary>

![G07-R2 DÉFAITE — zoom ×4 NEAREST](g07_r2_defaite_x4.png)

</details>

[Master raster 1448 × 1086](g07_r2_source_1448x1086.png) ·
[Générateur reproductible](build_g07_r2.py) ·
[Empreintes SHA-256 et contrôles](g07_r2_provenance.json).

**Reconstruction :** `python3 -m pip install Pillow`, puis
`python3 design/ecrans/fin-partie/build_g07_r2.py`. Le natif est issu de la
source haute résolution par rééchantillonnage `LANCZOS`. Le zoom ×4 est
la reproduction exacte des pixels du natif, `NEAREST`, testée par le script.

## Éditabilité et provenance

Le Graphiste a remis **une illustration PNG aplatie 1448 × 1086 pixels**
(transmise dans la conversation le 04/11/2026). Le master est conservé
sans modifications. **Les éléments internes ne sont PAS éditables par calques** :
bois, laiton, mer, impacts, lettrage et boutons sont peints dans le même raster.
`build_g07_r2.py` est éditable pour la chaîne de conversion, **pas pour la scène**.
Pas de revendication d'une source SVG/ORA/PSD pleinement éditable ou d'assets
firmware. Aucune ressource stock ou police ajoutée par l'étape d'export ;
la provenance des éventuelles références incorporées par le Graphiste
reste à documenter.

## Contrôle des critères de l'Issue #53

| Exigence métier décidée par le PO | Preuve / réserve réelle |
| --- | --- |
| Une grille 10×10 | Apparente sur le raster ; **inspection VISUELLE seulement** |
| Origine (21,53), cases 18×18, surface 180×180 | **NON CONFORME** : grille peinte plus petite que la géométrie mandatée |
| Deux tirs de dernière manche révélés simultanément | **NON DÉMONTRÉ** par ce seul état statique |
| Verdict initial puis compact, informatif et non tactile | **NON LIVRÉ** : le large pavé DÉFAITE peut ressembler à un bouton |
| Trois verdicts VICTOIRE / DÉFAITE / MATCH NUL | **DÉFAITE UNIQUEMENT** ; autres variantes absentes |
| Pas de FEU ni nouvelle sélection, impacts conservés | FEU absent du visuel ; interactions non testées |
| ATTAQUE ↔ DÉFENSE, REJOUER et ACCUEIL | Libellés affichés ; actions/hitboxes non validées |
| VOIR SA FLOTTE uniquement après DÉFAITE, vers G08 A/B/C | Commande affichée dans DÉFAITE ; **aucune transition G08 prouvée** |
| Coques adverses secrètes avant choix | Pas de coque adverse révélée visuellement |
| CYD TFT, stylet, Flash/RAM | **ON_DEVICE_UNVERIFIED** |

### Lecture à la résolution native

La composition maritime est riche (coucher de soleil, lanterne, gréement,
bois et laiton) mais une partie du lettrage est petite à 320 × 240 et
le cadre du verdict a un relief analogue aux touches tactiles. Avant GO PO,
il faut distinguer plus clairement **verdict informatif** et **commande**,
mettre le plateau au contrat géométrique, livrer les trois issues et
la séquence annonce→compact. La jonction avec G08 exige un test spécifique.

**DÉCIDÉ PO :** règles métier ci-dessus et accès volontaire G08 après défaite.
**PROPOSÉ DA :** composition actuelle du raster DÉFAITE.
**NON TESTÉ CYD :** affichage physique, stylet, performances et mémoire.

## Statut de la G07 R1

Les fichiers historiques `g07_verdict_*`, `g07_variantes_*` et
`build_g07.py` encore présents dans cette PR Draft proviennent de la
**R1 REFUSÉE ARTISTIQUEMENT** malgré la fusion historique PR #54.
**Ne pas les utiliser comme références visuelles actives**. Leur retrait
est requis avant réception de R2 complète ; pas de dossier d'archives concurrent.
Le suivi de l'ancienne version reste dans l'historique Git.

**Aucun GO artistique PO, aucune vérification matérielle, aucune fusion.**
Pilotage garde l'exclusivité de la réception et de la fusion.
