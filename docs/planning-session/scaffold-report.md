# Scaffold report (planning session, phase 2a)

The repository scaffold from `CURSOR_PLAN_PROMPT.md` section 4.2 is in place and green on this machine. Debug,
Headless and Release all build from scratch with zero warnings. Every test passes in all three configurations. The
three static checks pass on the scaffold and each one fails on deliberately broken input. The `ui` executable shows an
empty window titled "APP". `Cli.exe --version` prints `APP 0.0.0` and exits 0. The one item that could not be verified
is `scripts/docs.ps1`: Doxygen is not installed, and installing it was out of scope.

No git commits or pushes were made. The planning documents owned by the other worker (`docs/IMPLEMENTATION_CHECKLIST.md`,
`docs/stages/`, `docs/traceability.csv`, `docs/RISKS.md`, `docs/cpp-features.md`, `docs/provenance.md`,
`docs/data-sources.md`, `docs/adr/`) were not touched.

## 1. Acceptance items

| Item (section 4.2) | Result |
|---|---|
| `scripts/build.ps1 -Configuration Debug` with zero warnings | Pass (clean rebuild, 12 s, empty warnings log) |
| `scripts/build.ps1 -Configuration Headless` with zero warnings | Pass (clean rebuild, 6 s, empty warnings log) |
| `scripts/test.ps1` runs every test project, all tests pass | Pass. Headless runs 17 test executables (Ui.Tests is not part of Headless); Debug and Release run all 18 |
| `check-layering.ps1` passes, and fails on bad input | Pass / fails on 3 bad inputs (section 4) |
| `check-spdx.ps1` passes, and fails on bad input | Pass / fails on 3 bad inputs (section 4) |
| `check-network-apis.ps1` passes, and fails on bad input | Pass / fails on 3 bad inputs (section 4) |
| `ui` shows an empty main window titled "APP" | Pass (Debug and Release; window title read back as `APP`; screenshot `out/logs/ui-main-window.png`) |
| `cli` prints its version and exits 0 | Pass (`APP 0.0.0`, exit 0, in Debug, Release and Headless) |
| `scripts/docs.ps1` generates HTML with no Doxygen warnings | **Not verified**: Doxygen is not installed (section 5) |
| Project references follow the layering; no module references `ui` | Pass (`check-layering.ps1`: 36 projects, 62 C++ files) |

Extra, beyond the acceptance list: a Release build and a Release test run, a guard that rejects UI projects in
Headless, and `-Coverage` / `-UpdateGolden` switches on `test.ps1` (from `docs/notepads/Testing_Strategy.md`).

## 2. Files created

This phase wrote 200 files under `C:\dev\CADence`. `README.md` already existed as a one-line stub and was replaced;
all the others are new. Paths are relative to the repository root.

**Root (10):** `APP.slnx`, `Directory.Build.props`, `Directory.Build.targets`, `vcpkg.json`,
`vcpkg-configuration.json`, `LICENSE`, `README.md` (replaced), `THIRD_PARTY_NOTICES.md`, `.gitignore`,
`.gitattributes`. `.clang-format`, `.editorconfig` and `Doxyfile` were provided and are unchanged.

**Build properties (6):** `build/props/Common.props`, `Cpp.props`, `Vcpkg.props`, `Qt.props`, `Headless.props`,
`Tests.props`.

**Modules (95):** one folder per module in spec 5.1. Each has `<Project>.vcxproj`, `<Project>.vcxproj.filters`,
`Module.h`, `Module.cpp` and `README.md`:

| Folder | Project | Namespace | Folder | Project | Namespace |
|---|---|---|---|---|---|
| `src/core` | Core | `app::core` | `src/ecad/sch` | EcadSch | `app::ecad::sch` |
| `src/platform` | Platform | `app::platform` | `src/ecad/pcb` | EcadPcb | `app::ecad::pcb` |
| `src/doc` | Doc | `app::doc` | `src/ecad/out` | EcadOut | `app::ecad::out` |
| `src/geom` | Geom | `app::geom` | `src/link` | Link | `app::link` |
| `src/sketch` | Sketch | `app::sketch` | `src/sim` | Sim | `app::sim` |
| `src/part` | Part | `app::part` | `src/io` | Io | `app::io` |
| `src/asm` | Asm | `app::assembly` | `src/render` | Render | `app::render` |
| `src/drw` | Drw | `app::drw` | `src/cli` | Cli (exe) | `app::cli` |
| `src/ecad/lib` | EcadLib | `app::ecad::lib` | `src/ui` | Ui (exe) | `app::ui` |

