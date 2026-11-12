# Module core {#module-core}

**Purpose.** Foundation services: math, units, the unit-aware expression engine, ObjectId, events, the command bus, undo, jobs and logging.

| Item | Value |
|---|---|
| Folder | `src/core/` |
| Project | `Core.vcxproj` (static library) |
| Namespace | `app::core` |
| Tests | `tests/core/Core.Tests.vcxproj` |
| Content from stage | S0 |
| Main requirements | A-03, A-04, A-08, A-10 |

## Allowed dependencies

- Application modules: Nothing in the application (A-01).
- Third-party libraries: Standard library; Eigen (rule 030).

The table in `.cursor/rules/030-architecture-layering.mdc` is authoritative and `scripts/check-layering.ps1` enforces
it on project references and `#include` paths. It changes only together with an ADR.

## Main types

`ModuleInfo` and the product version constants (scaffold). Planned: `Result`/`Error`, `ObjectId`, `Command`, `CommandBus`, `UndoStack`, `JobSystem`, `EventBus`, `ExpressionEngine`.
