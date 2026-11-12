# Module render {#module-render}

**Purpose.** OpenGL 4.5 viewport behind `render::Device`, 2D canvas, picking and the offline path tracer.

| Item | Value |
|---|---|
| Folder | `src/render/` |
| Project | `Render.vcxproj` (static library) |
| Namespace | `app::render` |
| Tests | `tests/render/Render.Tests.vcxproj` |
| Content from stage | S0 |
| Main requirements | M-VIS-01 |

## Allowed dependencies

- Application modules: `core`, `doc`, `geom`, `platform`
- Third-party libraries: OpenGL; Intel Embree, and only here.

The table in `.cursor/rules/030-architecture-layering.mdc` is authoritative and `scripts/check-layering.ps1` enforces
it on project references and `#include` paths. It changes only together with an ADR.

## Main types

`moduleInfo()` (scaffold). Planned: `Device`, `Scene`, `Canvas2D`, `PathTracer`.
