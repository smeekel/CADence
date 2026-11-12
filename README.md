# APP

APP (working name) is a clean-room, offline, Windows-first desktop CAD application. It combines parametric 3D
mechanical design (sketches, parts, assemblies, drawings, sheet metal, weldments, motion) with electronics design
(libraries, schematic, SPICE, PCB layout, manufacturing outputs) in **one shared parametric model**, so a circuit
board and its enclosure are designed together and stay fitted when either changes.

- Specification: `docs/spec/cad-functional-requirements.md` (authoritative)
- Implementation plan: `docs/IMPLEMENTATION_CHECKLIST.md`, stage plans in `docs/stages/`
- Repository layout: `docs/FOLDER_STRUCTURE.md`
- Licence: GPL-3.0-or-later (`LICENSE`); third-party libraries in `THIRD_PARTY_NOTICES.md`

The repository currently holds the **scaffold**: every module is an empty, documented project; the `ui` executable
shows an empty window titled "APP" and the `cli` executable prints its version.

## Requirements

- Windows 11 x64
- Visual Studio 2026 (version 18) with the **Desktop development with C++** workload (MSVC v145 toolset, Windows 11
  SDK) and the **vcpkg package manager** component (installed with the workload by default)
- Git
- Internet access for the first build only: vcpkg downloads and builds the dependencies, then caches them
- Optional: Doxygen (HTML documentation); a LaTeX distribution such as MiKTeX (PDF manual)

No CMake, no separate Qt installer and no other tools are needed. Dependencies come from vcpkg in manifest mode
(`vcpkg.json`, baseline pinned in `vcpkg-configuration.json`). The build uses the vcpkg that ships with Visual
Studio unless the `VCPKG_ROOT` environment variable points at another vcpkg checkout; a per-user
`vcpkg integrate install` is ignored.

## Build

In Visual Studio: open `APP.slnx`, pick `Debug`, `Release` or `Headless` and `x64`, then build. Set `Ui` (desktop
application) or `Cli` (headless runner) as the startup project.

From a terminal (Windows PowerShell 5.1 or PowerShell 7):

```powershell
powershell -ExecutionPolicy Bypass -File scripts\build.ps1 -Configuration Debug
powershell -ExecutionPolicy Bypass -File scripts\build.ps1 -Configuration Headless
```

`scripts\build.ps1` runs `msbuild APP.slnx /m /p:Configuration=<Configuration> /p:Platform=x64` with the MSBuild of
Visual Studio 2026 and fails on any warning. Output goes to `out\<Configuration>\<Project>\`.

| Configuration | Contents |
|---|---|
| `Debug` | Everything, debug runtime, Qt from the `ui` manifest feature |
| `Release` | Everything, optimised |
| `Headless` | Optimised; every module, `Cli` and the tests, without Qt or any UI project (B-04, P-04) |

The first `Debug` or `Release` build compiles Qt from source through vcpkg (about 10 minutes on a fast machine);
later builds reuse the vcpkg binary cache. vcpkg installs into `vcpkg_installed\full\` (Debug, Release) and
`vcpkg_installed\headless\` (Headless).

## Test

```powershell
powershell -ExecutionPolicy Bypass -File scripts\test.ps1                         # Headless
powershell -ExecutionPolicy Bypass -File scripts\test.ps1 -Configuration Debug    # also runs Ui.Tests
```

`scripts\test.ps1` builds the configuration, runs every `*.Tests.exe` (GoogleTest), the CLI smoke test and the
static checks, and writes JUnit XML to `out\test-results\<Configuration>\`. Use `-Filter "<gtest filter>"` to run
a subset and `-NoBuild` to skip the build. `-Coverage` collects code coverage (Cobertura XML) with the Visual
Studio code coverage console; `-Bench` and `-Fuzz` run benchmark and fuzz executables instead of the tests;
`-UpdateGolden -Reason "<why>"` lets golden-data tests rewrite their expected outputs.

Static checks, also usable on their own:

| Script | Checks |
|---|---|
| `scripts\check-layering.ps1` | Module dependencies in project references and `#include` paths (A-01) |
| `scripts\check-spdx.ps1` | SPDX headers and provenance of reused code (CR-07, P-30) |
| `scripts\check-network-apis.ps1` | No networking APIs in sources, link inputs, vcpkg features or binaries (P-03) |
| `scripts\check-licences.ps1` | Every vcpkg port and reused project has a spec 4.3 licence recorded in `docs\licence-manifest.json` or `docs\provenance.md`, and `THIRD_PARTY_NOTICES.md` is up to date (P-30) |
| `scripts\check-selftest.ps1` | Each check above fails on a deliberately broken copy of the repository |
| `scripts\analyze.ps1` | MSVC Code Analysis and clang-tidy on every project (T-11); `-SelfTest` proves a seeded null dereference is reported |

## Documentation

```powershell
powershell -ExecutionPolicy Bypass -File scripts\docs.ps1          # HTML: out\docs\html\index.html
powershell -ExecutionPolicy Bypass -File scripts\docs.ps1 -Pdf     # also out\docs\latex\refman.pdf (needs LaTeX)
```

Doxygen builds the manual offline from the `Doxyfile`: `docs\mainpage.md`, the Markdown pages in `docs\` and the
documented C++ headers in `src\`. The script fails on any Doxygen warning.

## Contributing

Read `.cursor/rules/` (code style, documentation, layering, build, testing, licensing) before changing code. Every
source file starts with an SPDX header; every data change is a command on the command bus; no network code; no
material from proprietary CAD products (spec section 2).
