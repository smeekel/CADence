# Functional Requirements: Integrated MCAD + ECAD Desktop Application

| | |
|---|---|
| Document status | Draft v0.2 |
| Date | 2026-11-11 |
| Owner | Steven |
| Audience | An AI coding agent (primary) and human reviewers |
| Working product name | `APP` (placeholder; final name must not reference any third-party trademark) |
| Product licence | GPL-3.0-or-later (see P-30) |

### Change log

| Version | Changes |
|---|---|
| v0.2 | Product is GPL: GPL code may be reused with attribution, but fresh implementation is preferred (CR-04, 4.3, CR-07). Build uses Visual Studio solutions directly, with no CMake for now (P-02, 4.4). Language raised to C++26 (P-05, 4.5). Added a fixed file-format magic UUID (F-09). Added three-scope libraries: global seed, per-user, per-project (F-10). GoogleTest selected as test framework (D-11). |
| v0.1 | First draft. |

---

## 1. Purpose and goal

Build a **clean-room, offline, desktop CAD application** that combines:

1. **Parametric 3D mechanical design** (sketching, parts, assemblies, drawings, sheet metal, weldments, motion simulation), and
2. **Electronics design** (component libraries, schematic capture, circuit simulation, PCB layout, manufacturing outputs),

in **one shared parametric model**, so that a circuit board and the product enclosure around it are designed together and stay fitted when either one changes.

