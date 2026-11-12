# Global seed library: reuse-blocks

Reusable schematic plus layout blocks with revisions.

| Item | Value |
|---|---|
| Library `kind` | `reuseBlock` |
| First content | S4/S5 |
| Requirements | E-PRJ-04 |
| Data sources | Original designs. |

This folder is part of the **global seed** shipped with APP (F-10). On first run it is copied to
`%ProgramData%\APP\libraries\reuse-blocks\`. Items resolve project first, then user, then global, so a project or user item with the
same library ID shadows the item here. Files use the open library format (F-07, `docs/format/README.md`) and start
with the `$magic` UUID (F-09). Every seed item is listed in `libraries/seed-manifest.json` (Stage 0).

Empty in the scaffold.
