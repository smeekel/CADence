# Global seed library: templates

Sheet and drawing templates with title blocks (ISO A4 to A0, ANSI A to E).

| Item | Value |
|---|---|
| Library `kind` | `template` |
| First content | S4/S7 |
| Requirements | M-DRW-01, E-SCH-01 |
| Data sources | Original designs. |

This folder is part of the **global seed** shipped with APP (F-10). On first run it is copied to
`%ProgramData%\APP\libraries\templates\`. Items resolve project first, then user, then global, so a project or user item with the
same library ID shadows the item here. Files use the open library format (F-07, `docs/format/README.md`) and start
with the `$magic` UUID (F-09). Every seed item is listed in `libraries/seed-manifest.json` (Stage 0).

Empty in the scaffold.
