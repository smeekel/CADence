# Module part {#module-part}

**Purpose.** Part features, feature tree, regeneration, materials and mass properties; later sheet metal and weldments.

| Item | Value |
|---|---|
| Folder | `src/part/` |
| Project | `Part.vcxproj` (static library) |
| Namespace | `app::part` |
| Tests | `tests/part/Part.Tests.vcxproj` |
| Content from stage | S2 |
| Main requirements | M-PRT-* |

## Allowed dependencies

- Application modules: `core`, `doc`, `geom`, `sketch`
- Third-party libraries: None.

The table in `.cursor/rules/030-architecture-layering.mdc` is authoritative and `scripts/check-layering.ps1` enforces
it on project references and `#include` paths. It changes only together with an ADR.

## Main types

`moduleInfo()` (scaffold). Planned: `Feature`, `FeatureRegistry`, `MassProperties`.
