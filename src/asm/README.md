# Module asm {#module-asm}

**Purpose.** Assemblies, mates and the mate solver, interference detection, exploded views and the animation timeline. The namespace is `app::assembly` because `asm` is a C++ keyword.

| Item | Value |
|---|---|
| Folder | `src/asm/` |
| Project | `Asm.vcxproj` (static library) |
| Namespace | `app::assembly` |
| Tests | `tests/asm/Asm.Tests.vcxproj` |
| Content from stage | S3 |
| Main requirements | M-ASM-* |

## Allowed dependencies

- Application modules: `core`, `doc`, `geom`, `sketch`, `part`
- Third-party libraries: Eigen (proposed, see planning conflict C-08).

The table in `.cursor/rules/030-architecture-layering.mdc` is authoritative and `scripts/check-layering.ps1` enforces
it on project references and `#include` paths. It changes only together with an ADR.

`asm` is a C++ keyword, so the folder and project keep the spec name while the namespace is `app::assembly` (planning conflict C-13).

## Main types

`moduleInfo()` (scaffold). Planned: `Assembly`, `MateSolver`, `InterferenceChecker`.
