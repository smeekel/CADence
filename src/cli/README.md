# Module cli {#module-cli}

**Purpose.** Headless command-line runner: version, command scripts, journal replay and batch export (A-02).

| Item | Value |
|---|---|
| Folder | `src/cli/` |
| Project | `Cli.vcxproj` (executable) |
| Namespace | `app::cli` |
| Tests | `tests/cli/Cli.Tests.vcxproj` |
| Content from stage | S0 |
| Main requirements | A-02 |

## Allowed dependencies

- Application modules: `core`, `doc`, `geom`, `sketch`, `part`, `asm`, `drw`, `ecad/lib`, `ecad/sch`, `ecad/pcb`, `ecad/out`, `link`, `sim`, `io`
- Third-party libraries: None.

The table in `.cursor/rules/030-architecture-layering.mdc` is authoritative and `scripts/check-layering.ps1` enforces
it on project references and `#include` paths. It changes only together with an ADR.

The rule-030 entry for `cli` is "every domain module; never `ui`". Domain modules are `doc` ... `io` (spec A-01); `core` is included because every module may use it. `platform` and `render` are not referenced until an ADR adds them (planning conflict C-07).

## Main types

`moduleInfo()`, `run()` (scaffold: prints the version).
