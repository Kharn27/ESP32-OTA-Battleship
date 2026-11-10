# Bibliothèque d'assets approuvés

Ce répertoire accueille **uniquement les composants graphiques individuels approuvés artistiquement par le PO**, réutilisables dans plusieurs écrans : `fonds/`, `boutons/`, `logos/`, `navires/`, `effets/` (familles indicatives, à créer à l'arrivée d'un asset réel).

Chaque fichier visuel doit apparaître dans [`manifest.json`](manifest.json) avec un ID stable, un SHA-256 exact, la source, un lien de décision PO et son statut de validation matérielle. Le GO artistique sur PC n'implique pas qu'un sprite RGB565/alpha soit intégrable ni validé sur CYD.

**Aucun asset n'est encore reclassé par cette PR** : l'index est volontairement vide. Le contenu legacy de `design/` doit faire l'objet d'un inventaire distinct avant promotion, même lorsqu'une maquette a reçu un GO PO.

Voir [contrat](../docs/GRAPHIC_PIPELINE.md) et [instructions agents](../AGENTS.md).
