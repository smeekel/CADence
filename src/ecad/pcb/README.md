# Module ecad/pcb {#module-ecad-pcb}

**Purpose.** Board database in integer nanometres, layer stack, placement, routing, copper pours, design rules and DRC.

| Item | Value |
|---|---|
| Folder | `src/ecad/pcb/` |
| Project | `EcadPcb.vcxproj` (static library) |
| Namespace | `app::ecad::pcb` |
| Tests | `tests/ecad/pcb/EcadPcb.Tests.vcxproj` |
| Content from stage | S5 |
| Main requirements | E-PCB-*, E-RUL-*, A-09 |

## Allowed dependencies

- Application modules: `core`, `doc`, `ecad/lib`, `ecad/sch`
- Third-party libraries: Clipper2, and only here.

The table in `.cursor/rules/030-architecture-layering.mdc` is authoritative and `scripts/check-layering.ps1` enforces
it on project references and `#include` paths. It changes only together with an ADR.

## Main types

`moduleInfo()` (scaffold). Planned: `BoardDb`, `Router`, `PourEngine`, `RuleResolver`, `DrcEngine`.
