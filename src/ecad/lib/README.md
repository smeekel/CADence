# Module ecad/lib {#module-ecad-lib}

**Purpose.** Electronics library model: symbols, footprints, components and the footprint generator.

| Item | Value |
|---|---|
| Folder | `src/ecad/lib/` |
| Project | `EcadLib.vcxproj` (static library) |
| Namespace | `app::ecad::lib` |
| Tests | `tests/ecad/lib/EcadLib.Tests.vcxproj` |
| Content from stage | S4 |
| Main requirements | E-LIB-* |

## Allowed dependencies

- Application modules: `core`, `doc`
- Third-party libraries: None.

The table in `.cursor/rules/030-architecture-layering.mdc` is authoritative and `scripts/check-layering.ps1` enforces
it on project references and `#include` paths. It changes only together with an ADR.

## Main types

`moduleInfo()` (scaffold). Planned: `Symbol`, `Footprint`, `Component`, `FootprintGenerator`.