**Reference scenario (the product's reason to exist):**
A user designs a PCB with a USB-C connector, four mounting holes and a tall electrolytic capacitor. In the same project they model a two-part plastic enclosure. The enclosure's inner cavity, screw bosses, connector cut-out and lid height are *driven by* the board outline, hole positions, connector location and tallest component. When the user widens the board from 50 mm to 60 mm or moves the connector 5 mm, the enclosure updates, interference checks re-run, and both the PCB fabrication outputs and the enclosure's 3D-print files regenerate correctly.

### 1.1 How to read this document

- Requirement IDs in this document use the prefixes:
  - `P-` platform / non-functional
  - `A-` architecture
  - `B-` build system, `L-` C++ language policy
  - `F-` file format
  - `U-` user interface
  - `T-` test harness
  - `M-` mechanical functional
  - `E-` electronics functional
  - `X-` cross-domain (MCAD ↔ ECAD integration)
- Each functional requirement lists its **source ID** from the companion capability survey (`SW-…` / `AD-…`), the **stage** it is built in, and **acceptance criteria** (`Accept:`). A requirement is done only when every acceptance criterion passes as an automated test (section 7) unless marked *manual*.
- "Shall" = mandatory for the stage. "Should" = desirable; may slip to a later stage with a recorded decision.
- Section 15 is a traceability table from every requested source ID to the requirements here.

---

## 2. Clean-room rules (mandatory)

The implementation must be independently created. These rules apply to every contributor, human or AI.

- **CR-01** Do not use, read, decompile, or reference any source code, binaries, SDK headers, API documentation, help text, icons, images, templates, sample files or UI layouts from SOLIDWORKS, Altium Designer, or any other proprietary CAD product.
- **CR-02** This specification describes *functions* in generic engineering terms. Where a term is common industry vocabulary (extrude, fillet, mate, netlist, via, design rule), use it freely. Do not reuse product-specific trademarked feature names in the UI (for example, use "Hole tool", not a vendor's branded hole-feature name; "Guided routing", not a vendor's branded router name).
- **CR-03** Do not import or export proprietary native file formats of other CAD products in the scoped stages. Interoperability is through open/neutral formats only (section 13).
- **CR-04** The product is GPL-3.0-or-later, so code from GPL-compatible open-source projects (for example KiCad, SolveSpace, FreeCAD, LibreCAD) **may** be reused, but **fresh implementation is preferred**. Reuse is allowed only when (a) the source licence is compatible with GPL-3.0-or-later (section 4.3), (b) the reuse is recorded under CR-07, and (c) it saves substantial effort or brings a proven, hard-to-get-right algorithm (for example a push-and-shove router or a robust polygon pour). Studying published algorithms (papers, textbooks, open standards) is always allowed.
- **CR-07 Attribution and provenance.** Every file containing reused or adapted third-party code keeps the original copyright and licence notice and an `SPDX-License-Identifier` header. Each reuse is logged in `docs/provenance.md` with source project, URL, version or commit, original path, licence, and the files it went into. `THIRD_PARTY_NOTICES.md` lists every reused project and linked library. *Accept:* a static check (T-11) fails the build if a source file lacks an SPDX header, or if a file marked as third-party has no `provenance.md` entry.
- **CR-05** Standards documents that are paid or copyrighted (for example IPC-7351, ISO fastener standards) must not be reproduced verbatim. Implement calculators and generators with user-editable parameter tables; ship default data only where it is factual dimensional data from publicly available sources, and record the source of every shipped table in `docs/data-sources.md`.
- **CR-06** No telemetry, licence servers, or "phone home" code of any kind (see P-03).

---

## 3. Platform and non-functional requirements

### 3.1 Platform

- **P-01 Desktop application.** The product shall be a native desktop application. No browser runtime is required to use any feature.
- **P-02 Initial target.** Windows 11 x64, built and debugged from **Visual Studio 2026** (MSVC v145 platform toolset) using native Visual Studio solution and project files (`APP.slnx` + one `.vcxproj` per module; section 4.4). No CMake for now. A clean clone must build after installing only Visual Studio (with the C++ desktop workload) and Git; vcpkg dependencies restore automatically on the first build.
- **P-03 Fully offline.** The application shall perform no network access at runtime: no update checks, licence checks, telemetry, online part search, cloud storage, or online help. All help, libraries and sample data ship locally. *Accept:* a test run with the network adapter disabled exercises every stage's smoke test with no errors; a static check (T-11) fails the build if networking APIs are linked outside an allow-list (empty by default).
- **P-04 Portability.** Platform-specific code shall be isolated in a `platform/` module behind interfaces. No Win32/COM/DirectX calls are permitted outside that module. The code base shall be structured so that Linux and macOS builds can be added later without touching domain modules. *Accept:* the `Headless` solution configuration (4.4) builds every domain module and the CLI with no Qt Widgets or Windows UI dependency. Linux/macOS builds start when the build system moves to a cross-platform generator (Q-05).
- **P-05 Language.** C++26 (see D-01 and the language policy in 4.5). C# is not used for the core.
- **P-06 Units.** Internal mechanical length unit: millimetre, `double`. Angles: radians. PCB geometry: signed 64-bit integer nanometres (see A-09). All user-facing values display in user-selectable units (mm, cm, m, in, mil, thou, µm) with per-document defaults.
- **P-07 Precision.** Mechanical geometric tolerance default 1e-7 mm (kernel confusion tolerance). PCB clearance computations are exact on the integer grid.

### 3.2 Performance targets (initial, measured on a reference machine: 8-core CPU, 32 GB RAM, mid-range discrete GPU)

| ID | Scenario | Target |
|---|---|---|
| P-10 | Cold start to usable empty window | < 3 s |
| P-11 | Viewport frame rate, 5 million triangles, shaded with edges | ≥ 60 fps |
| P-12 | Full rebuild of a 200-feature part | < 2 s |
| P-13 | Sketch solve, 500 entities / 800 constraints, interactive drag | < 16 ms per drag update |
| P-14 | Open a 2,000-part assembly in lightweight mode | < 20 s |
| P-15 | PCB with 2,000 components, 5,000 nets: interactive routing response | < 50 ms per mouse move |
| P-16 | Batch DRC, 8-layer board, 2,000 components | < 15 s |
| P-17 | Save or load a 50 MB project | < 5 s |

Targets are tracked by benchmarks (T-08). Missing a target is a defect, not a blocker, unless it exceeds the target by more than 3×.

### 3.3 Reliability

- **P-20 Crash safety.** Autosave every 5 minutes (configurable) to a recovery file; on restart after a crash, offer recovery.
- **P-21 No silent data loss.** A failed feature rebuild never deletes the feature or downstream features. Features enter an error state with a message and remain editable.
- **P-22 Undo/redo.** Unlimited undo/redo within a session for every model-changing action (memory-capped, default 2 GB).
- **P-23 Logging.** Structured logs to a local rotating file; a "Copy diagnostics" command packages logs + system info for the user to share manually.

### 3.4 Licensing of the product

- **P-30** The product is licensed **GPL-3.0-or-later**. The repository contains `LICENSE` (GPL-3.0 text), an `SPDX-License-Identifier: GPL-3.0-or-later` header in every original source file, and `THIRD_PARTY_NOTICES.md` regenerated from the vcpkg manifest and `docs/provenance.md`. The About box shows the licence, the source-code offer and the third-party notices (all available offline).

---

## 4. Technology stack and key decisions

### 4.1 Decisions

| ID | Decision | Rationale |
|---|---|---|
| D-01 | **C++26** for all core and UI code, within the policy in 4.5 | The best available open geometry kernel, physics engine and SPICE engine are C/C++. Wrapping them for C# would cost more than it saves. |
| D-02 | **Visual Studio solution (`.slnx`) + `.vcxproj` per module + vcpkg manifest mode via MSBuild integration** (section 4.4). Moving to a cross-platform build system later is planned. | Simplest path for Windows-first development; vcpkg works with MSBuild projects directly. |
| D-03 | **Qt 6 (Widgets)** for the UI, with a custom theme | Mature, portable, high-DPI aware, LGPL-usable. Widgets suit dense CAD tool UIs better than QML. |
| D-04 | **Open CASCADE Technology (OCCT)** as the B-rep geometry kernel, behind an adapter (A-05) | Only mature open B-rep kernel with booleans, fillets, offsets, sweeps, lofts, STEP/IGES. |
| D-05 | **Own 2D sketch constraint solver** (Newton / Levenberg–Marquardt with DOF analysis, Eigen for linear algebra) | Core differentiator; must be fast and debuggable. Fresh implementation preferred (CR-04); open solvers such as SolveSpace's or FreeCAD's PlaneGCS may serve as references or fallbacks under CR-07. |
| D-06 | **Own assembly mate solver** for kinematics; **Project Chrono** for dynamics (M-SIM) | Mates need tight integration; Chrono provides contacts, friction and integrators. |
| D-07 | **ngspice** as a shared library for circuit simulation | Mature SPICE engine with a documented shared-library API. |
| D-08 | **Clipper2** for 2D polygon operations in the PCB engine | Robust integer polygon booleans and offsets, which fit the nm integer grid. |
| D-09 | **nlohmann/json** for JSON and BSON serialisation, using `nlohmann::ordered_json` | One library writes both encodings from the same in-memory tree. The ordered variant keeps key order, which F-03 and F-09 need; the default `json` type sorts keys alphabetically. |
| D-10 | **lib3mf** for 3MF export | Reference implementation of the 3MF spec. |
| D-11 | **GoogleTest** (with GoogleMock) for tests | Visual Studio's Test Explorer runs GoogleTest projects through its included adapter, which suits a solution-based build with no CMake/CTest. |
| D-12 | Rendering: **OpenGL 4.5** in Stage 0 behind a `render::Device` interface; a Vulkan/D3D12 backend may be added later | Fastest route to a working viewport, portable, with no lock-in thanks to the interface. |
| D-13 | Photoreal rendering: **Intel Embree** + an in-house path tracer | Permissive licence, CPU-only, so it runs on any machine offline. |

### 4.2 Alternatives considered

- *C# with .NET + Avalonia:* portable UI and fast to write, but no comparable open B-rep kernel. Rejected for the core. A C# plug-in layer may be considered in the Future stage (API).
- *Dear ImGui:* excellent for tools, but weaker for accessible, themeable, dockable production UIs. It may be used for internal debug overlays only.

### 4.3 Dependency and reuse licence policy

The product is GPL-3.0-or-later, so every linked library and every reused code fragment must be compatible with GPL-3.0.

| Licence of dependency or reused code | Status |
|---|---|
| MIT, BSD-2/3, Boost (BSL-1.0), zlib, ISC | Allowed |
| Apache-2.0 | Allowed (compatible with GPL-3.0, not with GPL-2.0-only) |
| MPL-2.0 | Allowed (its secondary-licence clause permits combination with GPL) |
| LGPL-2.1-or-later, LGPL-3.0 (for example Qt, OCCT) | Allowed; static or dynamic linking is fine for a GPL product |
| GPL-2.0-or-later, GPL-3.0, GPL-3.0-or-later (for example KiCad, SolveSpace) | Allowed with attribution (CR-07); fresh implementation preferred (CR-04) |
| AGPL-3.0 | Allowed only by recorded decision; avoid by default |
| GPL-2.0-only, LGPL-2.1-only code copied into GPL-3.0 files | **Not allowed** (incompatible with GPL-3.0); linking an LGPL-2.1-only library stays allowed |
| Proprietary, "source-available", non-commercial, or no licence | **Not allowed** |

Every dependency is pinned in `vcpkg.json` with its version, and its licence is re-verified when it is added or upgraded.

### 4.4 Build system (Visual Studio solutions)

- **B-01 Solution.** One solution `APP.slnx` at the repository root (the XML solution format Visual Studio 2026 uses by default). Solution folders mirror the module layout (section 5.1).
- **B-02 One project per module.** Each module in 5.1 is one `.vcxproj` (static library by default; `ui` and `cli` are executables; tests are one GoogleTest executable per module). Projects reference each other only along the allowed dependency directions (A-01).
- **B-03 Shared settings.** All compiler and linker settings live in `build/props/*.props`, imported through a root `Directory.Build.props`, not in individual `.vcxproj` files: language standard (4.5), warning level `/W4 /WX`, `/permissive-`, `/utf-8`, `/Zc:__cplusplus`, multi-processor build, output and intermediate folders (`out/<Configuration>/<Project>/`). A `.vcxproj` contains only its file list, project references and module-specific overrides.
- **B-04 Configurations.** `Debug`, `Release` and `Headless` (Release without Qt Widgets or any UI project, for P-04 and automated test runs). Platform: x64 only. ARM64 is a *should* for later.
- **B-05 Dependencies.** vcpkg in manifest mode (`vcpkg.json` + `vcpkg-configuration.json` with a pinned baseline), enabled per project through the MSBuild integration properties in the shared props. Qt is pulled through vcpkg so no separate Qt installer is needed; Qt's `moc`/`uic`/`rcc` steps run through custom build rules defined once in `build/props/qt.props`.
- **B-06 Command-line build.** `msbuild APP.slnx /m /p:Configuration=Release /p:Platform=x64` builds everything from a Developer PowerShell; `scripts/build.ps1` and `scripts/test.ps1` wrap the build and the test run (T-13).
- **B-07 Migration-ready.** Because a cross-platform build system will replace the solution later: no build logic in source code (no `#pragma comment(lib, …)`); no reliance on Visual Studio-only project features beyond the props files; precompiled headers optional per project and never required for correctness; the module layout maps one-to-one to future build targets.

### 4.5 C++26 language policy

- **L-01 Standard switch.** Projects compile with `/std:c++latest`, the MSVC switch that enables C++26 features as they are implemented; MSVC has no separate `/std:c++26` switch yet. Switch to `/std:c++26` when MSVC provides it.
- **L-02 Supported features only.** C++26 language and library features are used only once the MSVC version in use implements them (check Microsoft's language-conformance table). Features still partial in MSVC (for example static reflection) must not be required by the code.
- **L-03 Portable subset.** Because Linux and macOS come later (P-04), a feature is used only if current GCC and Clang also support it. Permitted C++23/26 features are listed in `docs/cpp-features.md`; adding a feature to the list requires an ADR.
- **L-04 Feature-test macros.** Where a feature may be absent on another compiler, code checks the relevant `__cpp_*` / `__cpp_lib_*` macro and provides a fallback.
- **L-05 Modules.** C++ named modules (`import`) are not used until all three compilers and the future build system support them reliably; headers + precompiled headers are used instead.

---

## 5. Architecture requirements

### 5.1 Module layout

```
APP.slnx                solution (B-01)
Directory.Build.props   imports build/props/*.props (B-03)
vcpkg.json              dependency manifest (B-05)
/build/props  shared MSBuild settings
/scripts      build.ps1, test.ps1, checks
/libraries    global seed libraries shipped with the app (F-10)
/src          one .vcxproj per module below
  core/       math, units, expression engine, IDs, events, command bus, undo, logging
  doc/        document model, project container, serialisation (JSON/BSON), schema migration
  geom/       kernel adapter over OCCT, tessellation, topological naming
  sketch/     2D sketch model + constraint solver
  part/       features, feature tree, regeneration engine, materials, mass properties
  asm/        assemblies, mates, mate solver, interference, exploded views
  drw/        drawings: views, annotations, tables, PDF/DXF/SVG output
  ecad/
    lib/      symbols, footprints, components, library storage
    sch/      schematic model, netlist compiler, ERC
    pcb/      board database (integer nm), stackup, routing, pours, rules, DRC
    out/      Gerber X2, Excellon, IPC-2581, pick-and-place, IPC-D-356, BOM
  link/       cross-domain parametric links (MCAD <-> ECAD)
  sim/        motion (Chrono), SPICE (ngspice), tolerance stack-up
  io/         STEP, IGES, STL, 3MF, OBJ, glTF, DXF, VRML import/export
  render/     GPU viewport, 2D canvas, picking, path tracer
  ui/         Qt application, panels, tools, themes
  cli/        headless runner (batch, scripting of commands, test replay)
  platform/   OS-specific code only
/tests        unit, integration, golden, UI replay, fuzz, benchmark
/docs         format spec, ADRs, user guide, data sources
```

- **A-01 Layering.** `core` depends on nothing in the app. Domain modules (`doc` … `io`) shall not depend on `ui` or Qt Widgets. Qt Core types are not allowed in domain module public APIs (use standard library types). *Accept:* `scripts/check-layering.ps1` (run by T-11) fails on any forbidden `ProjectReference` in a `.vcxproj` or any forbidden `#include` path.
- **A-02 Headless first.** Every domain operation is callable without a UI through the command bus. The `cli` executable can open a project, run commands, regenerate, export and save.
- **A-03 Command bus.** Every user action that changes data is a **command**: a name, a JSON parameter object, and an execute/undo pair. Commands are journaled to a session journal. This one mechanism provides undo/redo, macro recording, UI test replay (T-05) and the future API (FUT-API).
- **A-04 Stable identities.** Every persistent object (document, feature, sketch entity, component, net, pad, track, rule, link) has a UUID (v7, time-ordered) that never changes for the object's life and survives save/load.
- **A-05 Kernel adapter.** All geometry kernel calls go through `geom::` interfaces. No OCCT type appears in headers outside `geom/`. This keeps the kernel replaceable and testable.
- **A-06 Topological naming.** Features that reference faces, edges or vertices of earlier features shall resolve them through a persistent-naming scheme, not raw kernel indices. The reference is recorded as (originating feature ID + generation role + geometric signature). When upstream edits change topology, the system re-matches references and reports a "reference lost" error instead of silently picking the wrong entity. *Accept:* golden tests (T-04) for at least 20 classic breakage cases (for example, add a hole before a fillet, change an extrude sketch from 4 to 6 sides) re-resolve correctly or flag an error; none attach to a wrong face.
- **A-07 Regeneration engine.** Model updates use a dependency graph (DAG) spanning sketches, features, parameters, assemblies, drawings and cross-domain links. Only dirty nodes are recomputed, independent branches may compute in parallel, cycles are detected and reported, and each node exposes state (OK, warning, error, suppressed, out of date).
- **A-08 Expression engine.** One shared, unit-aware expression language is used everywhere a number can be typed: sketch dimensions, feature parameters, global variables, PCB rules, link mappings. It supports `+ - * / ^`, parentheses, functions (`sin cos tan asin acos atan atan2 sqrt abs min max floor ceil round if`), units in literals (`10 mm + 0.5 in`), references to named parameters (`Board.Width`), and unit checking (adding mm to degrees is an error).
- **A-09 PCB coordinate model.** PCB geometry is stored in integer nanometres (int64) in a board-local 2D frame plus layer index. Conversion to the mechanical double-precision frame happens only in `link/` and `render/`.
- **A-10 Threading.** The UI thread never blocks on regeneration, DRC, simulation, rendering or file I/O longer than 100 ms. Long tasks run on a job system with progress reporting and cancellation.
- **A-11 Extensibility seam.** Feature types, export formats and DRC checks are registered through factories, so later stages add them without editing a central switch statement.

---

## 6. Open project file format

- **F-01 Open and documented.** The project format is fully specified in `docs/format/` with human-readable Markdown and a **JSON Schema (draft 2020-12)** for every document type. The specification is versioned alongside the code.
- **F-02 Two equivalent encodings.**
  - **Text (`.apj` folder form):** a project directory of pretty-printed UTF-8 JSON files, one per document, plus a `blobs/` folder. Designed for Git diffs and merges.
  - **Binary (`.apb` single file):** one BSON file containing the same tree, with blobs embedded as BSON binary. Designed for fast load and single-file sharing.
  - Conversion between the two is lossless and available from the UI and the CLI. *Accept:* round-trip tests (T-03) prove `json → bson → json` is byte-identical for every golden project.
  - File extensions are placeholders until the product is named.
- **F-03 Deterministic output.** JSON output is deterministic: `$magic` first (F-09), then object keys in schema order, arrays in stable order, floating-point values written in shortest round-trip form, LF line endings. Saving an unchanged project produces byte-identical files.
- **F-04 Parametric source of truth.** Files store the **design intent** (sketches, constraints, feature parameters, mates, schematic, PCB objects, rules, links). Derived data (B-rep cache, tessellation, DRC results, simulation results) is optional, stored in `cache/` or as cache blobs, flagged non-authoritative, and fully regenerable. Deleting the cache must never lose design data.
- **F-05 Versioning and migration.** Every file has `formatVersion` (semantic version). The application loads all older versions, migrating forward with logged steps. Newer minor versions load with a warning; newer major versions are refused with a clear message.
- **F-06 Blobs.** Large binary payloads (imported meshes, textures, STEP files embedded as references, render images) are stored by SHA-256 content hash and deduplicated.
- **F-07 Library files.** Component libraries, material libraries, profile libraries and fastener libraries use the same encodings and schema conventions, so they can be shared independently of projects. Library scopes are defined in F-10.
- **F-08 Robust loading.** Unknown fields are preserved on load and written back on save (forward compatibility). Malformed files produce a precise error (file, JSON path, reason) and never crash the application. *Accept:* fuzz tests (T-09).

- **F-09 Magic value (file identification).** Every project, document and library file carries a fixed format-identifier UUID, so the application (and other tools) can recognise its files without relying on the extension.
  - **Magic UUID:** `d9a3877a-3f27-4df8-9528-f8dfde59773d`. It identifies the format family, is the same for every document type and every format version, and **never changes**. Versioning is carried separately by `formatVersion` (F-05); the document kind by `type`.
  - **JSON encoding:** the first key of the root object is `"$magic"`, whose value is the UUID as a lowercase string. A reader recognises a file by parsing the first key; a quick check may also look for the UUID string in the first 256 bytes.
  - **BSON encoding:** the first element of the root document is `$magic`, encoded as BSON binary subtype `0x04` (UUID, RFC 4122 byte order). Because BSON fixes the layout of that first element, **bytes 4 to 32 of every valid file are constant**:

    ```
    offset  0–3   int32  total document length (varies)
    offset  4     0x05   element type: binary
    offset  5–11  24 6D 61 67 69 63 00   "$magic\0"
    offset 12–15  10 00 00 00            binary length 16 (little-endian)
    offset 16     0x04                   subtype: UUID
    offset 17–32  D9 A3 87 7A 3F 27 4D F8 95 28 F8 DF DE 59 77 3D
    ```

    A reader identifies the file by comparing those 29 bytes, then reads it as ordinary BSON, so the files stay readable by any standard BSON tool.
  - Files whose magic is missing or different are rejected with "not an APP file" (or, for JSON files without `$magic`, opened only through an explicit "import legacy/unknown JSON" path).
  - Blob files under `blobs/` are raw payloads and carry no magic; they are reached only through hashed references from files that do.
  - *Accept:* tests confirm the exact BSON byte prefix for every document type, that JSON output starts with `{"$magic":` (ignoring whitespace), and that files with a wrong or missing magic are rejected without a crash.

- **F-10 Library scopes (global seed, per-user, per-project).** Every kind of library (components, symbols, footprints, 3D models, reuse blocks and snippets, materials, appearances, structural profiles, sheet-metal gauge tables, hole and thread tables, fastener and standoff tables, footprint-generator parameter tables, drafting standards, rule sets, sheet and drawing templates) is resolved from three scopes:

  | Scope | Location (default, configurable) | Contents | Editable |
  |---|---|---|---|
  | **Global** | `%ProgramData%\APP\libraries\` | Seeded on install from the `/libraries` folder shipped with the app; shared by all users of the machine | Yes, by default |
  | **User** | `%APPDATA%\APP\libraries\` | The user's own items and overrides | Yes |
  | **Project** | `<project>/libraries/` | Items specific to one project, and copies of anything the project uses | Yes |

  - **Resolution order:** project → user → global. An item in a narrower scope with the same library ID **overrides** (shadows) the wider one. The library browser shows each item's scope and marks overridden items.
  - **Editing:** a global or user item can be edited in place, or "overridden into" the user or project scope, which copies it there with a link to its origin. Lifecycle and revision rules (E-LIB-02) apply in every scope.
  - **Seed versions:** the shipped seed has its own version number. On upgrade, the installer never overwrites changed global items. It installs the new seed alongside and offers a review: unchanged items update automatically; items changed both locally and in the new seed are shown as a three-way difference (old seed, new seed, local) for the user to resolve.
  - **Self-contained projects:** on save (option on by default), every library item a project uses is copied into the project scope, recording origin scope, library ID and revision. The project therefore opens identically on another machine without that machine's user or global libraries. "Update from library" (E-LIB-02) compares these copies against the user and global scopes.
  - **Configuration:** scope folders and extra read-only library folders (for example a team share or a Git checkout) are set in Preferences, stored in the user settings file.
  - *Accept:* with the same component ID in all three scopes, the project item is used; deleting it falls back to the user item, then the global item. A project saved on one machine opens on a clean machine with identical results. An upgrade test with a locally modified seed item presents the three-way review and changes nothing until the user resolves it.

### 6.1 Project layout (text form)

```
MyProduct.apj/
  project.json            manifest: format version, units, document list, global parameters
  parts/<uuid>.part.json
  assemblies/<uuid>.asm.json
  drawings/<uuid>.drw.json
  schematics/<uuid>.sch.json
  boards/<uuid>.pcb.json
  links/links.json        cross-domain link definitions (section 12)
  libraries/              project-scope libraries (F-10), incl. copies of used items
  blobs/<sha256>.bin
  cache/                  optional, deletable
```

### 6.2 Illustrative excerpt (normative content lives in the schema)

```json
{
  "$magic": "d9a3877a-3f27-4df8-9528-f8dfde59773d",
  "formatVersion": "1.0.0",
  "type": "part",
  "id": "0193a7c2-6c1e-7a4b-9f0e-2b1c3d4e5f60",
  "name": "EnclosureBase",
  "units": { "length": "mm", "angle": "deg" },
  "parameters": [
    { "name": "WallThickness", "expr": "2 mm" },
    { "name": "CavityWidth", "expr": "link:BoardOutline.Width + 2 * 0.5 mm" }
  ],
  "features": [
    {
      "id": "0193a7c2-6c1f-7000-8000-000000000001",
      "type": "Extrude",
      "name": "Base",
      "sketch": "0193a7c2-6c1f-7000-8000-0000000000a0",
      "direction": "normal",
      "depth": { "expr": "CavityHeight + WallThickness" },
      "operation": "new-body"
    }
  ]
}
```

---

## 7. Test harness requirements

The test harness is a deliverable of Stage 0 and grows with every stage. No stage is complete while its tests are red.

- **T-01 Unit tests** for every domain module, as one GoogleTest executable per module (`tests/<module>.Tests.vcxproj`), run from Visual Studio Test Explorer or `scripts/test.ps1`.
- **T-02 Integration tests** run command scripts against the headless `cli` (A-02) and assert on model state.
- **T-03 Format round-trip tests.** For every golden project: load → save (JSON) → reload → save (BSON) → reload → save (JSON); outputs must be byte-identical.
- **T-04 Geometry golden tests.** Golden parts and assemblies under `tests/data/` with expected volume, surface area, centre of mass, bounding box, face/edge/vertex counts, validity check, and mate DOF counts. Numeric tolerance is relative 1e-6 unless stated.
- **T-05 UI replay tests.** A recorded command journal plus scripted UI events replays in an off-screen window and asserts on model state and selected widget states. Uses Qt Test.
- **T-06 Visual snapshot tests.** Off-screen renders of golden scenes are compared to reference images with a perceptual tolerance (for example SSIM ≥ 0.98). References are updated only through an explicit command.
- **T-07 Output conformance tests.** Generated Gerber, Excellon, IPC-2581, STEP, STL, 3MF, DXF and PDF files are checked by (a) structural parsers in the test suite, (b) re-import where the app supports it, and (c) comparison to golden outputs with volatile fields (dates, IDs) masked.
- **T-08 Benchmarks** for every P-1x target, run on demand and in nightly builds, with results appended to `bench/history.csv`.
- **T-09 Fuzzing** of every file loader and expression parser (libFuzzer or equivalent on Clang, at least nightly).
- **T-10 Domain verification suites:**
  - Sketch solver: over/under-constrained detection, conflicting-constraint reporting, drag stability.
  - Motion: analytic benchmarks (simple pendulum period, slider-crank kinematics, free fall, spring-mass frequency) within 0.5 %.
  - SPICE: RC step response, RLC resonance, diode rectifier, op-amp gain, compared to analytic results within 1 %.
  - DRC: a library of seeded-violation boards where every violation must be found, with zero false positives on clean boards.
  - Tolerance stack-up: textbook 1D stacks, worst-case and RSS exact; Monte Carlo within 2 % of RSS for normal inputs.
- **T-11 Static checks:** compiler warnings as errors (`/W4 /WX`), MSVC Code Analysis and Clang-Tidy (both run from Visual Studio on MSBuild projects), layering check (A-01), network-API ban (P-03), SPDX header and provenance check (CR-07), licence manifest check (P-30).
- **T-12 Coverage.** Line coverage reported per module; target ≥ 80 % for `core`, `doc`, `sketch`, `ecad/pcb` rules and DRC.
- **T-13 One-command run.** `scripts/test.ps1` builds the `Headless` configuration with MSBuild and runs every GoogleTest executable plus the CLI integration and replay suites, writing JUnit-style XML results to `out/test-results/`. Benchmarks and fuzzing run separately (`-Bench`, `-Fuzz` switches).
- **T-14 Reference scenario test.** The section 1 reference scenario is automated end to end from Stage 6 onward and is the release gate for every later stage.

---

## 8. User interface requirements

- **U-01 Modern visual design.** Flat, clean UI with light and dark themes; consistent 4/8 px spacing grid; a single icon set designed for this product (CR-01); smooth 60 fps transitions where used; no dated 3D bevels.
- **U-02 High-DPI and multi-monitor.** Correct scaling at 100–300 % and per-monitor DPI; panels can be undocked onto other monitors.
- **U-03 Workspace layout.** Dockable, tabbed panels (model tree, properties, layers, nets, libraries, messages, simulation plots). Layouts save per workspace mode (Part, Assembly, Drawing, Schematic, PCB, Simulation) and can be reset.
- **U-04 Contextual command bar.** A top command bar (tabbed ribbon or toolbar) shows commands relevant to the active document and selection.
- **U-05 Command palette.** `Ctrl+K` opens a fuzzy-search palette of every command, recent files and settings.
- **U-06 Properties inspector.** Selecting any object shows its editable properties, with expression entry (A-08) in every numeric field.
- **U-07 In-canvas editing.** Dimensions and parameters can be edited by double-clicking them in the viewport. Feature dialogs show live preview before committing.
- **U-08 Navigation.** 3D: orbit, pan, zoom-to-cursor, view cube, standard views, section view, configurable mouse presets (including presets that mimic common CAD conventions without copying any vendor's UI). 2D (schematic/PCB): pan, zoom, fit, cross-probe.
- **U-09 Selection.** Click, box (window/crossing), lasso, selection filters (face, edge, vertex, body, component, net, pad, track), and "select other" for occluded items.
- **U-10 Keyboard.** Every command can be bound to a shortcut; bindings are stored in an open JSON file.
- **U-11 Messages.** A non-modal message panel lists warnings and errors with click-to-locate. Modal dialogs are used only for destructive confirmations.
- **U-12 Accessibility.** Keyboard navigation of all panels and dialogs, screen-reader names on controls, a colour-blind-safe default palette for layers and nets, and minimum 4.5:1 text contrast.
- **U-13 Localisation-ready.** All user-visible strings are externalised (Qt translation files). English only at first.
- **U-14 Start page.** Recent projects, templates (blank part, blank assembly, PCB-in-enclosure template), and links to local tutorials.
- **U-15 Offline help.** Context help (F1) opens a locally bundled HTML or Markdown user guide.

---

## 9. Staged delivery plan

Each stage ends with a runnable build, green tests and an updated traceability file (section 16). Stages are ordered so that the reference scenario (section 1) works end to end at **Stage 6**; later stages deepen capability.

| Stage | Name | Requirements | Exit criteria |
|---|---|---|---|
| **S0** | Foundation | P-*, A-01…A-11, F-01…F-08, T-01…T-13 scaffolding, U-01…U-06, U-08 and U-09 (basic), U-10, U-11 | `APP.slnx` builds from a clean clone in Visual Studio 2026. App launches with themed shell, dockable panels and a 3D viewport showing a kernel-generated box. A box command → save JSON → reload → save BSON → reload → export STL round-trips in an automated test, with F-09 magic checks. Global seed, user and project library folders resolve per F-10 (with an empty seed). CLI runs a command script headless. CI runs all tests. |
| **S1** | Sketcher | M-SKT-01…04 | Fully constrained sketches of 10 golden profiles solve; conflict diagnosis works; P-13 is met; global parameters drive sketch dimensions. |
| **S2** | Part modeling core | M-PRT-01…04, M-PRT-09, M-PRT-10, M-IO-01 (STEP export + import), M-3DP-01 (STL/3MF basic) | The 30 golden parts in T-04 rebuild with the expected mass properties; editing an early feature regenerates downstream features with correct topological re-referencing (A-06). |
| **S3** | Assemblies core | M-ASM-01, M-ASM-02, M-ASM-04, M-ASM-06, M-ASM-07, M-PRT-11 | Golden mechanisms show the expected DOF; an in-context part follows a changed reference part; interference detection finds the seeded clashes; exploded view and keyframe animation export frames. |
| **S4** | Electronics: libraries and schematic | E-PRJ-01, E-PRJ-04, E-LIB-01…03, E-SCH-01…05 | A 4-sheet hierarchical golden schematic compiles to the expected netlist; ERC finds the seeded errors; footprint generator produces the expected pad geometry for 20 package cases. |
| **S5** | PCB layout | E-PCB-01…05, E-PCB-07, E-PCB-08, E-RUL-01, E-RUL-02, E-3D-01 | The golden board is placed and routed interactively (UI replay test); DRC finds every seeded violation with no false positives; 3D board view matches the stack-up. |
| **S6** | MCAD ↔ ECAD integration | X-01…X-10, E-MB-01, E-PRJ-03, E-MCAD-02 | **Reference scenario (T-14) passes end to end**, including the board-width and connector-move edits. |
| **S7** | Documentation and manufacturing | M-DRW-01…03, E-DOC-01, E-OUT-01, E-BOM-01, M-IO-01 (full), M-3DP-01 (full) | Golden drawings match snapshots; Gerber/Excellon/IPC-2581 outputs pass conformance (T-07); enclosure parts export as valid watertight 3MF. |
| **S8** | Advanced modeling | M-PRT-05, M-PRT-06, M-PRT-07, M-SHM-01, M-SHM-02, M-WLD-01, M-WLD-02 | Golden sheet-metal flat patterns match analytic lengths; golden weldment cut list matches; surfacing golden tests pass. |
| **S9** | Simulation and analysis | M-SIM-04, M-SIM-05, E-SIM-01 | T-10 motion, tolerance and SPICE verification suites pass. |
| **S10** | Scale and polish | M-ASM-03, M-ASM-05, M-VIS-01, M-INS-01; U-07…U-09 and U-12…U-15 complete | P-14 met; fasteners auto-insert into golden hole series; renders pass snapshot tests; inspection report exports. |
| **F** | Future | Section 14 | Planned only; architecture seams are kept open now. |

Stage order may change by recorded decision (ADR). A stage may start early work on a later requirement only if its own exit criteria are already green.

---

## 10. Mechanical functional requirements

### 10.1 Sketching

#### M-SKT-01 Sketch entities and tools
`source: SW-SKT-01` · `stage: S1`
- The system shall create 2D sketches on a reference plane, a planar face or a sketch-placement coordinate system.
- Entities: point, line, construction line, centre-point/3-point/tangent arc, circle, ellipse, partial ellipse, parabola (should), interpolating spline, control-point spline, straight and arc slot, corner/centre/3-point rectangle, parallelogram, regular polygon, text (converted to outline geometry from a chosen font).
- Tools: trim (power-trim drag), extend, split, offset (with optional cap ends), mirror (static and dynamic), linear and circular sketch patterns, sketch fillet, sketch chamfer, convert/project model edges into the sketch, intersection curves, toggle construction.
- Any entity can be toggled to construction geometry, which is excluded from profiles.
- Profiles: closed regions are detected automatically, including nested and touching contours; the user can select individual regions for features.
- **Accept:** every entity and tool is reachable through a command (A-03); golden tests for trim/extend/offset on 15 cases; region detection golden tests on 10 nested-contour sketches.

#### M-SKT-02 Sketch constraints and dimensions
`source: SW-SKT-02` · `stage: S1`
- Geometric constraints: coincident, horizontal, vertical, parallel, perpendicular, tangent (including G2 curvature continuity for splines), concentric, coradial, equal, collinear, midpoint, symmetric, fix, on-curve (point on entity), pierce (with 3D geometry), merge.
- Dimensions: linear (aligned, horizontal, vertical), angular, radius, diameter, arc length, ellipse axes, point-to-curve distance; each dimension is driving or driven (reference).
- Auto-inference: while drawing, the system infers horizontal, vertical, tangent, coincident and parallel constraints, shows them as hints, and applies them on commit; inference can be suppressed by holding a modifier key.
- Status: entities are coloured by state (under-defined, fully defined, over-defined, unsolvable), and remaining degrees of freedom (DOF) are displayed.
- Conflicts: when a constraint over-defines the sketch, the system identifies a minimal conflicting set and offers to delete or convert one to driven.
- Dragging an under-defined entity solves interactively and respects constraints.
- **Accept:** the T-10 solver suite passes; P-13 is met; the DOF count is correct on 20 golden sketches.

#### M-SKT-03 3D sketching
`source: SW-SKT-03` · `stage: S1` (entities) / `S8` (full use in sweeps and weldments)
- The system shall create 3D sketches containing points, lines, arcs (on an auto-chosen plane), splines and projected curves in model space.
- Constraints: along X/Y/Z, parallel to a plane, coincident, tangent, perpendicular, equal, fixed; dimensions: length, angle, point-to-point distance.
- 3D sketches are valid as sweep paths, loft guide curves, structure (weldment) paths and future routing paths.
- **Accept:** a golden handrail-frame 3D sketch solves fully defined and drives a sweep in S2 and a structure member set in S8.

#### M-SKT-04 Equations and global parameters
`source: SW-SKT-04` · `stage: S1`
- The system shall provide named parameters at part, assembly and **project** scope, editable in a parameter table.
- Any dimension or numeric feature input may be an expression (A-08) referencing parameters, other dimensions (by name) and, from S6, linked cross-domain values.
- The parameter table shows value, expression, unit, scope, where used, and an error status; circular references are detected and reported.
- Parameters can be imported from and exported to CSV and JSON.
- **Accept:** changing `Project.WallThickness` in a golden project regenerates every dependent part and assembly; a cycle `A = B + 1, B = A` is rejected with a message naming both parameters.

### 10.2 Part modeling

#### M-PRT-10 Reference geometry
`source: implied by SW-SKT-01 / SW-PRT-01` · `stage: S2`
- The system shall create reference planes (offset, angled, through 3 points, normal to curve, mid-plane, tangent), axes (two planes, two points, cylinder axis, edge), points (centre, intersection, on curve at distance or percentage) and coordinate systems.
- Reference geometry is parametric and appears in the feature tree.
- **Accept:** golden tests for each creation method.

#### M-PRT-01 Solid features
`source: SW-PRT-01` · `stage: S2`
- **Extrude** with end conditions: blind, through all, up to next, up to vertex, up to surface, offset from surface, mid-plane; second direction; draft angle; thin-feature (wall) option; start offset.
- **Revolve** with angle, mid-plane and two-direction options; thin option.
- **Sweep** along a path, with optional guide curves, twist (none, angle, follow path) and thin option.
- **Loft** through two or more profiles, with optional guide curves, centreline and start/end tangency conditions.
- **Boundary** feature from curves in two directions with tangency/curvature conditions (should in S2, must by S8).
- Each feature is either additive (boss) or subtractive (cut), and either merges into or creates bodies; a feature scope selects which bodies it affects.
- **Accept:** T-04 golden parts for each feature type and end condition; invalid inputs (open profile, self-intersecting result) produce a feature error, not a crash.

#### M-PRT-02 Applied features
`source: SW-PRT-02` · `stage: S2`
- **Fillet:** constant radius, variable radius, face fillet, full-round fillet, with tangent-propagation and setback options.
- **Chamfer:** distance-distance, angle-distance, vertex chamfer.
- **Shell:** uniform thickness with per-face thickness overrides; inward or outward.
- **Draft:** neutral plane and parting line methods.
- **Rib:** from an open sketch, with thickness, direction and draft.
- **Hole tool:** simple, counterbore, countersink, counterdrill, tapped and pipe-tapped holes, placed on faces by sketch points. Sizes come from user-editable standard tables (metric and inch) shipped as JSON (CR-05). End conditions follow M-PRT-01.
- **Cosmetic thread:** stored as thread metadata (designation, length, class) and shown as a texture; carried into drawings (M-DRW-02) and fastener matching (M-ASM-05). Modelled helical threads are a *should* (useful for 3D printing).
- **Accept:** T-04 golden tests for each feature; a hole created from the M6 tapped table has the tabulated tap-drill diameter and thread metadata.

#### M-PRT-03 Patterns and mirrors
`source: SW-PRT-03` · `stage: S2`
- Linear pattern (one or two directions, spacing or total length, instances to skip), circular pattern (equal spacing or angle), curve-driven pattern, sketch-driven pattern, table-driven pattern (coordinates from a table or CSV), and variable pattern (per-instance dimension changes from a table).
- Mirror features, faces or bodies about a plane or planar face.
- Patterns can apply to features, faces or bodies; "geometry pattern" mode skips re-solving end conditions per instance for speed.
- **Accept:** golden pattern tests, including skipped instances and a variable pattern of 10 instances.

#### M-PRT-04 Feature tree
`source: SW-PRT-04` · `stage: S2`
- The system shall show an ordered feature tree: origin, reference geometry, sketches, features, bodies folder, material, and parameter/equation folder.
- Operations: edit feature, edit sketch, rename, suppress/unsuppress, delete (with dependants listed and optional cascade), reorder by drag with dependency validation, group into folders, and a rollback bar to insert features earlier in history.
- Each node shows its state (A-07), and a "what's wrong" panel lists errors and warnings with click-to-locate.
- A parent/child view shows a feature's dependencies in both directions.
- **Accept:** UI replay tests for reorder, rollback-insert and delete-with-dependants; reorder that would break dependencies is refused with an explanation.

#### M-PRT-05 Multibody parts
`source: SW-PRT-05` · `stage: S8`
- A part may hold any number of solid and surface bodies.
- Operations: combine (add, subtract, common), split (by plane, surface or sketch) with optional save-to-new-part, move/copy body, delete/keep body, body folders, insert another part as a derived body (linked, with update).
- Feature scope controls which bodies each feature affects.
- **Accept:** a golden "enclosure split into base and lid" part produces two bodies and two derived parts that update when the master changes.

#### M-PRT-06 Surface modeling
`source: SW-PRT-06` · `stage: S8`
- Surface features: extruded, revolved, swept, lofted, boundary, filled (n-sided patch with tangency/curvature), offset, ruled, planar, and mid-surface (should).
- Surface operations: knit (with optional "try to form solid"), trim (standard and mutual), untrim, extend, thicken (to solid), delete face, replace face with surface, cut solid with surface.
- Surface continuity analysis: zebra stripes and curvature colour map (should).
- **Accept:** golden surface-to-solid models: a knit closed set forms a valid solid; a thickened lofted surface has the expected volume.

#### M-PRT-07 Direct editing
`source: SW-PRT-07` · `stage: S8`
- Without editing history, the user can move/rotate faces, offset faces, delete faces with healing, replace faces and resize fillets, on native or imported (STEP/IGES) bodies.
- Each direct edit is recorded as a feature in the tree and is itself parametric.
- **Accept:** golden tests on an imported STEP bracket: move a hole face 5 mm, delete a fillet, offset a face by 1 mm; all bodies remain valid.

#### M-PRT-09 Materials and mass properties
`source: SW-PRT-09` · `stage: S2`
- A material library (open JSON, F-07) defines density, elastic modulus, Poisson's ratio, yield and tensile strength, thermal conductivity, specific heat, thermal expansion, and a default appearance. A starter set of common engineering materials ships with the app, with sources recorded (CR-05).
- Materials are assigned per part or per body.
- Mass properties report mass, volume, surface area, centre of mass, inertia tensor and principal moments/axes about a selectable coordinate system; mass can be overridden (for purchased components).
- Bounding box: axis-aligned and minimum-volume oriented box, with dimensions exposed as parameters.
- **Accept:** golden mass properties within relative 1e-6 of analytic values for primitives and within 1e-4 of reference values for golden parts.

#### M-PRT-11 Configurations
`source: SW-PRT-11` · `stage: S3`
- Parts and assemblies may hold named configurations. A configuration can vary: parameter values, feature suppression, material, custom properties, and (assemblies) component suppression, visibility, configuration-of-component and mate suppression.
- A configuration table (in-app grid, importable/exportable as CSV) drives many configurations at once. No external spreadsheet application is required.
- Derived configurations (child inherits from parent and overrides) are a *should*.
- From S6, configurations can be linked to electronic assembly variants (X-08).
- **Accept:** a golden bolt part with 12 configurations from a table produces 12 correct mass values; an assembly configuration swaps component configurations correctly.

### 10.3 Sheet metal and weldments

#### M-SHM-01 Sheet metal features
`source: SW-SHM-01` · `stage: S8`
- Sheet-metal parameters per part or body: thickness, default bend radius, bend allowance method (K-factor, bend allowance, bend deduction, or bend table), relief type and size. Gauge tables (JSON) provide thickness/radius/K-factor sets per material.
- Features: base flange/tab, edge flange (length and position options), miter flange, hem (closed, open, teardrop, rolled), jog, sketched bend, lofted bend (formed and bent), closed corner, corner relief and corner treatments (break corner, weld corner), forming tools from a user library of form features, and conversion of a shelled solid to sheet metal (should).
- Unfold/fold features allow cuts across bends.
- **Accept:** golden sheet-metal parts rebuild; bends carry radius and angle metadata.

#### M-SHM-02 Flat pattern
`source: SW-SHM-02` · `stage: S8`
- Every sheet-metal body has an automatically maintained flat pattern, shown as a derived configuration.
- The flat pattern shows bend lines with bend direction (up/down), angle, radius and order notes, and reports overall flat size.
- Export the flat pattern to DXF with configurable layer mapping (outline, interior cuts, bend-up lines, bend-down lines, etch/marking).
- **Accept:** flat lengths for 10 golden bent parts match the analytic K-factor calculation within 0.01 mm; DXF output re-imports with the correct layers.

#### M-WLD-01 Weldments and structural frames
`source: SW-WLD-01` · `stage: S8`
- A profile library (open JSON) of parametric cross-sections: square/rectangular tube, round tube, pipe, angle, channel, I/H-beam, T-section, flat bar, round bar, and user-defined sketch profiles. Default sizes come from public dimensional data (CR-05).
- Place structural members along 2D/3D sketch segments, with profile rotation, pierce point (profile anchor) and member groups.
- Corner treatments at joints: miter, butt (either member trimmed), and trim/extend to faces or bodies with gap control.
- End caps, gussets (triangular or polygonal, chamfered, with thickness) and weld beads (cosmetic with optional simple geometry; size, length, intermittent pitch).
- **Accept:** a golden rectangular frame with mitred corners has the expected member lengths and mass; changing the 3D sketch updates all members.

#### M-WLD-02 Cut lists
`source: SW-WLD-02` · `stage: S8`
- An automatically maintained cut list groups identical members and reports: item number, quantity, profile description, material, length, end angles (both ends, both planes), mass, and custom properties.
- The cut list updates on every regeneration, can be shown as a drawing table (M-DRW-03), and exports to CSV.
- **Accept:** the golden frame cut list matches the expected quantities, lengths and angles.

### 10.4 Assemblies

#### M-ASM-01 Mates
`source: SW-ASM-01` · `stage: S3`
- Components are placed as instances of parts or sub-assemblies (with a configuration), fixed or floating.
- **Standard mates:** coincident, parallel, perpendicular, tangent, concentric, distance, angle, lock; each with aligned/anti-aligned flip.
- **Advanced mates:** symmetric, width (centre between two face pairs), path (point along a curve), linear/linear coupler, limit distance, limit angle, profile centre, slot.
- **Mechanical mates:** gear (ratio), rack and pinion, cam follower, screw (pitch), hinge (concentric + coincident in one), universal joint.
- The mate solver (D-06) reports remaining DOF per component, identifies over-defined or conflicting mates with a minimal conflict set, and supports dragging components while respecting mates.
- Mates reference geometry through persistent naming (A-06).
- **Accept:** 15 golden mechanisms (slider-crank, four-bar, gear pair, rack and pinion, cam, screw jack, hinge, universal joint) report the expected DOF and move correctly in drag replay tests.

#### M-ASM-02 Top-down (in-context) design
`source: SW-ASM-02` · `stage: S3`
- Edit a part in the context of an assembly, with other components visible and selectable as references.
- References to another component's geometry create **external references** that are tracked, listed (where used, status), and can be locked (frozen) or broken by the user.
- Assembly-level **layout sketches** and **skeleton parts** can drive the geometry of several parts.
- Assembly features (cuts or holes across several components) are a *should*.
- This capability is the mechanism used by cross-domain links (section 12).
- **Accept:** a golden in-context lid, referencing a base part's rim, updates when the base rim changes; locking the reference stops the update; the external-reference list shows the correct status.

#### M-ASM-03 Large assembly handling
`source: SW-ASM-03` · `stage: S10` (lightweight mode foundations from S3)
- **Load modes:** full; lightweight (display mesh, bounding volumes and cached mass properties loaded, B-rep resolved on demand when edited or measured); review-only (meshes only, with measure, section, hide/show and interference preview).
- **Selective loading:** open only the components matching a filter (sub-assembly, named selection, spatial box, property query); unloaded components appear as placeholders.
- **Simplified representations:** generate defeatured versions (remove internal faces, small features under a size threshold, or replace with an envelope) and switch between full and simplified per component.
- Geometry instancing for repeated components, level-of-detail rendering, and background loading with a progress indicator.
- **Accept:** P-14 met on a generated 2,000-part assembly; editing a lightweight component resolves it automatically.

#### M-ASM-04 Interference and clearance detection
`source: SW-ASM-04` · `stage: S3`
- Detect volume interferences between selected components (or all), listing each pair with interference volume and highlighting the overlap.
- Options: treat coincident faces as interference or not, include/ignore hidden components, ignore cosmetic-thread overlaps, include multibody parts' own bodies.
- **Clearance check:** report pairs closer than a user threshold, with minimum distance.
- **Hole alignment check:** report holes in mating components whose axes deviate beyond a tolerance.
- From S6, PCB components and boards participate as ordinary components (X-06).
- **Accept:** a golden assembly with 6 seeded interferences, 3 clearance violations and 2 misaligned holes reports exactly those.

#### M-ASM-05 Smart components and fasteners
`source: SW-ASM-05` · `stage: S10`
- A parametric fastener and hardware library (CR-05): socket head, button head, pan head, countersunk and hex screws; hex nuts; washers; dowel pins; threaded heat-set inserts; and **PCB standoffs/spacers**. Each is generated from size tables, in metric and inch series.
- **Auto-fastening:** detect aligned hole series (from the Hole tool or geometry) through stacked components and insert fasteners, choosing size from hole metadata and length from stack thickness rounded up to the nearest stocked length; washers and nuts are optional.
- Inserted fasteners re-size automatically when the hole size or stack thickness changes, with a warning if no stocked length fits.
- **Smart components:** a part may carry associated features that are applied to the parts it is mated to (for example, a heat-set insert cuts its pilot hole into a boss).
- **Accept:** auto-fastening a golden enclosure with 4 hole series inserts 4 correctly sized screws; changing the lid thickness by 2 mm updates the screw lengths.

#### M-ASM-06 Exploded views and assembly animation
`source: SW-ASM-06` · `stage: S3`
- Create one or more named exploded views per assembly configuration, with ordered steps of linear, radial and rotational moves; auto-explode (should).
- Explode lines, with jogs, can be shown in the 3D view and in drawings.
- Animate explode/collapse in the viewport and export it as an image sequence.
- **Accept:** a golden exploded view saves, reloads and replays identically; its drawing view shows the explode lines.

#### M-ASM-07 Basic motion and animation
`source: SW-ASM-07` · `stage: S3`
- A timeline with keyframes for component positions, camera, visibility and appearance.
- Kinematic playback driven by mates, with rotary and linear **motors** (constant speed, or speed/position profiles as expressions or point tables), **springs** (linear, with free length and stiffness) and **gravity**, using the mate solver.
- Optional contact detection that stops motion at first contact (no dynamics).
- Export animation as a PNG image sequence; MP4 export *should* be provided via an optional, dynamically loaded encoder.
- **Accept:** a golden slider-crank driven by a 60 rpm motor produces slider positions that match the analytic kinematics within 0.1 % at 100 sampled frames.

### 10.5 Drawings

#### M-DRW-01 Drawing views
`source: SW-DRW-01` · `stage: S7`
- Drawing documents with multiple sheets; sheet sizes ISO A0–A4, ANSI A–E and custom; sheet templates with title blocks whose fields bind to document properties; first- or third-angle projection per sheet.
- **Views:** named model views (front, top, right, isometric…), projected views, section views (full, half, offset/stepped, aligned) with hatching, detail views, broken views, auxiliary views, cropped views, broken-out sections, and exploded views.
- **Display:** hidden lines removed, hidden lines visible, shaded, shaded with edges; tangent edges visible/hidden/phantom; per-component visibility; view of a chosen configuration.
- Views are fully associative: model changes update views, and annotations re-attach through persistent naming.
- Output: vector PDF, DXF, SVG, PNG and printing.
- **Accept:** snapshot tests (T-06) for 10 golden drawings; changing a golden part's hole position updates the section and detail views and the dimension values.

#### M-DRW-02 Annotations
`source: SW-DRW-02` · `stage: S7`
- **Dimensions:** linear, aligned, ordinate, baseline and chain, angular, radius, diameter, chamfer, arc length; insert model (driving) dimensions; driven dimensions are reference.
- **Tolerances:** symmetric, bilateral, limit, basic, min/max, and fit designations from a user-editable ISO 286 limits-and-fits table.
- **GD&T:** feature control frames per ASME Y14.5 / ISO 1101 symbol sets, datum features, datum targets, composite frames.
- **Other symbols:** surface finish, weld symbols, centre marks, centrelines, notes with property links, leaders, revision clouds, balloons (manual and auto-balloon linked to BOM item numbers, M-DRW-03), and thread callouts from cosmetic-thread metadata.
- Drafting presets for ISO and ANSI (text height, arrow style, units, decimal places); user-editable and saved as open JSON.
- **Accept:** snapshot tests; a golden drawing's tolerances round-trip into M-SIM-05 and M-INS-01.

#### M-DRW-03 Drawing tables
`source: SW-DRW-03` · `stage: S7`
- **BOM table:** top-level only, parts only or indented; columns bound to properties; item numbers synchronised with balloons; configurable sort and grouping; includes electronic components from linked boards (X-07) when selected.
- **Hole table** (from an origin, with tag, position, size), **revision table**, **weldment cut-list table**, **sheet-metal bend table**, and a **general table**.
- Tables update with the model; every table exports to CSV.
- **Accept:** golden BOMs match expected rows; changing an assembly updates the BOM and balloons.

### 10.6 Motion simulation and tolerance analysis

#### M-SIM-04 Motion analysis (rigid-body dynamics)
`source: SW-SIM-04` · `stage: S9`
- Convert mates to joints automatically; the user can add or override joints, bushings (should) and rigid groups.
- Loads: gravity, forces and torques (constant, or expression of time and state), springs and dampers (linear and torsional), and motors (speed, position or torque driven, with profiles).
- **Contacts:** solid-body contact between selected components using convex decomposition or mesh collision, with friction (static/dynamic coefficients) and restitution.
- Solver settings: duration, frame rate, integrator accuracy; uses Project Chrono (D-06).
- **Results:** plots and CSV export of displacement, velocity, acceleration (linear and angular), joint reaction forces, motor torque and power, contact forces, and energy; trace paths of points; animated playback; interference over time.
- **Accept:** T-10 motion benchmarks within 0.5 %; a golden cam-follower with contact keeps the follower on the cam at the design speed.

#### M-SIM-05 Tolerance stack-up analysis
`source: SW-SIM-05` · `stage: S9`
- Define a stack-up by selecting a measurement (gap between two faces in an assembly) and the chain of contributing dimensions; the system proposes the chain from the mates (should).
- Tolerances are read from model and drawing dimensions (M-DRW-02) or entered directly.
- Methods: worst case (min/max), root sum square (RSS), and Monte Carlo with normal or uniform distributions per contributor.
- Report: nominal, min, max, sigma, % contribution and sensitivity ranking per contributor, and pass/fail against the gap requirement; export to PDF and CSV.
- 1D stacks are required; 2D/3D vector-loop stacks are a *should*.
- **Accept:** T-10 tolerance suite.

### 10.7 Inspection and visualization

#### M-INS-01 Inspection documentation
`source: SW-INS-01` · `stage: S10`
- From any drawing, create an inspection project that auto-balloons every characteristic (dimension, tolerance, GD&T frame, note, surface finish) with sequential numbers in a configurable balloon style, without altering the original drawing.
- A characteristic table lists number, type, nominal, upper/lower limits, units, sheet and zone, and inspection method.
- The user can enter measured values; pass/fail is computed.
- Export the ballooned drawing (PDF) and the characteristic report (CSV and PDF) through a configurable report template.
- Characteristic numbers stay stable when a drawing is revised; new characteristics take new numbers.
- **Accept:** a golden drawing with 40 characteristics produces the expected table; revising one dimension keeps all other numbers unchanged.

#### M-VIS-01 Photorealistic rendering
`source: SW-VIS-01` · `stage: S10`
- Physically based appearances (metallic-roughness model) with a starter library: metals, plastics (gloss/matte/textured), rubber, glass, emissive, and electronics finishes (solder mask colours, silkscreen, gold/HASL pads, FR-4 edge).
- Textures, decals (projected images) and per-face appearance overrides.
- Environments: HDR images and studio presets (all shipped assets created for this product or with permissive licences), ground shadows and reflections; lights (directional, point, spot, area) and cameras (perspective, orthographic, depth of field).
- A progressive CPU path tracer (D-13) with optional denoising; output PNG and EXR up to 8K; turntable, exploded and motion-study animations rendered to image sequences.
- The real-time viewport uses the same appearances in a rasterised approximation.
- **Accept:** snapshot tests on 5 golden scenes; 1920×1080 preview quality in < 60 s on the reference machine.

---

## 11. Electronics functional requirements

### 11.1 Project, libraries and reuse

#### E-PRJ-01 Unified project and compilation
`source: AD-PRJ-01` · `stage: S4`
- One project (section 6) holds mechanical documents, schematic sheets, boards, project libraries, output configurations and links.
- **Compile:** the schematic compiles to a connectivity model (components, pins, nets, net classes, directives); compile messages appear in the message panel with click-to-locate.
- **Synchronise:** differences between schematic and board are presented as a change list (add/remove/modify component, footprint change, net add/remove/rename, pin swap, net-class and rule changes). The user accepts or rejects each item, in either direction (schematic → board, or board → schematic for pin/gate swaps and annotation).
- A project navigator shows the document hierarchy, components and nets, with **cross-probing and cross-selection** between schematic, PCB and 3D views.
- **Accept:** a golden project compiles to the expected netlist (T-02); a scripted change list applied in both directions leaves schematic and board in sync with zero differences.

#### E-PRJ-03 Assembly variants
`source: AD-PRJ-03` · `stage: S6`
- Define named variants; each component can be fitted, not fitted (DNP), or replaced by an alternate component, and parameters can be overridden per variant.
- The active variant affects schematic display (DNP marked), BOM, pick-and-place, assembly drawings and 3D (DNP components hidden).
- Variants link to mechanical configurations (X-08).
- **Accept:** golden project with 3 variants produces 3 correct BOMs and pick-and-place files.

#### E-PRJ-04 Design reuse
`source: AD-PRJ-04` · `stage: S4` (schematic) / `S5` (layout)
- **Reuse blocks:** a saved combination of schematic circuit and its PCB placement/routing that can be placed into another project, keeping relative placement and routing.
- **Snippets:** saved fragments of schematic or PCB objects only.
- Reuse blocks and snippets live in libraries (F-07) with revisions (E-LIB-02); placed instances record their source and revision, with "update from library" showing a diff (should).
- **Accept:** placing a golden reuse block (buck regulator) brings in correct schematic, nets and placed/routed layout; designators re-annotate without clashes.

#### E-LIB-01 Component model
`source: AD-LIB-01` · `stage: S4`
- A component bundles:
  - **Symbol(s):** one or more parts (gates) per component, with pins carrying number, name, electrical type (input, output, bidirectional, passive, power, open-collector, open-emitter, high-Z), length and orientation; pin-swap and gate-swap groups (should).
  - **Footprint(s):** one default and optional alternates. Pads have SMD or through-hole type, shape (round, rectangle, rounded rectangle, oblong, chamfered, custom polygon), per-layer padstacks, mask and paste expansion, and round or slotted, plated or non-plated holes. Footprints also have silkscreen, courtyard, assembly outline, fiducials, designator/value text and a 3D body reference.
  - **3D model:** either a native part from this application (preferred, parametric) or an imported STEP, with placement offset and rotation, aligned visually in a footprint 3D preview.
  - **Simulation model:** a SPICE model or subcircuit with pin mapping (E-SIM-01).
  - **Parameters:** value, tolerance, ratings, description, manufacturer, manufacturer part number, local datasheet file path, and custom fields.
  - **Pin–pad map** between symbol pins and footprint pads, validated for missing or extra mappings.
- **Accept:** golden components (resistor, op-amp with 2 gates + power part, MCU in QFN, USB-C connector) validate cleanly; a mismatched pin-pad map is reported.

#### E-LIB-02 Local library management
`source: AD-LIB-02` · `stage: S4`
- Libraries are local open files or folders (F-07), resolved from the global seed, user and project scopes (F-10), and suitable for storing in Git.
- A library manager lets users browse, search (by any parameter), filter, create, edit, copy and validate components, symbols, footprints, 3D models and reuse blocks.
- **Revisions and lifecycle:** each item has a revision identifier and a lifecycle state from a user-configurable list (default: Draft → Released → Deprecated → Obsolete). Released revisions are immutable; editing creates a new revision.
- Projects record the library item and revision used. The project shows when newer revisions exist and offers "update from library" with a change preview.
- "Where used" searches the project folders configured by the user.
- **Accept:** editing a released golden component creates revision B; a project using revision A reports the update and applies it on request.

#### E-LIB-03 Footprint generator
`source: AD-LIB-03` · `stage: S4`
- A wizard generates footprints for package families: chip (0201–2512), MELF, SOD, SOT-23/SOT-223/SOT-89 variants, SOIC, SSOP, TSSOP, QFP, QFN/DFN (with exposed pad and paste subdivision), BGA (full, perimeter, staggered, depopulated), LGA, PLCC, DPAK/D2PAK, DIP, pin headers/SIP, radial and axial through-hole parts.
- Inputs: body and lead dimensions with min/max tolerances, pitch, pin count, and density level (least/nominal/most).
- Pad sizes are computed from toe, heel and side solder-fillet goals plus fabrication and placement tolerances. **All fillet goals and tolerance parameters are user-editable tables** (CR-05); users may enter values from standards they hold licences for.
- The generator also produces courtyard, silkscreen, assembly outline, pin-1 marker and a simple parametric native 3D body.
- **Accept:** 20 package cases produce the expected pad sizes and positions (golden data computed independently by the test suite from the documented formula).

### 11.2 Schematic

#### E-SCH-01 Schematic capture
`source: AD-SCH-01` · `stage: S4`
- Multi-sheet schematics with sheet sizes, templates and title blocks (shared with drawings, M-DRW-01).
- Place components from libraries, with designator, value and parameter display; multi-part components place gates independently.
- Wires with automatic junctions and rubber-banding on drag; buses with bus entries and bus naming (for example `D[0..7]`); net labels; power ports (ground, supply symbols bound to net names); sheet ports; no-connect markers.
- Graphics: lines, shapes, text, text frames, images (stored as blobs), and notes.
- Editing: move, rotate, mirror, copy/paste with designator handling, align, find and replace across sheets, and selection filters.
- **Accept:** UI replay test draws the golden 2-sheet schematic and compiles it to the expected netlist.

#### E-SCH-02 Hierarchy and multi-channel design
`source: AD-SCH-02` · `stage: S4`
- **Hierarchical design:** sheet symbols with sheet entries referencing child sheets, to any depth.
- **Flat multi-sheet design:** connectivity through sheet ports and global labels; the net-scope rules (local, sheet, global) are documented and selectable per project.
- **Multi-channel design:** one child sheet instantiated N times (by a repeat expression), with configurable designator and net naming per channel, and room creation per channel on the PCB.
- **Signal bundles:** a named group of signals (for example an SPI bundle) carried through one connection object and expanded at each end.
- **Accept:** golden hierarchical 4-sheet design and an 8-channel design compile with the expected designators and nets.

#### E-SCH-03 Electrical rules check (ERC)
`source: AD-SCH-03` · `stage: S4`
- A user-editable connection matrix defines error/warning/allowed for every pair of pin electrical types.
- Checks include: unconnected input pins; multiple output drivers on one net; nets with a single connection; undriven power nets; duplicate designators; duplicate or conflicting net names; missing footprint or model; unmatched sheet ports/entries; floating labels; off-grid objects.
- Results appear in a violation browser with click-to-locate; individual violations can be waived with a stored comment.
- **Accept:** a golden schematic with 12 seeded ERC errors reports all 12 and no others.

#### E-SCH-04 Annotation
`source: AD-SCH-04` · `stage: S4`
- Auto-annotate designators by a chosen order (for example across-then-down per sheet), scoped to the project, selected sheets or selected parts; keep or reset existing designators; channel-aware for multi-channel designs.
- Back-annotate from the PCB: renumber by board position (with selectable pattern), and apply pin and gate swaps made on the board.
- **Accept:** golden annotation results match expected designators; board-position renumbering back-annotates correctly.

#### E-SCH-05 Schematic directives
`source: AD-SCH-05` · `stage: S4`
- Directives can be placed on wires, nets, buses or areas to assign: net class, differential pair (with automatic pairing of `_P`/`_N` naming), length-match group, impedance profile, rule parameters (width, clearance, via style), and ERC suppression.
- Directives become PCB rules and classes on synchronisation (E-RUL-01).
- **Accept:** a golden USB pair directive creates a differential-pair class with the specified impedance profile on the board.

#### E-SIM-01 Circuit simulation
`source: AD-SIM-01` · `stage: S9`
- Mixed-signal SPICE simulation via ngspice (D-07), run from the schematic with the active variant.
- Analyses: operating point, DC sweep (nested), transient, AC small-signal, noise, temperature sweep, parameter sweep, and Monte Carlo using component tolerances.
- Sources: DC, pulse, sine, piecewise-linear, exponential, and file-driven PWL.
- Probes on nets and pins for voltage, current and power; a waveform viewer with multiple plots, cursors, automatic measurements (rise/fall time, overshoot, RMS, average, peak, frequency), math expressions, and CSV export.
- Models come from component simulation models (E-LIB-01) and user-supplied model files; simulation setups are saved in the project; engine errors are mapped back to schematic objects.
- **Accept:** the T-10 SPICE suite.

### 11.3 PCB layout

#### E-PCB-01 Layer stack
`source: AD-PCB-01` · `stage: S5`
- A stack manager defines copper, dielectric (core or prepreg, with thickness, Dk, Df and material), solder mask, paste, silkscreen and mechanical/documentation layers.
- At least **64 signal layers** are supported; 128 is a *should*.
- Via spans (through, blind, buried, micro) are defined against the stack (E-PCB-08).
- **Impedance profiles:** single-ended and differential microstrip, embedded microstrip, stripline and coplanar types. The system computes track width and gap for a target impedance using published closed-form approximations (S5); a 2D field solver is a *should*.
- The stack reports total thickness and can export a stack table for fabrication drawings (E-DOC-01).
- **Accept:** golden 4- and 8-layer stacks produce expected thickness; impedance calculations match reference closed-form values within 1 %.

#### E-PCB-02 Board shape and regions
`source: AD-PCB-02` · `stage: S5`
- The board outline is defined with lines and arcs in the PCB editor, or (from S6) driven by linked mechanical geometry (X-02). Internal cutouts and slots are supported.
- Regions: keepouts per layer and per object type (copper, via, track, component); rooms (named regions bound to component groups); component height regions (maximum component height), defined manually or projected from the enclosure (X-04).
- Board origin, multiple snap grids (Cartesian; polar is a *should*) and unit switching.
- **Accept:** golden boards with cutouts and keepouts validate and render correctly in 2D and 3D.

#### E-PCB-03 Component placement
`source: AD-PCB-03` · `stage: S5`
- Components arrive from synchronisation (E-PRJ-01) and are placed interactively: move, rotate (any angle), flip to the opposite side, snap to grid or to other objects, align, distribute, arrange within a rectangle or room, and cluster by sheet or channel.
- A live ratsnest shows unrouted connections, with optional per-net colours.
- **Cross-select** with schematic (select a schematic block, then place its parts together).
- **Room copy:** replicate placement (and routing) from one channel's room to the others.
- Placement rules (courtyard clearance, height, permitted side and orientation) are checked live.
- Components can be locked; mechanically driven components (X-05) are locked automatically and display their link.
- **Accept:** UI replay test places the golden board; room copy replicates 4 channels identically.

#### E-PCB-04 Interactive routing
`source: AD-PCB-04` · `stage: S5`
- Routing modes: walk-around, push-and-shove (pushes tracks and vias), hug-and-push, and ignore-obstacles (violations then flagged by online DRC).
- Track width from rules (minimum/preferred/maximum), cyclable during routing; via insertion with layer change while routing.
- Corner styles: 45°, 90°, arc, and any-angle.
- Automatic cleanup: loop removal, glossing (retrace for smoother/shorter paths).
- Multi-track (bus) routing of several nets in parallel.
- Drag existing tracks and vias with shove; unroute a net, connection or component; auto-complete a connection to its target where the path is clear.
- **Accept:** P-15; UI replay routes the golden board; push-and-shove golden tests leave zero DRC violations.

#### E-PCB-05 High-speed routing
`source: AD-PCB-05` · `stage: S5`
- **Differential pairs:** coupled routing with width and gap from rules or impedance profiles; max uncoupled length; phase matching within the pair.
- **Length tuning:** accordion/trombone/sawtooth patterns with amplitude, spacing and corner-style rules; interactive tuning toward a target length or the longest member of a match group.
- **Signal paths:** pad-to-pad paths that span series components (such as termination resistors), defined manually or auto-detected from 2-terminal passives, used for length and delay matching.
- A nets/length panel reports routed length and propagation delay (from stack Dk) per net, pair and signal path, with match-group status.
- **Return-path check:** reports track segments crossing splits or voids in the adjacent reference plane.
- **Accept:** golden DDR-style match group tuned within ±0.1 mm of target; return-path check finds seeded crossings.

#### E-PCB-07 Copper pours
`source: AD-PCB-07` · `stage: S5`
- Pours on any copper layer, assigned to a net: solid or hatched (line width, grid, angle), with clearance from rules.
- Pad and via connection styles: direct, thermal relief (spoke count, width, gap, angle) or none, per rule scope.
- Island removal below a minimum area, neck removal below a minimum width, pour priority ordering, and pour cutouts.
- Repour on demand; automatic repour during editing is a *should* (must by S10).
- Via stitching (grid inside a region) and via shielding (along a net's route).
- **Accept:** golden pours match expected areas within 0.1 %; thermal reliefs match spoke geometry; no pour violates clearance rules.

#### E-PCB-08 HDI and fabrication features
`source: AD-PCB-08` · `stage: S5`
- Via types: through, blind, buried and micro (laser) vias; stacked and staggered microvias; via-in-pad with fill/cap flags; tenting options.
- **Backdrilling:** from a surface to a layer, with stub length rule, carried into drill outputs.
- **Teardrops** on pads and vias (configurable length, width, curve) as removable, regenerable objects.
- **Embedded components** placed on inner layers, with an automatically created cavity.
- **Cavities:** board regions with reduced thickness.
- Castellated holes (should).
- All features are reflected in DRC, 3D and manufacturing outputs (E-OUT-01).
- **Accept:** a golden 8-layer HDI board with each feature produces correct drill spans and IPC-2581 layer stack data.

### 11.4 Rules, DRC and 3D

#### E-RUL-01 Design rules
`source: AD-RUL-01` · `stage: S5`
- Rule categories:
  - *Electrical:* clearance, short circuit, unrouted net, unconnected pin.
  - *Routing:* width, via style, routing layers, corners, differential pair (width, gap, max uncoupled length).
  - *SMT:* pad entry, neck-down.
  - *Mask and paste:* expansions.
  - *Pours and planes:* connect style, clearance.
  - *Manufacturing:* minimum annular ring, hole size range, hole-to-hole, silk-to-mask, silk-to-silk, acute angle, slivers, board-outline clearance.
  - *High-speed:* length, matched length, maximum via count, parallel segment, maximum stub.
  - *Placement:* component clearance, height, permitted side, orientation.
  - *Impedance-driven width:* from impedance profiles.
- **Scoping:** each rule has a scope written as a query expression over objects (net, net class, layer, component, footprint, room, pad type, differential pair). The query language is documented in `docs/rules-language.md` and shares the expression engine (A-08).
- Rules have explicit priority; the highest-priority matching rule applies, and the UI can show which rule applies to any object.
- **Rule sets:** named groups of rules (for example per fabricator capability) that can be enabled or disabled together and imported/exported as JSON.
- A constraint grid UI edits common rules as matrices (for example clearance by net class × net class).
- **Accept:** golden rule-resolution tests (30 cases) select the expected rule for each object.

#### E-RUL-02 Design rule check (DRC)
`source: AD-RUL-02` (requested as "AD-RUL-002") · `stage: S5`
- **Online DRC:** violations are checked as the user edits and are marked in the canvas.
- **Batch DRC:** full check with a report (HTML, JSON and CSV) and a violation browser with click-to-locate.
- Waivers per violation with comment, author and date stored in the project.
- 3D checks: component-body to component-body clearance from 3D models, and (from S6) clearance to enclosure geometry (X-06). Z-axis clearance between copper on different layers is a *should*.
- Geometry checks use the exact integer model (A-09).
- **Accept:** the T-10 DRC suite; P-16.

#### E-3D-01 3D board model
`source: AD-3D-01` · `stage: S5`
- The system generates the 3D board from the stack: substrate with true thickness, outline and cutouts; plated holes; solder mask, silkscreen and copper shown as textures or (optionally) as thin solids; component 3D models placed from footprints, honouring side and rotation.
- The PCB editor switches between 2D and 3D in place; 3D supports section views, measurement, layer visibility and 3D clearance highlighting.
- The 3D board is the same object used as a mechanical component (X-01).
- **Accept:** snapshot tests; board thickness and hole positions in 3D match the stack and the 2D database exactly.

### 11.5 Documentation, outputs and BOM

#### E-DOC-01 Fabrication and assembly drawings
`source: AD-DOC-01` · `stage: S7`
- PCB drawings use the drawing engine (M-DRW-01) with PCB-specific view types: board view (top/bottom, selectable layers), drill drawing with automatic symbol per hole size and a drill table, layer stack table (E-PCB-01), impedance table, board dimensions, fabrication notes, and assembly views (top/bottom with designators, variant-aware) with BOM table (E-BOM-01).
- Board-edge detail and section views, and an isometric 3D view.
- All content is associative to the board and updates on change.
- **Accept:** snapshot tests for golden fabrication and assembly drawings; adding a hole size adds a drill-table row and symbol.

#### E-OUT-01 Manufacturing outputs
`source: AD-OUT-01` · `stage: S7`
- **Gerber X2** (RS-274X with attributes) per layer: copper, mask, paste, silkscreen, outline, mechanical; plus a **Gerber job file**.
- **Drill:** Excellon (plated and non-plated separated, slots, backdrill spans), and Gerber X2 drill as an option.
- **IPC-2581** (revision B or later) full-board export.
- **Pick-and-place** CSV (designator, X, Y, rotation, side, value, footprint), with configurable origin, units and rotation convention; variant-aware.
- **IPC-D-356** netlist for bare-board electrical test.
- Output configurations (a named list of outputs with settings) are saved in the project and run in one action from the UI or CLI, writing to a dated output folder with optional zip packaging.
- ODB++ is out of scope pending Q-03.
- **Accept:** T-07 conformance: generated Gerbers parse with the test suite's Gerber reader, and pad/track/hole counts and positions match the board database exactly.

#### E-BOM-01 Basic parts list
`source: AD-BOM-01` (basic scope) · `stage: S7`
- A parts list grouped by identical component (same library item and revision, or same value + footprint + manufacturer part number).
- Columns: item, quantity, designators (range-compressed, for example `R1–R4, R7`), value, footprint, description, manufacturer, manufacturer part number, and selected custom parameters.
- Variant-aware, with a separate DNP list.
- Export to CSV and JSON; shown as a drawing table (M-DRW-03, E-DOC-01).
- No pricing, stock or supplier data (offline; see section 14).
- **Accept:** golden BOMs match expected rows for each variant.

#### E-MB-01 Multi-board systems
`source: AD-MB-01` · `stage: S6`
- A multi-board system is an assembly (M-ASM-01) containing two or more boards (X-01) plus mechanical parts.
- Inter-board connections are defined through mating connector pairs, with pin-to-pin mapping (direct, or user-defined cross-mapping).
- The system compiles cross-board connectivity and reports mismatches: pin-mapping conflicts, net-name mismatches across a connection, unconnected mating connectors, and connectors whose 3D mates do not align.
- Boards are positioned in 3D by mates and take part in interference checks.
- Cable harnesses between boards are out of scope (AD-HAR-01, section 14).
- **Accept:** a golden 2-board stack (main board + daughter board via header) compiles; a seeded swapped pin pair is reported.

---

## 12. MCAD ↔ ECAD integration requirements

This section realises `AD-MCAD-01` (and the mechanical side of `AD-MCAD-02`) as a **single shared model**, not as file exchange between two programs. It builds on top-down design (M-ASM-02) and the regeneration graph (A-07).

**Link model.** A *link* connects a quantity owned by one domain (the **driver**) to a quantity in the other domain (the **driven**). Every link has exactly one driver. Links are stored in `links/links.json` (section 6.1), take part in the dependency graph, and can be locked (frozen at the last value) or broken (converted to a plain value).

#### X-01 Board as an assembly component
`source: AD-MCAD-01` · `stage: S6`
- Any board document can be inserted into an assembly as a component; its 3D model (E-3D-01) is its geometry.
- Board faces, edges, hole axes, component bodies and component reference frames (for example a connector's mating face) are valid mate and sketch references, resolved by persistent naming (A-06).
- **Accept:** the golden board mates into the golden enclosure base by 2 hole axes and the bottom face; DOF reported as 0.

#### X-02 Board outline linking
`source: AD-MCAD-01` · `stage: S6`
- Either direction:
  - **Mechanical drives board:** the outline comes from a mechanical sketch, a projected face boundary, or an offset of an enclosure's inner wall (with clearance expression).
  - **Board drives mechanical:** the outline is exposed to mechanical sketches as projected geometry and as parameters (`Board.Width`, `Board.Height`, `Board.Thickness`, outline as a curve set).
- Cutouts link the same way.
- **Accept:** in the reference scenario, changing `Board.Width` (board-driven) from 50 to 60 mm resizes the enclosure cavity; in a second golden project, changing the enclosure inner wall (mechanical-driven) resizes the board outline, and DRC reports any components now outside the board.

#### X-03 Mounting holes and fixings
`source: AD-MCAD-01` · `stage: S6`
- Board mounting holes are exposed as linked points with diameter and fastener metadata.
- Mechanical features can be driven by them (screw bosses, standoff positions, heat-set insert pilots, lid screw clearance holes), or the reverse (enclosure bosses drive the board hole positions).
- From S10, auto-fastening (M-ASM-05) uses linked holes to insert screws and standoffs.
- **Accept:** moving a board mounting hole 3 mm moves the matching enclosure boss 3 mm with no manual edits.

#### X-04 Height limits and keepouts from the enclosure
`source: AD-MCAD-01` · `stage: S6`
- The system projects enclosure interior geometry onto the board to create **component height regions** (maximum height at each region = distance from board top/bottom surface to the nearest enclosure face, minus a clearance expression) and **keepouts** (for example under ribs, bosses and snap features).
- Projected regions update automatically and are used by placement rules and DRC (E-RUL-01, E-RUL-02).
- **Accept:** lowering the golden lid by 3 mm makes the tall capacitor violate its height region in DRC.

#### X-05 Mechanically constrained components
`source: AD-MCAD-01` · `stage: S6`
- Selected components (connectors, switches, LEDs, displays, mounting hardware) can be marked as mechanically constrained:
  - **Mechanical drives component:** the component's position and rotation follow a mechanical reference (for example centred on an enclosure opening); it is locked in the PCB editor and shows the link.
  - **Component drives mechanical:** mechanical features reference the component, for example a **component-driven cut** that removes the component's 3D body (or a named "mating envelope" body from the library) plus a clearance from the enclosure wall.
- **Accept:** moving the USB-C connector 5 mm along the board edge moves the enclosure cut-out 5 mm; the cut-out keeps a 0.3 mm clearance around the connector envelope.

#### X-06 Unified interference and clearance
`source: AD-MCAD-01` · `stage: S6`
- Boards and their components take part in mechanical interference and clearance checks (M-ASM-04) as ordinary components.
- PCB DRC can include 3D clearance between component bodies and selected mechanical bodies (E-RUL-02).
- Results cross-probe: selecting a violation highlights it in the 3D view, the PCB editor and the mechanical tree.
- **Accept:** the reference scenario's seeded clash (capacitor vs lid rib) is found by both the assembly check and the PCB DRC.

#### X-07 Combined product BOM
`source: AD-MCAD-01, AD-BOM-01, SW-DRW-03` · `stage: S7`
- An assembly BOM can include the electronic components of contained boards (as an indented level under each board, or flattened), together with mechanical parts and fasteners.
- **Accept:** golden product BOM lists enclosure parts, screws, the board, and its components with correct quantities.

#### X-08 Configurations and variants
`source: AD-PRJ-03, SW-PRT-11` · `stage: S6`
- An assembly configuration selects a board variant (E-PRJ-03).
- DNP components are hidden in 3D and excluded from interference.
- Mechanical features can be suppressed or changed by the variant (for example, no connector cut-out if the connector is not fitted).
- **Accept:** switching the golden assembly to the "no USB" configuration removes the connector and suppresses the cut-out.

#### X-09 Cross-domain change propagation and review
`source: AD-MCAD-01` · `stage: S6`
- By default, linked changes propagate on regeneration.
- A **cross-domain changes** panel lists every link whose value changed since the user last acknowledged it (old value, new value, affected objects).
- Optional **review mode** per link or per project: driven quantities hold their previous values until the user accepts the change, in the style of an engineering change order.
- **Accept:** in review mode, widening the board leaves the enclosure unchanged until the change is accepted; accepting regenerates it.

#### X-10 Link manager
`source: AD-MCAD-01` · `stage: S6`
- A panel lists all links: driver object, driven object, quantity, direction, current value, status (OK, out of date, broken, locked, cycle), and where used.
- Actions: create, edit, re-target, lock, unlock, break, reverse direction (where both sides support it).
- Broken links never delete geometry; they convert to errors on the driven feature (P-21).
- **Accept:** UI replay creates, locks, breaks and re-targets links in the golden project; a link cycle is rejected at creation.

#### E-MCAD-02 3D export of boards and products
`source: AD-MCAD-02` · `stage: S6` (STEP) / `S7` (others)
- Export a board alone or a whole product assembly as STEP AP214/AP242 with colours, names and component structure; as IDF 3.0 (board and component outlines with heights) for exchange with other MCAD tools; and as STL/3MF for printing fit-check mock-ups (M-3DP-01).
- IDX incremental exchange is a *should* (Future if not done by S7).
- Native formats of other CAD products are excluded (CR-03).
- **Accept:** exported STEP re-imports (M-IO-01) with the same component count, positions within 1e-4 mm, and colours.

---

## 13. Import, export and 3D printing

#### M-IO-01 Neutral file formats
`source: SW-IO-01` · `stage: S2` (STEP) / `S7` (rest)

| Format | Import | Export | Notes |
|---|---|---|---|
| STEP AP203 / AP214 / AP242 | Yes | Yes | Assembly structure, names, colours; semantic PMI export is a *should* |
| IGES 5.3 | Yes | Yes | Surfaces and solids |
| STL (binary/ASCII) | Yes (as mesh body) | Yes | See M-3DP-01 |
| 3MF | Should | Yes | See M-3DP-01 |
| OBJ | Yes (as mesh body) | Yes | |
| glTF 2.0 / GLB | No | Yes | Appearances mapped to PBR materials |
| VRML 2.0 | No | Yes | |
| DXF | Yes (into sketches and drawings) | Yes (drawings, flat patterns, sketches, PCB layers) | |
| PDF | No | Yes (drawings) | Vector output |
| SVG / PNG | No | Yes (drawings, views) | |

- Imported B-rep bodies are checked and healed (sewing, small-edge removal) with a report; they can be edited by direct editing (M-PRT-07).
- Imports run in the background (A-10), with a queue for multiple files.
- Out of scope pending Q-04: Parasolid, ACIS, DWG and 3D PDF (proprietary formats or commercial SDK needed).
- **Accept:** T-07 round-trip tests for each export with an import counterpart; 20 golden third-party STEP files import as valid solids.

#### M-3DP-01 Export for 3D printing
`source: user requirement ("Export parts and assemblies for 3D printing")` · `stage: S2` (basic) / `S7` (full)
- Export a body, part, selected components or a whole assembly as **STL** (binary or ASCII), **3MF** (units, multiple objects, per-object colour/material, build items) and **OBJ**, either as one file or one file per body/component, with a filename template.
- Tessellation controls: chordal deviation, angular deviation, maximum edge length, with a live preview and triangle count.
- **Mesh validation** before writing: watertight, manifold, consistent outward normals, no self-intersections, no degenerate triangles. Problems are reported per body with an option to export anyway.
- Choice of output coordinate system, units and scale; option to place each object on the build plane (lowest Z = 0).
- Batch "export all printable parts" for an assembly (excluding purchased parts, fasteners and boards unless selected).
- PCB fit-check mock-up export (board plus simplified component bodies) for test printing.
- **Accept:** golden parts export watertight; mesh volume within 0.1 % of B-rep volume at default settings; 3MF files validate with lib3mf.

---

## 14. Future scope (not built now; seams required now)

These are planned. The architecture must not block them; where a seam is noted, it is a requirement in the current stages.

| Source ID | Capability | Seam required now |
|---|---|---|
| SW-SIM-01, SW-SIM-02, SW-SIM-03 | Structural FEA: linear static, frequency, buckling, thermal, fatigue, nonlinear, dynamics | Material model carries mechanical and thermal properties (M-PRT-09). `sim/` exposes a generic mesh and result-field interface with a field-plot viewer. Candidate solvers must meet the licence policy (4.3); GPL solvers such as CalculiX are acceptable if their licence version is GPL-3.0-compatible. |
| SW-RTE-01 | Pipe and tube routing | 3D sketches (M-SKT-03) and connection points on parts. |
| SW-RTE-02 + AD-HAR-01 | Electrical harness routing in 3D, and harness design (wiring diagrams, layout drawings, connection tables) | Plan as **one** cable/harness module linking E-MB-01 connectivity to 3D routes. Connectors carry pin data in the library (E-LIB-01). |
| SW-CAM-01 | CAM toolpaths (2.5-axis milling, turning) | Hole and feature metadata kept semantic (M-PRT-02) for feature recognition. |
| SW-PDM-02 | Workflow and lifecycle for documents | Lifecycle states already exist for library items (E-LIB-02); reuse the same state machine for documents. Projects work in Git. |
| SW-API-01 + AD-API-01 | Automation API (to be implemented differently from either source product) | Command bus (A-03) with stable, documented command names and JSON parameters from S0. Candidate front ends: an embedded scripting language, a C# plug-in host, or local JSON-RPC over a named pipe (to be decided). |
| AD-PCB-06 | Guided routing, BGA fanout, autorouter | Router core exposes path-finding and shove as services, not as UI-only code. |
| AD-PCB-09 | Rigid-flex (multiple stacks per board, bend lines, 3D folding) | Board database allows **board regions referencing stack definitions** from S5, even if only one region exists. |
| AD-PCB-10 | Wire bonding / chip-on-board; printed electronics | Layer model allows non-copper conductive and dielectric print layers. |
| AD-SI-01 | Signal integrity (reflection, crosstalk, IBIS models) | Impedance profile and stack data model (E-PCB-01); routed-geometry extraction API. |
| AD-PI-01 | Power integrity (DC IR drop, current density) | Copper geometry exportable per net as polygons with layer and thickness. |
| AD-OUT-03 | CAM data viewer/editor for Gerber/Excellon/IPC-2581 | The Gerber/Excellon readers written for T-07 become the basis of the viewer. |
| (new) | Online supplier data, pricing and stock | Optional opt-in module only; must keep P-03 by default. |

---

## 15. Traceability: requested capabilities → requirements

| Requested source ID | Requirement(s) | Stage |
|---|---|---|
| SW-SKT-01 | M-SKT-01 | S1 |
| SW-SKT-02 | M-SKT-02 | S1 |
| SW-SKT-03 | M-SKT-03 | S1 / S8 |
| SW-SKT-04 | M-SKT-04 | S1 |
| SW-PRT-01 | M-PRT-01, M-PRT-10 | S2 |
| SW-PRT-02 | M-PRT-02 | S2 |
| SW-PRT-03 | M-PRT-03 | S2 |
| SW-PRT-04 | M-PRT-04 | S2 |
| SW-PRT-05 | M-PRT-05 | S8 |
| SW-PRT-06 | M-PRT-06 | S8 |
| SW-PRT-07 | M-PRT-07 | S8 |
| SW-PRT-09 | M-PRT-09 | S2 |
| SW-PRT-11 | M-PRT-11, X-08 | S3 / S6 |
| SW-SHM-01 | M-SHM-01 | S8 |
| SW-SHM-02 | M-SHM-02 | S8 |
| SW-WLD-01 | M-WLD-01 | S8 |
| SW-WLD-02 | M-WLD-02 | S8 |
| SW-ASM-01 | M-ASM-01 | S3 |
| SW-ASM-02 | M-ASM-02 | S3 |
| SW-ASM-03 | M-ASM-03 | S10 |
| SW-ASM-04 | M-ASM-04, X-06 | S3 / S6 |
| SW-ASM-05 | M-ASM-05 | S10 |
| SW-ASM-06 | M-ASM-06 | S3 |
| SW-ASM-07 | M-ASM-07 | S3 |
| SW-DRW-01 | M-DRW-01 | S7 |
| SW-DRW-02 | M-DRW-02 | S7 |
| SW-DRW-03 | M-DRW-03, X-07 | S7 |
| SW-SIM-04 | M-SIM-04 | S9 |
| SW-SIM-05 | M-SIM-05 | S9 |
| Export for 3D printing | M-3DP-01 | S2 / S7 |
| SW-INS-01 | M-INS-01 | S10 |
| SW-VIS-01 | M-VIS-01 | S10 |
| SW-IO-01 | M-IO-01 | S2 / S7 |
| AD-PRJ-01 | E-PRJ-01 | S4 |
| AD-PRJ-03 | E-PRJ-03, X-08 | S6 |
| AD-PRJ-04 | E-PRJ-04 | S4 / S5 |
| AD-LIB-01 | E-LIB-01 | S4 |
| AD-LIB-02 | E-LIB-02 | S4 |
| AD-LIB-03 | E-LIB-03 | S4 |
| AD-SCH-01 | E-SCH-01 | S4 |
| AD-SCH-02 | E-SCH-02 | S4 |
| AD-SCH-03 | E-SCH-03 | S4 |
| AD-SCH-04 | E-SCH-04 | S4 |
| AD-SCH-05 | E-SCH-05 | S4 |
| AD-SIM-01 | E-SIM-01 | S9 |
| AD-MB-01 | E-MB-01 | S6 |
| AD-PCB-01 | E-PCB-01 | S5 |
| AD-PCB-02 | E-PCB-02 | S5 |
| AD-PCB-03 | E-PCB-03 | S5 |
| AD-PCB-04 | E-PCB-04 | S5 |
| AD-PCB-05 | E-PCB-05 | S5 |
| AD-PCB-07 | E-PCB-07 | S5 |
| AD-PCB-08 | E-PCB-08 | S5 |
| AD-RUL-01 | E-RUL-01 | S5 |
| AD-RUL-02 (listed as "AD-RUL-002") | E-RUL-02 | S5 |
| AD-3D-01 | E-3D-01 | S5 |
| AD-MCAD-01 | X-01 … X-10 | S6 |
| AD-MCAD-02 | E-MCAD-02 | S6 / S7 |
| AD-DOC-01 | E-DOC-01 | S7 |
| AD-OUT-01 | E-OUT-01 | S7 |
| AD-BOM-01 (basic) | E-BOM-01, X-07 | S7 |

Notes: AD-PCB-04, AD-PCB-05 and AD-PCB-07 appeared twice in the request and are covered once. SW-PRT-08 (feature recognition) and SW-PRT-10 (point-cloud import) were not requested; direct editing (M-PRT-07) covers editing imported geometry.

---

## 16. Working agreement for the AI implementer

1. **Work stage by stage** (section 9). Before starting a stage, write `docs/stages/Sn.md` with the task breakdown, dependencies and risks.
2. **Requirement by requirement.** For each requirement: update the schema and format docs if data changes → implement → write tests that encode every `Accept:` item → update `docs/traceability.csv` (columns: `id, status, tests, notes`; status ∈ `not-started | in-progress | done | deferred`).
3. **No fake completion.** Never mark a requirement done with stubbed, skipped or placeholder tests, or with code that returns hard-coded results. Partial work is `in-progress` with notes.
4. **Command first.** Implement every capability as a command (A-03) usable from the CLI before building its UI.
5. **Reproducible test data.** Build golden models from command scripts stored in `tests/data/scripts/`, not opaque binary files. Commit the generated golden outputs alongside the scripts that made them.
6. **Decisions.** Where this spec is ambiguous, choose the simplest option consistent with it, record an ADR in `docs/adr/NNNN-title.md`, and add the question to section 17 of this spec.
7. **Format discipline.** Every schema change bumps `formatVersion` appropriately, adds a migration, and adds a round-trip test (T-03).
8. **Keep it green.** The main branch always builds in Visual Studio from a clean clone and passes `scripts/test.ps1`. Commits are small and reference requirement IDs.
9. **Clean room and provenance.** Section 2 applies to every line of code, asset and data file. Write fresh code by default. Reuse GPL-compatible open-source code only under CR-04, and log it under CR-07 in the same commit. If the origin of something is uncertain, do not use it.
10. **Performance.** Add or update a benchmark (T-08) when touching a path covered by a P-1x target.
11. **Libraries.** Seed data goes in `/libraries` (the global seed), never hard-coded in source. Every seed table records its source in `docs/data-sources.md` (CR-05).

---

## 17. Open questions

| ID | Question | Impact |
|---|---|---|
| Q-01 | ~~Product licence~~ **Resolved v0.2:** GPL. Assumed GPL-3.0-or-later; confirm. (GPL-2.0-only would exclude Apache-2.0 and LGPL-3.0 dependencies, including Qt 6 under LGPL.) | 4.3, P-30. |
| Q-02 | Final product name and file extensions | Replaces the `APP`, `.apj` and `.apb` placeholders. The magic UUID (F-09) does not change with the name. |
| Q-03 | Is ODB++ output wanted, and are the specification's licence terms acceptable? | E-OUT-01 scope. |
| Q-04 | Should Parasolid, ACIS, DWG or 3D PDF be supported through commercial SDKs? | M-IO-01 scope; conflicts with "fully open" goals. |
| Q-05 | When should Linux and macOS builds become release targets, and which build system replaces the Visual Studio solution then? | P-04, B-07. |
| Q-06 | ~~Catch2 or GoogleTest?~~ **Resolved v0.2:** GoogleTest. | D-11. |
| Q-07 | ~~Qt licence?~~ **Resolved v0.2:** Qt under LGPL-3.0/GPL-3.0, which fits a GPL-3.0-or-later product. | 4.3. |
| Q-08 | Should MP4 export bundle an FFmpeg build (LGPL or GPL build both fit now)? | M-ASM-07, M-VIS-01. |
| Q-09 | Should the global library folder be writable by all users of the machine, or only by administrators (with users editing via overrides)? | F-10. |
