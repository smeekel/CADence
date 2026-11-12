# Module sim {#module-sim}

**Purpose.** Simulation: motion (Project Chrono), circuit simulation (ngspice) and tolerance stack-up.

| Item | Value |
|---|---|
| Folder | `src/sim/` |
| Project | `Sim.vcxproj` (static library) |
| Namespace | `app::sim` |
| Tests | `tests/sim/Sim.Tests.vcxproj` |
| Content from stage | S9 |
| Main requirements | M-SIM-*, E-SIM-01 |

## Allowed dependencies

- Application modules: `core`, `doc`, `geom`, `asm`, `ecad/sch`
- Third-party libraries: Project Chrono and ngspice, and only here; Eigen (proposed, C-08).

The table in `.cursor/rules/030-architecture-layering.mdc` is authoritative and `scripts/check-layering.ps1` enforces
it on project references and `#include` paths. It changes only together with an ADR.

## Main types

`moduleInfo()` (scaffold). Planned: `MotionEngine`, `SpiceEngine`, `StackUpAnalyzer`.
