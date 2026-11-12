# Module io {#module-io}

**Purpose.** Neutral-format import and export: STEP, IGES, STL, 3MF, OBJ, glTF, VRML, DXF and IDF.

| Item | Value |
|---|---|
| Folder | `src/io/` |
| Project | `Io.vcxproj` (static library) |
| Namespace | `app::io` |
| Tests | `tests/io/Io.Tests.vcxproj` |
| Content from stage | S0 (STL), S2 (STEP), S7 (rest) |
| Main requirements | M-IO-01, M-3DP-01 |

## Allowed dependencies

- Application modules: `core`, `doc`, `geom`, `part`, `asm`, `ecad/pcb`
- Third-party libraries: lib3mf, and only here.

The table in `.cursor/rules/030-architecture-layering.mdc` is authoritative and `scripts/check-layering.ps1` enforces
it on project references and `#include` paths. It changes only together with an ADR.

## Main types

`moduleInfo()` (scaffold). Planned: `IImporter`, `IExporter` registries.
