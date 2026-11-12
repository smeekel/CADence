# Module link {#module-link}

**Purpose.** Parametric links between the mechanical and electronics models (spec section 12).

| Item | Value |
|---|---|
| Folder | `src/link/` |
| Project | `Link.vcxproj` (static library) |
| Namespace | `app::link` |
| Tests | `tests/link/Link.Tests.vcxproj` |
| Content from stage | S6 |
| Main requirements | X-01 ... X-10 |

## Allowed dependencies

- Application modules: `core`, `doc`, `geom`, `sketch`, `part`, `asm`, `ecad/pcb`
- Third-party libraries: None.

The table in `.cursor/rules/030-architecture-layering.mdc` is authoritative and `scripts/check-layering.ps1` enforces
it on project references and `#include` paths. It changes only together with an ADR.

## Main types

`moduleInfo()` (scaffold). Planned: `LinkManager`.
