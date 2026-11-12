# Module ecad/sch {#module-ecad-sch}

**Purpose.** Schematic model, netlist compiler, electrical rule check and annotation.

| Item | Value |
|---|---|
| Folder | `src/ecad/sch/` |
| Project | `EcadSch.vcxproj` (static library) |
| Namespace | `app::ecad::sch` |
| Tests | `tests/ecad/sch/EcadSch.Tests.vcxproj` |
| Content from stage | S4 |
| Main requirements | E-SCH-* |

## Allowed dependencies

- Application modules: `core`, `doc`, `ecad/lib`
- Third-party libraries: None.

The table in `.cursor/rules/030-architecture-layering.mdc` is authoritative and `scripts/check-layering.ps1` enforces
it on project references and `#include` paths. It changes only together with an ADR.

## Main types

`moduleInfo()` (scaffold). Planned: `NetlistCompiler`, `ErcChecker`.
