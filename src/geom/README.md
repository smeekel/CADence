# Module geom {#module-geom}

**Purpose.** Geometry-kernel adapter over Open CASCADE Technology, tessellation and persistent topological naming (`TopoRef`).

| Item | Value |
|---|---|
| Folder | `src/geom/` |
| Project | `Geom.vcxproj` (static library) |
| Namespace | `app::geom` |
| Tests | `tests/geom/Geom.Tests.vcxproj` |
| Content from stage | S0 |
| Main requirements | A-05, A-06 |

## Allowed dependencies

- Application modules: `core`, `doc`
- Third-party libraries: OCCT, and only here (A-05).

The table in `.cursor/rules/030-architecture-layering.mdc` is authoritative and `scripts/check-layering.ps1` enforces
it on project references and `#include` paths. It changes only together with an ADR.

## Main types

`moduleInfo()` (scaffold). Planned: `Kernel`, `Shape`, `TopoRef`, `TopoNaming`.
