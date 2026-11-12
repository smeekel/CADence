# Module sketch {#module-sketch}

**Purpose.** 2D and 3D sketch model and the in-house geometric constraint solver.

| Item | Value |
|---|---|
| Folder | `src/sketch/` |
| Project | `Sketch.vcxproj` (static library) |
| Namespace | `app::sketch` |
| Tests | `tests/sketch/Sketch.Tests.vcxproj` |
| Content from stage | S1 |
| Main requirements | M-SKT-01 ... M-SKT-04 |

## Allowed dependencies

- Application modules: `core`, `doc`, `geom`
- Third-party libraries: Eigen (proposed, see planning conflict C-08).

The table in `.cursor/rules/030-architecture-layering.mdc` is authoritative and `scripts/check-layering.ps1` enforces
it on project references and `#include` paths. It changes only together with an ADR.

## Main types

`moduleInfo()` (scaffold). Planned: `Sketch`, `SketchSolver`.