There are also some extra source files:

- `src/cli/CliApp.h`, `CliApp.cpp` and `main.cpp`: the argument handling, kept testable.
- `src/ui/MainWindow.h`, `MainWindow.cpp` and `main.cpp`.

**Tests (57):** `tests/<module>/<Project>.Tests.vcxproj`, `.vcxproj.filters` and `ModuleTests.cpp` for all 18
modules, plus `tests/cli/CliAppTests.cpp` and `tests/ui/MainWindowTests.cpp`. The folder `tests/data/scripts/` holds
only `README.md`.

**Libraries (19):** `libraries/README.md`, plus `README.md` in each kind folder:

- `components`, `symbols`, `footprints`, `models-3d`
- `reuse-blocks`, `snippets`
- `materials`, `appearances`
- `profiles`, `gauges`, `hole-tables`, `fasteners`
- `footprint-params`, `drafting-standards`, `rule-sets`, `templates`
- `layer-stacks`, `environments`

**Scripts (8):** `scripts/build.ps1`, `test.ps1`, `docs.ps1`, `check-layering.ps1`, `check-spdx.ps1`,
`check-network-apis.ps1`, `check-selftest.ps1`, `common.ps1`.

**Docs (4):** `docs/mainpage.md`, `docs/images/.gitkeep`, `docs/format/README.md`, `docs/commands/README.md`.

**This report:** `docs/planning-session/scaffold-report.md`.

Generated, not source (ignored by `.gitignore`):

- `out/`: binaries, logs, test results.
- `vcpkg_installed/full/` (gtest plus Qt, 11 packages) and `vcpkg_installed/headless/` (gtest only).

## 3. Commands run and results

All commands ran in Windows PowerShell 5.1 from `C:\dev\CADence`, as
`powershell -NoProfile -ExecutionPolicy Bypass -File <script> <arguments>`. The toolchain was Visual Studio 2026
Community 18.10 (toolset v145, MSVC 19.51), MSBuild 18 (amd64) and the vcpkg bundled with Visual Studio. Logs are
in `out/logs/`.

| Command | Result |
|---|---|
| `scripts\build.ps1 -Configuration Headless -Rebuild` | `build: OK (Headless, 0 warnings)`, exit 0, 6 s |
| `scripts\build.ps1 -Configuration Debug -Rebuild` | `build: OK (Debug, 0 warnings)`, exit 0, 12 s |
| `scripts\build.ps1 -Configuration Release -Rebuild` | `build: OK (Release, 0 warnings)`, exit 0, 12 s |
| First Debug build (vcpkg manifest install of qtbase 6.11.3 and its dependencies, `--x-feature=ui`) | 11 packages built from source in about 7.5 minutes; `out/logs/vcpkg-install-full.log` |
| `scripts\test.ps1` (Headless) | 17 executables, 18 tests pass; CLI smoke `APP 0.0.0` (exit 0); checks OK; `test: OK`, exit 0 |
| `scripts\test.ps1 -Configuration Debug` | 18 executables (adds Ui.Tests, 3 tests); all pass; `test: OK`, exit 0 |
| `scripts\test.ps1 -Configuration Release -NoBuild` | 18 executables; all pass; `test: OK`, exit 0 |
| `out\<cfg>\Cli\Cli.exe --version` for Headless, Debug and Release | `APP 0.0.0`, exit 0 |
| `out\Debug\Cli\Cli.exe --bogus` | Prints an error and `Usage: Cli [--version | --help]`, exit 2 |
| Start `out\Debug\Ui\Ui.exe` and `out\Release\Ui\Ui.exe`, read `MainWindowTitle`, then `CloseMainWindow()` | Title `APP`, exit code 0, for both |
| `scripts\check-layering.ps1` | `check-layering: OK (36 projects, 62 C++ files)`, exit 0 |
| `scripts\check-spdx.ps1` | `check-spdx: OK (114 files)`, exit 0 |
| `scripts\check-network-apis.ps1 -BinaryDir out\Debug` (inside `test.ps1`) | `info: Qt6Cored.dll (third-party) imports ws2_32.dll`; `OK (42 source files, 44 build files, vcpkg.json, 20 executables; allow-list: 0 entries)`, exit 0 |
| `scripts\check-network-apis.ps1 -BinaryDir out\Release` (inside `test.ps1`) | Same, with `Qt6Core.dll`; exit 0 |
| `scripts\check-selftest.ps1` | `check-selftest: OK (3 checks pass on the clean copy; 8 deliberate violations detected)`, exit 0 (section 4) |
| `scripts\docs.ps1` | `docs: doxygen.exe not found. Install Doxygen (...) or pass -Doxygen <path>.`, exit 1 (expected here) |
| `scripts\test.ps1 -NoBuild -NoChecks -UpdateGolden` | `test: -UpdateGolden needs -Reason "<why the golden outputs change>".`, exit 1 |
| `scripts\test.ps1 -NoBuild -NoChecks -Coverage -Filter "Core*:CliApp*"` | Tests pass; one Cobertura XML per executable in `out/test-results/Headless/`; exit 0. The coverage console passes the exit code of the program it runs through (checked with a program that exits 3) |
| `MSBuild.exe src\ui\Ui.vcxproj /p:Configuration=Headless /p:Platform=x64` | `Headless.props(17,5): error : Ui uses Qt and is not part of the Headless configuration (B-04, P-04).`, exit 1 (guard works) |
| `clang-format.exe -i --style=file` (clang-format 22.1.3, bundled with Visual Studio) on all 62 C++ files | 8 files reflowed (line wrapping and one include-group blank line); all builds rerun green afterwards |

