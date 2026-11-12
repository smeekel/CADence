# Module ecad/out {#module-ecad-out}

**Purpose.** Manufacturing outputs: Gerber X2, Excellon, IPC-2581, pick-and-place, IPC-D-356 and BOM.

| Item | Value |
|---|---|
| Folder | `src/ecad/out/` |
| Project | `EcadOut.vcxproj` (static library) |
| Namespace | `app::ecad::out` |
| Tests | `tests/ecad/out/EcadOut.Tests.vcxproj` |
| Content from stage | S7 |
| Main requirements | E-OUT-01, E-BOM-01, E-DOC-01 |

## Allowed dependencies

- Application modules: `core`, `doc`, `ecad/lib`, `ecad/pcb`, `drw`
- Third-party libraries: None.

The table in `.cursor/rules/030-architecture-layering.mdc` is authoritative and `scripts/check-layering.ps1` enforces
it on project references and `#include` paths. It changes only together with an ADR.

## Main types

`moduleInfo()` (scaffold). Planned: `OutputJob`, `IFabWriter`.
