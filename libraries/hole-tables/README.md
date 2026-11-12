# Global seed library: hole-tables

Hole and thread size tables, metric and inch: tap drill, clearance, counterbore and countersink sizes.

| Item | Value |
|---|---|
| Library `kind` | `holeTable` |
| First content | S2 |
| Requirements | M-PRT-02 |
| Data sources | Factual dimensional data from public sources; paid standards are never reproduced (CR-05). |

This folder is part of the **global seed** shipped with APP (F-10). On first run it is copied to
`%ProgramData%\APP\libraries\hole-tables\`. Items resolve project first, then user, then global, so a project or user item with the
same library ID shadows the item here. Files use the open library format (F-07, `docs/format/README.md`) and start
with the `$magic` UUID (F-09). Every seed item is listed in `libraries/seed-manifest.json` (Stage 0).

Empty in the scaffold.