## 4. Failing-input demos

`scripts/check-selftest.ps1` copies `src`, `tests`, `build`, `scripts`, `docs`, `libraries` and the root files into a
fresh `%TEMP%\app-check-selftest-*` folder. It runs the three checks on the clean copy, then applies one violation
per case to a new copy and expects the check to fail. Output from the final run:

```
[clean] check-layering -> exit 0
    check-layering: OK (36 projects, 62 C++ files)
[clean] check-spdx -> exit 0
    check-spdx: OK (114 files)
[clean] check-network-apis -> exit 0
    check-network-apis: OK (42 source files, 44 build files, vcpkg.json; allow-list: 0 entries)
[layering-include] core includes a ui header: check-layering -> exit 1
    check-layering: FAILED with 1 violation(s):
      src/core/Module.cpp:19: module 'core' must not include 'ui/MainWindow.h' from module 'ui'
[layering-project-reference] geom references the ui project: check-layering -> exit 1
    check-layering: FAILED with 1 violation(s):
      src/geom/Geom.vcxproj: 'geom' must not reference module 'ui' (allowed: core, doc)
[layering-third-party] part includes an OCCT header and doc includes a Qt Widgets header: check-layering -> exit 1
    check-layering: FAILED with 2 violation(s):
      src/doc/Module.cpp:19: Qt Widgets header 'QWidget' is allowed only in: ui
      src/part/Module.cpp:19: OCCT header 'gp_Pnt.hxx' is allowed only in: geom
[spdx-missing-header] a new C++ file without an SPDX header: check-spdx -> exit 1
    check-spdx: FAILED with 1 violation(s):
      src/core/NoHeader.cpp: missing 'SPDX-License-Identifier:' in the first 10 lines
[spdx-third-party-without-provenance] an MIT-licensed adapted file with no docs/provenance.md entry: check-spdx -> exit 1
    check-spdx: FAILED with 1 violation(s):
      src/core/Adapted.cpp: third-party file (licence 'MIT') but docs/provenance.md does not exist (CR-07)
[network-source] io includes winsock2.h and calls WSAStartup: check-network-apis -> exit 1
    check-network-apis: FAILED with 2 violation(s):
      src/io/Net.cpp:3: network header 'winsock2.h'
      src/io/Net.cpp:4: network API call 'WSAStartup'
[network-link-input] Cli.vcxproj links ws2_32.lib: check-network-apis -> exit 1
    check-network-apis: FAILED with 1 violation(s):
      src/cli/Cli.vcxproj:33: link input 'ws2_32.lib' is a network library
[network-vcpkg] vcpkg.json adds curl and the qtbase network feature: check-network-apis -> exit 1
    check-network-apis: FAILED with 2 violation(s):
      vcpkg.json: dependency 'curl' is a networking library
      vcpkg.json: qtbase feature 'network' enables networking
check-selftest: OK (3 checks pass on the clean copy; 8 deliberate violations detected)
```

