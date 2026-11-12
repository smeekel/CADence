# Module drw {#module-drw}

**Purpose.** Drawings: sheets, views, annotations, tables and PDF/DXF/SVG output.

| Item | Value |
|---|---|
| Folder | `src/drw/` |
| Project | `Drw.vcxproj` (static library) |
| Namespace | `app::drw` |
| Tests | `tests/drw/Drw.Tests.vcxproj` |
| Content from stage | S7 |
| Main requirements | M-DRW-* |

## Allowed dependencies

- Application modules: `core`, `doc`, `geom`, `part`, `asm`
- Third-party libraries: None.

The table in `.cursor/rules/030-architecture-layering.mdc` is authoritative and `scripts/check-layering.ps1` enforces
it on project references and `#include` paths. It changes only together with an ADR.

## Main types

`moduleInfo()` (scaffold). Planned: `ViewGenerator`, `IBomProvider`, `IViewTypeProvider`.
