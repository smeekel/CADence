# Global seed library: profiles

Structural cross-sections for weldments (tubes, angles, channels, beams, flat bar).

| Item | Value |
|---|---|
| Library `kind` | `profile` |
| First content | S8 |
| Requirements | M-WLD-01 |
| Data sources | Dimensional data from public catalogues, recorded in `docs/data-sources.md`. |

This folder is part of the **global seed** shipped with APP (F-10). On first run it is copied to
`%ProgramData%\APP\libraries\profiles\`. Items resolve project first, then user, then global, so a project or user item with the
same library ID shadows the item here. Files use the open library format (F-07, `docs/format/README.md`) and start
with the `$magic` UUID (F-09). Every seed item is listed in `libraries/seed-manifest.json` (Stage 0).

Empty in the scaffold.