`docs/provenance.md` did not exist yet during this run because the other worker owns it. A manual demo in a
temporary copy covered the other provenance case:

1. A `docs/provenance.md` without an entry for the file gives:
   `src/core/Adapted.cpp: third-party file (licence 'MIT') has no entry for 'src/core/Adapted.cpp' in docs/provenance.md (CR-07)`
   and exit 1.
2. After adding a table row for `src/core/Adapted.cpp`, the result is `check-spdx: OK (115 files)` and exit 0.

## 5. Not verified, and why

- **`scripts/docs.ps1` and Doxygen output (HTML and `-Pdf`).** Doxygen is not installed, and installing system tools
  was out of scope. The script was run and fails cleanly with the "doxygen.exe not found" message above.
  - **What was checked by hand.** All C++ declarations carry Doxygen comments, including both `main()` functions.
    Each module `README.md` has a `{#module-...}` anchor that `docs/mainpage.md` refers to. The Markdown pages keep
    angle-bracket placeholders inside code spans, so Doxygen will not read them as HTML tags.
  - **Known risks when it first runs:**
    1. `Doxyfile` lists `docs/adr` in `INPUT`. Until the other worker creates that folder, Doxygen warns that the
       input does not exist.
    2. `Q_OBJECT` in `src/ui/MainWindow.h` is not in `PREDEFINED`. If Doxygen reports it as an undocumented member,
       add `Q_OBJECT=` to `PREDEFINED`. I did not edit `Doxyfile`, because it is a provided file.
  - **Commands to run once Doxygen is installed:**
    `winget install DimitriVanHeesch.Doxygen`, then
    `powershell -NoProfile -ExecutionPolicy Bypass -File scripts\docs.ps1`; for the PDF (needs MiKTeX or TeX Live),
    `scripts\docs.ps1 -Pdf`.
- **PowerShell 7.** The scripts were run only in Windows PowerShell 5.1, which is the only version installed. They
  avoid 7-only syntax, so they should also run in 7, but this was not tested.
- **GCC and Clang portability (spec 4.5).** Only MSVC was used. No other compiler is installed, and the scaffold code
  uses only `std::format`, `std::span` and `std::string_view`.
- **A clean machine.** Everything ran on this machine. The first build on a new machine also downloads and builds the
  vcpkg ports (about 7.5 minutes for Qt here), and that needs network access for the build only.

## 6. Deviations and decisions

1. **The `asm` namespace is `app::assembly`, because `asm` is a C++ keyword.** The folder stays `asm` and the project
   is `Asm`.
2. **I added `Directory.Build.targets`**, which section 4.2 does not list. It imports vcpkg's MSBuild `vcpkg.targets`
   after the project, which MSBuild integration requires. It also fails the build with a clear message when the
   vcpkg root is missing.
