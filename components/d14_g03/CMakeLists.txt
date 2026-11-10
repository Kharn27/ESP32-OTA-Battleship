# D14 #69 — PO-controlled production graphic boundary (2026-11-10).
# The legacy G03 maquette is a visual oracle, NEVER a firmware asset.
# The old generator extracted 1 background + 10 H/V alpha sprites from
# that composed SVG/PNG; none is approved or declared in assets/manifest.json.
# Until Pilotage/PO approves an individual G03 catalog and the developer
# explicitly integrates its validated exports, fail closed at configure.
# Avoid copying legacy screen pixels into a production "assets" folder.
#
# See docs/D14_G03_ASSETS_BLOCKER.md and Issue #69 comment 6773142382.
message(FATAL_ERROR
  "BLOCAGE_ASSETS_G03: aucun fond G03 nu ni sprites G02/G03 H/V approuves "
  "dans assets/manifest.json. Le vieux g03_deploiement_320x240.svg/png de "
  "maquettes/deploiement/legacy/ est un ORACLE DE TEST, pas un asset "
  "firmware. Livrer les exports individuels avec IDs/SHA256/decision PO, "
  "puis remplacer cette garde par une generation Flash depuis assets/ "
  "apres revue Pilotage. Ne pas flasher un ancien BIN D14.")
