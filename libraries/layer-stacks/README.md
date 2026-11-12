# Global seed library: layer-stacks

Generic 2-, 4-, 6- and 8-layer board stack-ups (proposed kind, planning conflict C-19).

| Item | Value |
|---|---|
| Library `kind` | `layerStack` |
| First content | S5 |
| Requirements | E-PCB-01 |
| Data sources | Original generic stacks. |

This folder is part of the **global seed** shipped with APP (F-10). On first run it is copied to
`%ProgramData%\APP\libraries\layer-stacks\`. Items resolve project first, then user, then global, so a project or user item with the
same library ID shadows the item here. Files use the open library format (F-07, `docs/format/README.md`) and start
with the `$magic` UUID (F-09). Every seed item is listed in `libraries/seed-manifest.json` (Stage 0).

Empty in the scaffold.