3. **The vcpkg root is chosen explicitly.** The order is: the `AppVcpkgRoot` MSBuild property, then `VCPKG_ROOT`, then
   the vcpkg bundled with Visual Studio (`<VS>\VC\vcpkg\`). `VcpkgLocalAppDataDisabled=true` ignores the old
   user-wide integration that points at `C:\vcpkg`. The baseline `177e8175121536d414f405ed8b3099b325a42638` is a
   real commit on `microsoft/vcpkg` (2026-11-12, "[libressl] Update to 4.4.1 (#55013)"), confirmed through the
   GitHub API.
4. **Qt is a vcpkg manifest feature (`ui`), not a top-level dependency, and there are two install trees.**
   - Debug and Release install with `--x-feature=ui` into `vcpkg_installed/full/`.
   - Headless installs only gtest, into `vcpkg_installed/headless/`, so Headless never builds or links Qt (B-04).
   - `qtbase` uses `default-features: false` with only `gui` and `widgets`, so Qt Network is not built (P-03).
5. **`VcpkgAutoLink=false`.** Each props file names its libraries explicitly: Qt in `Qt.props`, gtest in
   `Tests.props`. This keeps third-party linking confined to the modules allowed to use it.
6. **gtest is a DLL** (`x64-windows` triplet, `GTEST_LINKED_AS_SHARED_LIBRARY=1`). `gtest_main` comes from
   `lib/manual-link`.
7. **`Ui.Tests` and `Cli.Tests` compile the executable's own source files.** `ui` and `cli` are executables, so tests
   cannot reference them as libraries. `Ui.Tests` compiles `src/ui/Module.cpp` and `MainWindow.cpp` and runs moc on
   `MainWindow.h`; `Cli.Tests` compiles `src/cli/Module.cpp` and `CliApp.cpp`. `check-layering.ps1` lets a test
   project compile files from its own module and from the modules that module may use, directly or indirectly. A
   project under `src/` may compile files only from its own folder.
8. **`Ui.Tests` uses Qt's `minimal` platform plugin.** The vcpkg Qt build has no `offscreen` plugin; it ships only
   `qwindows`, `qminimal` and `qdirect2d`. The build copies the `platforms` and `styles` plugins next to `Ui.exe`
   and `Ui.Tests.exe`.
9. **`test.ps1` defaults to Headless, as section 4.2 says, so it runs 17 of the 18 test projects.** `Ui.Tests` runs
   with `-Configuration Debug` or `Release`. The script also runs a CLI smoke test and the three checks, and writes
   JUnit XML for both.
10. **I added three scripts beyond the list.** `scripts/check-selftest.ps1` produces the failing-input demos.
    `scripts/common.ps1` holds shared helpers: finding Visual Studio, MSBuild and dumpbin, walking the source tree,
    and writing JUnit XML.
11. **`test.ps1` has extra switches from the testing notepad:** `-Coverage`, `-UpdateGolden -Reason`, `-Bench` and
    `-Fuzz`. `-Bench` and `-Fuzz` fail with "no executables found" until such projects exist.
12. **`.gitignore` and `.gitattributes` were added.** `docs/FOLDER_STRUCTURE.md` marks them as created now.
    `.gitattributes` uses LF by default and CRLF for Visual Studio and PowerShell files, to match `.editorconfig`.
    Golden and project binaries are marked `binary`.
13. **The global seed kind list includes `layer-stacks` and `environments`.** These are proposed kinds (planning
    conflict C-19 in `phase1-understanding.md`), and their READMEs say so.
14. **The version lives in one place.** `build/props/Cpp.props` sets `AppVersionMajor`, `AppVersionMinor` and
    `AppVersionPatch` (0.0.0) and passes them as `APP_VERSION_*` defines. `core/Module.h` turns them into
    `app::core::kVersion` and fails to compile if they are missing.
15. **`core` has no dependency on any other module.** It defines the shared `ModuleInfo` type and the product
    identity, `kProductName = "APP"`. Every other module's `moduleInfo()` returns that type.
16. **The network check reports third-party DLL imports without failing.** Qt's own `Qt6Core(d).dll` imports
    `ws2_32.dll`, even without Qt Network. `check-network-apis.ps1` fails on network imports in APP's own executables
    and on Qt Network DLLs, and reports other third-party imports as `info`. This affects how P-03 is worded and
    tested; Phase 1 already flagged it, and the risk register should cover it.
17. **The scripts are ASCII-only with CRLF line endings.** Windows PowerShell 5.1 reads UTF-8 files without a BOM as
    ANSI, so any non-ASCII character would break them.
18. **The scaffold was produced by generator scripts kept outside the repository**, in the agent's scratch store:
    `gen-scaffold.ps1` for the module and test projects and `gen-libraries.ps1` for the library READMEs. Some generated
    files were edited by hand afterwards, so rerunning a generator would overwrite those edits.

## 7. Inputs for the planning documents (other worker)

- **`docs/provenance.md` must exist with a table header.** `check-spdx.ps1` reads it as soon as any third-party file
  appears.
- **Every new third-party library needs updates in three places:**
  1. a row in `THIRD_PARTY_NOTICES.md`;
  2. an entry in `scripts/check-layering.ps1` (the `$ThirdParty` table) naming the modules allowed to include it;
  3. if it is a networking library, a review against `scripts/check-network-apis.ps1`.
- **P-03 risk:** Qt Core links Windows sockets (`ws2_32.dll`), as noted in deviation 16.
- **Build-time network access:** building the vcpkg ports needs network access the first time. The product itself
  stays offline.
