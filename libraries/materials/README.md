# Global seed library: materials

Engineering materials: density, mechanical and thermal properties, with sources.

| Item | Value |
|---|---|
| Library `kind` | `material` |
| First content | S2 |
| Requirements | M-PRT-09, CR-05 |
| Data sources | Factual values from public sources, each recorded in `docs/data-sources.md`. |

This folder is part of the **global seed** shipped with APP (F-10). On first run it is copied to
`%ProgramData%\APP\libraries\materials\`. Items resolve project first, then user, then global, so a project or user item with the
same library ID shadows the item here. Files use the open library format (F-07, `docs/format/README.md`) and start
with the `$magic` UUID (F-09). Every seed item is listed in `libraries/seed-manifest.json` (Stage 0).

Empty in the scaffold.
