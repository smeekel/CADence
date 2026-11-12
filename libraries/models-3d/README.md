# Global seed library: models-3d

Native parametric and STEP 3D models of components.

| Item | Value |
|---|---|
| Library `kind` | `model3d` |
| First content | S4 |
| Requirements | E-LIB-01, E-3D-01 |
| Data sources | Original or permissively licensed models; licence recorded per file. |

This folder is part of the **global seed** shipped with APP (F-10). On first run it is copied to
`%ProgramData%\APP\libraries\models-3d\`. Items resolve project first, then user, then global, so a project or user item with the
same library ID shadows the item here. Files use the open library format (F-07, `docs/format/README.md`) and start
with the `$magic` UUID (F-09). Every seed item is listed in `libraries/seed-manifest.json` (Stage 0).

Empty in the scaffold.
