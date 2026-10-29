# Journal d'import des originaux — PR #2 (blocage levé)

**ARCHIVEE / EXPLORATION_D_AMBIANCE / NON_VALIDEE_PO / NON_RUNTIME**

## Historique du problème

Au dépôt initial, la branche ne contenait que **quatre JPEG avec pertes** produits par un service d'aperçu, faute de transfert direct fiable des octets des PNG locaux vers le connecteur GitHub. Les JPEG n'étaient pas substituables aux images originales. La revue Pilotage a demandé des PNG exacts, des aperçus natifs et leurs empreintes.

## Résolution constatée

Le dépôt a reçu manuellement **sept PNG exacts** via la branche existante `art/etude-bataille-en-cours-20261029`, dans le commit d'import :

`686e3e2b27f95860a22272e210b04c729fdf0976` (`Ajout manuelle`).

Les deux originaux 1448 × 1086, les deux exports 320 × 240, les deux zooms 1280 × 960 et le comparatif 640 × 240 figurent dans l'arbre GitHub. Pour les sept : **taille et Git blob SHA-1 identiques** aux calculs effectués à partir des fichiers sources locaux ; les empreintes SHA-256 sont consignées dans [MANIFEST_SHA256.md](MANIFEST_SHA256.md). Aucun fichier n'a été remplacé par un JPEG renommé.

**Le blocage d'archivage binaire est levé.** Le README et la description de PR doivent désormais présenter les PNG comme références de revue et réserver les JPEG à l'historique.

## Gouvernance

L'ajout des images n'approuve aucune direction artistique, n'autorise aucune révélation de positions de la flotte adverse en phase de combat et ne constitue ni une preuve d'intégration firmware ni un test CYD.

**La PR reste DRAFT, non fusionnée, et relève de la contre-revue Pilotage.**
