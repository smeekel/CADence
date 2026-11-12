# Global seed library: footprint-params

Goal and tolerance tables for the footprint generator.

| Item | Value |
|---|---|
| Library `kind` | `footprintParams` |
| First content | S4 |
| Requirements | E-LIB-03, CR-05 |
| Data sources | Documented, non-paywalled sources; users may enter their own licensed values. |

This folder is part of the **global seed** shipped with APP (F-10). On first run it is copied to
`%ProgramData%\APP\libraries\footprint-params\`. Items resolve project first, then user, then global, so a project or user item with the
same library ID shadows the item here. Files use the open library format (F-07, `docs/format/README.md`) and start
with the `$magic` UUID (F-09). Every seed item is listed in `libraries/seed-manifest.json` (Stage 0).

Empty in the scaffold.
