# Global seed library: components

Electronic components: a symbol, footprint and 3D model plus parameters, lifecycle and simulation model.

| Item | Value |
|---|---|
| Library `kind` | `component` |
| First content | S4 |
| Requirements | E-LIB-01, E-LIB-02 |
| Data sources | Generic parts only (R, C, L, diode, LED, BJT, MOSFET, op-amp, regulator, headers, USB-C receptacle, test point, mounting hole, fiducial). No vendor logos or copied datasheet drawings. |

This folder is part of the **global seed** shipped with APP (F-10). On first run it is copied to
`%ProgramData%\APP\libraries\components\`. Items resolve project first, then user, then global, so a project or user item with the
same library ID shadows the item here. Files use the open library format (F-07, `docs/format/README.md`) and start
with the `$magic` UUID (F-09). Every seed item is listed in `libraries/seed-manifest.json` (Stage 0).

Empty in the scaffold.
