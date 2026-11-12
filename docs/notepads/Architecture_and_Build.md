# Architecture and build

Spec: sections 3 (platform), 4 (stack, licence policy, build B-01…B-07, language L-01…L-05), 5 (A-01…A-11).

## Platform constraints

- Windows 11 x64 first; Visual Studio 2026, MSVC v145 toolset. Linux/macOS later (P-04), so platform-specific code lives only in `platform/`.
- Fully offline at runtime (P-03): no networking APIs linked in product code.
- C++26 via `/std:c++latest`, limited to features MSVC implements and GCC/Clang support (L-01…L-05). No C++ named modules yet.
- Units: mechanical mm (`double`), radians; PCB int64 nanometres (P-06, A-09). Kernel tolerance 1e-7 mm (P-07).

## Technology stack (spec D-01 … D-13)

| Concern | Choice | Module |
|---|---|---|
| UI | Qt 6 Widgets + custom theme; docking via Qt Advanced Docking System **(proposed)** | `ui` |
| B-rep kernel | Open CASCADE Technology (OCCT) | `geom` only |
| Linear algebra | Eigen | `core`, `sketch`, `asm`, `sim` |
| Sketch solver | In-house (Newton / Levenberg–Marquardt, DOF analysis) | `sketch` |
| Mate solver | In-house kinematic solver | `asm` |
| Dynamics | Project Chrono | `sim` only |
| SPICE | ngspice shared library | `sim` only |
| 2D polygons | Clipper2 | `ecad/pcb` |
| JSON/BSON | nlohmann/json (`ordered_json`) | `doc` |
| 3MF | lib3mf | `io` |
| Path tracing | Intel Embree + in-house tracer (+ Open Image Denoise, optional) | `render` |
| Viewport | OpenGL 4.5 behind `render::Device` | `render` |
| Logging | spdlog **(proposed)** | `core` |
| Tests | GoogleTest + GoogleMock | `tests/*` |
| Docs | Doxygen (HTML + LaTeX/PDF) | repo root `Doxyfile` |

All via vcpkg manifest mode. Licences must satisfy spec 4.3 (product is GPL-3.0-or-later).

## Module layout and dependencies

```
src/
  core/      math, units, expression engine, ObjectId, events, command bus, undo, jobs, logging
  platform/  OS-specific code (file dialogs, paths, DPI, crash handler)
  doc/       document model, project container, JSON/BSON, schema migration, library resolution
  geom/      OCCT adapter, tessellation, persistent naming (TopoRef)
  sketch/    sketch model + constraint solver
  part/      features, feature tree, regeneration, materials, mass properties, sheet metal, weldments
  asm/       assemblies, mates, mate solver, interference, exploded views, animation timeline
  drw/       drawings: sheets, views, annotations, tables, PDF/DXF/SVG output
  ecad/lib/  symbols, footprints, components, footprint generator
  ecad/sch/  schematic, netlist compiler, ERC, annotation
  ecad/pcb/  board database (int nm), stack, placement, routing, pours, rules, DRC, 3D board
  ecad/out/  Gerber X2, Excellon, IPC-2581, pick-and-place, IPC-D-356, BOM, PCB drawing views
  link/      MCAD<->ECAD links (section 12 of the spec)
  sim/       motion (Chrono), SPICE (ngspice), tolerance stack-up
  io/        STEP, IGES, STL, 3MF, OBJ, glTF, VRML, DXF import/export
  render/    GPU viewport, 2D canvas, picking, path tracer
  cli/       headless runner (scripts, batch export, test replay)
  ui/        Qt application
```

Allowed dependencies are listed in `.cursor/rules/030-architecture-layering.mdc` and enforced by `scripts/check-layering.ps1`.

**Cross-module data without forbidden dependencies:** where a lower module must show data from a higher one (for example a drawing BOM table that lists PCB components, X-07, or PCB view types in drawings, E-DOC-01), the lower module defines a provider interface and the higher module registers an implementation at start-up (A-11 factory pattern). Example: `drw::IBomProvider`, implemented in `link/`.

## Solution layout (B-01 … B-07)

```
APP.slnx
Directory.Build.props        -> imports build/props/Common.props, Cpp.props, Vcpkg.props
build/props/
  Common.props   OutDir=out\$(Configuration)\$(ProjectName)\, IntDir=out\obj\..., x64, Unicode, /MP
  Cpp.props      /std:c++latest /permissive- /Zc:__cplusplus /Zc:preprocessor /utf-8 /W4 /WX
                 /external:anglebrackets /external:W0; Debug adds /sdl; defines APP_VERSION_*
  Vcpkg.props    VcpkgEnableManifest=true, VcpkgTriplet=x64-windows
  Qt.props       moc/uic/rcc custom build rules (ui, render if needed)
  Headless.props APP_HEADLESS=1; excludes UI projects from the build
  Tests.props    GoogleTest; output out\$(Configuration)\tests\
src/<module>/<Module>.vcxproj    (+ .filters)
tests/<module>/<Module>.Tests.vcxproj
```

Configurations: `Debug`, `Release`, `Headless` (Release-optimised, no UI projects). Platform x64.

## Runtime architecture

- **Application services** (in `core`): `CommandBus`, `UndoStack`, `JobSystem`, `EventBus`, `Log`, `Settings`, `ServiceLocator` (the only sanctioned global). Constructed by `ui` or `cli` at start-up, then injected.
- **Document model** (in `doc`): a `Project` owns `Document`s (part, assembly, drawing, schematic, board, library). Each document holds objects keyed by `ObjectId`.
- **Regeneration** (A-07): one `RegenGraph` per project spans all documents and cross-domain links. Nodes: sketch, feature, parameter, mate set, drawing view, netlist, pour, DRC, link. States: OK, Warning, Error, Suppressed, OutOfDate. Dirty propagation on change; recompute on demand or idle; independent branches in parallel on the `JobSystem`.
- **Threading** (A-10): the document is mutated only on the main thread through commands. Long computations run on worker threads against immutable snapshots, and results are applied through internal commands.
- **Rendering:** the `render` module consumes tessellated meshes and 2D primitive lists produced by domain modules; it never reaches into domain objects.

## Performance targets (P-10 … P-17)

Cold start < 3 s; 5 M triangles ≥ 60 fps; 200-feature rebuild < 2 s; sketch drag < 16 ms at 500 entities/800 constraints; 2,000-part lightweight open < 20 s; routing < 50 ms per move at 2,000 parts/5,000 nets; DRC < 15 s on an 8-layer board with 2,000 parts; 50 MB save/load < 5 s. Each has a benchmark (T-08).
