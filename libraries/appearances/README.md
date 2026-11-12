# Global seed library: appearances

Physically based appearances, including PCB finishes (solder mask, silkscreen, copper finishes).

| Item | Value |
|---|---|
| Library `kind` | `appearance` |
| First content | S2/S10 |
| Requirements | M-VIS-01 |
| Data sources | Original parameters; textures original or permissively licensed. |

This folder is part of the **global seed** shipped with APP (F-10). On first run it is copied to
`%ProgramData%\APP\libraries\appearances\`. Items resolve project first, then user, then global, so a project or user item with the
same library ID shadows the item here. Files use the open library format (F-07, `docs/format/README.md`) and start
with the `$magic` UUID (F-09). Every seed item is listed in `libraries/seed-manifest.json` (Stage 0).

Empty in the scaffold.
