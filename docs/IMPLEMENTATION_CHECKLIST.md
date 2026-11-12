# Implementation checklist {#implementation-checklist}

This is the ordered, step-by-step plan for building APP from the scaffold to the end of Stage 10. It covers every
requirement ID in `docs/spec/cad-functional-requirements.md` (v0.2, authoritative). Stage content follows spec
section 9 and `docs/notepads/Stage_Roadmap.md`; the resolved conflicts and assumptions are in `docs/adr/` (index in
ADR 0001) and `docs/planning-session/phase1-understanding.md`.

## How to use this checklist

- Work one step at a time with `/next-step`: take the first unchecked step whose dependencies are all checked.
- Step format and states follow `.cursor/rules/080-planning-checklist.mdc`: `[ ]` not started, `[~]` in progress
  (add a `Status:` line), `[x]` done. Tick a step only when every "Done when" item holds and the build, tests and
  checks are green.
- Step IDs (`S<stage>.<package>.<step>`) are permanent. Insert new steps with a letter suffix (`S2.03.4a`); strike
  removed steps (`~~S2.03.4~~`) with a reason. Never renumber.
- Each step is one focused change of about 0.5 to 2 days that ends with a green build. If a step turns out larger
  while working on it, split it with suffixes before continuing.
- "Depends on" lists only earlier steps. A step's dependencies on its own stage's earlier steps in the same work
  package are usually listed explicitly; every step also implicitly needs the previous stage gate.
- The UI for a capability always comes after its command and its headless test (spec section 16, item 4).

## Conventions used in the steps

- **Tests** name the test category (T-01 … T-14), the test project and the test or golden script, for example
  `T-04 Golden.Tests PartGolden.ExtrudeEndConditions` or `tests/data/scripts/extrude_end_conditions.cmd.json`.
  Golden scripts live in `tests/data/scripts/`, expected values in `tests/data/expected/<name>.expected.json`.
- A test that encodes a spec `Accept:` criterion carries a tag `(Accept: <ID><clause>)`, where the clause letter
  numbers the criteria inside that requirement's `Accept:` line in reading order (`a`, `b`, `c`). Single-criterion
  requirements use the bare ID. The same tag goes into the test source as `// Accept: <ID>` (rule 060).
- **Spikes** are time-boxed and end in an ADR that must be Accepted before dependent steps start. ADR numbers
  0034 … 0041 are reserved for spikes SP-1 … SP-8. Other new ADRs are written as `NNNN-<title>.md` and take the
  next free number when written.
- Layering extensions proposed in ADRs 0016 … 0021 are applied (rule 030 table, `scripts/check-layering.ps1` and the
  project reference updated together) in the step that first needs them, and only after the ADR is Accepted.
- Every stage ends with work package `Sx.99 Stage gate`.
- Test and check commands are the scaffold's scripts: `scripts/build.ps1`, `scripts/test.ps1` (Headless by default,
  which skips `Ui.Tests`; `-Configuration Debug` runs them; switches `-Filter`, `-NoBuild`, `-NoChecks`, `-Coverage`,
  `-Bench`, `-Fuzz`, and `-UpdateGolden -Reason "<why>"` for any golden or snapshot change), `scripts/check-*.ps1`,
  `scripts/check-selftest.ps1` (proves each check fails on a deliberate violation) and `scripts/docs.ps1`.
- Module `asm` is project `Asm` in `src/asm/` with namespace `app::assembly` (`asm` is a C++ keyword).

## Open decisions that affect this plan

| Topic | Default used in this plan | Where it matters | ADR |
|---|---|---|---|
| Continuous integration (Q2) | **Open.** Local `scripts/test.ps1` and `scripts/ci-local.ps1` now (`S0.15.1`); hosted CI is a later step (`S1.08.1`). The repository is https://github.com/smeekel/CADence | S0.15, S1.08, every stage gate | 0033 |
| Tool installs (Q3): PowerShell 7 | Scripts stay Windows PowerShell 5.1 compatible (ASCII-only, as the scaffold does). Doxygen 1.18.0 is installed and `scripts/docs.ps1` builds the HTML manual with no warnings; LaTeX (for `-Pdf`) is not installed | S0.14.1, S0.15.1, every stage gate | 0032 |
| MP4 export bundling FFmpeg (spec Q-08) | Seam only: optional dynamically loaded encoder, PNG sequences are the required output | S3.08.3, S10.04.4 | to be written in S3.08.3 |
| Global library folder permissions (spec Q-09) | Writable by default (spec F-10 table) | S0.07.2 | 0032 |

Decided by the owner on 2026-11-11:

| Topic | Decision | Where it matters | ADR |
|---|---|---|---|
| Reference scenario at S6 needs fabrication outputs (C-01) | Minimal Gerber X2, Excellon, IPC-2581, BOM and pick-and-place writers are built in S6 (`S6.09`); full E-OUT-01 options stay in S7 | S6.09, S6.11, S7.04, S7.05 | 0015 (Accepted) |
| Qt6Core imports `ws2_32.dll` even without Qt Network (C-12) | Third-party network imports are allowed: `check-network-apis.ps1` notes them and prints a summary (library, DLL, APIs), committed as `docs/network-imports.md`; network APIs in APP's own code in `src/` still fail | S0.02.3, S0.08.7, every stage gate | 0026 (Accepted) |

## Stage S0: Foundation

Stage plan: `docs/stages/S0.md`. Exit: clean-clone build, themed shell with a kernel box, box → JSON → BSON → STL
round-trip with F-09 magic checks, F-10 scopes resolve (empty seed), CLI runs a script headless, all tests run
locally (hosted CI open, ADR 0033).

### S0.01 Toolchain spike and build verification (SP-1)

- [x] **S0.01.1: Spike SP-1a: verify the scaffold toolchain from a clean clone (time box 1 day)**
  - Requirements: P-02, B-01, B-05, B-06, L-01
  - Modules: build, scripts
  - Depends on: none
  - Deliverables: clean-clone build log and first-build timings; DLL import lists of Qt6Core, OCCT TKernel and the product executables (`dumpbin /imports`), compared with `docs/network-imports.md` (ADR 0026); confirmed vcpkg baseline commit recorded in ADR 0029
  - Tests: T-11 `scripts/build.ps1 -Configuration Debug`, `-Configuration Release` and `-Configuration Headless` with zero warnings; T-13 `scripts/test.ps1` green
  - Done when: a fresh clone in a temporary folder builds all three configurations with only VS 2026 and Git installed; `vcpkg-configuration.json` baseline matches ADR 0029; every third-party network import in the lists appears in `docs/network-imports.md`
  - Notes: results in ADR 0029 (now Accepted). Scratch clones must not live under `%TEMP%` (MSB8029 warning fails the build). Release `Qt6Core.dll` → `WS2_32.dll` was missing from `docs/network-imports.md` and is now listed. OCCT `TKernel` import list moved to S0.09.1 (OCCT not yet a dependency).
- [~] **S0.01.2: Spike SP-1b: C++23/26 feature matrix on MSVC, GCC and Clang, ending in ADR 0034 (time box 1 day)**
  - Status: blocked. MSVC column verified with `tests/core/spikes/cpp_feature_probe.cpp` and `scripts/probe-cpp-features.ps1` (all rows match `docs/cpp-features.md`); ADR 0034 written as Proposed. GCC 14 and Clang 18 are not installed in WSL (Ubuntu 26.04.1 has no C++ compiler; installing `g++-14 clang-18 libc++-18-dev libc++abi-18-dev` needs the owner's approval), and the ADR needs owner acceptance.
  - Requirements: P-05, L-01, L-02, L-03, L-04, L-05
  - Modules: build, docs
  - Depends on: S0.01.1
  - Deliverables: `docs/adr/0034-spike-sp1-toolchain.md`; every row of `docs/cpp-features.md` verified for MSVC v145, GCC 14 and Clang 18 (WSL)
  - Tests: feature probe compiled with MSVC `/std:c++latest`, GCC and Clang `-std=c++2c`; result table in the ADR
  - Done when: each listed feature has a verified status on all three compilers; ADR 0034 Accepted by the owner
- [x] **S0.01.3: Add the Headless dependency check**
  - Requirements: P-04, B-04, A-01
  - Modules: scripts, build
  - Depends on: S0.01.1
  - Deliverables: `scripts/check-headless.ps1` (walks the Headless project graph and link inputs; fails on any `Ui` project, Qt6Widgets/Qt6Gui, or Windows UI headers and APIs outside `src/platform/`); called by `scripts/test.ps1`
  - Tests: T-11 check passes on the solution and fails on a fixture project that links Qt6Widgets (Accept: P-04)
  - Done when: check green on the repository, red on the fixture, and run by `scripts/test.ps1`
- [x] **S0.01.4: Add the build-hygiene check for migration-ready projects**
  - Requirements: B-01, B-02, B-03, B-07
  - Modules: scripts, build
  - Depends on: S0.01.1
  - Deliverables: `scripts/check-build-hygiene.ps1` (fails on `#pragma comment(lib`, warning or standard settings inside a `.vcxproj`, a source that needs the PCH to compile, any CMake file, a module folder without exactly one project)
  - Tests: T-11 check passes on the solution and fails on one fixture per rule
  - Done when: check green on the repository, red on every fixture, run by `scripts/test.ps1`
- [ ] **S0.01.5: Add the C++ feature allow-list check and the feature-test helper header**
  - Requirements: L-03, L-04, L-05
  - Modules: core, scripts
  - Depends on: S0.01.2
  - Deliverables: `scripts/check-cpp-features.ps1` (reads the "Not allowed" rows of `docs/cpp-features.md`; fails on `import`/`export module`, banned headers such as `<generator>`); `src/core/Compat.h` with `__cpp_*` / `__cpp_lib_*` helpers and fallbacks
  - Tests: T-11 check fails on a fixture using `<generator>` and on one using `import std;`; T-01 `Core.Tests Compat.*`
  - Done when: check and tests green; `Compat.h` documented with `@req L-04`
- [x] **S0.01.6: Pin dependency versions and record licence verification**
  - Requirements: B-05, P-30
  - Modules: build, docs
  - Depends on: S0.01.1
  - Deliverables: `vcpkg.json` with a version constraint per dependency and `qtbase` `default-features: false`; licence verification section in ADRs 0002 … 0014 (OCCT and ngspice already verified and accepted by the owner on 2026-11-11, recorded in `THIRD_PARTY_NOTICES.md` and ADRs 0005 and 0008)
  - Tests: T-11 `scripts/check-licences.ps1` dry run (manual until S0.02.2); clean-clone build repeated
  - Done when: every dependency has a pinned version and a verified licence entry, or a recorded open item

### S0.02 Static checks and clean-room guards

- [x] **S0.02.1: Tighten the SPDX check's provenance rule to the provenance table columns**
  - Requirements: CR-07, CR-04
  - Modules: scripts, docs
  - Depends on: S0.01.1
  - Deliverables: the scaffold's `scripts/check-spdx.ps1` already fails on a missing SPDX header and on a third-party file not mentioned anywhere in `docs/provenance.md`; this step parses the provenance table and requires the file path (or its third-party project folder) in the "Destination file(s)" column of a complete row (licence, URL, version, step ID filled in)
  - Tests: T-11 `scripts/check-selftest.ps1` cases: a source without an SPDX header fails (Accept: CR-07a); a third-party-marked file without a provenance row, and one named only in prose or in an incomplete row, fail (Accept: CR-07b); the repository passes
  - Done when: both failure modes detected, repository green, check documented in `scripts/README` header comment
- [x] **S0.02.2: Add the licence manifest check and regenerate THIRD_PARTY_NOTICES**
  - Requirements: P-30, CR-07
  - Modules: scripts, docs
  - Depends on: S0.02.1, S0.01.6
  - Deliverables: `scripts/check-licences.ps1` (every `vcpkg.json` dependency has an allowed licence per spec 4.3 and a `THIRD_PARTY_NOTICES.md` entry); `scripts/update-notices.ps1` (builds `THIRD_PARTY_NOTICES.md` from installed vcpkg copyright files plus `docs/provenance.md`)
  - Tests: T-11 check fails on a fixture manifest with a GPL-2.0-only dependency; regeneration is idempotent (second run produces no diff)
  - Done when: check green on the repository and red on the fixture; notices regenerated
  - Notes: the reviewed licence of every port (direct, transitive and build-time) lives in `docs/licence-manifest.json` with the version it was verified for, because vcpkg's metadata is missing or wrong for several ports (qtbase, double-conversion, opengl-registry, ngspice). `scripts/common-licences.ps1` evaluates SPDX expressions against spec 4.3 (`link` for libraries; `copy` for `docs/provenance.md` rows, where LGPL-2.1-only is refused). `update-notices.ps1` regenerates only the marked sections of `THIRD_PARTY_NOTICES.md` and records the SHA-256 of each installed licence text instead of quoting it. Corrections found: the notices had the Khronos header licences swapped (opengl-registry is MIT, egl-registry Apache-2.0, as in ADR 0004), and pcre2 is `BSD-2-Clause AND BSD-3-Clause WITH PCRE2-exception` (sljit), not only the latter as ADR 0004 says. `check-selftest.ps1` now also checks the failure message of each licence case.
- [ ] **S0.02.3: Harden the network-API check and document third-party socket imports**
  - Requirements: P-03, CR-06, T-11
  - Modules: scripts, docs
  - Depends on: S0.01.1
  - Deliverables: `scripts/check-network-apis.ps1` per ADR 0026 (Accepted). The scaffold scans APP's sources in `src/`, project and props link inputs, `vcpkg.json` ports and features and the imports of APP-built executables, all of which fail; it already allows third-party network imports and prints their summary (library, network DLL, imported APIs, folders), and `-SummaryPath` writes it to `docs/network-imports.md` (first entry: `Qt6Cored.dll` → `WS2_32.dll`, present even without Qt Network). This step extends the failing scan to APP-built DLLs (today only APP executables are matched, by project name) and documents the check and how to regenerate the summary. App allow-list stays empty
  - Tests: T-11 `scripts/check-selftest.ps1` cases fail on a fixture source including `winsock2.h` and calling `WSAStartup`, a fixture project linking `ws2_32.lib`, `vcpkg.json` adding the `qtbase` network feature, an APP-built executable importing `ws2_32.dll`, and an APP-built binary importing `WINHTTP.dll`; a third-party DLL importing `ws2_32.dll` passes and is listed in the summary, and Qt6Core's `ws2_32.dll` import is noted, not a failure (Accept: P-03b)
  - Done when: every failure mode and the allowed case covered by `check-selftest.ps1`; repository green; `docs/network-imports.md` regenerated and matches the check's printed summary
- [ ] **S0.02.4: Run MSVC Code Analysis and clang-tidy from a script**
  - Requirements: T-11
  - Modules: scripts, build
  - Depends on: S0.01.1
  - Deliverables: `.clang-tidy`; `scripts/analyze.ps1` (MSBuild `RunCodeAnalysis=true` with a ruleset in `build/props/`, clang-tidy from the VS-bundled LLVM)
  - Tests: T-11 `scripts/analyze.ps1` reports zero findings on the scaffold and fails on a fixture with a seeded null dereference
  - Done when: zero findings on the repository; script called by `scripts/ci-local.ps1` once it exists
- [ ] **S0.02.5: Add the clang-format check**
  - Requirements: T-11
  - Modules: scripts
  - Depends on: S0.01.1
  - Deliverables: `scripts/check-format.ps1` (VS-bundled clang-format, `--dry-run --Werror` over `src/` and `tests/`)
  - Tests: T-11 check fails on a mis-formatted fixture and passes on the repository
  - Done when: repository formatted and green; check run by `scripts/test.ps1`
- [ ] **S0.02.6: Add clean-room and data-source guards**
  - Requirements: CR-01, CR-02, CR-03, CR-05, CR-06
  - Modules: scripts, docs, libraries
  - Depends on: S0.01.1
  - Deliverables: `docs/clean-room.md` (contributor checklist for CR-01 … CR-07); `scripts/check-clean-room.ps1` (banned vendor trademark terms in UI strings and translations, banned native CAD file extensions in importer/exporter registrations, telemetry and update-check keywords, every data file under `libraries/` listed in `docs/data-sources.md`)
  - Tests: T-11 one failing fixture per rule; passes on the repository
  - Done when: all rules detected on fixtures; repository green; term list reviewed by the owner
- [ ] **S0.02.7: Run every check from test.ps1 and extend check-selftest.ps1 to all checks**
  - Requirements: T-11, T-13, A-01
  - Modules: scripts
  - Depends on: S0.01.3, S0.01.4, S0.01.5, S0.02.1, S0.02.2, S0.02.3, S0.02.5, S0.02.6
  - Deliverables: `scripts/check-selftest.ps1` (from the scaffold: copies the tree to `%TEMP%`, applies one deliberate violation per case and expects failure) extended with cases for `check-headless`, the C++ feature check, `check-licences`, `check-format` and `check-clean-room`; `scripts/test.ps1` runs every check by default (`-NoChecks` skips them for local speed)
  - Tests: T-11 `scripts/check-selftest.ps1` passes on the clean copy and detects every deliberate violation; T-13 `scripts/test.ps1` writes a JUnit file per check to `out/test-results/<Configuration>/`
  - Done when: every check fails on its violation and passes on the repository; `scripts/test.ps1` green

### S0.03 Core foundations

- [ ] **S0.03.1: Apply layering extension ADR 0016 (core may use nlohmann/json)**
  - Requirements: A-01
  - Modules: core, scripts
  - Depends on: S0.02.7
  - Deliverables: rule `030` table, `scripts/check-layering.ps1` third-party table and `Core.vcxproj` updated together; ADR 0016 set to Accepted
  - Tests: T-11 `check-layering.ps1` passes with `core` including `<nlohmann/json.hpp>` and still fails when `platform` includes it
  - Done when: ADR Accepted, rule, script and project agree, checks green
- [ ] **S0.03.2: Implement the error model and Result type**
  - Requirements: A-02, A-03
  - Modules: core
  - Depends on: S0.01.5, S0.03.1
  - Deliverables: `src/core/Error.{h,cpp}`, `src/core/Result.h` (`std::expected<T, Error>`, error codes, message/detail/objects); `docs/adr/NNNN-error-model.md`
  - Tests: T-01 `Core.Tests Result.*`, `Error.*`
  - Done when: tests green; every public function returning `Result` is `[[nodiscard]]`; Doxygen clean
- [ ] **S0.03.3: Implement ObjectId (UUID v7)**
  - Requirements: A-04
  - Modules: core
  - Depends on: S0.03.2
  - Deliverables: `src/core/ObjectId.{h,cpp}` (generate with injectable clock and random source, parse, lowercase format, RFC 4122 byte order, compare, hash)
  - Tests: T-01 `Core.Tests ObjectId.*` (version and variant bits, monotonic ordering, string round-trip, 1e6 unique IDs with fixed seed)
  - Done when: tests green; deterministic generation available for tests
- [ ] **S0.03.4: Implement units, quantities and precision constants**
  - Requirements: P-06, P-07
  - Modules: core
  - Depends on: S0.03.2
  - Deliverables: `src/core/Units.{h,cpp}`, `src/core/Quantity.h`, `src/core/Tolerance.h` (mm, cm, m, in, mil, thou, µm, deg, rad; default 1e-7 mm confusion tolerance; display formatting with per-document defaults)
  - Tests: T-01 `Core.Tests Units.*` (exact conversion factors, formatting round-trip, unit parsing incl. `µm` and `um`)
  - Done when: tests green
- [ ] **S0.03.5: Implement math primitives and integer-nanometre geometry types**
  - Requirements: P-06, A-09
  - Modules: core
  - Depends on: S0.03.4
  - Deliverables: `src/core/math/{Vec,Mat,Transform,Box}.h`, `src/core/math/IntGeom.{h,cpp}` (int64 nm points, vectors and rectangles with overflow-checked arithmetic; no nm↔mm conversion here)
  - Tests: T-01 `Core.Tests Transform.*`, `IntGeom.*` (overflow detection, exact integer ops)
  - Done when: tests green; no floating-point use in `IntGeom`
- [ ] **S0.03.6: Implement the expression lexer and parser**
  - Requirements: A-08
  - Modules: core
  - Depends on: S0.03.4
  - Deliverables: `src/core/expr/{Lexer,Parser,Ast}.{h,cpp}` (operators `+ - * / ^`, parentheses, unit literals `10 mm + 0.5 in`, dotted references `Board.Width`, function calls)
  - Tests: T-01 `Core.Tests ExprParser.*` (precedence, right-associative `^`, error column positions)
  - Done when: tests green; grammar documented in `docs/format/expressions.md`
- [ ] **S0.03.7: Implement the expression evaluator with unit checking**
  - Requirements: A-08
  - Modules: core
  - Depends on: S0.03.6
  - Deliverables: `src/core/expr/Evaluator.{h,cpp}`, `src/core/expr/Functions.cpp` (`sin cos tan asin acos atan atan2 sqrt abs min max floor ceil round if`)
  - Tests: T-01 `Core.Tests ExprEval.*` (every function, `1 mm + 1 deg` is a unit error, dimensionless angles)
  - Done when: tests green
- [ ] **S0.03.8: Add expression dependency extraction and parameter scopes**
  - Requirements: A-08, A-07
  - Modules: core
  - Depends on: S0.03.7
  - Deliverables: `src/core/expr/Dependencies.{h,cpp}`, `src/core/expr/IParameterScope.h` (named lookup through nested scopes)
  - Tests: T-01 `Core.Tests ExprDeps.*`
  - Done when: tests green
- [ ] **S0.03.9: Implement the event bus**
  - Requirements: A-03, A-07
  - Modules: core
  - Depends on: S0.03.3
  - Deliverables: `src/core/EventBus.{h,cpp}` (`DocumentChanged`, `RegenStateChanged`, `JobProgress`; handlers may post commands but never mutate)
  - Tests: T-01 `Core.Tests EventBus.*`
  - Done when: tests green
- [ ] **S0.03.10: Implement structured logging (ADR 0031)**
  - Requirements: P-23
  - Modules: core, build
  - Depends on: S0.03.2
  - Deliverables: `src/core/Log.{h,cpp}` (JSON-lines records, rotating file sink in an injected folder); spdlog added to `vcpkg.json` and `THIRD_PARTY_NOTICES.md`; ADR 0031 Accepted
  - Tests: T-01 `Core.Tests Log.*` (rotation in a temporary folder, structured fields present)
  - Done when: tests and licence check green
- [ ] **S0.03.11: Implement the job system with progress and cancellation**
  - Requirements: A-10
  - Modules: core
  - Depends on: S0.03.2
  - Deliverables: `src/core/jobs/{JobSystem,CancellationToken,Progress}.{h,cpp}`
  - Tests: T-01 `Core.Tests JobSystem.*` (cancellation within 50 ms, progress monotonic, dependent tasks ordered)
  - Done when: tests green and stable over 100 repetitions
- [ ] **S0.03.12: Implement the factory registry and service locator**
  - Requirements: A-11
  - Modules: core
  - Depends on: S0.03.2
  - Deliverables: `src/core/Registry.h`, `src/core/ServiceLocator.{h,cpp}`
  - Tests: T-01 `Core.Tests Registry.*` (duplicate registration is an error, lookup by name)
  - Done when: tests green
- [ ] **S0.03.13: Add known-folder and settings interfaces**
  - Requirements: F-10, P-04, U-10
  - Modules: core
  - Depends on: S0.03.1, S0.03.2
  - Deliverables: `src/core/IKnownFolders.h`, `src/core/Settings.{h,cpp}` (user settings JSON at an injected path) per ADR 0024
  - Tests: T-01 `Core.Tests Settings.*` (load, save, unknown keys preserved)
  - Done when: tests green; no OS calls in `core`

### S0.04 Command bus, undo and journal

- [ ] **S0.04.1: Define the command interface and registry**
  - Requirements: A-03, A-11
  - Modules: core, docs
  - Depends on: S0.03.12
  - Deliverables: `src/core/command/{Command,CommandRegistry,CommandContext}.{h,cpp}`; `registerCommands()` convention; `docs/commands/README.md` naming rules applied; `docs/adr/NNNN-command-contract.md`
  - Tests: T-01 `Core.Tests CommandRegistry.*`
  - Done when: tests green; ADR Accepted
- [ ] **S0.04.2: Validate command parameters against JSON Schemas**
  - Requirements: A-03, F-01
  - Modules: core, build
  - Depends on: S0.04.1
  - Deliverables: schema validator (decision in `docs/adr/NNNN-json-schema-validator.md`: candidate libraries with draft 2020-12 support versus an in-house subset); `src/core/command/ParamValidator.{h,cpp}`
  - Tests: T-01 `Core.Tests ParamValidator.*` (required, types, enums, units-bearing expression strings)
  - Done when: invalid parameters are rejected with a JSON path; licence check green
- [ ] **S0.04.3: Implement the command bus, undo stack and groups**
  - Requirements: A-03, P-22
  - Modules: core
  - Depends on: S0.04.2, S0.03.9
  - Deliverables: `src/core/command/{CommandBus,UndoStack}.{h,cpp}` (run, undo, redo, `beginGroup`/`endGroup`, `mergeWith` for drags, memory cap default 2 GB, configurable)
  - Tests: T-01 `Core.Tests CommandBus.*` (1,000-step undo/redo restores state, merge coalesces, cap evicts oldest)
  - Done when: tests green
- [ ] **S0.04.4: Write the session journal**
  - Requirements: A-03
  - Modules: core
  - Depends on: S0.04.3, S0.03.13
  - Deliverables: `src/core/command/JournalWriter.{h,cpp}` (JSON Lines; header with `$magic`, `formatVersion`, `type: "journal"`; flush per command; folder injected)
  - Tests: T-01 `Core.Tests JournalWriter.*` (header first, one line per command, survives abrupt close)
  - Done when: tests green
- [ ] **S0.04.5: Read and replay journals and record macros**
  - Requirements: A-03, T-02
  - Modules: core
  - Depends on: S0.04.4
  - Deliverables: `src/core/command/{JournalReader,Replayer}.{h,cpp}`; macro recording writes `*.cmd.json` scripts in the same format the tests use
  - Tests: T-01 `Core.Tests Replayer.*` (record → replay reproduces state; malformed line reports line number)
  - Done when: tests green; format documented in `docs/format/journal.md`

### S0.05 Document model and regeneration graph

- [ ] **S0.05.1: Implement Project, Document and the object store**
  - Requirements: A-04, F-04
  - Modules: doc
  - Depends on: S0.03.3, S0.04.3
  - Deliverables: `src/doc/{Project,Document,ObjectStore}.{h,cpp}` (documents keyed by ObjectId; design-intent data only)
  - Tests: T-01 `Doc.Tests Project.*`, `ObjectStore.*`
  - Done when: tests green
- [ ] **S0.05.2: Add parameters at document and project scope**
  - Requirements: A-08, A-04
  - Modules: doc
  - Depends on: S0.05.1, S0.03.8
  - Deliverables: `src/doc/Parameters.{h,cpp}` (parameters with ObjectId, name, expression, comment; scope chain document → project implementing `IParameterScope`)
  - Tests: T-01 `Doc.Tests Parameters.*` (`Project.WallThickness` resolves from a document)
  - Done when: tests green
- [ ] **S0.05.3: Implement the regeneration graph (dirty tracking, states, cycles)**
  - Requirements: A-07
  - Modules: doc
  - Depends on: S0.05.1
  - Deliverables: `src/doc/regen/{RegenGraph,RegenNode,RegenState}.{h,cpp}` (OK, Warning, Error, Suppressed, OutOfDate)
  - Tests: T-01 `Doc.Tests RegenGraph.*` (only dirty nodes recompute, cycle reported with node names, states propagate)
  - Done when: tests green
- [ ] **S0.05.4: Regenerate independent branches in parallel and keep last good output**
  - Requirements: A-07, A-10, P-21
  - Modules: doc, core
  - Depends on: S0.05.3, S0.03.11
  - Deliverables: parallel evaluation on `JobSystem` against immutable snapshots; results applied through internal commands; failing node keeps its definition and last good output, dependants become OutOfDate
  - Tests: T-01 `Doc.Tests RegenParallel.*` (deterministic results over 100 runs), `RegenFailure.KeepsDefinition_WhenNodeFails`
  - Done when: tests green; no data race under the MSVC address sanitizer build
- [ ] **S0.05.5: Add generic project, document and parameter commands**
  - Requirements: A-02, A-03
  - Modules: doc, docs
  - Depends on: S0.05.2, S0.04.2
  - Deliverables: commands `project.create`, `document.create`, `document.rename`, `document.delete`, `parameter.add`, `parameter.set`, `parameter.remove` with schemas in `docs/commands/`
  - Tests: T-01 `Doc.Tests DocCommands.*` (each undoes exactly)
  - Done when: tests green; command docs listed

### S0.06 Open file format: JSON, BSON and magic

- [ ] **S0.06.1: Write the format specification skeleton and first schemas**
  - Requirements: F-01, F-09
  - Modules: docs
  - Depends on: S0.05.1
  - Deliverables: `docs/format/{envelope,project,blobs,versioning}.md`; `docs/format/schemas/{envelope,project}.schema.json` (draft 2020-12); schema IDs `urn:app:schema:<type>:<major>` per ADR 0032
  - Tests: T-03 `Format.Tests Schemas.AllSchemasAreValidDraft202012`
  - Done when: schemas validate; ADR Accepted
- [ ] **S0.06.2: Implement the deterministic JSON writer**
  - Requirements: F-03, F-09
  - Modules: doc
  - Depends on: S0.06.1
  - Deliverables: `src/doc/io/JsonWriter.{h,cpp}` (`$magic` first, then `formatVersion`, `type`, then schema key order; shortest round-trip floats via `std::to_chars`; LF; 2-space indent)
  - Tests: T-03 `Format.Tests JsonWriter.*` (saving twice gives identical bytes; float edge cases)
  - Done when: tests green
- [ ] **S0.06.3: Implement the JSON reader with validation and unknown-field preservation**
  - Requirements: F-08, F-03
  - Modules: doc
  - Depends on: S0.06.2, S0.04.2
  - Deliverables: `src/doc/io/JsonReader.{h,cpp}` (errors carry file, JSON path and reason; unknown keys kept in place per ADR 0027)
  - Tests: T-03 `Format.Tests JsonReader.*` (unknown fields written back byte-identical, malformed inputs give precise errors)
  - Done when: tests green
- [ ] **S0.06.4: Implement the BSON writer and reader**
  - Requirements: F-02, F-09
  - Modules: doc
  - Depends on: S0.06.3
  - Deliverables: `src/doc/io/{BsonWriter,BsonReader}.{h,cpp}` (`$magic` first as binary subtype 0x04; ObjectIds binary by schema `format: uuid` per ADR 0028)
  - Tests: T-03 `Format.Tests Bson.*` (bytes 4 to 32 equal the F-09 constant)
  - Done when: tests green
- [ ] **S0.06.5: Implement file identification by magic (FileSniffer)**
  - Requirements: F-09
  - Modules: doc
  - Depends on: S0.06.4
  - Deliverables: `src/doc/io/FileSniffer.{h,cpp}` (reads at most 4 KB; JSON first key or first-256-byte check; BSON 29-byte compare; "not an APP file" error; explicit "import legacy/unknown JSON" path)
  - Tests: T-03 `Format.Tests FileSniffer.*` (wrong magic, missing magic, truncated file, wrong extension)
  - Done when: tests green
- [ ] **S0.06.6: Save and load the project folder form with the blob store**
  - Requirements: F-02, F-04, F-06
  - Modules: doc
  - Depends on: S0.06.5
  - Deliverables: `src/doc/io/{ProjectFolderStore,BlobStore}.{h,cpp}` (`project.json`, per-document files, `links/`, `libraries/`, `blobs/<sha256>.bin` deduplicated, `cache/`)
  - Tests: T-03 `Format.Tests ProjectFolder.*`, `BlobStore.DeduplicatesByHash`
  - Done when: tests green
- [ ] **S0.06.7: Save and load the single-file BSON form and add the convert command**
  - Requirements: F-02
  - Modules: doc, cli
  - Depends on: S0.06.6
  - Deliverables: `src/doc/io/BinaryProjectStore.{h,cpp}`; command `project.convert` (JSON folder ↔ BSON file)
  - Tests: T-03 `Format.Tests BinaryProject.*`
  - Done when: tests green
- [ ] **S0.06.8: Add formatVersion handling and the migration framework**
  - Requirements: F-05
  - Modules: doc
  - Depends on: S0.06.3
  - Deliverables: `src/doc/migrations/{MigrationChain,Migrate_0_9_0_to_1_0_0}.{h,cpp}` (synthetic first migration); fixture `tests/data/format/0.9.0/`; newer minor loads with a warning, newer major refused
  - Tests: T-03 `Format.Tests Migration.*` (logged steps, warning, refusal message)
  - Done when: tests green; migration rules in `docs/format/versioning.md`
- [ ] **S0.06.9: Treat cache data as non-authoritative**
  - Requirements: F-04
  - Modules: doc
  - Depends on: S0.06.6
  - Deliverables: cache flags and folder handling in `ProjectFolderStore`; `project.cache.clear` command
  - Tests: T-03 `Format.Tests Cache.DeletingCacheLosesNoDesignData`
  - Done when: tests green
- [ ] **S0.06.10: Define the library file envelope and kinds**
  - Requirements: F-07, F-09
  - Modules: doc, docs
  - Depends on: S0.06.5
  - Deliverables: `docs/format/library.md`, `docs/format/schemas/library.schema.json` (kinds from F-10 plus `layerStack` and `environment` per ADR 0032); `src/doc/library/LibraryFile.{h,cpp}`
  - Tests: T-03 `Format.Tests LibraryFile.*`
  - Done when: tests green
- [ ] **S0.06.11: Build the format round-trip and magic suite**
  - Requirements: T-03, F-02, F-09
  - Modules: tests, doc
  - Depends on: S0.06.7, S0.06.10
  - Deliverables: `tests/format/RoundTripTests.cpp` iterating every golden project under `tests/data/` (load → JSON → reload → BSON → reload → JSON)
  - Tests: T-03 `Format.Tests RoundTrip.JsonBsonJsonIsByteIdentical` (Accept: F-02); `Magic.BsonPrefixPerDocumentType` (Accept: F-09a); `Magic.JsonStartsWithMagicKey` (Accept: F-09b); `Magic.RejectsWrongOrMissingMagic` (Accept: F-09c)
  - Done when: suite green and picks up new golden projects automatically

### S0.07 Library scopes

- [ ] **S0.07.1: Implement the library resolver (project → user → global)**
  - Requirements: F-10, F-07
  - Modules: doc
  - Depends on: S0.06.10, S0.03.13
  - Deliverables: `src/doc/library/LibraryResolver.{h,cpp}` (shadowing by `libraryId`, extra read-only folders, injected scope paths, scope reported per item)
  - Tests: T-01 `Doc.Tests LibraryResolver.ProjectShadowsUserShadowsGlobal` and `FallsBackWhenProjectItemDeleted` (Accept: F-10a)
  - Done when: tests green; no file-system access outside the injected folders
- [ ] **S0.07.2: Add the seed manifest and first-run seeding**
  - Requirements: F-10
  - Modules: doc, scripts, libraries
  - Depends on: S0.07.1
  - Deliverables: `libraries/seed-manifest.json` (seed version and content hash per item); `scripts/update-seed-manifest.ps1`; `src/doc/library/SeedInstaller.{h,cpp}` (copies the seed to the global folder on first run; ADR 0032, C-22)
  - Tests: T-01 `Doc.Tests SeedInstaller.*` (empty seed installs, second run is a no-op)
  - Done when: tests green
- [ ] **S0.07.3: Add library scope commands**
  - Requirements: F-10
  - Modules: doc
  - Depends on: S0.07.2
  - Deliverables: commands `library.scope.list`, `library.item.overrideInto` (records origin scope, ID and revision), `library.folder.add` (read-only extra folder)
  - Tests: T-01 `Doc.Tests LibraryCommands.*`; T-02 `tests/data/scripts/library_scopes.cmd.json`
  - Done when: tests green

### S0.08 Test harness

- [ ] **S0.08.1: Build the shared test support library**
  - Requirements: T-01, T-04
  - Modules: tests
  - Depends on: S0.03.2
  - Deliverables: `tests/support/` (temporary folders, tolerance comparisons, `*.expected.json` loader, golden update guard that rewrites expected files only when `scripts/test.ps1 -UpdateGolden -Reason "<why>"` sets it)
  - Tests: T-01 `Support.Tests` self-tests
  - Done when: tests green; float comparisons never use `==`
- [ ] **S0.08.2: Add the Format, Golden and Integration test projects to test.ps1**
  - Requirements: T-13, T-02, T-03, T-04
  - Modules: tests, scripts
  - Depends on: S0.08.1, S0.02.7
  - Deliverables: `tests/format/Format.Tests.vcxproj`, `tests/golden/Golden.Tests.vcxproj`, `tests/integration/Integration.Tests.vcxproj` (runs `cli` on `*.cmd.json` scripts and compares model state to expected JSON); projects picked up by the existing `*.Tests.exe` discovery in `scripts/test.ps1` and its `-Filter` and `-UpdateGolden -Reason` switches
  - Tests: T-13 `scripts/test.ps1` runs all projects and writes JUnit XML; T-02 `Integration.Tests CliSmoke.VersionExitsZero`; `scripts/test.ps1 -UpdateGolden` without `-Reason` still refuses to run
  - Done when: all projects discovered by Test Explorer and `scripts/test.ps1`
- [ ] **S0.08.3: Add analytic oracles and the mesh validator**
  - Requirements: T-04, T-07, T-10
  - Modules: tests
  - Depends on: S0.08.1
  - Deliverables: `tests/support/oracles/MassProperties.{h,cpp}` (box, cylinder, cone, sphere, torus), `tests/support/mesh/MeshValidator.{h,cpp}` (watertight, manifold, outward normals, self-intersection, degenerate triangles)
  - Tests: T-01 `Support.Tests Oracles.*`, `MeshValidator.*` with hand-made defective meshes
  - Done when: tests green
- [ ] **S0.08.4: Add the benchmark project and history file**
  - Requirements: T-08, P-17
  - Modules: tests, scripts
  - Depends on: S0.08.2
  - Deliverables: `tests/bench/Bench.vcxproj` building `Bench.Bench.exe` (median of N runs) so the scaffold's `scripts/test.ps1 -Bench` (which runs `*.Bench.exe`) finds it; `-Bench` extended to append `bench/history.csv`; first benchmark `Bench.SaveLoadProject` (basic P-17 size)
  - Tests: T-08 `scripts/test.ps1 -Bench` appends one row per benchmark with machine info
  - Done when: benchmark runs and records history
- [ ] **S0.08.5: Add fuzz targets for loaders and the expression parser**
  - Requirements: T-09, F-08
  - Modules: tests, scripts
  - Depends on: S0.08.2, S0.06.4, S0.03.7
  - Deliverables: `tests/fuzz/{fuzz_json_loader,fuzz_bson_loader,fuzz_expression}.cpp` built as `*.Fuzz.exe` with MSVC `/fsanitize=fuzzer,address`; corpora seeded from golden files; the scaffold's `scripts/test.ps1 -Fuzz` extended with a `-Seconds <n>` time budget
  - Tests: T-09 each target runs 10 minutes without crash or hang (Accept: F-08)
  - Done when: targets run locally; nightly budget defined in `scripts/ci-local.ps1`
- [ ] **S0.08.6: Report line coverage per module**
  - Requirements: T-12
  - Modules: scripts
  - Depends on: S0.08.2
  - Deliverables: the scaffold's `scripts/test.ps1 -Coverage` (Cobertura XML per executable in `out/test-results/<Configuration>/`) extended with a per-module summary in `out/test-results/coverage.md` and a threshold check (80 %) for `core` and `doc`
  - Tests: T-12 `scripts/test.ps1 -Coverage` produces per-module figures and fails below the threshold on a fixture report
  - Done when: report generated; thresholds enforced from the S0 gate
- [ ] **S0.08.7: Add the offline smoke run**
  - Requirements: P-03, T-11
  - Modules: scripts
  - Depends on: S0.08.2
  - Deliverables: `scripts/smoke-offline.ps1` (blocks outbound traffic for the product and test executables with a temporary Windows Firewall rule, or runs with adapters disabled when elevated; runs every stage's smoke scripts). Third-party network imports such as Qt6Core → `ws2_32.dll` are allowed by the static check (ADR 0026), so this runtime check is the proof that APP performs no network access: it fails on errors and on any outbound connection attempt while blocked
  - Tests: T-11 offline smoke of `cli`, `ui` (start and close) and `Integration.Tests` with no errors and no outbound connection attempts (Accept: P-03a)
  - Done when: smoke run green; procedure documented for the stage gates

### S0.09 Geometry kernel adapter and topological-naming spike (SP-2)

- [ ] **S0.09.1: Define the kernel interface, shape handles and primitives**
  - Requirements: A-05
  - Modules: geom
  - Depends on: S0.03.5
  - Deliverables: `opencascade` in `vcpkg.json` (licence `LGPL-2.1-only WITH OCCT-exception-1.0`, already recorded in `THIRD_PARTY_NOTICES.md`, ADR 0005); `src/geom/{Kernel,Shape}.h` (pimpl, no OCCT types), `src/geom/occt/OcctKernel.{h,cpp}`; box, cylinder, sphere, cone, torus; `dumpbin /dependents` list of OCCT `TKernel` recorded in ADR 0029 and its network imports in `docs/network-imports.md` (moved from S0.01.1, where OCCT was not yet a dependency)
  - Tests: T-01 `Geom.Tests Primitives.*`
  - Done when: tests green; no OCCT include in any `src/geom/*.h` public header
- [ ] **S0.09.2: Add mass properties, bounding box, validity and topology counts**
  - Requirements: A-05
  - Modules: geom
  - Depends on: S0.09.1, S0.08.3
  - Deliverables: `src/geom/Analysis.{h,cpp}`
  - Tests: T-04 `Geom.Tests Analysis.PrimitivesMatchAnalytic` (relative 1e-6 against `tests/support` oracles from S0.08.3)
  - Done when: tests green
- [ ] **S0.09.3: Tessellate shapes into core meshes**
  - Requirements: A-05
  - Modules: geom, core
  - Depends on: S0.09.1
  - Deliverables: `src/core/mesh/Mesh.h`, `src/geom/Tessellator.{h,cpp}` (chordal and angular deviation, per-face groups)
  - Tests: T-01 `Geom.Tests Tessellator.*` (closed box mesh, triangle counts monotonic with tolerance)
  - Done when: tests green
- [ ] **S0.09.4: Enforce OCCT confinement in the layering check**
  - Requirements: A-05, A-01
  - Modules: scripts
  - Depends on: S0.09.1, S0.02.7
  - Deliverables: `scripts/check-layering.ps1` rule: OCCT headers only under `src/geom/`, never in a `src/geom/*.h` public header
  - Tests: T-11 fixture header outside `geom` that includes `TopoDS_Shape.hxx` fails (Accept: A-01)
  - Done when: check green on the repository, red on the fixture
- [ ] **S0.09.5: Spike SP-2a: prototype topological-naming schemes on 20 breakage cases (time box 2 days)**
  - Requirements: A-06
  - Modules: geom, tests
  - Depends on: S0.09.2
  - Deliverables: prototype in `tests/geom/spikes/` comparing (a) role + index + geometric signature and (b) OCCT naming services, on the 20 A-06 breakage cases built with raw kernel calls
  - Tests: spike harness reporting correct / flagged / wrong attachment per case and scheme
  - Done when: both schemes measured on all 20 cases, or the time box ends with the result recorded
- [ ] **S0.09.6: Spike SP-2b: choose the TopoRef scheme in ADR 0035**
  - Requirements: A-06
  - Modules: geom, docs
  - Depends on: S0.09.5
  - Deliverables: `docs/adr/0035-spike-sp2-topological-naming.md` (scheme, `TopoRef` JSON shape, signature fields, ambiguity rule)
  - Tests: spike results attached to the ADR; zero wrong attachments for the chosen scheme
  - Done when: ADR Accepted by the owner before S2 feature work
- [ ] **S0.09.7: Implement the TopoRef type, serialisation and resolver interface**
  - Requirements: A-06, A-04
  - Modules: geom, doc, docs
  - Depends on: S0.09.6, S0.06.3
  - Deliverables: `src/geom/naming/{TopoRef,TopoResolver}.{h,cpp}`; `docs/format/schemas/toporef.schema.json`
  - Tests: T-01 `Geom.Tests TopoRef.*` (round-trip, "reference lost" error type)
  - Done when: tests green

### S0.10 First vertical slice and CLI

- [ ] **S0.10.1: Create a minimal part document with a box feature**
  - Requirements: A-11, A-07, A-02
  - Modules: part, docs
  - Depends on: S0.05.4, S0.09.3, S0.06.6
  - Deliverables: `src/part/{PartDocument,Feature,FeatureRegistry}.{h,cpp}`, `src/part/features/BoxFeature.{h,cpp}`; command `part.feature.box.create`; `docs/format/schemas/part.schema.json` (initial)
  - Tests: T-01 `Part.Tests BoxFeature.*`; T-04 `Golden.Tests PartGolden.Box` (mass properties vs oracle)
  - Done when: tests green; feature registered through the factory
- [ ] **S0.10.2: Export STL through an exporter registry**
  - Requirements: A-11, M-3DP-01
  - Modules: io
  - Depends on: S0.10.1, S0.08.3
  - Deliverables: `src/io/{ExporterRegistry,IExporter}.h`, `src/io/stl/StlExporter.{h,cpp}` (binary and ASCII); command `io.export.stl`
  - Tests: T-07 `Io.Tests StlExporter.BoxIsWatertight` using the mesh validator
  - Done when: tests green
- [ ] **S0.10.3: Apply layering extension ADR 0017 (cli references platform and render)**
  - Requirements: A-01
  - Modules: cli, scripts
  - Depends on: S0.03.1
  - Deliverables: rule `030`, `scripts/check-layering.ps1` and `Cli.vcxproj` updated; ADR 0017 Accepted
  - Tests: T-11 `check-layering.ps1` green; fixture where `cli` includes a `ui` header still fails
  - Done when: ADR Accepted; checks green
- [ ] **S0.10.4: Implement the headless CLI runner**
  - Requirements: A-02, T-02
  - Modules: cli
  - Depends on: S0.10.2, S0.10.3, S0.04.5, S0.06.7
  - Deliverables: `src/cli/` commands `run <script.cmd.json>`, `replay <journal>`, `open`, `regenerate`, `export`, `save`, `--version`, `--list-commands --json`; documented exit codes
  - Tests: T-02 `Integration.Tests Cli.*` (script runs, failing command exits non-zero with message)
  - Done when: tests green
- [ ] **S0.10.5: Automate the S0 exit round-trip (box → JSON → BSON → STL)**
  - Requirements: T-03, F-02, F-09, T-02
  - Modules: tests
  - Depends on: S0.10.4, S0.06.11
  - Deliverables: `tests/data/scripts/box_roundtrip.cmd.json`, `tests/data/expected/box_roundtrip.expected.json`
  - Tests: T-02 `Integration.Tests Scripts.box_roundtrip` (magic checked on every written file; STL watertight; volume matches oracle)
  - Done when: test green from a clean checkout

### S0.11 Platform services and reliability

- [ ] **S0.11.1: Implement known folders on Windows**
  - Requirements: P-04, F-10
  - Modules: platform
  - Depends on: S0.03.13
  - Deliverables: `src/platform/win/KnownFolders.{h,cpp}` (`%ProgramData%`, `%APPDATA%`, `%LOCALAPPDATA%` under `APP\`)
  - Tests: T-01 `Platform.Tests KnownFolders.*`
  - Done when: tests green; no Win32 include outside `src/platform/`
- [ ] **S0.11.2: Add autosave and recovery files**
  - Requirements: P-20
  - Modules: doc, core
  - Depends on: S0.06.7, S0.03.11
  - Deliverables: `src/doc/Autosave.{h,cpp}` (interval default 5 minutes, configurable; recovery file per project in the injected folder; recovery detection API)
  - Tests: T-01 `Doc.Tests Autosave.*` (injected clock; recovery offered after simulated crash)
  - Done when: tests green
- [ ] **S0.11.3: Install a crash handler that writes minidumps**
  - Requirements: P-20, P-23
  - Modules: platform
  - Depends on: S0.11.1
  - Deliverables: `src/platform/win/CrashHandler.{h,cpp}` (minidump plus last log lines in the local folder)
  - Tests: T-01 `Platform.Tests CrashHandler.WritesDumpOnFault` (child process)
  - Done when: tests green
- [ ] **S0.11.4: Add system information and the "Copy diagnostics" command**
  - Requirements: P-23
  - Modules: platform, core, cli
  - Depends on: S0.11.3, S0.03.10
  - Deliverables: `src/platform/win/SystemInfo.{h,cpp}`; command `diagnostics.copy` (zip of logs and system info written locally, never sent)
  - Tests: T-01 `Platform.Tests SystemInfo.*`; T-02 `Integration.Tests Scripts.diagnostics_copy`
  - Done when: tests green
- [ ] **S0.11.5: Add a dynamic library loading interface**
  - Requirements: P-04
  - Modules: core, platform
  - Depends on: S0.11.1
  - Deliverables: `src/core/IDynamicLibrary.h`, `src/platform/win/DynamicLibrary.{h,cpp}` (for ngspice and optional encoders later)
  - Tests: T-01 `Platform.Tests DynamicLibrary.*`
  - Done when: tests green
- [ ] **S0.11.6: Provide an off-screen OpenGL context for headless rendering**
  - Requirements: P-04, A-02
  - Modules: platform
  - Depends on: S0.11.1
  - Deliverables: `src/platform/win/OffscreenGlContext.{h,cpp}` behind `core::IGlContext`; ADR 0023 applied: the `Qt` entry in `scripts/check-layering.ps1` narrowed to `ui`, rules 010 and 030 updated together
  - Tests: T-01 `Platform.Tests OffscreenGl.CreatesCoreProfile45`; T-11 `scripts/check-selftest.ps1` case where `render` includes a Qt header now fails
  - Done when: test green on the reference machine; skipped with a clear reason only where no GPU driver exists

### S0.12 Rendering foundation

- [ ] **S0.12.1: Implement render::Device with an OpenGL 4.5 backend**
  - Requirements: P-01, P-11
  - Modules: render, build
  - Depends on: S0.11.6, S0.03.5
  - Deliverables: `src/render/{Device,Buffer,Shader}.h`, `src/render/gl/GlDevice.{h,cpp}`; GL function loader dependency with `docs/adr/NNNN-opengl-loader.md`
  - Tests: T-01 `Render.Tests GlDevice.*` (headless clear and read-back)
  - Done when: tests green; licence check green
- [ ] **S0.12.2: Render shaded meshes with edges and render to an image headless**
  - Requirements: P-11, T-06
  - Modules: render
  - Depends on: S0.12.1, S0.09.3
  - Deliverables: `src/render/{Scene,Camera,Renderer}.{h,cpp}`; `render.image` export for CLI
  - Tests: T-06 `Render.Tests Snapshot.BoxShadedWithEdges` (SSIM ≥ 0.98 against `tests/data/snapshots/box_shaded.png`)
  - Done when: test green
- [ ] **S0.12.3: Add picking for faces, edges, vertices and bodies**
  - Requirements: U-09
  - Modules: render
  - Depends on: S0.12.2
  - Deliverables: `src/render/Picking.{h,cpp}` (ID buffer)
  - Tests: T-01 `Render.Tests Picking.*` (known pixels pick known faces)
  - Done when: tests green
- [ ] **S0.12.4: Implement 3D navigation, standard views and view-cube geometry**
  - Requirements: U-08
  - Modules: render
  - Depends on: S0.12.2
  - Deliverables: `src/render/Navigation.{h,cpp}` (orbit, pan, zoom-to-cursor, standard views, mouse presets as data)
  - Tests: T-01 `Render.Tests Navigation.*` (zoom keeps the cursor point fixed)
  - Done when: tests green
- [ ] **S0.12.5: Implement the 2D canvas primitives**
  - Requirements: U-08
  - Modules: render
  - Depends on: S0.12.1
  - Deliverables: `src/render/canvas/Canvas2D.{h,cpp}` (lines, arcs, polygons, text; pan, zoom, fit)
  - Tests: T-06 `Render.Tests Snapshot.Canvas2DPrimitives`
  - Done when: tests green

### S0.13 UI shell

- [ ] **S0.13.1: Bootstrap the Qt application with themes and string conventions**
  - Requirements: U-01, U-12, U-13
  - Modules: ui
  - Depends on: S0.10.4, S0.11.1
  - Deliverables: `src/ui/App.{h,cpp}`; `src/ui/theme/tokens.json` compiled to style sheet and palette (light and dark); `tr()` for every string; accessible-name convention; `src/ui/translations/app_en.ts`
  - Tests: T-05 `Ui.Tests Theme.TextContrastAtLeast4_5` for both themes; `Ui.Tests Strings.AllWidgetsHaveAccessibleNames`
  - Done when: tests green; window titled "APP" in both themes
- [ ] **S0.13.2: Add dockable panels and per-workspace layouts (ADR 0030)**
  - Requirements: U-02, U-03
  - Modules: ui, build
  - Depends on: S0.13.1
  - Deliverables: Qt Advanced Docking System via vcpkg (licence check); `src/ui/shell/{DockManager,WorkspaceLayouts}.{h,cpp}` (layouts saved per workspace mode, reset, floating panels on other monitors, per-monitor DPI awareness in the manifest)
  - Tests: T-05 `Ui.Tests Docking.LayoutSavesAndResets`; manual check at 100 %, 150 %, 300 % recorded in `docs/stages/S0.md`
  - Done when: tests green; ADR 0030 Accepted
- [ ] **S0.13.3: Create the original icon set and resource pipeline**
  - Requirements: U-01, CR-01
  - Modules: ui
  - Depends on: S0.13.1
  - Deliverables: `src/ui/resources/icons/*.svg` (original designs), `.qrc` files built through `Qt.props`
  - Tests: T-11 `scripts/check-clean-room.ps1` passes; T-05 `Ui.Tests Icons.AllCommandIconsResolve`
  - Done when: tests green; icon provenance noted as original in `docs/provenance.md` header text
- [ ] **S0.13.4: Host the viewport and show a kernel-generated box**
  - Requirements: U-08, P-01
  - Modules: ui, render
  - Depends on: S0.13.2, S0.12.4
  - Deliverables: `src/ui/viewport/ViewportWidget.{h,cpp}` (hands a current GL context to `render`), view cube, standard views
  - Tests: T-06 `Ui.Tests Snapshot.ShellWithBox`
  - Done when: app launches with themed shell, dockable panels and the box
- [ ] **S0.13.5: Add basic selection (click, window/crossing box, filters)**
  - Requirements: U-09
  - Modules: ui, render
  - Depends on: S0.13.4, S0.12.3
  - Deliverables: `src/ui/viewport/SelectionTool.{h,cpp}` (face, edge, vertex, body filters)
  - Tests: T-05 `Ui.Tests Selection.*`
  - Done when: tests green
- [ ] **S0.13.6: Add the contextual command bar**
  - Requirements: U-04
  - Modules: ui
  - Depends on: S0.13.2, S0.04.1
  - Deliverables: `src/ui/shell/CommandBar.{h,cpp}` driven by command metadata (workspace, selection predicates)
  - Tests: T-05 `Ui.Tests CommandBar.ShowsCommandsForActiveDocument`
  - Done when: tests green
- [ ] **S0.13.7: Add the command palette (Ctrl+K)**
  - Requirements: U-05
  - Modules: ui
  - Depends on: S0.13.6
  - Deliverables: `src/ui/shell/CommandPalette.{h,cpp}` (fuzzy search across commands, recent files, settings; shows shortcuts)
  - Tests: T-05 `Ui.Tests CommandPalette.*`
  - Done when: tests green
- [ ] **S0.13.8: Add the properties inspector with expression entry**
  - Requirements: U-06, A-08
  - Modules: ui
  - Depends on: S0.13.2, S0.05.5
  - Deliverables: `src/ui/panels/PropertiesPanel.{h,cpp}`, `src/ui/widgets/ExpressionEdit.{h,cpp}` (evaluated value, unit errors inline)
  - Tests: T-05 `Ui.Tests Properties.ExpressionEditsRunCommands`
  - Done when: tests green
- [ ] **S0.13.9: Make every command bindable to a shortcut stored as JSON**
  - Requirements: U-10
  - Modules: ui, core
  - Depends on: S0.13.6, S0.03.13
  - Deliverables: `src/ui/shell/Shortcuts.{h,cpp}`; bindings file in the user settings folder; `docs/format/shortcuts.md`
  - Tests: T-05 `Ui.Tests Shortcuts.*` (rebinding persists, conflicts reported)
  - Done when: tests green
- [ ] **S0.13.10: Add the non-modal message panel with click-to-locate**
  - Requirements: U-11
  - Modules: ui
  - Depends on: S0.13.2
  - Deliverables: `src/ui/panels/MessagesPanel.{h,cpp}` (group by document and severity; locate selects objects)
  - Tests: T-05 `Ui.Tests Messages.ClickLocatesObject`
  - Done when: tests green
- [ ] **S0.13.11: Keep the UI thread responsive with job progress and cancellation**
  - Requirements: A-10
  - Modules: ui, core
  - Depends on: S0.13.2, S0.03.11
  - Deliverables: status-bar job progress, cancel button; UI-thread watchdog in debug builds that logs any stall over 100 ms
  - Tests: T-05 `Ui.Tests Responsiveness.LongJobNeverBlocksOver100ms`
  - Done when: tests green
- [ ] **S0.13.12: Add the About box with licence, source offer and notices**
  - Requirements: P-30
  - Modules: ui
  - Depends on: S0.13.1, S0.02.2
  - Deliverables: `src/ui/dialogs/AboutDialog.{h,cpp}` showing `LICENSE`, the written source offer and `THIRD_PARTY_NOTICES.md` from local resources
  - Tests: T-05 `Ui.Tests About.ShowsLicenceAndNotices`
  - Done when: tests green; works with networking blocked
- [ ] **S0.13.13: Offer crash recovery and expose Copy diagnostics in the UI**
  - Requirements: P-20, P-23
  - Modules: ui
  - Depends on: S0.11.2, S0.11.4, S0.13.10
  - Deliverables: recovery prompt on start-up after an unclean exit; Help menu "Copy diagnostics"
  - Tests: T-05 `Ui.Tests Recovery.OffersRecoveryAfterCrash`
  - Done when: tests green
- [ ] **S0.13.14: Build the UI replay and visual snapshot harness**
  - Requirements: T-05, T-06
  - Modules: tests, ui, scripts
  - Depends on: S0.13.4, S0.04.5
  - Deliverables: replay driver in the scaffold's `tests/ui/Ui.Tests.vcxproj` (journal plus Qt Test events on Qt's `minimal` platform plugin, since the vcpkg Qt build ships no `offscreen` plugin); viewport snapshots rendered through the off-screen GL context from `platform` (S0.11.6); SSIM comparer; reference images updated only through `scripts/test.ps1 -UpdateGolden -Reason "<why>"`
  - Tests: T-05 `Ui.Tests Replay.CreateBoxAndUndo`; T-06 `Ui.Tests Snapshot.*`
  - Done when: harness green in `scripts/test.ps1 -Configuration Debug` (Headless runs skip `Ui.Tests`)
- [ ] **S0.13.15: Measure and meet cold start under 3 s**
  - Requirements: P-10
  - Modules: ui, tests
  - Depends on: S0.13.14, S0.08.4
  - Deliverables: `Bench.ColdStartToEmptyWindow` (process start to first idle event)
  - Tests: T-08 benchmark median under 3 s on the reference machine
  - Done when: target met or a defect recorded per spec 3.2

### S0.14 Documentation and command reference

- [ ] **S0.14.1: Gate the documentation build on zero Doxygen warnings**
  - Requirements: F-01, T-11
  - Modules: scripts, docs
  - Depends on: S0.06.1, S0.08.2
  - Deliverables: prerequisite: Doxygen installed on the reference machine (Doxygen 1.18.0 is installed with `winget install DimitriVanHeesch.Doxygen`; another copy can be passed with `scripts/docs.ps1 -Doxygen <path>`). The scaffold's `scripts/docs.ps1` gains `-FailOnWarning`; `docs/format/` and `docs/adr/` included in the manual; `@req` traceability page
  - Tests: T-11 `scripts/docs.ps1 -FailOnWarning` builds HTML with zero warnings
  - Done when: HTML builds offline with zero warnings. If Doxygen is still missing, mark the step `[~]` with "blocked: Doxygen not installed" and continue with steps that do not depend on it
- [ ] **S0.14.2: Check that every command has a schema and a reference entry**
  - Requirements: A-03, F-01
  - Modules: scripts, cli, docs
  - Depends on: S0.10.4, S0.06.1
  - Deliverables: `scripts/check-commands.ps1` (compares `cli --list-commands --json` with `docs/commands/*.schema.json` and `docs/commands/README.md`)
  - Tests: T-11 check fails when a registered command lacks a schema
  - Done when: check green and run by `scripts/test.ps1`

### S0.15 Local continuous integration (hosted CI open)

- [ ] **S0.15.1: Add the local CI script**
  - Requirements: T-13, T-11
  - Modules: scripts
  - Depends on: S0.02.4, S0.08.5, S0.08.6, S0.08.7, S0.14.2
  - Deliverables: `scripts/ci-local.ps1` (Debug, Release and Headless builds; `scripts/test.ps1 -Coverage` per configuration; `scripts/check-selftest.ps1`; `scripts/analyze.ps1`; `scripts/docs.ps1 -FailOnWarning` when Doxygen is present, otherwise a clear "skipped: Doxygen not installed" line; offline smoke; `-Nightly` adds `scripts/test.ps1 -Bench` and `scripts/test.ps1 -Fuzz`); results archived under `out/test-results/`; ADR 0033 notes the open hosted-CI decision
  - Tests: T-13 `scripts/ci-local.ps1` green on the reference machine
  - Done when: one command reproduces the full gate run

### S0.99 Stage gate

- [ ] **S0.99.1: Run the full S0 test suite and all checks**
  - Requirements: T-13, T-11, P-03, P-04
  - Modules: all
  - Depends on: S0.15.1, S0.13.15, S0.10.5
  - Deliverables: `out/test-results/` from `scripts/ci-local.ps1 -Nightly`, offline smoke log
  - Tests: T-13 all suites incl. `*_Slow`; T-11 all checks and `scripts/check-selftest.ps1`; T-12 `scripts/test.ps1 -Coverage` ≥ 80 % for `core` and `doc`; network check green, and its third-party import summary matches `docs/network-imports.md` (ADR 0026)
  - Done when: everything green; results summarised in `docs/stages/S0.md`
- [ ] **S0.99.2: Run the S0 benchmarks (P-10, basic P-17)**
  - Requirements: P-10, P-17, T-08
  - Modules: tests
  - Depends on: S0.99.1
  - Deliverables: rows appended to `bench/history.csv`
  - Tests: T-08 `Bench.ColdStartToEmptyWindow`, `Bench.SaveLoadProject`
  - Done when: results recorded; misses over 3× raised as blockers
- [ ] **S0.99.3: Update traceability and documentation for S0**
  - Requirements: T-13
  - Modules: docs
  - Depends on: S0.99.2, S0.14.1
  - Deliverables: `docs/traceability.csv` statuses and test names, `docs/stages/S0.md`, `docs/format/`, `docs/commands/`, `docs/cpp-features.md`
  - Tests: `/check-traceability` reports no gaps for S0 requirements; `scripts/docs.ps1 -FailOnWarning` clean (needs Doxygen, see S0.14.1)
  - Done when: every S0 requirement is `done` or `in-progress` with a note
- [ ] **S0.99.4: Human review checkpoint: stop and wait for approval**
  - Requirements: all S0 requirements (see `docs/stages/S0.md`)
  - Modules: none
  - Depends on: S0.99.3
  - Deliverables: S0 summary for the owner (results, deviations, open questions)
  - Tests: none (review)
  - Done when: the owner approves S0 in writing

## Stage S1: Sketcher

Stage plan: `docs/stages/S1.md`. Exit: 10 golden profiles solve fully constrained, conflict diagnosis works, P-13 met,
global parameters drive sketch dimensions.

### S1.01 Sketch solver spike (SP-3)

- [ ] **S1.01.1: Apply ADR 0022 for the sketch module (Eigen in sketch)**
  - Requirements: A-01
  - Modules: sketch, scripts
  - Depends on: S0.99.4
  - Deliverables: rule `030` third-party column, `scripts/check-layering.ps1` and `Sketch.vcxproj` updated; ADR 0022 Accepted
  - Tests: T-11 `check-layering.ps1` green; Eigen include in `ecad/pcb` still fails
  - Done when: ADR Accepted; checks green
- [ ] **S1.01.2: Spike SP-3a: prototype decomposition plus Levenberg–Marquardt on a 500/800 system (time box 2 days)**
  - Requirements: M-SKT-02, P-13
  - Modules: sketch, tests
  - Depends on: S1.01.1
  - Deliverables: prototype under `tests/sketch/spikes/` with a generated 500-entity / 800-constraint sketch; drag-update timings; minimal-conflict-set prototype on seeded over-constrained cases
  - Tests: spike harness: median and worst drag-update time, conflict sets found
  - Done when: measurements recorded, or the time box ends with results recorded
- [ ] **S1.01.3: Spike SP-3b: decide the solver design in ADR 0036**
  - Requirements: M-SKT-02, P-13
  - Modules: sketch, docs
  - Depends on: S1.01.2
  - Deliverables: `docs/adr/0036-spike-sp3-sketch-solver.md` (decomposition, damping, rank analysis, drag model; fresh implementation or CR-04 reference)
  - Tests: spike results attached
  - Done when: ADR Accepted before S1.03.4

### S1.02 Sketch model and entities

- [ ] **S1.02.1: Define the sketch model, placement and schema**
  - Requirements: M-SKT-01, F-01
  - Modules: sketch, docs
  - Depends on: S0.99.4
  - Deliverables: `src/sketch/{Sketch,Entity,SketchPlacement}.{h,cpp}` (on a reference plane, planar face via TopoRef or coordinate system; construction flag); `docs/format/schemas/sketch.schema.json`, `docs/format/sketch.md`
  - Tests: T-01 `Sketch.Tests SketchModel.*`; T-03 `Format.Tests RoundTrip` picks up `sketch_basic` golden
  - Done when: tests green
- [ ] **S1.02.2: Add points, lines, construction lines, circles and arcs**
  - Requirements: M-SKT-01
  - Modules: sketch
  - Depends on: S1.02.1
  - Deliverables: entity classes and commands `sketch.entity.{point,line,constructionLine,circle,arcCentre,arc3Point,arcTangent}.add`, `sketch.entity.toggleConstruction`
  - Tests: T-01 `Sketch.Tests Entities.Basic*`
  - Done when: tests green; command schemas documented
- [ ] **S1.02.3: Add ellipses, partial ellipses and parabolas**
  - Requirements: M-SKT-01
  - Modules: sketch
  - Depends on: S1.02.2
  - Deliverables: commands `sketch.entity.{ellipse,ellipseArc,parabola}.add`
  - Tests: T-01 `Sketch.Tests Entities.Conics*`
  - Done when: tests green
- [ ] **S1.02.4: Add interpolating and control-point splines**
  - Requirements: M-SKT-01
  - Modules: sketch
  - Depends on: S1.02.2
  - Deliverables: `src/sketch/Spline.{h,cpp}`; commands `sketch.entity.{splineInterp,splineControl}.add`
  - Tests: T-01 `Sketch.Tests Splines.*` (evaluation, derivatives, knot handling)
  - Done when: tests green
- [ ] **S1.02.5: Add rectangles, parallelogram, polygon and slots as composite tools**
  - Requirements: M-SKT-01
  - Modules: sketch
  - Depends on: S1.02.2
  - Deliverables: commands `sketch.tool.{rectCorner,rectCentre,rect3Point,parallelogram,polygon,slotStraight,slotArc}` (entities plus their constraints)
  - Tests: T-01 `Sketch.Tests CompositeTools.*`
  - Done when: tests green
- [ ] **S1.02.6: Convert text to outline geometry**
  - Requirements: M-SKT-01
  - Modules: sketch, geom
  - Depends on: S1.02.4
  - Deliverables: `geom::textToCurves` (kernel font support behind the adapter); command `sketch.entity.text.add` (font, height, string)
  - Tests: T-01 `Sketch.Tests Text.*` (closed outlines, glyph count)
  - Done when: tests green; only fonts available locally are used

### S1.03 Sketch solver

- [ ] **S1.03.1: Implement residuals for basic geometric constraints**
  - Requirements: M-SKT-02
  - Modules: sketch
  - Depends on: S1.02.2, S1.01.3
  - Deliverables: `src/sketch/solver/Constraints*.cpp`: coincident, horizontal, vertical, parallel, perpendicular, fix, on-curve, midpoint; command `sketch.constraint.add`
  - Tests: T-01 `Sketch.Tests ConstraintResiduals.*` (analytic Jacobian vs finite differences)
  - Done when: tests green
- [ ] **S1.03.2: Implement tangent, G2, concentric, coradial, equal, collinear, symmetric and merge**
  - Requirements: M-SKT-02
  - Modules: sketch
  - Depends on: S1.03.1, S1.02.4
  - Deliverables: remaining 2D constraints including G2 curvature continuity for splines
  - Tests: T-01 `Sketch.Tests ConstraintResiduals.Advanced*`
  - Done when: tests green
- [ ] **S1.03.3: Implement driving and driven dimensions**
  - Requirements: M-SKT-02
  - Modules: sketch
  - Depends on: S1.03.1
  - Deliverables: linear (aligned, horizontal, vertical), angular, radius, diameter, arc length, ellipse axes, point-to-curve; command `sketch.dimension.add`, `sketch.dimension.setDriven`
  - Tests: T-01 `Sketch.Tests Dimensions.*`
  - Done when: tests green
- [ ] **S1.03.4: Implement graph decomposition and the damped Newton solver (ADR 0036)**
  - Requirements: M-SKT-02
  - Modules: sketch
  - Depends on: S1.03.2, S1.03.3
  - Deliverables: `src/sketch/solver/{Decomposer,LmSolver}.{h,cpp}`; command `sketch.solve`
  - Tests: T-10 `Sketch.Tests Solver.ConvergesOnGoldenProfiles`
  - Done when: tests green; failed solve leaves geometry unchanged
- [ ] **S1.03.5: Add DOF analysis and per-entity constraint state**
  - Requirements: M-SKT-02
  - Modules: sketch
  - Depends on: S1.03.4
  - Deliverables: numerical rank analysis; states under-defined, fully defined, over-defined, unsolvable; remaining DOF per sketch
  - Tests: T-10 `Sketch.Tests Dof.*`
  - Done when: tests green
- [ ] **S1.03.6: Find minimal conflicting sets and offer fixes**
  - Requirements: M-SKT-02
  - Modules: sketch
  - Depends on: S1.03.5
  - Deliverables: `src/sketch/solver/ConflictFinder.{h,cpp}`; commands `sketch.constraint.delete`, `sketch.dimension.setDriven` offered as fixes
  - Tests: T-10 `Sketch.Tests Conflicts.MinimalSet_WhenOverConstrained`
  - Done when: tests green; set is minimal (removing any member resolves)
- [ ] **S1.03.7: Solve interactive drags**
  - Requirements: M-SKT-02
  - Modules: sketch
  - Depends on: S1.03.5
  - Deliverables: soft "follow the cursor" constraint; command `sketch.entity.drag` merging consecutive drags; fixed and fully defined geometry never moves
  - Tests: T-10 `Sketch.Tests Drag.*` (stability over 1,000 steps)
  - Done when: tests green
- [ ] **S1.03.8: Add the pierce constraint against 3D geometry**
  - Requirements: M-SKT-02
  - Modules: sketch, geom
  - Depends on: S1.03.4, S0.09.7
  - Deliverables: pierce residual using `geom` curve–plane intersection via TopoRef
  - Tests: T-01 `Sketch.Tests Pierce.*`
  - Done when: tests green
- [ ] **S1.03.9: Drive dimensions with expressions and parameters**
  - Requirements: M-SKT-02, A-08, M-SKT-04
  - Modules: sketch, doc
  - Depends on: S1.03.3, S0.05.2
  - Deliverables: dimension values as expressions; regeneration node per sketch in `RegenGraph`
  - Tests: T-01 `Sketch.Tests DimensionExpressions.*`
  - Done when: tests green
- [ ] **S1.03.10: Build the T-10 sketch-solver verification suite**
  - Requirements: T-10, M-SKT-02
  - Modules: tests
  - Depends on: S1.03.6, S1.03.7
  - Deliverables: `tests/golden/SketchSolverSuite.cpp` (over/under-constrained detection, conflict reporting, drag stability)
  - Tests: T-10 `Golden.Tests SketchSolverSuite.*` (Accept: M-SKT-02a)
  - Done when: suite green
- [ ] **S1.03.11: Meet P-13 (500 entities / 800 constraints, under 16 ms per drag update)**
  - Requirements: P-13, M-SKT-02
  - Modules: sketch, tests
  - Depends on: S1.03.10, S0.08.4
  - Deliverables: `Bench.SketchDrag500x800`; profiling-driven optimisations
  - Tests: T-08 `Bench.SketchDrag500x800` median under 16 ms (Accept: M-SKT-02b)
  - Done when: target met or a defect recorded per spec 3.2
- [ ] **S1.03.12: Verify the DOF count on 20 golden sketches**
  - Requirements: M-SKT-02
  - Modules: tests
  - Depends on: S1.03.5
  - Deliverables: `tests/data/scripts/sketch_dof_01..20.cmd.json` with hand-computed DOF in `*.expected.json`
  - Tests: T-04 `Golden.Tests SketchDof.*` (Accept: M-SKT-02c)
  - Done when: all 20 green

### S1.04 Sketch tools and profiles

- [ ] **S1.04.1: Implement 2D curve intersection utilities**
  - Requirements: M-SKT-01
  - Modules: sketch
  - Depends on: S1.02.4
  - Deliverables: `src/sketch/geometry/Intersect.{h,cpp}` (lines, arcs, conics, splines; tolerance-aware)
  - Tests: T-01 `Sketch.Tests Intersect.*`
  - Done when: tests green
- [ ] **S1.04.2: Add trim (including power-trim drag), extend and split**
  - Requirements: M-SKT-01
  - Modules: sketch
  - Depends on: S1.04.1
  - Deliverables: commands `sketch.tool.trim`, `sketch.tool.trimPath`, `sketch.tool.extend`, `sketch.tool.split`
  - Tests: T-01 `Sketch.Tests Trim.*`, `Extend.*`, `Split.*`
  - Done when: tests green; constraints on trimmed entities preserved where valid
- [ ] **S1.04.3: Add offset with optional cap ends**
  - Requirements: M-SKT-01
  - Modules: sketch
  - Depends on: S1.04.1
  - Deliverables: command `sketch.tool.offset`
  - Tests: T-01 `Sketch.Tests Offset.*`
  - Done when: tests green
- [ ] **S1.04.4: Add mirror (static and dynamic) and linear/circular sketch patterns**
  - Requirements: M-SKT-01
  - Modules: sketch
  - Depends on: S1.03.2
  - Deliverables: commands `sketch.tool.mirror` (dynamic adds symmetric constraints), `sketch.tool.patternLinear`, `sketch.tool.patternCircular`
  - Tests: T-01 `Sketch.Tests Mirror.*`, `SketchPattern.*`
  - Done when: tests green
- [ ] **S1.04.5: Add sketch fillet and chamfer**
  - Requirements: M-SKT-01
  - Modules: sketch
  - Depends on: S1.04.2
  - Deliverables: commands `sketch.tool.fillet`, `sketch.tool.chamfer`
  - Tests: T-01 `Sketch.Tests SketchFillet.*`
  - Done when: tests green
- [ ] **S1.04.6: Project model edges and create intersection curves**
  - Requirements: M-SKT-01
  - Modules: sketch, geom
  - Depends on: S1.02.1, S0.09.7
  - Deliverables: commands `sketch.tool.projectEdges`, `sketch.tool.intersectionCurve` (TopoRef-linked, regenerate on upstream change)
  - Tests: T-01 `Sketch.Tests Projection.*`
  - Done when: tests green
- [ ] **S1.04.7: Detect closed regions, including nested and touching contours**
  - Requirements: M-SKT-01
  - Modules: sketch
  - Depends on: S1.04.1
  - Deliverables: `src/sketch/Regions.{h,cpp}`; region selection API used by features; construction geometry excluded
  - Tests: T-04 `Golden.Tests SketchRegions.Nested01..10` (Accept: M-SKT-01c)
  - Done when: all 10 golden nested-contour sketches give the expected regions
- [ ] **S1.04.8: Golden tool cases and command coverage for entities and tools**
  - Requirements: M-SKT-01, A-03
  - Modules: tests
  - Depends on: S1.04.2, S1.04.3, S1.04.4, S1.04.5, S1.04.6, S1.02.3, S1.02.5, S1.02.6
  - Deliverables: `tests/data/scripts/sketch_tool_01..15.cmd.json`; `tests/sketch/CommandCoverageTests.cpp` (list of every M-SKT-01 entity and tool → registered command with schema)
  - Tests: T-04 `Golden.Tests SketchTools.TrimExtendOffset01..15` (Accept: M-SKT-01b); T-01 `Sketch.Tests CommandCoverage.AllEntitiesAndTools` (Accept: M-SKT-01a)
  - Done when: all green
- [ ] **S1.04.9: Fully constrain the 10 golden profiles**
  - Requirements: M-SKT-01, M-SKT-02
  - Modules: tests
  - Depends on: S1.04.7, S1.03.12
  - Deliverables: `tests/data/scripts/profile_01..10.cmd.json` with expected DOF 0 and region counts
  - Tests: T-04 `Golden.Tests Profiles.*`
  - Done when: all 10 solve fully defined (S1 exit criterion)

### S1.05 3D sketches

- [ ] **S1.05.1: Add the 3D sketch model and entities**
  - Requirements: M-SKT-03
  - Modules: sketch, docs
  - Depends on: S1.02.4
  - Deliverables: `src/sketch/sketch3d/`: points, lines, arcs on an auto-chosen plane, splines, projected curves; schema update with migration if needed
  - Tests: T-01 `Sketch.Tests Sketch3d.Entities*`
  - Done when: tests green
- [ ] **S1.05.2: Add 3D constraints and dimensions to the solver**
  - Requirements: M-SKT-03
  - Modules: sketch
  - Depends on: S1.05.1, S1.03.5
  - Deliverables: along X/Y/Z, parallel to plane, coincident, tangent, perpendicular, equal, fixed; length, angle, point-to-point distance
  - Tests: T-10 `Sketch.Tests Sketch3d.Solver*`
  - Done when: tests green
- [ ] **S1.05.3: Solve the golden handrail-frame 3D sketch**
  - Requirements: M-SKT-03
  - Modules: tests
  - Depends on: S1.05.2
  - Deliverables: `tests/data/scripts/handrail_frame_3d.cmd.json`
  - Tests: T-04 `Golden.Tests Sketch3d.HandrailFullyDefined` (Accept: M-SKT-03a)
  - Done when: DOF 0 and expected point positions within 1e-6

### S1.06 Parameters and equations

- [ ] **S1.06.1: Provide parameter tables at part, assembly and project scope**
  - Requirements: M-SKT-04
  - Modules: doc, sketch
  - Depends on: S1.03.9
  - Deliverables: table model (value, expression, unit, scope, where used, error status; every parameter carries an ObjectId per ADR 0032, C-27); commands `parameter.table.*`
  - Tests: T-01 `Doc.Tests ParameterTable.*`
  - Done when: tests green
- [ ] **S1.06.2: Reject circular parameter references with a message naming all members**
  - Requirements: M-SKT-04, A-07
  - Modules: doc
  - Depends on: S1.06.1
  - Deliverables: cycle check on `parameter.set`/`parameter.add`
  - Tests: T-01 `Doc.Tests ParameterCycle.RejectsAandB` (Accept: M-SKT-04b)
  - Done when: `A = B + 1, B = A` rejected with a message naming `A` and `B`
- [ ] **S1.06.3: Import and export parameters as CSV and JSON**
  - Requirements: M-SKT-04
  - Modules: doc
  - Depends on: S1.06.1
  - Deliverables: commands `parameter.import`, `parameter.export`
  - Tests: T-01 `Doc.Tests ParameterIo.*` (round-trip)
  - Done when: tests green
- [ ] **S1.06.4: Drive sketch dimensions from global parameters (golden)**
  - Requirements: M-SKT-04, A-08
  - Modules: tests
  - Depends on: S1.06.2, S1.04.9
  - Deliverables: `tests/data/scripts/global_params_sketch.cmd.json`
  - Tests: T-02 `Integration.Tests Scripts.global_params_sketch` (changing `Project.Width` re-solves dependent sketches)
  - Done when: test green (S1 exit criterion)

### S1.07 Sketch user interface

- [ ] **S1.07.1: Add sketch mode with drawing tools that call commands**
  - Requirements: M-SKT-01, U-04
  - Modules: ui, render
  - Depends on: S1.04.8, S0.12.5
  - Deliverables: `src/ui/sketch/` sketch mode on the placement plane, tool state machines for every entity and tool
  - Tests: T-05 `Ui.Tests SketchMode.DrawRectangle`
  - Done when: tests green; no direct document mutation from UI code
- [ ] **S1.07.2: Show auto-inference hints and apply them on commit**
  - Requirements: M-SKT-02
  - Modules: ui, sketch
  - Depends on: S1.07.1
  - Deliverables: `sketch::InferenceEngine` (headless) plus UI hints; modifier key suppresses inference
  - Tests: T-01 `Sketch.Tests Inference.*`; T-05 `Ui.Tests SketchMode.InferenceSuppressedWithModifier`
  - Done when: tests green
- [ ] **S1.07.3: Colour entities by state and show remaining DOF**
  - Requirements: M-SKT-02
  - Modules: ui
  - Depends on: S1.07.1, S1.03.5
  - Deliverables: theme tokens for constraint states; DOF in the status bar
  - Tests: T-06 `Ui.Tests Snapshot.SketchStates`
  - Done when: tests green
- [ ] **S1.07.4: Edit dimensions in the canvas by double-click**
  - Requirements: U-07, U-06
  - Modules: ui
  - Depends on: S1.07.3, S0.13.8
  - Deliverables: in-canvas `ExpressionEdit` overlay (Enter commits, Esc cancels)
  - Tests: T-05 `Ui.Tests SketchMode.DoubleClickDimensionEdits`
  - Done when: tests green
- [ ] **S1.07.5: Add the parameter table panel**
  - Requirements: M-SKT-04, U-06
  - Modules: ui
  - Depends on: S1.06.3, S0.13.2
  - Deliverables: `src/ui/panels/ParametersPanel.{h,cpp}` (where used, status, CSV/JSON import/export)
  - Tests: T-05 `Ui.Tests Parameters.EditAndCycleMessage`
  - Done when: tests green
- [ ] **S1.07.6: Present conflicts in the message panel with fix actions**
  - Requirements: M-SKT-02, U-11
  - Modules: ui
  - Depends on: S1.07.3, S0.13.10
  - Deliverables: conflict entries listing the minimal set with "delete" and "make driven" actions
  - Tests: T-05 `Ui.Tests SketchMode.ConflictOffersFixes`
  - Done when: tests green
- [ ] **S1.07.7: Record sketch UI replay tests**
  - Requirements: T-05, M-SKT-01, M-SKT-02
  - Modules: tests
  - Depends on: S1.07.6, S1.07.4
  - Deliverables: `tests/ui/replays/sketch_profile.journal.jsonl`
  - Tests: T-05 `Ui.Tests Replay.SketchProfileDrawConstrainDrag`
  - Done when: replay green and deterministic

### S1.08 Hosted continuous integration (open decision)

- [ ] **S1.08.1: Add hosted CI once the owner decides (open)**
  - Requirements: T-13
  - Modules: scripts, build
  - Depends on: S0.15.1
  - Deliverables: ADR 0033 updated to Accepted with the chosen service; pipeline definition that runs `scripts/ci-local.ps1` on Windows with a vcpkg binary cache
  - Tests: T-13 pipeline green on the main branch
  - Done when: hosted CI green; if still undecided at the S1 gate, strike this step with the reason and re-insert it in the next stage

### S1.99 Stage gate

- [ ] **S1.99.1: Run the full S1 test suite and all checks**
  - Requirements: T-13, T-11, P-03
  - Modules: all
  - Depends on: S1.07.7, S1.06.4, S1.05.3, S1.03.11
  - Deliverables: `scripts/ci-local.ps1 -Nightly` results (runs `scripts/test.ps1 -Coverage`, `-Bench`, `-Fuzz` and `scripts/check-selftest.ps1`); offline smoke
  - Tests: T-13 all suites; T-12 coverage ≥ 80 % for `sketch`
  - Done when: everything green
- [ ] **S1.99.2: Run the S1 benchmarks (P-13, regression of P-10 and P-17)**
  - Requirements: P-13, T-08
  - Modules: tests
  - Depends on: S1.99.1
  - Deliverables: `bench/history.csv` rows
  - Tests: T-08 `Bench.SketchDrag500x800`, `Bench.ColdStartToEmptyWindow`, `Bench.SaveLoadProject`
  - Done when: results recorded
- [ ] **S1.99.3: Update traceability and documentation for S1**
  - Requirements: T-13
  - Modules: docs
  - Depends on: S1.99.2
  - Deliverables: `docs/traceability.csv`, `docs/stages/S1.md`, `docs/format/sketch.md`, `docs/commands/`
  - Tests: `/check-traceability` reports no gaps for S1; `scripts/docs.ps1 -FailOnWarning` clean (needs Doxygen, see S0.14.1)
  - Done when: S1 requirements updated
- [ ] **S1.99.4: Human review checkpoint: stop and wait for approval**
  - Requirements: all S1 requirements (see `docs/stages/S1.md`)
  - Modules: none
  - Depends on: S1.99.3
  - Deliverables: S1 summary for the owner
  - Tests: none (review)
  - Done when: the owner approves S1 in writing

## Stage S2: Part modeling core

Stage plan: `docs/stages/S2.md`. Exit: the 30 golden parts rebuild with expected mass properties; editing an early
feature regenerates downstream features with correct topological re-referencing (A-06).

### S2.01 Part document and feature framework

- [ ] **S2.01.1: Complete the part document schema**
  - Requirements: M-PRT-04, F-01, F-05
  - Modules: part, doc, docs
  - Depends on: S1.99.4
  - Deliverables: `docs/format/schemas/part.schema.json` and `docs/format/part.md` (reference geometry, sketches, ordered features, bodies, material, folders); `formatVersion` bump with migration from the S0 part schema
  - Tests: T-03 `Format.Tests Migration.Part_S0_to_S2`; `RoundTrip` over part goldens
  - Done when: tests green
- [ ] **S2.01.2: Implement the feature base, bodies, feature scope and error state**
  - Requirements: M-PRT-01, P-21, A-07
  - Modules: part, geom
  - Depends on: S2.01.1
  - Deliverables: `src/part/{Feature,Body,FeatureScope,RegenContext}.{h,cpp}` (boss/cut, new body/merge, scope selects bodies; failing feature keeps definition, enters Error, downstream OutOfDate)
  - Tests: T-01 `Part.Tests FeatureState.KeepsDefinitionOnFailure`, `FeatureScope.*`
  - Done when: tests green
- [ ] **S2.01.3: Generate TopoRefs from feature construction history**
  - Requirements: A-06
  - Modules: geom, part
  - Depends on: S2.01.2, S0.09.7
  - Deliverables: `src/geom/naming/RoleRecorder.{h,cpp}` (records generation roles for the kernel operations used by features, per ADR 0035)
  - Tests: T-01 `Geom.Tests RoleRecorder.*`
  - Done when: tests green

### S2.02 Reference geometry

- [ ] **S2.02.1: Add reference planes**
  - Requirements: M-PRT-10
  - Modules: part, geom
  - Depends on: S2.01.3
  - Deliverables: `src/part/features/RefPlaneFeature.{h,cpp}` (offset, angled, 3 points, normal to curve, mid-plane, tangent); command `part.ref.plane.create`
  - Tests: T-01 `Part.Tests RefPlane.*`
  - Done when: tests green
- [ ] **S2.02.2: Add reference axes, points and coordinate systems**
  - Requirements: M-PRT-10
  - Modules: part, geom
  - Depends on: S2.02.1
  - Deliverables: commands `part.ref.axis.create` (two planes, two points, cylinder axis, edge), `part.ref.point.create` (centre, intersection, on curve at distance or percentage), `part.ref.csys.create`
  - Tests: T-01 `Part.Tests RefAxis.*`, `RefPoint.*`, `RefCsys.*`
  - Done when: tests green
- [ ] **S2.02.3: Golden tests for every reference-geometry creation method**
  - Requirements: M-PRT-10
  - Modules: tests
  - Depends on: S2.02.2
  - Deliverables: `tests/data/scripts/refgeom_*.cmd.json` (one per method)
  - Tests: T-04 `Golden.Tests RefGeometry.*` (Accept: M-PRT-10)
  - Done when: all methods covered and green

### S2.03 Solid features

- [ ] **S2.03.1: Extrude: blind, through all, mid-plane, second direction, start offset**
  - Requirements: M-PRT-01
  - Modules: part, geom
  - Depends on: S2.01.3, S1.04.9
  - Deliverables: `src/part/features/ExtrudeFeature.{h,cpp}`; command `part.feature.extrude.create`
  - Tests: T-04 `Golden.Tests PartGolden.ExtrudeBasic*`
  - Done when: tests green; mass properties within 1e-6 of oracles
- [ ] **S2.03.2: Extrude end conditions up to next, vertex, surface and offset from surface**
  - Requirements: M-PRT-01
  - Modules: part, geom
  - Depends on: S2.03.1, S2.02.2
  - Deliverables: end-condition parameters on `part.feature.extrude.create`
  - Tests: T-04 `Golden.Tests PartGolden.ExtrudeUpTo*`; T-02 `extrude_end_conditions.cmd.json`
  - Done when: tests green
- [ ] **S2.03.3: Extrude draft angle and thin-feature option**
  - Requirements: M-PRT-01
  - Modules: part, geom
  - Depends on: S2.03.1
  - Deliverables: draft and thin parameters
  - Tests: T-04 `Golden.Tests PartGolden.ExtrudeDraftThin*`
  - Done when: tests green
- [ ] **S2.03.4: Revolve with angle, mid-plane, two directions and thin option**
  - Requirements: M-PRT-01
  - Modules: part, geom
  - Depends on: S2.03.1
  - Deliverables: `src/part/features/RevolveFeature.{h,cpp}`; command `part.feature.revolve.create`
  - Tests: T-04 `Golden.Tests PartGolden.Revolve*`
  - Done when: tests green
- [ ] **S2.03.5: Sweep along 2D or 3D paths with guides, twist and thin option**
  - Requirements: M-PRT-01, M-SKT-03
  - Modules: part, geom
  - Depends on: S2.03.1, S1.05.3
  - Deliverables: `src/part/features/SweepFeature.{h,cpp}`; command `part.feature.sweep.create`
  - Tests: T-04 `Golden.Tests PartGolden.Sweep*`; `Golden.Tests Sketch3d.HandrailDrivesSweep` (Accept: M-SKT-03b)
  - Done when: tests green
- [ ] **S2.03.6: Loft through profiles with guides, centreline and end tangency**
  - Requirements: M-PRT-01
  - Modules: part, geom
  - Depends on: S2.03.1
  - Deliverables: `src/part/features/LoftFeature.{h,cpp}`; command `part.feature.loft.create`
  - Tests: T-04 `Golden.Tests PartGolden.Loft*`
  - Done when: tests green
- [ ] **S2.03.7: Boundary feature from curves in two directions (should in S2)**
  - Requirements: M-PRT-01
  - Modules: part, geom
  - Depends on: S2.03.6
  - Deliverables: `src/part/features/BoundaryFeature.{h,cpp}`; command `part.feature.boundary.create` (tangency and curvature conditions)
  - Tests: T-04 `Golden.Tests PartGolden.Boundary*`
  - Done when: tests green, or the slip to S8 is recorded in an ADR (must by S8, see S8.02.6)
- [ ] **S2.03.8: Turn invalid feature inputs into feature errors, never crashes**
  - Requirements: M-PRT-01, P-21
  - Modules: part
  - Depends on: S2.03.2, S2.03.4, S2.03.5, S2.03.6
  - Deliverables: input validation and kernel-failure mapping for every solid feature
  - Tests: T-04 `Golden.Tests PartErrors.OpenProfile`, `PartErrors.SelfIntersectingResult` (Accept: M-PRT-01b)
  - Done when: tests green; feature stays editable after the error
- [ ] **S2.03.9: Golden parts for every solid feature type and end condition**
  - Requirements: M-PRT-01, T-04
  - Modules: tests
  - Depends on: S2.03.8, S2.03.3, S2.03.7
  - Deliverables: `tests/data/scripts/solid_*.cmd.json` with expected volume, area, centre of mass, bounding box, topology counts, validity
  - Tests: T-04 `Golden.Tests PartGolden.SolidFeatureMatrix` (Accept: M-PRT-01a)
  - Done when: every feature type and end condition covered and green

### S2.04 Applied features

- [ ] **S2.04.1: Constant-radius fillet with tangent propagation and setback**
  - Requirements: M-PRT-02
  - Modules: part, geom
  - Depends on: S2.03.1
  - Deliverables: `src/part/features/FilletFeature.{h,cpp}`; command `part.feature.fillet.create`
  - Tests: T-04 `Golden.Tests PartGolden.FilletConstant*`
  - Done when: tests green; kernel failures become feature errors
- [ ] **S2.04.2: Variable, face and full-round fillets**
  - Requirements: M-PRT-02
  - Modules: part, geom
  - Depends on: S2.04.1
  - Deliverables: fillet variants
  - Tests: T-04 `Golden.Tests PartGolden.FilletVariable*`, `FilletFace*`, `FilletFullRound*`
  - Done when: tests green
- [ ] **S2.04.3: Chamfer distance-distance, angle-distance and vertex**
  - Requirements: M-PRT-02
  - Modules: part, geom
  - Depends on: S2.04.1
  - Deliverables: `src/part/features/ChamferFeature.{h,cpp}`
  - Tests: T-04 `Golden.Tests PartGolden.Chamfer*`
  - Done when: tests green
- [ ] **S2.04.4: Shell with per-face thickness, inward or outward**
  - Requirements: M-PRT-02
  - Modules: part, geom
  - Depends on: S2.03.1
  - Deliverables: `src/part/features/ShellFeature.{h,cpp}`
  - Tests: T-04 `Golden.Tests PartGolden.Shell*`
  - Done when: tests green
- [ ] **S2.04.5: Draft by neutral plane and parting line**
  - Requirements: M-PRT-02
  - Modules: part, geom
  - Depends on: S2.03.1
  - Deliverables: `src/part/features/DraftFeature.{h,cpp}`
  - Tests: T-04 `Golden.Tests PartGolden.Draft*`
  - Done when: tests green
- [ ] **S2.04.6: Rib from an open sketch**
  - Requirements: M-PRT-02
  - Modules: part, geom
  - Depends on: S2.03.1
  - Deliverables: `src/part/features/RibFeature.{h,cpp}` (thickness, direction, draft)
  - Tests: T-04 `Golden.Tests PartGolden.Rib*`
  - Done when: tests green
- [ ] **S2.04.7: Seed hole and thread tables with recorded sources**
  - Requirements: M-PRT-02, CR-05, F-07
  - Modules: libraries, docs
  - Depends on: S0.07.3
  - Deliverables: `libraries/hole-tables/` (ISO metric coarse/fine M1.6–M24, UNC/UNF #2–1/2", clearance close/normal/loose) from public factual sources; rows in `docs/data-sources.md`
  - Tests: T-11 `check-clean-room.ps1` data-source rule; T-01 `Doc.Tests HoleTables.LoadAndResolve`
  - Done when: tables load through the resolver; every table has a source row
- [ ] **S2.04.8: Hole tool for all hole types with semantic metadata**
  - Requirements: M-PRT-02
  - Modules: part, geom
  - Depends on: S2.04.7, S2.03.2
  - Deliverables: `src/part/features/HoleFeature.{h,cpp}` (simple, counterbore, countersink, counterdrill, tapped, pipe-tapped; placed by sketch points; M-PRT-01 end conditions; semantic hole metadata kept for CAM and fasteners)
  - Tests: T-04 `Golden.Tests PartGolden.Hole*`; `Golden.Tests Hole.M6TappedMatchesTable` (Accept: M-PRT-02b)
  - Done when: M6 hole has the tabulated tap-drill diameter and thread metadata
- [ ] **S2.04.9: Cosmetic threads (modelled helical threads as a should)**
  - Requirements: M-PRT-02
  - Modules: part, render
  - Depends on: S2.04.8
  - Deliverables: thread metadata (designation, length, class) on faces; texture display; optional helical thread feature
  - Tests: T-01 `Part.Tests CosmeticThread.*`
  - Done when: tests green
- [ ] **S2.04.10: Golden tests for every applied feature**
  - Requirements: M-PRT-02, T-04
  - Modules: tests
  - Depends on: S2.04.2, S2.04.3, S2.04.4, S2.04.5, S2.04.6, S2.04.9
  - Deliverables: `tests/data/scripts/applied_*.cmd.json`
  - Tests: T-04 `Golden.Tests PartGolden.AppliedFeatureMatrix` (Accept: M-PRT-02a)
  - Done when: all applied features covered and green

### S2.05 Patterns and mirrors

- [ ] **S2.05.1: Linear and circular feature patterns with skipped instances**
  - Requirements: M-PRT-03
  - Modules: part
  - Depends on: S2.03.1
  - Deliverables: `src/part/features/PatternFeature.{h,cpp}`; commands `part.feature.patternLinear.create`, `part.feature.patternCircular.create`
  - Tests: T-04 `Golden.Tests Patterns.Linear*`, `Circular*`
  - Done when: tests green
- [ ] **S2.05.2: Curve-driven, sketch-driven and table-driven patterns**
  - Requirements: M-PRT-03
  - Modules: part
  - Depends on: S2.05.1
  - Deliverables: pattern variants; CSV coordinate import
  - Tests: T-04 `Golden.Tests Patterns.Curve*`, `Sketch*`, `Table*`
  - Done when: tests green
- [ ] **S2.05.3: Variable patterns and geometry-pattern mode**
  - Requirements: M-PRT-03
  - Modules: part
  - Depends on: S2.05.2
  - Deliverables: per-instance dimension changes from a table; fast geometry-pattern mode
  - Tests: T-04 `Golden.Tests Patterns.Variable10Instances`
  - Done when: tests green
- [ ] **S2.05.4: Mirror features, faces and bodies; pattern faces and bodies**
  - Requirements: M-PRT-03
  - Modules: part
  - Depends on: S2.05.1
  - Deliverables: `src/part/features/MirrorFeature.{h,cpp}`
  - Tests: T-04 `Golden.Tests Patterns.Mirror*`
  - Done when: tests green
- [ ] **S2.05.5: Golden pattern suite**
  - Requirements: M-PRT-03
  - Modules: tests
  - Depends on: S2.05.3, S2.05.4
  - Deliverables: `tests/data/scripts/pattern_*.cmd.json`
  - Tests: T-04 `Golden.Tests Patterns.Suite` incl. skipped instances and a 10-instance variable pattern (Accept: M-PRT-03)
  - Done when: green

### S2.06 Topological naming robustness

- [ ] **S2.06.1: Record roles and signatures for every S2 feature**
  - Requirements: A-06
  - Modules: part, geom
  - Depends on: S2.04.10, S2.05.5
  - Deliverables: role tables per feature type; signature re-matching with ambiguity → "reference lost"
  - Tests: T-01 `Part.Tests TopoRoles.*`
  - Done when: every feature output face/edge has a role
- [ ] **S2.06.2: Run the 20 classic breakage cases**
  - Requirements: A-06, T-04
  - Modules: tests
  - Depends on: S2.06.1
  - Deliverables: `tests/data/scripts/toponaming_01..20.cmd.json` (e.g. add a hole before a fillet, 4 → 6 sided extrude sketch)
  - Tests: T-04 `Golden.Tests TopoNaming.Case01..20` (Accept: A-06)
  - Done when: each case re-resolves correctly or flags "reference lost"; zero wrong attachments

### S2.07 Feature tree operations

- [ ] **S2.07.1: Edit, rename, suppress and delete with dependants**
  - Requirements: M-PRT-04
  - Modules: part
  - Depends on: S2.01.2
  - Deliverables: commands `part.feature.edit`, `.rename`, `.suppress`, `.unsuppress`, `.delete` (lists dependants, optional cascade)
  - Tests: T-01 `Part.Tests FeatureTree.DeleteListsDependants`
  - Done when: tests green
- [ ] **S2.07.2: Reorder with dependency validation, folders and rollback insertion**
  - Requirements: M-PRT-04
  - Modules: part
  - Depends on: S2.07.1
  - Deliverables: commands `part.feature.reorder` (refused with explanation if it breaks dependencies), `part.folder.*`, `part.rollback.set`
  - Tests: T-01 `Part.Tests FeatureTree.ReorderRefusedWithExplanation` (Accept: M-PRT-04b)
  - Done when: tests green
- [ ] **S2.07.3: Parent/child queries and the "what's wrong" list**
  - Requirements: M-PRT-04, A-07
  - Modules: part
  - Depends on: S2.07.1
  - Deliverables: dependency queries both directions; error/warning list with object IDs
  - Tests: T-01 `Part.Tests FeatureTree.ParentChild*`
  - Done when: tests green

### S2.08 Materials and mass properties

- [ ] **S2.08.1: Define the material schema and seed materials and appearances**
  - Requirements: M-PRT-09, CR-05, F-07
  - Modules: part, libraries, docs
  - Depends on: S0.07.3
  - Deliverables: `docs/format/schemas/material.schema.json` (density, E, ν, yield, UTS, k, cp, α, appearance; FEA seam); `libraries/materials/`, `libraries/appearances/` starter set; `docs/data-sources.md` rows
  - Tests: T-01 `Part.Tests Materials.SeedLoads`; T-11 data-source check
  - Done when: tests green
- [ ] **S2.08.2: Assign materials and compute mass properties about any frame**
  - Requirements: M-PRT-09
  - Modules: part, geom
  - Depends on: S2.08.1, S0.09.2
  - Deliverables: commands `part.material.assign` (part or body), `part.massProperties.compute` (mass, volume, area, centre of mass, inertia tensor, principal moments and axes), `part.mass.override`
  - Tests: T-04 `Golden.Tests MassProps.PrimitivesWithin1e-6` (Accept: M-PRT-09a)
  - Done when: tests green
- [ ] **S2.08.3: Axis-aligned and minimum-volume oriented bounding boxes as parameters**
  - Requirements: M-PRT-09
  - Modules: part, geom
  - Depends on: S2.08.2
  - Deliverables: bounding-box parameters exposed for expressions
  - Tests: T-04 `Golden.Tests BoundingBox.*`
  - Done when: tests green
- [ ] **S2.08.4: Build the 30 golden parts with reference mass properties**
  - Requirements: M-PRT-09, T-04
  - Modules: tests
  - Depends on: S2.08.3, S2.06.2
  - Deliverables: `tests/data/scripts/golden_part_01..30.cmd.json`; references computed independently and hand-checked
  - Tests: T-04 `Golden.Tests GoldenParts.MassWithin1e-4` (Accept: M-PRT-09b)
  - Done when: all 30 green (S2 exit criterion)

### S2.09 STEP exchange and basic 3D-print export

- [ ] **S2.09.1: Export parts as STEP with names and colours**
  - Requirements: M-IO-01
  - Modules: io, geom
  - Depends on: S2.08.2, S0.10.2
  - Deliverables: `src/io/step/StepExporter.{h,cpp}` (AP214/AP242 through `geom`); command `io.export.step`
  - Tests: T-07 `Io.Tests StepExport.*` (structural parse)
  - Done when: tests green
- [ ] **S2.09.2: Import STEP with checking, healing and background jobs**
  - Requirements: M-IO-01, A-10
  - Modules: io, geom
  - Depends on: S2.09.1
  - Deliverables: `src/io/step/StepImporter.{h,cpp}`; heal report (sewing, small-edge removal); command `io.import.step` as a job
  - Tests: T-07 `Io.Tests StepImport.*`; `StepRoundTrip.PartMassPreserved` (Accept: M-IO-01a)
  - Done when: tests green
- [ ] **S2.09.3: Import 20 third-party STEP files as valid solids**
  - Requirements: M-IO-01
  - Modules: tests
  - Depends on: S2.09.2
  - Deliverables: `tests/data/import/step/` (20 files with licence notes and sources in `docs/data-sources.md`)
  - Tests: T-07 `Golden.Tests StepImport.ThirdParty01..20` (Accept: M-IO-01b)
  - Done when: all 20 import valid
- [ ] **S2.09.4: Export 3MF through lib3mf**
  - Requirements: M-3DP-01
  - Modules: io, build
  - Depends on: S0.10.2
  - Deliverables: lib3mf in `vcpkg.json` (ADR 0011 licence check); `src/io/threemf/ThreeMfExporter.{h,cpp}` (units, objects); command `io.export.3mf`
  - Tests: T-07 `Io.Tests ThreeMf.BoxExport`
  - Done when: tests green; only `io` links lib3mf
- [ ] **S2.09.5: Validate meshes and control tessellation before export**
  - Requirements: M-3DP-01
  - Modules: io, geom
  - Depends on: S2.09.4, S2.08.4
  - Deliverables: tessellation controls (chordal, angular, max edge length); per-body validation report with "export anyway"
  - Tests: T-07 `Golden.Tests Print.GoldenPartsWatertight` (Accept: M-3DP-01a); `Print.MeshVolumeWithin0_1Percent` (Accept: M-3DP-01b); `Print.ThreeMfValidatesWithLib3mf` (Accept: M-3DP-01c)
  - Done when: tests green for all 30 golden parts

### S2.10 Part performance

- [ ] **S2.10.1: Meet P-12 (200-feature part rebuild under 2 s)**
  - Requirements: P-12, A-07
  - Modules: part, tests
  - Depends on: S2.08.4
  - Deliverables: generated 200-feature part script; `Bench.Rebuild200Features`; caching of unchanged feature results
  - Tests: T-08 `Bench.Rebuild200Features` median under 2 s
  - Done when: target met or a defect recorded per spec 3.2

### S2.11 Part user interface

- [ ] **S2.11.1: Feature tree panel with states, folders, rollback bar and drag reorder**
  - Requirements: M-PRT-04, U-03
  - Modules: ui
  - Depends on: S2.07.3, S0.13.2
  - Deliverables: `src/ui/panels/FeatureTreePanel.{h,cpp}`
  - Tests: T-05 `Ui.Tests FeatureTree.StatesShown`
  - Done when: tests green
- [ ] **S2.11.2: Feature property panels with live preview**
  - Requirements: U-07, M-PRT-01, M-PRT-02
  - Modules: ui, part
  - Depends on: S2.11.1, S2.04.10
  - Deliverables: preview mode that regenerates a temporary copy on a job; commit runs the command
  - Tests: T-05 `Ui.Tests FeatureDialogs.PreviewDoesNotChangeDocument`
  - Done when: tests green
- [ ] **S2.11.3: "What's wrong" panel and parent/child view**
  - Requirements: M-PRT-04, U-11
  - Modules: ui
  - Depends on: S2.11.1
  - Deliverables: panels with click-to-locate
  - Tests: T-05 `Ui.Tests FeatureTree.WhatsWrongLocates`
  - Done when: tests green
- [ ] **S2.11.4: UI replay tests for reorder, rollback insert and delete with dependants**
  - Requirements: M-PRT-04, T-05
  - Modules: tests
  - Depends on: S2.11.3, S2.07.2
  - Deliverables: `tests/ui/replays/feature_tree_*.journal.jsonl`
  - Tests: T-05 `Ui.Tests Replay.FeatureTreeReorder`, `Replay.RollbackInsert`, `Replay.DeleteWithDependants` (Accept: M-PRT-04a)
  - Done when: replays green
- [ ] **S2.11.5: Material and mass-properties panels**
  - Requirements: M-PRT-09, U-06
  - Modules: ui
  - Depends on: S2.08.3, S0.13.8
  - Deliverables: material picker (scope shown), mass-properties report with frame choice
  - Tests: T-05 `Ui.Tests MassProps.ReportShown`
  - Done when: tests green
- [ ] **S2.11.6: Export dialogs for STEP, STL and 3MF with tessellation preview**
  - Requirements: M-3DP-01, M-IO-01
  - Modules: ui
  - Depends on: S2.09.5, S2.09.2
  - Deliverables: export dialog with live triangle count and validation report
  - Tests: T-05 `Ui.Tests Export.TriangleCountUpdates`
  - Done when: tests green

### S2.99 Stage gate

- [ ] **S2.99.1: Run the full S2 test suite and all checks**
  - Requirements: T-13, T-11, P-03
  - Modules: all
  - Depends on: S2.11.6, S2.11.4, S2.10.1
  - Deliverables: `scripts/ci-local.ps1 -Nightly` results (runs `scripts/test.ps1 -Coverage`, `-Bench`, `-Fuzz` and `scripts/check-selftest.ps1`); offline smoke
  - Tests: T-13 all suites incl. 30 golden parts and the 20 naming cases
  - Done when: everything green
- [ ] **S2.99.2: Run the S2 benchmarks (P-12, regressions of P-10, P-13, P-17)**
  - Requirements: P-12, T-08
  - Modules: tests
  - Depends on: S2.99.1
  - Deliverables: `bench/history.csv` rows
  - Tests: T-08 `Bench.Rebuild200Features` and earlier benchmarks
  - Done when: results recorded
- [ ] **S2.99.3: Update traceability and documentation for S2**
  - Requirements: T-13
  - Modules: docs
  - Depends on: S2.99.2
  - Deliverables: `docs/traceability.csv`, `docs/stages/S2.md`, `docs/format/part.md`, `docs/commands/`, `docs/data-sources.md`
  - Tests: `/check-traceability` reports no gaps for S2; `scripts/docs.ps1 -FailOnWarning` clean (needs Doxygen, see S0.14.1)
  - Done when: S2 requirements updated
- [ ] **S2.99.4: Human review checkpoint: stop and wait for approval**
  - Requirements: all S2 requirements (see `docs/stages/S2.md`)
  - Modules: none
  - Depends on: S2.99.3
  - Deliverables: S2 summary for the owner
  - Tests: none (review)
  - Done when: the owner approves S2 in writing

## Stage S3: Assemblies core

Stage plan: `docs/stages/S3.md`. Exit: golden mechanisms show the expected DOF; an in-context part follows a changed
reference part; interference detection finds the seeded clashes; exploded view and keyframe animation export frames.

### S3.01 Mate solver spike (SP-4)

- [ ] **S3.01.1: Apply ADR 0022 for the assembly module (Eigen in asm)**
  - Requirements: A-01
  - Modules: asm, scripts
  - Depends on: S2.99.4
  - Deliverables: rule `030`, `scripts/check-layering.ps1` and `Asm.vcxproj` updated
  - Tests: T-11 `check-layering.ps1` green
  - Done when: checks green
- [ ] **S3.01.2: Spike SP-4a: prototype a rigid-cluster mate solver on the 15 golden mechanisms (time box 2 days)**
  - Requirements: M-ASM-01, M-ASM-07
  - Modules: asm, tests
  - Depends on: S3.01.1
  - Deliverables: prototype under `tests/asm/spikes/` with mechanisms as abstract rigid bodies and mate equations (slider-crank, four-bar, gear pair, rack and pinion, cam, screw jack, hinge, universal joint and variants)
  - Tests: spike harness: DOF per mechanism, drag stability over 1,000 steps, solve time
  - Done when: all 15 measured, or time box ends with results recorded
- [ ] **S3.01.3: Spike SP-4b: decide the mate solver design in ADR 0037**
  - Requirements: M-ASM-01
  - Modules: asm, docs
  - Depends on: S3.01.2
  - Deliverables: `docs/adr/0037-spike-sp4-mate-solver.md`
  - Tests: spike results attached
  - Done when: ADR Accepted before S3.03.2

### S3.02 Assembly document and components

- [ ] **S3.02.1: Define the assembly document schema**
  - Requirements: M-ASM-01, F-01
  - Modules: asm, doc, docs
  - Depends on: S2.99.4
  - Deliverables: `docs/format/schemas/assembly.schema.json`, `docs/format/assembly.md` (components with source document, configuration, transform, fixed/floating, load state; mates; exploded views; motion studies; configurations)
  - Tests: T-03 `Format.Tests RoundTrip` over assembly goldens
  - Done when: tests green
- [ ] **S3.02.2: Insert, move and fix components; component geometry provider**
  - Requirements: M-ASM-01, A-11
  - Modules: asm
  - Depends on: S3.02.1
  - Deliverables: commands `asm.component.insert`, `.move`, `.fix`, `.float`, `.remove`; `app::assembly::IComponentGeometryProvider` registry (parts now, boards in S6 per ADR 0025)
  - Tests: T-01 `Asm.Tests Components.*`
  - Done when: tests green
- [ ] **S3.02.3: Lay lightweight-mode foundations (cached mesh, bounds, mass)**
  - Requirements: M-ASM-03, F-04
  - Modules: asm, doc
  - Depends on: S3.02.2
  - Deliverables: per-component cache entries (display mesh, bounding volume, mass properties) in `cache/`; B-rep resolved on demand
  - Tests: T-01 `Asm.Tests Lightweight.OpensFromCache`, `Lightweight.ResolvesOnDemand`
  - Done when: tests green; deleting the cache only costs time
- [ ] **S3.02.4: Spike SP-8a: measure a generated 2,000-part assembly early (time box 1 day)**
  - Requirements: M-ASM-03, P-14, P-11
  - Modules: asm, render, tests
  - Depends on: S3.02.3, S0.12.2
  - Deliverables: generator script `tests/data/scripts/generated_2000_parts.cmd.json`; `Bench.OpenLightweight2000`, `Bench.Viewport5MTriangles`; interim note in `docs/stages/S3.md` (ADR 0041 is written in S10.01.2)
  - Tests: T-08 benchmarks recorded
  - Done when: baseline numbers and memory recorded

### S3.03 Mates and mate solver

- [ ] **S3.03.1: Model standard mates on TopoRef geometry**
  - Requirements: M-ASM-01, A-06
  - Modules: asm, geom
  - Depends on: S3.02.2, S2.06.2
  - Deliverables: coincident, parallel, perpendicular, tangent, concentric, distance, angle, lock with aligned/anti-aligned flip; command `asm.mate.add`
  - Tests: T-01 `Asm.Tests Mates.Standard*`
  - Done when: tests green
- [ ] **S3.03.2: Implement the mate solver (ADR 0037)**
  - Requirements: M-ASM-01
  - Modules: asm
  - Depends on: S3.03.1, S3.01.3
  - Deliverables: `src/asm/solver/{MateSolver,RigidClusters}.{h,cpp}` (DOF per component; last solved pose kept on failure)
  - Tests: T-10 `Asm.Tests MateSolver.*`
  - Done when: tests green
- [ ] **S3.03.3: Report over-defined and conflicting mates with a minimal set**
  - Requirements: M-ASM-01
  - Modules: asm
  - Depends on: S3.03.2
  - Deliverables: conflict finder; messages with object IDs
  - Tests: T-10 `Asm.Tests MateConflicts.MinimalSet`
  - Done when: tests green
- [ ] **S3.03.4: Add advanced mates**
  - Requirements: M-ASM-01
  - Modules: asm
  - Depends on: S3.03.2
  - Deliverables: symmetric, width, path, linear coupler, limit distance, limit angle, profile centre, slot
  - Tests: T-01 `Asm.Tests Mates.Advanced*`
  - Done when: tests green
- [ ] **S3.03.5: Add mechanical mates**
  - Requirements: M-ASM-01
  - Modules: asm
  - Depends on: S3.03.2
  - Deliverables: gear (ratio), rack and pinion, cam follower, screw (pitch), hinge, universal joint
  - Tests: T-01 `Asm.Tests Mates.Mechanical*`
  - Done when: tests green
- [ ] **S3.03.6: Drag components while respecting mates**
  - Requirements: M-ASM-01
  - Modules: asm
  - Depends on: S3.03.2
  - Deliverables: command `asm.component.drag` (incremental solve from current pose, merged drag commands)
  - Tests: T-10 `Asm.Tests Drag.*`
  - Done when: tests green
- [ ] **S3.03.7: Verify the 15 golden mechanisms (DOF and drag replay)**
  - Requirements: M-ASM-01, T-04
  - Modules: tests
  - Depends on: S3.03.3, S3.03.4, S3.03.5, S3.03.6
  - Deliverables: `tests/data/scripts/mechanism_01..15.cmd.json` and drag journals
  - Tests: T-04 `Golden.Tests Mechanisms.DofAndDragReplay01..15` (Accept: M-ASM-01)
  - Done when: all 15 report the expected DOF and move correctly in replay

### S3.04 Configurations

- [ ] **S3.04.1: Add part configurations**
  - Requirements: M-PRT-11
  - Modules: part, docs
  - Depends on: S2.99.4
  - Deliverables: configuration model varying parameters, suppression, material, custom properties; commands `part.config.*`; schema update and migration
  - Tests: T-01 `Part.Tests Configurations.*`
  - Done when: tests green
- [ ] **S3.04.2: Drive configurations from a table with CSV import and export**
  - Requirements: M-PRT-11
  - Modules: part, doc
  - Depends on: S3.04.1
  - Deliverables: configuration table model; derived configurations (should)
  - Tests: T-01 `Part.Tests ConfigTable.CsvRoundTrip`
  - Done when: tests green
- [ ] **S3.04.3: Add assembly configurations**
  - Requirements: M-PRT-11
  - Modules: asm
  - Depends on: S3.04.1, S3.03.2
  - Deliverables: component suppression, visibility, configuration-of-component and mate suppression per configuration
  - Tests: T-01 `Asm.Tests Configurations.*`
  - Done when: tests green
- [ ] **S3.04.4: Golden bolt with 12 configurations and an assembly configuration swap**
  - Requirements: M-PRT-11
  - Modules: tests
  - Depends on: S3.04.2, S3.04.3
  - Deliverables: `tests/data/scripts/bolt_12_configs.cmd.json`, `asm_config_swap.cmd.json`
  - Tests: T-04 `Golden.Tests Configs.Bolt12MassValues` (Accept: M-PRT-11a); `Configs.AssemblySwapsComponentConfigs` (Accept: M-PRT-11b)
  - Done when: tests green

### S3.05 Top-down (in-context) design

- [ ] **S3.05.1: Track external references when editing in context**
  - Requirements: M-ASM-02
  - Modules: asm, part
  - Depends on: S3.03.2
  - Deliverables: in-context edit session; external reference objects with status (OK, out of date, locked, broken), where used
  - Tests: T-01 `Asm.Tests ExternalRefs.*`
  - Done when: tests green
- [ ] **S3.05.2: Lock and break external references**
  - Requirements: M-ASM-02
  - Modules: asm
  - Depends on: S3.05.1
  - Deliverables: commands `asm.extref.lock`, `.unlock`, `.break`, `asm.extref.list`
  - Tests: T-01 `Asm.Tests ExternalRefs.LockStopsUpdate`
  - Done when: tests green
- [ ] **S3.05.3: Layout sketches and skeleton parts**
  - Requirements: M-ASM-02
  - Modules: asm, sketch
  - Depends on: S3.05.1
  - Deliverables: assembly-level sketch; skeleton part driving several parts; assembly features (should)
  - Tests: T-02 `Integration.Tests Scripts.skeleton_drives_parts`
  - Done when: tests green
- [ ] **S3.05.4: Golden in-context lid**
  - Requirements: M-ASM-02
  - Modules: tests
  - Depends on: S3.05.2, S3.05.3
  - Deliverables: `tests/data/scripts/incontext_lid.cmd.json`
  - Tests: T-02 `Integration.Tests InContext.LidFollowsBaseRim` (Accept: M-ASM-02a); `InContext.LockStopsUpdate` (Accept: M-ASM-02b); `InContext.ExternalRefListStatus` (Accept: M-ASM-02c)
  - Done when: tests green (S3 exit criterion)
- [ ] **S3.05.5: Project parameter drives every dependent part and assembly**
  - Requirements: M-SKT-04
  - Modules: tests
  - Depends on: S3.05.4, S1.06.4
  - Deliverables: `tests/data/scripts/project_wall_thickness.cmd.json`
  - Tests: T-02 `Integration.Tests Params.WallThicknessRegeneratesAll` (Accept: M-SKT-04a)
  - Done when: test green

### S3.06 Interference and clearance

- [ ] **S3.06.1: Detect volume interferences between components**
  - Requirements: M-ASM-04
  - Modules: asm, geom
  - Depends on: S3.03.2
  - Deliverables: `src/asm/InterferenceChecker.{h,cpp}` (pairs, interference volume, overlap shape; options: coincident faces, hidden components, cosmetic threads, multibody); runs as a job; command `asm.check.interference`
  - Tests: T-01 `Asm.Tests Interference.*`
  - Done when: tests green
- [ ] **S3.06.2: Clearance and hole-alignment checks**
  - Requirements: M-ASM-04
  - Modules: asm, geom
  - Depends on: S3.06.1
  - Deliverables: minimum-distance pairs under a threshold; hole axis deviation check using hole metadata
  - Tests: T-01 `Asm.Tests Clearance.*`, `HoleAlignment.*`
  - Done when: tests green
- [ ] **S3.06.3: Golden assembly with 6 interferences, 3 clearance violations and 2 misaligned holes**
  - Requirements: M-ASM-04
  - Modules: tests
  - Depends on: S3.06.2
  - Deliverables: `tests/data/scripts/interference_seeded.cmd.json`
  - Tests: T-04 `Golden.Tests Interference.SeededExactly` (Accept: M-ASM-04)
  - Done when: exactly the seeded findings are reported

### S3.07 Exploded views

- [ ] **S3.07.1: Model exploded views with steps and explode lines**
  - Requirements: M-ASM-06
  - Modules: asm
  - Depends on: S3.04.3
  - Deliverables: named exploded views per configuration; linear, radial, rotational steps; explode lines with jogs; auto-explode (should); commands `asm.explode.*`
  - Tests: T-01 `Asm.Tests Explode.*`
  - Done when: tests green
- [ ] **S3.07.2: Animate explode/collapse and export image sequences headless**
  - Requirements: M-ASM-06
  - Modules: asm, render, cli
  - Depends on: S3.07.1, S0.12.2
  - Deliverables: command `asm.explode.exportFrames` (PNG sequence through the off-screen renderer)
  - Tests: T-06 `Golden.Tests Explode.FrameSnapshots`
  - Done when: tests green
- [ ] **S3.07.3: Golden exploded view saves, reloads and replays identically**
  - Requirements: M-ASM-06
  - Modules: tests
  - Depends on: S3.07.2
  - Deliverables: `tests/data/scripts/exploded_view.cmd.json`
  - Tests: T-02 `Integration.Tests Explode.SaveReloadReplayIdentical` (Accept: M-ASM-06a)
  - Done when: test green

### S3.08 Motion timeline and kinematic playback

- [ ] **S3.08.1: Keyframe timeline for positions, camera, visibility and appearance**
  - Requirements: M-ASM-07
  - Modules: asm
  - Depends on: S3.03.6
  - Deliverables: `src/asm/motion/Timeline.{h,cpp}`; commands `asm.motion.keyframe.*`
  - Tests: T-01 `Asm.Tests Timeline.*`
  - Done when: tests green
- [ ] **S3.08.2: Kinematic playback with motors, springs and gravity**
  - Requirements: M-ASM-07
  - Modules: asm
  - Depends on: S3.08.1
  - Deliverables: rotary/linear motors (constant, expression or point-table profiles), linear springs, gravity through the mate solver; optional stop at first contact
  - Tests: T-10 `Asm.Tests Kinematics.*`
  - Done when: tests green
- [ ] **S3.08.3: Export animation frames (MP4 encoder as an optional seam)**
  - Requirements: M-ASM-07
  - Modules: asm, render, platform
  - Depends on: S3.08.2, S0.11.5
  - Deliverables: PNG sequence export; `IVideoEncoder` interface loaded dynamically when present; `docs/adr/NNNN-optional-video-encoder.md` (spec Q-08)
  - Tests: T-06 `Golden.Tests Motion.FrameSnapshots`
  - Done when: tests green; app works without any encoder installed
- [ ] **S3.08.4: Golden slider-crank driven at 60 rpm**
  - Requirements: M-ASM-07
  - Modules: tests
  - Depends on: S3.08.2
  - Deliverables: `tests/data/scripts/slider_crank_60rpm.cmd.json`; analytic slider position oracle in `tests/support/oracles/`
  - Tests: T-10 `Golden.Tests Motion.SliderCrankWithin0_1Percent` (Accept: M-ASM-07)
  - Done when: 100 sampled frames within 0.1 %

### S3.09 Assembly user interface

- [ ] **S3.09.1: Assembly tree, component insertion and mate creation UI**
  - Requirements: M-ASM-01, U-03
  - Modules: ui
  - Depends on: S3.03.7, S2.11.1
  - Deliverables: `src/ui/assembly/` (tree with components and mates, mate property panel with flip)
  - Tests: T-05 `Ui.Tests Assembly.CreateMate`
  - Done when: tests green
- [ ] **S3.09.2: Drag components in the viewport with mates**
  - Requirements: M-ASM-01, U-09
  - Modules: ui
  - Depends on: S3.09.1
  - Deliverables: drag tool calling `asm.component.drag`
  - Tests: T-05 `Ui.Tests Replay.DragSliderCrank`
  - Done when: tests green
- [ ] **S3.09.3: Interference results and configuration panels**
  - Requirements: M-ASM-04, M-PRT-11
  - Modules: ui
  - Depends on: S3.06.3, S3.04.4
  - Deliverables: interference panel with overlap highlight; configuration grid editor with CSV import/export
  - Tests: T-05 `Ui.Tests Interference.HighlightsPair`, `Configs.GridEdits`
  - Done when: tests green
- [ ] **S3.09.4: Exploded view editor and motion timeline panel**
  - Requirements: M-ASM-06, M-ASM-07
  - Modules: ui
  - Depends on: S3.07.3, S3.08.4
  - Deliverables: explode step editor, timeline panel with playback and export
  - Tests: T-05 `Ui.Tests Explode.EditSteps`, `Motion.PlayAndExport`
  - Done when: tests green
- [ ] **S3.09.5: Section view in the 3D viewport**
  - Requirements: U-08
  - Modules: render, ui
  - Depends on: S3.09.1
  - Deliverables: clipping-plane section view with capping
  - Tests: T-06 `Ui.Tests Snapshot.SectionView`
  - Done when: tests green

### S3.99 Stage gate

- [ ] **S3.99.1: Run the full S3 test suite and all checks**
  - Requirements: T-13, T-11, P-03
  - Modules: all
  - Depends on: S3.09.5, S3.09.4, S3.09.3, S3.09.2, S3.05.5
  - Deliverables: `scripts/ci-local.ps1 -Nightly` results (runs `scripts/test.ps1 -Coverage`, `-Bench`, `-Fuzz` and `scripts/check-selftest.ps1`); offline smoke
  - Tests: T-13 all suites incl. golden mechanisms
  - Done when: everything green
- [ ] **S3.99.2: Run the S3 benchmarks (early P-14 and P-11, regressions)**
  - Requirements: P-14, P-11, T-08
  - Modules: tests
  - Depends on: S3.99.1
  - Deliverables: `bench/history.csv` rows
  - Tests: T-08 `Bench.OpenLightweight2000`, `Bench.Viewport5MTriangles` and earlier benchmarks
  - Done when: results recorded (targets owned by S10)
- [ ] **S3.99.3: Update traceability and documentation for S3**
  - Requirements: T-13
  - Modules: docs
  - Depends on: S3.99.2
  - Deliverables: `docs/traceability.csv`, `docs/stages/S3.md`, `docs/format/assembly.md`, `docs/commands/`
  - Tests: `/check-traceability` reports no gaps for S3; `scripts/docs.ps1 -FailOnWarning` clean (needs Doxygen, see S0.14.1)
  - Done when: S3 requirements updated
- [ ] **S3.99.4: Human review checkpoint: stop and wait for approval**
  - Requirements: all S3 requirements (see `docs/stages/S3.md`)
  - Modules: none
  - Depends on: S3.99.3
  - Deliverables: S3 summary for the owner
  - Tests: none (review)
  - Done when: the owner approves S3 in writing

## Stage S4: Electronics libraries and schematic

Stage plan: `docs/stages/S4.md`. Exit: a 4-sheet hierarchical golden schematic compiles to the expected netlist; ERC
finds the seeded errors; the footprint generator produces the expected pad geometry for 20 package cases.

### S4.01 Electronics project and library lifecycle

- [ ] **S4.01.1: Define schematic and electronics library schemas in the project**
  - Requirements: E-PRJ-01, F-01, F-07
  - Modules: ecad/lib, ecad/sch, doc, docs
  - Depends on: S3.99.4
  - Deliverables: `docs/format/schemas/{schematic,symbol,footprint,component}.schema.json`, `docs/format/schematic.md`, `docs/format/ecad-library.md`; project manifest lists schematic documents
  - Tests: T-03 `Format.Tests RoundTrip` over schematic and library goldens
  - Done when: tests green
- [ ] **S4.01.2: Implement revisions and a reusable lifecycle state machine**
  - Requirements: E-LIB-02
  - Modules: doc
  - Depends on: S4.01.1
  - Deliverables: `src/doc/library/Lifecycle.{h,cpp}` (user-configurable states, default Draft → Released → Deprecated → Obsolete; released revisions immutable; generic so documents can reuse it, SW-PDM-02 seam)
  - Tests: T-01 `Doc.Tests Lifecycle.*`
  - Done when: tests green

### S4.02 Component model

- [ ] **S4.02.1: Symbols with gates and typed pins**
  - Requirements: E-LIB-01
  - Modules: ecad/lib
  - Depends on: S4.01.1
  - Deliverables: `src/ecad/lib/{Symbol,Pin}.{h,cpp}` (parts/gates; pin number, name, electrical type, length, orientation; pin and gate swap groups as should)
  - Tests: T-01 `EcadLib.Tests Symbol.*`
  - Done when: tests green
- [ ] **S4.02.2: Footprints with padstacks in integer nanometres**
  - Requirements: E-LIB-01, A-09
  - Modules: ecad/lib
  - Depends on: S4.01.1
  - Deliverables: `src/ecad/lib/{Footprint,Pad,Padstack}.{h,cpp}` (SMD/THT; round, rectangle, rounded rectangle, oblong, chamfered, custom polygon; per-layer padstacks; mask and paste expansion; round or slotted, plated or non-plated holes; silkscreen, courtyard, assembly outline, fiducials, designator/value text, 3D body reference)
  - Tests: T-01 `EcadLib.Tests Footprint.*`
  - Done when: tests green
- [ ] **S4.02.3: Components bundling symbols, footprints, 3D model, SPICE model and parameters**
  - Requirements: E-LIB-01
  - Modules: ecad/lib
  - Depends on: S4.02.1, S4.02.2
  - Deliverables: `src/ecad/lib/Component.{h,cpp}` (default and alternate footprints; 3D model as native part document reference or STEP blob with offset and rotation; SPICE model with pin map; parameters incl. local datasheet path and custom fields; connector pin data for the harness seam)
  - Tests: T-01 `EcadLib.Tests Component.*`
  - Done when: tests green
- [ ] **S4.02.4: Validate pin–pad maps and the golden components**
  - Requirements: E-LIB-01
  - Modules: ecad/lib, tests
  - Depends on: S4.02.3
  - Deliverables: validator for missing or extra mappings; golden components (resistor, op-amp with 2 gates plus power part, MCU in QFN, USB-C connector) in `tests/data/scripts/components_golden.cmd.json`
  - Tests: T-04 `Golden.Tests Components.ValidateCleanly` (Accept: E-LIB-01a); `Components.MismatchedPinPadReported` (Accept: E-LIB-01b)
  - Done when: tests green

### S4.03 Library management and scopes

- [ ] **S4.03.1: Library manager commands with search and validation**
  - Requirements: E-LIB-02
  - Modules: ecad/lib, doc
  - Depends on: S4.02.4, S4.01.2
  - Deliverables: commands `library.item.create`, `.edit`, `.copy`, `.validate`, `library.search` (any parameter, filters) across scopes
  - Tests: T-01 `EcadLib.Tests LibraryManager.*`
  - Done when: tests green
- [ ] **S4.03.2: Revisions on edit and "update from library" with preview**
  - Requirements: E-LIB-02
  - Modules: ecad/lib, doc
  - Depends on: S4.03.1
  - Deliverables: editing a released item creates the next revision; projects record library ID and revision; command `project.library.updateFrom` with change preview
  - Tests: T-02 `Integration.Tests Library.EditReleasedCreatesRevB` (Accept: E-LIB-02a); `Library.ProjectReportsAndAppliesUpdate` (Accept: E-LIB-02b)
  - Done when: tests green
- [ ] **S4.03.3: "Where used" across configured project folders**
  - Requirements: E-LIB-02
  - Modules: ecad/lib, doc
  - Depends on: S4.03.2
  - Deliverables: command `library.whereUsed` (folders from user settings; runs as a job)
  - Tests: T-01 `EcadLib.Tests WhereUsed.*`
  - Done when: tests green
- [ ] **S4.03.4: Copy used library items into the project on save**
  - Requirements: F-10
  - Modules: doc
  - Depends on: S4.03.2
  - Deliverables: self-contained save (default on) recording origin scope, library ID and revision
  - Tests: T-02 `Integration.Tests Library.OpensIdenticallyOnCleanMachine` (Accept: F-10b) (empty user and global scopes in a temporary profile)
  - Done when: test green
- [ ] **S4.03.5: Three-way review of seed upgrades**
  - Requirements: F-10
  - Modules: doc
  - Depends on: S4.03.4, S0.07.2
  - Deliverables: `src/doc/library/SeedUpgrade.{h,cpp}` (unchanged items update; locally changed items kept; both-changed items queued as old seed / new seed / local; nothing written until resolved); command `library.seed.review`
  - Tests: T-02 `Integration.Tests Library.SeedUpgradeThreeWayReview` (Accept: F-10c)
  - Done when: test green
- [ ] **S4.03.6: Seed the generic component library**
  - Requirements: E-LIB-01, CR-05
  - Modules: libraries, docs
  - Depends on: S4.02.4
  - Deliverables: `libraries/{components,symbols,footprints,models-3d}/` generic R, C, L, diode, LED, BJT, MOSFET, op-amp, regulator, headers, USB-C receptacle, test point, mounting hole, fiducial; `docs/data-sources.md` rows; seed manifest updated
  - Tests: T-01 `EcadLib.Tests Seed.AllItemsValidate`; T-11 data-source check
  - Done when: tests green

### S4.04 Footprint generator

- [ ] **S4.04.1: Generator framework with user-editable goal and tolerance tables**
  - Requirements: E-LIB-03, CR-05
  - Modules: ecad/lib, libraries, docs
  - Depends on: S4.02.2
  - Deliverables: `src/ecad/lib/fpgen/{Generator,FilletGoals}.{h,cpp}` (toe, heel, side goals; fabrication and placement tolerances; least/nominal/most density); `libraries/footprint-params/` default tables marked with sources or "user-supplied"; formula documented in `docs/format/footprint-generator.md`
  - Tests: T-01 `EcadLib.Tests FpGen.Framework*`
  - Done when: tests green; no paid-standard text reproduced
- [ ] **S4.04.2: Chip, MELF and SOD families**
  - Requirements: E-LIB-03
  - Modules: ecad/lib
  - Depends on: S4.04.1
  - Deliverables: generators for chip 0201–2512, MELF, SOD
  - Tests: T-01 `EcadLib.Tests FpGen.Chip*`, `Melf*`, `Sod*`
  - Done when: tests green
- [ ] **S4.04.3: SOT, SOIC, SSOP, TSSOP and QFP families**
  - Requirements: E-LIB-03
  - Modules: ecad/lib
  - Depends on: S4.04.1
  - Deliverables: SOT-23/SOT-223/SOT-89 variants, SOIC, SSOP, TSSOP, QFP
  - Tests: T-01 `EcadLib.Tests FpGen.Gullwing*`
  - Done when: tests green
- [ ] **S4.04.4: QFN/DFN, LGA, PLCC and DPAK/D2PAK families**
  - Requirements: E-LIB-03
  - Modules: ecad/lib
  - Depends on: S4.04.1
  - Deliverables: exposed pad with paste subdivision; LGA; PLCC; DPAK/D2PAK
  - Tests: T-01 `EcadLib.Tests FpGen.Qfn*`, `Lga*`, `Plcc*`, `Dpak*`
  - Done when: tests green
- [ ] **S4.04.5: BGA family (full, perimeter, staggered, depopulated)**
  - Requirements: E-LIB-03
  - Modules: ecad/lib
  - Depends on: S4.04.1
  - Deliverables: BGA generator with ball maps
  - Tests: T-01 `EcadLib.Tests FpGen.Bga*`
  - Done when: tests green
- [ ] **S4.04.6: Through-hole families (DIP, headers/SIP, radial, axial)**
  - Requirements: E-LIB-03
  - Modules: ecad/lib
  - Depends on: S4.04.1
  - Deliverables: THT generators
  - Tests: T-01 `EcadLib.Tests FpGen.Tht*`
  - Done when: tests green
- [ ] **S4.04.7: Generate courtyard, silkscreen, assembly outline, pin-1 marker and a native 3D body**
  - Requirements: E-LIB-03
  - Modules: ecad/lib
  - Depends on: S4.04.2, S4.04.3, S4.04.4, S4.04.5, S4.04.6
  - Deliverables: graphics generation; parametric 3D body emitted as a part document (JSON) that `part` regenerates (ADR 0025)
  - Tests: T-01 `EcadLib.Tests FpGen.Graphics*`; T-02 `Integration.Tests FpGen.BodyRegeneratesAsPart`
  - Done when: tests green
- [ ] **S4.04.8: 20 package cases against an independent oracle**
  - Requirements: E-LIB-03
  - Modules: tests
  - Depends on: S4.04.7
  - Deliverables: `tests/support/oracles/FootprintFormula.{h,cpp}` (independent implementation of the documented formula); `tests/data/scripts/fpgen_case_01..20.cmd.json`
  - Tests: T-04 `Golden.Tests FpGen.Cases01..20` (Accept: E-LIB-03)
  - Done when: all 20 pad sizes and positions match exactly on the nm grid

### S4.05 Schematic capture model

- [ ] **S4.05.1: Sheets, templates and placed components**
  - Requirements: E-SCH-01
  - Modules: ecad/sch, libraries
  - Depends on: S4.02.4
  - Deliverables: `src/ecad/sch/{Schematic,Sheet,SchComponent}.{h,cpp}`; sheet templates and title blocks in a format shared with drawings (`libraries/templates/`); commands `sch.component.place` (gates independently), `sch.sheet.*`
  - Tests: T-01 `EcadSch.Tests Placement.*`
  - Done when: tests green
- [ ] **S4.05.2: Wires, junctions, rubber-banding, net labels, power ports and no-connects**
  - Requirements: E-SCH-01
  - Modules: ecad/sch
  - Depends on: S4.05.1
  - Deliverables: commands `sch.wire.add`, `sch.label.add`, `sch.power.add`, `sch.noConnect.add`; automatic junctions; drag keeps connections
  - Tests: T-01 `EcadSch.Tests Wires.*`
  - Done when: tests green
- [ ] **S4.05.3: Buses, bus entries, bus naming and sheet ports**
  - Requirements: E-SCH-01
  - Modules: ecad/sch
  - Depends on: S4.05.2
  - Deliverables: `D[0..7]` bus expansion; commands `sch.bus.*`, `sch.port.add`
  - Tests: T-01 `EcadSch.Tests Buses.*`
  - Done when: tests green
- [ ] **S4.05.4: Graphics, text frames, images and notes**
  - Requirements: E-SCH-01, F-06
  - Modules: ecad/sch
  - Depends on: S4.05.1
  - Deliverables: graphic objects; images stored as blobs
  - Tests: T-01 `EcadSch.Tests Graphics.*`
  - Done when: tests green
- [ ] **S4.05.5: Editing operations across sheets**
  - Requirements: E-SCH-01
  - Modules: ecad/sch
  - Depends on: S4.05.3
  - Deliverables: move, rotate, mirror, copy/paste with designator handling, align, find and replace across sheets, selection filters
  - Tests: T-01 `EcadSch.Tests Editing.*`
  - Done when: tests green

### S4.06 Netlist compiler

- [ ] **S4.06.1: Compile flat multi-sheet connectivity with selectable net scopes**
  - Requirements: E-PRJ-01, E-SCH-02
  - Modules: ecad/sch, docs
  - Depends on: S4.05.3
  - Deliverables: `src/ecad/sch/compile/NetlistCompiler.{h,cpp}` (components, pins, nets, net classes, directives); net-scope rules (local, sheet, global) documented in `docs/format/schematic.md`
  - Tests: T-01 `EcadSch.Tests Compile.Flat*`
  - Done when: tests green
- [ ] **S4.06.2: Hierarchical sheets to any depth**
  - Requirements: E-SCH-02
  - Modules: ecad/sch
  - Depends on: S4.06.1
  - Deliverables: sheet symbols and entries
  - Tests: T-04 `Golden.Tests Netlist.Hierarchical4Sheet` (Accept: E-SCH-02a)
  - Done when: expected designators and nets (S4 exit criterion)
- [ ] **S4.06.3: Multi-channel repeats and signal bundles**
  - Requirements: E-SCH-02
  - Modules: ecad/sch
  - Depends on: S4.06.2
  - Deliverables: repeat expression, per-channel designator and net naming, room metadata per channel; bundles expanded at each end
  - Tests: T-04 `Golden.Tests Netlist.EightChannel` (Accept: E-SCH-02b)
  - Done when: test green
- [ ] **S4.06.4: Compile messages and the golden project netlist**
  - Requirements: E-PRJ-01
  - Modules: ecad/sch, tests
  - Depends on: S4.06.1
  - Deliverables: messages with object IDs for click-to-locate; `tests/data/scripts/golden_project_netlist.cmd.json`
  - Tests: T-02 `Integration.Tests Netlist.GoldenProjectCompiles` (Accept: E-PRJ-01a)
  - Done when: test green

### S4.07 Electrical rules check

- [ ] **S4.07.1: Editable connection matrix and check registry**
  - Requirements: E-SCH-03, A-11
  - Modules: ecad/sch
  - Depends on: S4.06.1
  - Deliverables: `src/ecad/sch/erc/{ConnectionMatrix,ErcEngine}.{h,cpp}`; checks registered through a factory
  - Tests: T-01 `EcadSch.Tests ErcMatrix.*`
  - Done when: tests green
- [ ] **S4.07.2: Implement the ERC checks**
  - Requirements: E-SCH-03
  - Modules: ecad/sch
  - Depends on: S4.07.1
  - Deliverables: unconnected inputs, multiple drivers, single-connection nets, undriven power nets, duplicate designators, duplicate/conflicting net names, missing footprint or model, unmatched ports/entries, floating labels, off-grid objects
  - Tests: T-01 `EcadSch.Tests ErcChecks.*` (one case per check)
  - Done when: tests green
- [ ] **S4.07.3: Violation list with waivers**
  - Requirements: E-SCH-03
  - Modules: ecad/sch
  - Depends on: S4.07.2
  - Deliverables: violation model with object IDs; command `sch.erc.waive` (stored comment)
  - Tests: T-01 `EcadSch.Tests ErcWaivers.*`
  - Done when: tests green
- [ ] **S4.07.4: Golden schematic with 12 seeded ERC errors**
  - Requirements: E-SCH-03, T-10
  - Modules: tests
  - Depends on: S4.07.3
  - Deliverables: `tests/data/scripts/erc_seeded_12.cmd.json`
  - Tests: T-04 `Golden.Tests Erc.Seeded12Exactly` (Accept: E-SCH-03)
  - Done when: all 12 found and no others (S4 exit criterion)

### S4.08 Annotation and directives

- [ ] **S4.08.1: Auto-annotate designators**
  - Requirements: E-SCH-04
  - Modules: ecad/sch
  - Depends on: S4.06.3
  - Deliverables: command `sch.annotate` (order pattern, scope project/sheets/selection, keep or reset, channel-aware)
  - Tests: T-04 `Golden.Tests Annotation.ExpectedDesignators` (Accept: E-SCH-04a)
  - Done when: test green
- [ ] **S4.08.2: Schematic directives**
  - Requirements: E-SCH-05
  - Modules: ecad/sch
  - Depends on: S4.06.1
  - Deliverables: directives on wires, nets, buses, areas: net class, differential pair (automatic `_P`/`_N` pairing), length-match group, impedance profile, rule parameters, ERC suppression; compiled into the connectivity model
  - Tests: T-01 `EcadSch.Tests Directives.*`
  - Done when: tests green (board side in S5.05.3)

### S4.09 Schematic reuse

- [ ] **S4.09.1: Schematic reuse blocks and snippets in libraries**
  - Requirements: E-PRJ-04
  - Modules: ecad/sch, ecad/lib
  - Depends on: S4.08.1, S4.03.2
  - Deliverables: save and place reuse blocks and snippets (schematic part) with revisions; placed instances record source and revision; designators re-annotated without clashes
  - Tests: T-02 `Integration.Tests Reuse.SchematicBlockPlaces`
  - Done when: tests green (layout part in S5.15.1)

### S4.10 Schematic and library user interface

- [ ] **S4.10.1: Schematic editor canvas and tools**
  - Requirements: E-SCH-01, U-08
  - Modules: ui, render
  - Depends on: S4.05.5, S0.12.5
  - Deliverables: `src/ui/schematic/` (tools for every capture command; pan, zoom, fit)
  - Tests: T-05 `Ui.Tests Schematic.DrawWire`
  - Done when: tests green
- [ ] **S4.10.2: Library browser and manager with scope marks**
  - Requirements: E-LIB-02, F-10
  - Modules: ui
  - Depends on: S4.03.3, S4.03.5
  - Deliverables: browser showing each item's scope and shadowed items; override-into; seed review dialog
  - Tests: T-05 `Ui.Tests Library.ShowsScopeAndShadowing`
  - Done when: tests green
- [ ] **S4.10.3: Footprint wizard with 3D preview**
  - Requirements: E-LIB-03, E-LIB-01
  - Modules: ui
  - Depends on: S4.04.8, S4.10.2
  - Deliverables: wizard UI; footprint 3D preview for model alignment
  - Tests: T-05 `Ui.Tests FootprintWizard.GeneratesQfn`
  - Done when: tests green
- [ ] **S4.10.4: Project navigator, compile messages and ERC browser**
  - Requirements: E-PRJ-01, E-SCH-03, U-11
  - Modules: ui
  - Depends on: S4.07.4, S4.10.1
  - Deliverables: navigator (documents, components, nets); ERC browser with click-to-locate and waive
  - Tests: T-05 `Ui.Tests Erc.ClickLocates`
  - Done when: tests green
- [ ] **S4.10.5: UI replay draws and compiles the golden 2-sheet schematic**
  - Requirements: E-SCH-01, T-05
  - Modules: tests
  - Depends on: S4.10.4
  - Deliverables: `tests/ui/replays/schematic_2sheet.journal.jsonl`
  - Tests: T-05 `Ui.Tests Replay.Schematic2SheetCompiles` (Accept: E-SCH-01)
  - Done when: replay green with the expected netlist

### S4.99 Stage gate

- [ ] **S4.99.1: Run the full S4 test suite and all checks**
  - Requirements: T-13, T-11, P-03
  - Modules: all
  - Depends on: S4.10.5, S4.10.3, S4.09.1, S4.08.2
  - Deliverables: `scripts/ci-local.ps1 -Nightly` results (runs `scripts/test.ps1 -Coverage`, `-Bench`, `-Fuzz` and `scripts/check-selftest.ps1`); offline smoke
  - Tests: T-13 all suites; T-09 fuzz targets for schematic and library loaders added to the nightly set
  - Done when: everything green
- [ ] **S4.99.2: Run the benchmark regressions**
  - Requirements: T-08
  - Modules: tests
  - Depends on: S4.99.1
  - Deliverables: `bench/history.csv` rows
  - Tests: T-08 all existing benchmarks
  - Done when: results recorded
- [ ] **S4.99.3: Update traceability and documentation for S4**
  - Requirements: T-13
  - Modules: docs
  - Depends on: S4.99.2
  - Deliverables: `docs/traceability.csv`, `docs/stages/S4.md`, `docs/format/`, `docs/commands/`, `docs/data-sources.md`
  - Tests: `/check-traceability` reports no gaps for S4; `scripts/docs.ps1 -FailOnWarning` clean (needs Doxygen, see S0.14.1)
  - Done when: S4 requirements updated
- [ ] **S4.99.4: Human review checkpoint: stop and wait for approval**
  - Requirements: all S4 requirements (see `docs/stages/S4.md`)
  - Modules: none
  - Depends on: S4.99.3
  - Deliverables: S4 summary for the owner
  - Tests: none (review)
  - Done when: the owner approves S4 in writing

## Stage S5: PCB layout

Stage plan: `docs/stages/S5.md`. Exit: the golden board is placed and routed interactively (UI replay test); DRC finds
every seeded violation with no false positives; the 3D board view matches the stack-up.

### S5.01 Router and pour spikes (SP-5, SP-6)

- [ ] **S5.01.1: Add Clipper2 to the PCB module (ADR 0009)**
  - Requirements: A-09, A-01
  - Modules: ecad/pcb, build, scripts
  - Depends on: S4.99.4
  - Deliverables: `clipper2` in `vcpkg.json` (licence check), referenced only by `EcadPcb.vcxproj`; layering third-party table confirms "Clipper2 only in ecad/pcb"
  - Tests: T-11 `check-licences.ps1`, `check-layering.ps1` green; Clipper2 include in `link` fails
  - Done when: checks green
- [ ] **S5.01.2: Spike SP-5a: prototype push-and-shove routing on a dense synthetic board (time box 2 days)**
  - Requirements: E-PCB-04, P-15
  - Modules: ecad/pcb, tests
  - Depends on: S5.01.1
  - Deliverables: prototype under `tests/ecad/pcb/spikes/` (fresh design; evaluation of adapting a GPL-3.0-compatible open router under CR-04/CR-07 recorded with licence and effort); response-time measurements on a 2,000-component / 5,000-net synthetic board
  - Tests: spike harness: per-mouse-move latency distribution, shove correctness on 20 seeded cases
  - Done when: measurements recorded or the time box ends with results recorded
- [ ] **S5.01.3: Spike SP-5b: decide the router approach in ADR 0038**
  - Requirements: E-PCB-04, E-PCB-05, P-15
  - Modules: ecad/pcb, docs
  - Depends on: S5.01.2
  - Deliverables: `docs/adr/0038-spike-sp5-router.md` (algorithm, data structures, services for future autorouting, provenance plan if any code is adapted)
  - Tests: spike results attached
  - Done when: ADR Accepted before S5.09.1
- [ ] **S5.01.4: Spike SP-6a: prototype the Clipper2 pour engine on a dense board (time box 2 days)**
  - Requirements: E-PCB-07
  - Modules: ecad/pcb, tests
  - Depends on: S5.01.1
  - Deliverables: prototype pour fill with clearance, thermal reliefs, island and neck removal on an 8-layer dense synthetic board; robustness on degenerate inputs
  - Tests: spike harness: fill time per layer, area error against an independent rasterised estimate, zero clearance violations
  - Done when: measurements recorded or the time box ends with results recorded
- [ ] **S5.01.5: Spike SP-6b: decide the pour engine design in ADR 0039**
  - Requirements: E-PCB-07
  - Modules: ecad/pcb, docs
  - Depends on: S5.01.4
  - Deliverables: `docs/adr/0039-spike-sp6-pours.md`
  - Tests: spike results attached
  - Done when: ADR Accepted before S5.11.1

### S5.02 Board database

- [ ] **S5.02.1: Define the board document schema with regions referencing stacks**
  - Requirements: E-PCB-02, A-09, F-01
  - Modules: ecad/pcb, docs
  - Depends on: S4.99.4
  - Deliverables: `docs/format/schemas/board.schema.json`, `docs/format/board.md` (all coordinates int64 nm; board regions referencing stack definitions even with one region, rigid-flex seam); `src/ecad/pcb/BoardDb.{h,cpp}`
  - Tests: T-03 `Format.Tests RoundTrip` over board goldens
  - Done when: tests green
- [ ] **S5.02.2: Exact integer polygon and arc geometry for boards**
  - Requirements: A-09, P-07
  - Modules: ecad/pcb
  - Depends on: S5.02.1, S5.01.1
  - Deliverables: `src/ecad/pcb/geometry/` (nm polygons with arcs, offsets, booleans on Clipper2; exact clearance distance)
  - Tests: T-01 `EcadPcb.Tests Geometry.*` (exact results on grid, no float in clearance decisions)
  - Done when: tests green
- [ ] **S5.02.3: Spatial index for board objects**
  - Requirements: E-RUL-02, P-15, P-16
  - Modules: ecad/pcb
  - Depends on: S5.02.2
  - Deliverables: `src/ecad/pcb/SpatialIndex.{h,cpp}` (per-layer R-tree, incremental updates)
  - Tests: T-01 `EcadPcb.Tests SpatialIndex.*`; T-08 `Bench.SpatialQuery`
  - Done when: tests green

### S5.03 Layer stack and impedance

- [ ] **S5.03.1: Stack manager model**
  - Requirements: E-PCB-01
  - Modules: ecad/pcb
  - Depends on: S5.02.1
  - Deliverables: copper, dielectric (core/prepreg; thickness, Dk, Df, material), mask, paste, silkscreen, mechanical/documentation layers; non-copper conductive and dielectric print layers allowed (AD-PCB-10 seam); at least 64 signal layers (128 should); total thickness; commands `pcb.stack.*`
  - Tests: T-01 `EcadPcb.Tests Stack.*` incl. 64 signal layers
  - Done when: tests green
- [ ] **S5.03.2: Define via spans against the stack**
  - Requirements: E-PCB-01, E-PCB-08
  - Modules: ecad/pcb
  - Depends on: S5.03.1
  - Deliverables: through, blind, buried, micro spans validated against the stack
  - Tests: T-01 `EcadPcb.Tests ViaSpans.*`
  - Done when: tests green
- [ ] **S5.03.3: Closed-form impedance profiles and stack table data**
  - Requirements: E-PCB-01
  - Modules: ecad/pcb, tests
  - Depends on: S5.03.1
  - Deliverables: single-ended and differential microstrip, embedded microstrip, stripline, coplanar from published closed-form approximations (sources cited in code docs); width/gap for a target impedance; stack table data for E-DOC-01; independent reference values in `tests/support/oracles/Impedance.{h,cpp}`
  - Tests: T-04 `Golden.Tests Stack.FourAndEightLayerThickness` (Accept: E-PCB-01a); `Golden.Tests Impedance.Within1Percent` (Accept: E-PCB-01b)
  - Done when: tests green
- [ ] **S5.03.4: Seed generic layer stacks and rule sets**
  - Requirements: E-PCB-01, E-RUL-01, CR-05
  - Modules: libraries, docs
  - Depends on: S5.03.1
  - Deliverables: `libraries/layer-stacks/` (2/4/6/8-layer generic), `libraries/rule-sets/` ("Generic 2-layer 6/6 mil", "Generic 4-layer 5/5 mil", "HDI 3/3 mil", original values); `docs/data-sources.md` rows
  - Tests: T-01 `EcadPcb.Tests Seed.StacksAndRuleSetsLoad`
  - Done when: tests green

### S5.04 Board shape and regions

- [ ] **S5.04.1: Board outline, cutouts and slots**
  - Requirements: E-PCB-02
  - Modules: ecad/pcb
  - Depends on: S5.02.2
  - Deliverables: outline from lines and arcs; internal cutouts and slots; validation (closed, non-self-intersecting); commands `pcb.outline.*`
  - Tests: T-01 `EcadPcb.Tests Outline.*`
  - Done when: tests green
- [ ] **S5.04.2: Keepouts, rooms, height regions, origin, grids and units**
  - Requirements: E-PCB-02
  - Modules: ecad/pcb
  - Depends on: S5.04.1
  - Deliverables: keepouts per layer and object type, rooms bound to component groups, manual height regions, board origin, multiple Cartesian grids (polar should), unit switching
  - Tests: T-01 `EcadPcb.Tests Regions.*`; T-04 `Golden.Tests Boards.CutoutsKeepoutsValidate`
  - Done when: tests green

### S5.05 Design rules

- [ ] **S5.05.1: Rule model, categories and the rule query language**
  - Requirements: E-RUL-01, A-08
  - Modules: ecad/pcb, core, docs
  - Depends on: S5.02.1
  - Deliverables: `src/ecad/pcb/rules/{Rule,RuleQuery}.{h,cpp}` (all categories in E-RUL-01; queries on the shared expression engine); `docs/rules-language.md`; priority convention per ADR 0032
  - Tests: T-01 `EcadPcb.Tests RuleQuery.*`
  - Done when: tests green; query language documented
- [ ] **S5.05.2: Rule resolution by priority, including pairwise scopes**
  - Requirements: E-RUL-01
  - Modules: ecad/pcb
  - Depends on: S5.05.1
  - Deliverables: `RuleResolver` (highest-priority enabled matching rule; two-scope rules such as clearance; "applied rule" query)
  - Tests: T-04 `Golden.Tests Rules.Resolution30Cases` (Accept: E-RUL-01)
  - Done when: all 30 cases select the expected rule
- [ ] **S5.05.3: Rule sets and the constraint grid model**
  - Requirements: E-RUL-01
  - Modules: ecad/pcb
  - Depends on: S5.05.2
  - Deliverables: rule sets enabled/disabled together, JSON import/export; class × class clearance matrix model
  - Tests: T-01 `EcadPcb.Tests RuleSets.*`, `ConstraintGrid.*`
  - Done when: tests green

### S5.06 Schematic–board synchronisation

- [ ] **S5.06.1: Change list from schematic to board**
  - Requirements: E-PRJ-01
  - Modules: ecad/pcb, ecad/sch
  - Depends on: S5.02.1, S4.06.4
  - Deliverables: `src/ecad/pcb/sync/ChangeList.{h,cpp}` (add/remove/modify component, footprint change, net add/remove/rename, pin swap, net-class and rule changes; accept or reject per item); command `project.sync.apply`
  - Tests: T-01 `EcadPcb.Tests Sync.SchematicToBoard*`
  - Done when: tests green
- [ ] **S5.06.2: Board-to-schematic updates and back-annotation**
  - Requirements: E-PRJ-01, E-SCH-04
  - Modules: ecad/pcb, ecad/sch
  - Depends on: S5.06.1
  - Deliverables: pin/gate swaps and annotation from board to schematic; renumber by board position with a selectable pattern
  - Tests: T-02 `Integration.Tests Sync.BothDirectionsZeroDifferences` (Accept: E-PRJ-01b); `Annotation.BackAnnotateByPosition` (Accept: E-SCH-04b)
  - Done when: tests green
- [ ] **S5.06.3: Directives become board classes and rules**
  - Requirements: E-SCH-05, E-RUL-01
  - Modules: ecad/pcb
  - Depends on: S5.06.1, S5.05.2, S5.03.3
  - Deliverables: net classes, differential-pair classes, match groups, impedance-driven rules created on synchronisation
  - Tests: T-02 `Integration.Tests Directives.UsbPairCreatesDiffPairClass` (Accept: E-SCH-05)
  - Done when: test green

### S5.07 Component placement

- [ ] **S5.07.1: Placement commands and locking**
  - Requirements: E-PCB-03
  - Modules: ecad/pcb
  - Depends on: S5.06.1, S5.04.2
  - Deliverables: commands `pcb.component.move`, `.rotate` (any angle), `.flip`, `.align`, `.distribute`, `.arrangeInRoom`, `.clusterBy` (sheet or channel), `.lock`; snap to grid and objects
  - Tests: T-01 `EcadPcb.Tests Placement.*`
  - Done when: tests green
- [ ] **S5.07.2: Live ratsnest**
  - Requirements: E-PCB-03
  - Modules: ecad/pcb
  - Depends on: S5.07.1
  - Deliverables: incremental minimum spanning tree per net; optional per-net colours
  - Tests: T-01 `EcadPcb.Tests Ratsnest.*`
  - Done when: tests green
- [ ] **S5.07.3: Live placement rule checks**
  - Requirements: E-PCB-03, E-RUL-01
  - Modules: ecad/pcb
  - Depends on: S5.07.1, S5.05.2
  - Deliverables: courtyard clearance, height, permitted side and orientation checked on every placement command
  - Tests: T-01 `EcadPcb.Tests PlacementRules.*`
  - Done when: tests green
- [ ] **S5.07.4: Room copy across channels**
  - Requirements: E-PCB-03
  - Modules: ecad/pcb
  - Depends on: S5.07.1, S4.06.3
  - Deliverables: command `pcb.room.copy` (placement, and routing once present)
  - Tests: T-04 `Golden.Tests Placement.RoomCopy4Channels` (Accept: E-PCB-03b)
  - Done when: test green

### S5.08 Design rule check

- [ ] **S5.08.1: Online DRC engine with registered checks**
  - Requirements: E-RUL-02, A-11
  - Modules: ecad/pcb
  - Depends on: S5.05.2, S5.02.3
  - Deliverables: `src/ecad/pcb/drc/DrcEngine.{h,cpp}` (incremental on edits, checks registered through a factory, exact integer geometry)
  - Tests: T-01 `EcadPcb.Tests DrcEngine.*`
  - Done when: tests green
- [ ] **S5.08.2: Electrical and manufacturing checks**
  - Requirements: E-RUL-02, E-RUL-01
  - Modules: ecad/pcb
  - Depends on: S5.08.1
  - Deliverables: clearance, short circuit, unrouted net, unconnected pin; annular ring, hole size range, hole-to-hole, silk-to-mask, silk-to-silk, acute angle, slivers, board-outline clearance
  - Tests: T-10 `EcadPcb.Tests DrcChecks.Electrical*`, `Manufacturing*`
  - Done when: tests green
- [ ] **S5.08.3: Routing, SMT, mask, pour, high-speed and placement checks**
  - Requirements: E-RUL-02, E-RUL-01
  - Modules: ecad/pcb
  - Depends on: S5.08.1
  - Deliverables: width, via style, routing layers, corners, differential pair; pad entry, neck-down; mask and paste expansions; pour connect and clearance; length, matched length, via count, parallel segment, stub; placement checks
  - Tests: T-10 `EcadPcb.Tests DrcChecks.Routing*`, `Smt*`, `HighSpeed*`, `Placement*`
  - Done when: tests green
- [ ] **S5.08.4: Batch DRC reports and waivers**
  - Requirements: E-RUL-02
  - Modules: ecad/pcb
  - Depends on: S5.08.2, S5.08.3
  - Deliverables: command `pcb.drc.run` (job) writing HTML, JSON and CSV reports; waivers with comment, author and date stored in the project
  - Tests: T-01 `EcadPcb.Tests DrcReport.*`, `DrcWaivers.*`
  - Done when: tests green
- [ ] **S5.08.5: Seeded-violation board suite (T-10 DRC)**
  - Requirements: E-RUL-02, T-10
  - Modules: tests
  - Depends on: S5.08.4
  - Deliverables: `tests/support/drc/SeededBoardGenerator.{h,cpp}` (each violation type injected at known locations); clean boards for false-positive checks
  - Tests: T-10 `Golden.Tests Drc.SeededSuite` (Accept: E-RUL-02a)
  - Done when: every seeded violation found; zero false positives
- [ ] **S5.08.6: Meet P-16 (batch DRC on an 8-layer, 2,000-component board under 15 s)**
  - Requirements: P-16, E-RUL-02
  - Modules: ecad/pcb, tests
  - Depends on: S5.08.5
  - Deliverables: generated benchmark board; `Bench.BatchDrc8Layer2000`
  - Tests: T-08 `Bench.BatchDrc8Layer2000` under 15 s (Accept: E-RUL-02b)
  - Done when: target met or a defect recorded per spec 3.2

### S5.09 Interactive routing

- [ ] **S5.09.1: Track, arc and via objects; router services**
  - Requirements: E-PCB-04
  - Modules: ecad/pcb
  - Depends on: S5.08.1, S5.01.3
  - Deliverables: `src/ecad/pcb/router/` core exposing path-finding and shove as services, not UI code (AD-PCB-06 seam); track/arc/via commands
  - Tests: T-01 `EcadPcb.Tests RouterCore.*`
  - Done when: tests green
- [ ] **S5.09.2: Walk-around routing mode**
  - Requirements: E-PCB-04
  - Modules: ecad/pcb
  - Depends on: S5.09.1
  - Deliverables: command `pcb.route.walkaround` (incremental, mergeable)
  - Tests: T-04 `Golden.Tests Router.Walkaround*`
  - Done when: tests green with zero DRC violations
- [ ] **S5.09.3: Push-and-shove for tracks**
  - Requirements: E-PCB-04
  - Modules: ecad/pcb
  - Depends on: S5.09.2
  - Deliverables: track shoving per ADR 0038
  - Tests: T-04 `Golden.Tests Router.ShoveTracks*`
  - Done when: tests green
- [ ] **S5.09.4: Shove vias, hug-and-push and ignore-obstacles modes**
  - Requirements: E-PCB-04
  - Modules: ecad/pcb
  - Depends on: S5.09.3
  - Deliverables: via shoving; hug-and-push; ignore-obstacles with violations flagged by online DRC
  - Tests: T-04 `Golden.Tests Router.ShoveVias*`, `HugAndPush*`, `IgnoreFlagsViolations`
  - Done when: tests green
- [ ] **S5.09.5: Rule widths, via insertion with layer change and corner styles**
  - Requirements: E-PCB-04, E-RUL-01
  - Modules: ecad/pcb
  - Depends on: S5.09.3
  - Deliverables: min/preferred/max width cycling; via insertion while routing; 45°, 90°, arc and any-angle corners
  - Tests: T-04 `Golden.Tests Router.WidthsViasCorners*`
  - Done when: tests green
- [ ] **S5.09.6: Loop removal and glossing**
  - Requirements: E-PCB-04
  - Modules: ecad/pcb
  - Depends on: S5.09.3
  - Deliverables: automatic cleanup after each route
  - Tests: T-04 `Golden.Tests Router.Cleanup*`
  - Done when: tests green
- [ ] **S5.09.7: Multi-track (bus) routing**
  - Requirements: E-PCB-04
  - Modules: ecad/pcb
  - Depends on: S5.09.3
  - Deliverables: several nets routed in parallel
  - Tests: T-04 `Golden.Tests Router.MultiTrack*`
  - Done when: tests green
- [ ] **S5.09.8: Drag tracks and vias with shove, unroute and auto-complete**
  - Requirements: E-PCB-04
  - Modules: ecad/pcb
  - Depends on: S5.09.4
  - Deliverables: commands `pcb.track.drag`, `pcb.via.drag`, `pcb.unroute` (net, connection, component), `pcb.route.autoComplete`
  - Tests: T-04 `Golden.Tests Router.DragUnrouteAutoComplete*`
  - Done when: tests green
- [ ] **S5.09.9: Push-and-shove golden suite with zero DRC violations**
  - Requirements: E-PCB-04
  - Modules: tests
  - Depends on: S5.09.5, S5.09.6, S5.09.7, S5.09.8, S5.08.5
  - Deliverables: `tests/data/scripts/shove_*.cmd.json`
  - Tests: T-04 `Golden.Tests Router.ShoveSuiteZeroDrc` (Accept: E-PCB-04c)
  - Done when: every case leaves zero DRC violations
- [ ] **S5.09.10: Meet P-15 (under 50 ms per mouse move at 2,000 components / 5,000 nets)**
  - Requirements: P-15, E-PCB-04
  - Modules: ecad/pcb, tests
  - Depends on: S5.09.9
  - Deliverables: `Bench.RouteMouseMove2000x5000`
  - Tests: T-08 `Bench.RouteMouseMove2000x5000` (Accept: E-PCB-04a)
  - Done when: target met or a defect recorded per spec 3.2

### S5.10 High-speed routing

- [ ] **S5.10.1: Differential-pair routing**
  - Requirements: E-PCB-05
  - Modules: ecad/pcb
  - Depends on: S5.09.3, S5.06.3
  - Deliverables: coupled routing with width and gap from rules or impedance profiles; max uncoupled length; intra-pair phase matching
  - Tests: T-04 `Golden.Tests HighSpeed.DiffPair*`
  - Done when: tests green
- [ ] **S5.10.2: Length tuning patterns**
  - Requirements: E-PCB-05
  - Modules: ecad/pcb
  - Depends on: S5.09.3
  - Deliverables: accordion, trombone and sawtooth with amplitude, spacing and corner rules; tune toward a target or the longest member of a match group
  - Tests: T-04 `Golden.Tests HighSpeed.Tuning*`
  - Done when: tests green
- [ ] **S5.10.3: Signal paths and per-net length and delay**
  - Requirements: E-PCB-05, E-PCB-01
  - Modules: ecad/pcb
  - Depends on: S5.10.1
  - Deliverables: pad-to-pad paths through series parts (manual or auto-detected from 2-terminal passives); routed length and propagation delay from stack Dk per net, pair, path; match-group status; routed-geometry extraction API (AD-SI-01 seam) and per-net copper polygon export with layer and thickness (AD-PI-01 seam)
  - Tests: T-01 `EcadPcb.Tests SignalPaths.*`, `LengthDelay.*`, `CopperExport.*`
  - Done when: tests green
- [ ] **S5.10.4: Return-path check**
  - Requirements: E-PCB-05
  - Modules: ecad/pcb
  - Depends on: S5.10.3
  - Deliverables: segments crossing splits or voids in the adjacent reference plane reported as violations
  - Tests: T-01 `EcadPcb.Tests ReturnPath.*`
  - Done when: tests green
- [ ] **S5.10.5: Golden DDR-style match group and seeded return-path crossings**
  - Requirements: E-PCB-05
  - Modules: tests
  - Depends on: S5.10.2, S5.10.4
  - Deliverables: `tests/data/scripts/ddr_match_group.cmd.json`, `return_path_seeded.cmd.json`
  - Tests: T-04 `Golden.Tests HighSpeed.DdrTunedWithin0_1mm` (Accept: E-PCB-05a); `HighSpeed.ReturnPathFindsSeeded` (Accept: E-PCB-05b)
  - Done when: tests green

### S5.11 Copper pours

- [ ] **S5.11.1: Solid and hatched pours with clearance, priority and cutouts**
  - Requirements: E-PCB-07
  - Modules: ecad/pcb
  - Depends on: S5.01.5, S5.05.2
  - Deliverables: `src/ecad/pcb/pour/PourEngine.{h,cpp}` per ADR 0039; commands `pcb.pour.create`, `pcb.pour.repour` (on demand, as a job)
  - Tests: T-04 `Golden.Tests Pours.SolidHatched*`
  - Done when: tests green
- [ ] **S5.11.2: Pad and via connection styles**
  - Requirements: E-PCB-07
  - Modules: ecad/pcb
  - Depends on: S5.11.1
  - Deliverables: direct, thermal relief (spoke count, width, gap, angle) or none, per rule scope
  - Tests: T-04 `Golden.Tests Pours.ThermalSpokes*`
  - Done when: tests green
- [ ] **S5.11.3: Island and neck removal**
  - Requirements: E-PCB-07
  - Modules: ecad/pcb
  - Depends on: S5.11.1
  - Deliverables: minimum area and minimum width rules
  - Tests: T-04 `Golden.Tests Pours.IslandsNecks*`
  - Done when: tests green
- [ ] **S5.11.4: Via stitching and via shielding**
  - Requirements: E-PCB-07
  - Modules: ecad/pcb
  - Depends on: S5.11.1
  - Deliverables: commands `pcb.via.stitch` (grid in region), `pcb.via.shield` (along a route)
  - Tests: T-04 `Golden.Tests Pours.Stitching*`
  - Done when: tests green
- [ ] **S5.11.5: Golden pour suite**
  - Requirements: E-PCB-07
  - Modules: tests
  - Depends on: S5.11.2, S5.11.3, S5.11.4, S5.08.5
  - Deliverables: `tests/data/scripts/pour_*.cmd.json` with independently computed areas
  - Tests: T-04 `Golden.Tests Pours.AreaWithin0_1Percent` (Accept: E-PCB-07a); `Pours.ThermalReliefGeometry` (Accept: E-PCB-07b); `Pours.NoClearanceViolations` (Accept: E-PCB-07c)
  - Done when: tests green

### S5.12 HDI and fabrication features

- [ ] **S5.12.1: Via types, stacked and staggered microvias, via-in-pad and tenting**
  - Requirements: E-PCB-08
  - Modules: ecad/pcb
  - Depends on: S5.03.2, S5.09.1
  - Deliverables: via objects with span, fill/cap flags and tenting; DRC checks for spans
  - Tests: T-01 `EcadPcb.Tests HdiVias.*`
  - Done when: tests green
- [ ] **S5.12.2: Backdrilling with stub-length rule**
  - Requirements: E-PCB-08
  - Modules: ecad/pcb
  - Depends on: S5.12.1
  - Deliverables: backdrill definition from a surface to a layer; stub rule check; drill-span data for outputs
  - Tests: T-01 `EcadPcb.Tests Backdrill.*`
  - Done when: tests green
- [ ] **S5.12.3: Teardrops as regenerable objects**
  - Requirements: E-PCB-08
  - Modules: ecad/pcb
  - Depends on: S5.09.1
  - Deliverables: teardrops on pads and vias (length, width, curve), regenerated on edit, removable
  - Tests: T-04 `Golden.Tests Hdi.Teardrops*`
  - Done when: tests green
- [ ] **S5.12.4: Embedded components, cavities and castellated holes**
  - Requirements: E-PCB-08
  - Modules: ecad/pcb
  - Depends on: S5.12.1, S5.07.1
  - Deliverables: inner-layer placement with automatic cavity; reduced-thickness regions; castellated holes (should)
  - Tests: T-01 `EcadPcb.Tests Embedded.*`, `Cavities.*`
  - Done when: tests green (IPC-2581 accept in S6.09.4)

### S5.13 3D board model (in link, ADR 0025)

- [ ] **S5.13.1: Build the 3D board in link**
  - Requirements: E-3D-01, A-09
  - Modules: link, geom
  - Depends on: S5.04.2, S5.12.4
  - Deliverables: ADR 0025 Accepted; ADR 0018 applied for `link` → `ecad/lib`; `src/link/board3d/BoardBuilder.{h,cpp}` (substrate with true thickness, outline, cutouts, cavities; plated holes; copper, mask and silkscreen as textures or optional thin solids; nm → mm conversion here only)
  - Tests: T-04 `Golden.Tests Board3d.ThicknessAndHolesExact` (Accept: E-3D-01b)
  - Done when: thickness and hole positions match the stack and 2D database exactly
- [ ] **S5.13.2: Place component 3D models from footprints**
  - Requirements: E-3D-01, E-LIB-01
  - Modules: link, part
  - Depends on: S5.13.1, S4.04.7
  - Deliverables: native part documents regenerated, STEP blobs imported; side, rotation and offset honoured
  - Tests: T-04 `Golden.Tests Board3d.ComponentPlacement`
  - Done when: tests green
- [ ] **S5.13.3: 3D body-to-body clearance as a DRC check**
  - Requirements: E-RUL-02, E-3D-01
  - Modules: link, ecad/pcb
  - Depends on: S5.13.2, S5.08.1
  - Deliverables: `link` registers a 3D clearance check into the `ecad/pcb` DRC registry (A-11); Z-axis copper clearance (should)
  - Tests: T-10 `Golden.Tests Drc.ComponentBody3dClearance`
  - Done when: tests green
- [ ] **S5.13.4: Golden boards with cutouts and keepouts validate in 2D and 3D**
  - Requirements: E-PCB-02, E-3D-01
  - Modules: tests
  - Depends on: S5.13.1
  - Deliverables: `tests/data/scripts/board_cutouts_keepouts.cmd.json`; reference images
  - Tests: T-06 `Golden.Tests Snapshot.BoardCutoutsKeepouts2d3d` (Accept: E-PCB-02)
  - Done when: snapshots match (SSIM ≥ 0.98)

### S5.14 Layout reuse

- [ ] **S5.14.1: Reuse blocks with placement and routing**
  - Requirements: E-PRJ-04
  - Modules: ecad/pcb, ecad/sch
  - Depends on: S5.09.8, S4.09.1
  - Deliverables: reuse blocks carry relative placement and routing; "update from library" diff (should)
  - Tests: T-02 `Integration.Tests Reuse.BuckRegulatorBlock` (Accept: E-PRJ-04)
  - Done when: schematic, nets and placed/routed layout arrive; designators re-annotate without clashes

### S5.15 PCB user interface

- [ ] **S5.15.1: PCB editor canvas with layers panel and colour-blind-safe palette**
  - Requirements: E-PCB-02, U-12, U-08
  - Modules: ui, render
  - Depends on: S5.04.2, S0.12.5
  - Deliverables: `src/ui/pcb/` canvas, layers panel, default palette from theme tokens
  - Tests: T-06 `Ui.Tests Snapshot.PcbCanvas`; T-05 `Ui.Tests Palette.ColourBlindSafe` (simulated deuteranopia distance check)
  - Done when: tests green
- [ ] **S5.15.2: Placement tools and cross-select; UI replay places the golden board**
  - Requirements: E-PCB-03, E-PRJ-01
  - Modules: ui, tests
  - Depends on: S5.15.1, S5.07.3
  - Deliverables: placement tools; cross-select and cross-probe with the schematic; `tests/ui/replays/pcb_place_golden.journal.jsonl`
  - Tests: T-05 `Ui.Tests Replay.PlaceGoldenBoard` (Accept: E-PCB-03a)
  - Done when: replay green
- [ ] **S5.15.3: Interactive router UI; UI replay routes the golden board**
  - Requirements: E-PCB-04, A-10
  - Modules: ui, tests
  - Depends on: S5.15.2, S5.09.10
  - Deliverables: router tool on the router services, never blocking the UI thread; `tests/ui/replays/pcb_route_golden.journal.jsonl`
  - Tests: T-05 `Ui.Tests Replay.RouteGoldenBoard` (Accept: E-PCB-04b)
  - Done when: replay green (S5 exit criterion)
- [ ] **S5.15.4: Rules editor, constraint grid, DRC browser and nets/length panel**
  - Requirements: E-RUL-01, E-RUL-02, E-PCB-05, U-11
  - Modules: ui
  - Depends on: S5.15.1, S5.08.4, S5.10.3
  - Deliverables: rule editor showing the applied rule for a selected object; constraint grid; DRC browser with click-to-locate and waivers; length/match panel
  - Tests: T-05 `Ui.Tests Drc.ClickLocates`, `Rules.ShowsAppliedRule`
  - Done when: tests green
- [ ] **S5.15.5: In-place 2D/3D switch with sections, measurement and clearance highlight**
  - Requirements: E-3D-01
  - Modules: ui, render
  - Depends on: S5.15.1, S5.13.3
  - Deliverables: 3D board view in the PCB editor; reference images
  - Tests: T-06 `Ui.Tests Snapshot.Board3dMatchesStack` (Accept: E-3D-01a)
  - Done when: tests green
- [ ] **S5.15.6: Stack manager and pour UI**
  - Requirements: E-PCB-01, E-PCB-07
  - Modules: ui
  - Depends on: S5.15.1, S5.11.5
  - Deliverables: stack editor with impedance calculator; pour properties and repour
  - Tests: T-05 `Ui.Tests Stack.EditLayers`, `Pours.RepourFromUi`
  - Done when: tests green

### S5.99 Stage gate

- [ ] **S5.99.1: Run the full S5 test suite and all checks**
  - Requirements: T-13, T-11, P-03
  - Modules: all
  - Depends on: S5.15.6, S5.15.5, S5.15.4, S5.15.3, S5.14.1, S5.13.4, S5.10.5
  - Deliverables: `scripts/ci-local.ps1 -Nightly` results (runs `scripts/test.ps1 -Coverage`, `-Bench`, `-Fuzz` and `scripts/check-selftest.ps1`); offline smoke
  - Tests: T-13 all suites incl. T-10 DRC; T-12 coverage ≥ 80 % for `ecad/pcb` rules and DRC; T-09 board loader fuzz target
  - Done when: everything green
- [ ] **S5.99.2: Run the S5 benchmarks (P-15, P-16, regressions)**
  - Requirements: P-15, P-16, T-08
  - Modules: tests
  - Depends on: S5.99.1
  - Deliverables: `bench/history.csv` rows
  - Tests: T-08 `Bench.RouteMouseMove2000x5000`, `Bench.BatchDrc8Layer2000` and earlier benchmarks
  - Done when: results recorded
- [ ] **S5.99.3: Update traceability and documentation for S5**
  - Requirements: T-13
  - Modules: docs
  - Depends on: S5.99.2
  - Deliverables: `docs/traceability.csv`, `docs/stages/S5.md`, `docs/format/board.md`, `docs/rules-language.md`, `docs/commands/`
  - Tests: `/check-traceability` reports no gaps for S5; `scripts/docs.ps1 -FailOnWarning` clean (needs Doxygen, see S0.14.1)
  - Done when: S5 requirements updated
- [ ] **S5.99.4: Human review checkpoint: stop and wait for approval**
  - Requirements: all S5 requirements (see `docs/stages/S5.md`)
  - Modules: none
  - Depends on: S5.99.3
  - Deliverables: S5 summary for the owner
  - Tests: none (review)
  - Done when: the owner approves S5 in writing

## Stage S6: MCAD ↔ ECAD integration

Stage plan: `docs/stages/S6.md`. Exit: **the reference scenario (T-14) passes end to end**, including the board-width
and connector-move edits. Per ADR 0015 (Accepted), minimal fabrication, BOM and pick-and-place writers are built here.

### S6.01 Cross-domain link spike (SP-7)

- [ ] **S6.01.1: Spike SP-7a: prototype link nodes across documents with review mode (time box 2 days)**
  - Requirements: X-09, X-10, A-07
  - Modules: link, doc, tests
  - Depends on: S5.99.4
  - Deliverables: prototype under `tests/link/spikes/`: board parameter → part sketch parameter link as a `RegenGraph` node spanning two documents; review mode holding the driven value; cycle detection at creation
  - Tests: spike harness: propagation correctness, held value in review mode, cycle rejection, regeneration cost
  - Done when: results recorded or the time box ends with results recorded
- [ ] **S6.01.2: Spike SP-7b: decide the link evaluation design in ADR 0040**
  - Requirements: X-09, X-10, A-07
  - Modules: link, docs
  - Depends on: S6.01.1
  - Deliverables: `docs/adr/0040-spike-sp7-links.md` (node model, review semantics, acknowledgement storage, unit crossing)
  - Tests: spike results attached
  - Done when: ADR Accepted before S6.02.1

### S6.02 Link model and manager

- [ ] **S6.02.1: Links document schema and LinkManager operations with cycle rejection**
  - Requirements: X-10, A-04, F-01
  - Modules: link, docs
  - Depends on: S6.01.2
  - Deliverables: `docs/format/schemas/links.schema.json`, `docs/format/links.md`; `src/link/LinkManager.{h,cpp}`; commands `link.create`, `.edit`, `.retarget`, `.lock`, `.unlock`, `.break`, `.reverse` (exactly one driver per link; targets by ObjectId and TopoRef)
  - Tests: T-01 `Link.Tests LinkManager.*`; `Link.Tests Cycles.RejectedAtCreation` (Accept: X-10b)
  - Done when: tests green
- [ ] **S6.02.2: Unit crossing and mapping expressions**
  - Requirements: A-09, A-08, X-02
  - Modules: link, core
  - Depends on: S6.02.1
  - Deliverables: int64 nm ↔ mm conversion only in `link`; mapping expressions such as `offset( driver, 0.5 mm )`
  - Tests: T-01 `Link.Tests Units.*`, `Mapping.*`
  - Done when: tests green
- [ ] **S6.02.3: Links as regeneration nodes with status; broken links never delete geometry**
  - Requirements: A-07, P-21, X-10
  - Modules: link, doc
  - Depends on: S6.02.2
  - Deliverables: link nodes in the project `RegenGraph`; status OK, out of date, broken, locked, cycle; driven feature goes to Error "link broken"
  - Tests: T-01 `Link.Tests Regen.*`, `Link.Tests Broken.KeepsGeometry`
  - Done when: tests green
- [ ] **S6.02.4: Review mode and the cross-domain change log**
  - Requirements: X-09
  - Modules: link
  - Depends on: S6.02.3
  - Deliverables: live, review and locked modes per link or per project; change entries (old value, new value, affected objects) since last acknowledgement; commands `link.change.accept`, `link.change.acknowledge`
  - Tests: T-02 `Integration.Tests Links.ReviewModeHoldsUntilAccepted` (Accept: X-09)
  - Done when: test green

### S6.03 Board as an assembly component

- [ ] **S6.03.1: Insert boards into assemblies with board TopoRefs**
  - Requirements: X-01, E-3D-01, A-06
  - Modules: link, asm
  - Depends on: S6.02.3, S5.13.2
  - Deliverables: board component geometry provider registered into `asm` (ADR 0025); board faces, edges, hole axes, component bodies and component frames as TopoRef targets
  - Tests: T-01 `Link.Tests BoardComponent.*`
  - Done when: tests green
- [ ] **S6.03.2: Mate the golden board into the enclosure base**
  - Requirements: X-01
  - Modules: tests
  - Depends on: S6.03.1
  - Deliverables: `tests/data/scripts/board_in_base.cmd.json`
  - Tests: T-02 `Integration.Tests Links.BoardMatesDof0` (Accept: X-01)
  - Done when: DOF reported as 0

### S6.04 Board outline linking

- [ ] **S6.04.1: Board drives mechanical (outline curves and size parameters)**
  - Requirements: X-02
  - Modules: link, sketch
  - Depends on: S6.03.1
  - Deliverables: outline exposed as projected sketch geometry and parameters `Board.Width`, `Board.Height`, `Board.Thickness`, curve set
  - Tests: T-02 `Integration.Tests Links.BoardWidthResizesCavity` (Accept: X-02a)
  - Done when: test green
- [ ] **S6.04.2: Mechanical drives board (sketch, face boundary or offset wall)**
  - Requirements: X-02, E-PCB-02
  - Modules: link, ecad/pcb
  - Depends on: S6.04.1
  - Deliverables: outline from a mechanical sketch, projected face boundary or offset of an inner wall with a clearance expression
  - Tests: T-02 `Integration.Tests Links.WallResizesBoardAndDrcReportsOutside` (Accept: X-02b)
  - Done when: test green
- [ ] **S6.04.3: Link cutouts in both directions**
  - Requirements: X-02
  - Modules: link
  - Depends on: S6.04.2
  - Deliverables: cutout links mirroring outline links
  - Tests: T-02 `Integration.Tests Links.CutoutsFollow`
  - Done when: test green

### S6.05 Holes, height limits and mechanically constrained components

- [ ] **S6.05.1: Mounting holes as linked points with fastener metadata**
  - Requirements: X-03
  - Modules: link, part
  - Depends on: S6.03.1
  - Deliverables: holes exposed with position, diameter and fastener metadata; bosses, standoffs, insert pilots and clearance holes driven by them, or the reverse
  - Tests: T-02 `Integration.Tests Links.HoleMoveMovesBoss3mm` (Accept: X-03)
  - Done when: test green
- [ ] **S6.05.2: Height regions and keepouts projected from the enclosure**
  - Requirements: X-04, E-PCB-02
  - Modules: link, ecad/pcb, geom
  - Depends on: S6.03.1, S5.08.3
  - Deliverables: projection of interior geometry onto the board (distance to nearest enclosure face minus clearance expression); keepouts under ribs, bosses, snaps; auto-update; used by placement rules and DRC
  - Tests: T-02 `Integration.Tests Links.LidLowered3mmCapViolatesHeight` (Accept: X-04)
  - Done when: test green
- [ ] **S6.05.3: Mechanical references drive component placement**
  - Requirements: X-05, E-PCB-03
  - Modules: link, ecad/pcb
  - Depends on: S6.03.1, S5.07.1
  - Deliverables: component position and rotation follow a mechanical reference; component locked in the PCB editor showing its link
  - Tests: T-02 `Integration.Tests Links.ConnectorFollowsOpening`
  - Done when: test green
- [ ] **S6.05.4: Component-driven cut with mating envelope and clearance**
  - Requirements: X-05
  - Modules: link, part
  - Depends on: S6.05.3
  - Deliverables: `src/link/features/ComponentCutFeature.{h,cpp}` registered into `part` (component body or library mating envelope plus clearance)
  - Tests: T-02 `Integration.Tests Links.ConnectorMove5mmMovesCutout` (Accept: X-05)
  - Done when: cut-out moves 5 mm and keeps 0.3 mm clearance

### S6.06 Unified interference and clearance

- [ ] **S6.06.1: Boards in assembly checks, enclosure bodies in PCB DRC, cross-probe results**
  - Requirements: X-06, M-ASM-04, E-RUL-02
  - Modules: link, asm, ecad/pcb
  - Depends on: S6.05.2, S5.13.3, S3.06.3
  - Deliverables: boards and components as ordinary components in interference/clearance; DRC 3D clearance to selected mechanical bodies; violation objects carry IDs for 3D view, PCB editor and mechanical tree
  - Tests: T-02 `Integration.Tests Links.CapVsRibFoundByBothChecks` (Accept: X-06)
  - Done when: test green

### S6.07 Variants and configurations

- [ ] **S6.07.1: Assembly variants for electronics**
  - Requirements: E-PRJ-03
  - Modules: ecad/sch, ecad/pcb, doc
  - Depends on: S5.99.4
  - Deliverables: named variants: fitted, DNP, alternate component, parameter overrides; active variant marks DNP in schematic display and hides DNP in 3D
  - Tests: T-01 `EcadSch.Tests Variants.*`
  - Done when: tests green
- [ ] **S6.07.2: Configurations select variants and drive feature suppression**
  - Requirements: X-08, M-PRT-11
  - Modules: link, asm, part
  - Depends on: S6.07.1, S6.05.4, S3.04.3
  - Deliverables: assembly configuration selects a board variant; DNP excluded from interference; features suppressed or changed by variant
  - Tests: T-02 `Integration.Tests Links.NoUsbConfigRemovesConnectorAndCutout` (Accept: X-08)
  - Done when: test green

### S6.08 Multi-board systems

- [ ] **S6.08.1: Mating connector pairs and cross-board connectivity**
  - Requirements: E-MB-01
  - Modules: link, ecad/sch
  - Depends on: S6.03.1
  - Deliverables: ADR 0018 applied for `link` → `ecad/sch`; connector pairs with direct or user-defined pin mapping; cross-board connectivity compile
  - Tests: T-01 `Link.Tests MultiBoard.*`
  - Done when: tests green
- [ ] **S6.08.2: Report multi-board mismatches**
  - Requirements: E-MB-01
  - Modules: link, tests
  - Depends on: S6.08.1
  - Deliverables: pin-mapping conflicts, net-name mismatches, unconnected mating connectors, misaligned 3D mates; `tests/data/scripts/two_board_stack.cmd.json`
  - Tests: T-02 `Integration.Tests MultiBoard.TwoBoardStackCompiles` (Accept: E-MB-01a); `MultiBoard.SwappedPinPairReported` (Accept: E-MB-01b)
  - Done when: tests green

### S6.09 Minimal manufacturing outputs (ADR 0015)

- [ ] **S6.09.1: Test-side Gerber X2 and Excellon readers**
  - Requirements: T-07, E-OUT-01
  - Modules: tests
  - Depends on: S5.99.4
  - Deliverables: `tests/support/fab/{GerberReader,ExcellonReader}.{h,cpp}` written from the public format specifications (basis of the future CAM viewer, AD-OUT-03)
  - Tests: T-01 `Support.Tests GerberReader.*`, `ExcellonReader.*` on hand-written samples
  - Done when: tests green
- [ ] **S6.09.2: Minimal Gerber X2 writer and job file**
  - Requirements: E-OUT-01, A-11
  - Modules: ecad/out
  - Depends on: S6.09.1, S5.11.5
  - Deliverables: `src/ecad/out/gerber/` copper, mask, paste, silkscreen and outline layers with X2 attributes; Gerber job file; writers registered through `IFabWriter`
  - Tests: T-07 `Golden.Tests Gerber.MinimalMatchesDatabase`
  - Done when: pad, track and region counts and positions match the board database exactly
- [ ] **S6.09.3: Minimal Excellon writer (plated and non-plated)**
  - Requirements: E-OUT-01
  - Modules: ecad/out
  - Depends on: S6.09.1, S5.12.2
  - Deliverables: `src/ecad/out/drill/ExcellonWriter.{h,cpp}` (PTH/NPTH split)
  - Tests: T-07 `Golden.Tests Excellon.MinimalMatchesDatabase`
  - Done when: hole counts and positions match exactly
- [ ] **S6.09.4: Minimal IPC-2581 writer with stack and drill spans; HDI acceptance**
  - Requirements: E-OUT-01, E-PCB-08
  - Modules: ecad/out, tests
  - Depends on: S6.09.3
  - Deliverables: `src/ecad/out/ipc2581/` (revision B or later: stack, layers, pads, drills, spans); test-side structural reader `tests/support/fab/Ipc2581Reader.{h,cpp}`; `tests/data/scripts/hdi_8layer.cmd.json`
  - Tests: T-07 `Golden.Tests Ipc2581.MinimalStructure`; `Golden.Tests Hdi.DrillSpansAndStackData` (Accept: E-PCB-08)
  - Done when: tests green
- [ ] **S6.09.5: Variant-aware parts list and pick-and-place for three variants**
  - Requirements: E-PRJ-03, E-BOM-01, E-OUT-01
  - Modules: ecad/out
  - Depends on: S6.07.1, S6.09.2
  - Deliverables: grouped parts-list core and CSV (full E-BOM-01 in S7.05.1); pick-and-place CSV (designator, X, Y, rotation, side, value, footprint); both use the active variant
  - Tests: T-04 `Golden.Tests Variants.ThreeVariantsBomAndPnp` (Accept: E-PRJ-03)
  - Done when: three correct BOMs and pick-and-place files

### S6.10 3D export of boards and products

- [ ] **S6.10.1: STEP export of boards and product assemblies**
  - Requirements: E-MCAD-02, M-IO-01
  - Modules: io, link
  - Depends on: S6.03.1, S2.09.2
  - Deliverables: AP214/AP242 export with colours, names and component structure; boards reached through the component geometry provider
  - Tests: T-07 `Golden.Tests StepExport.ProductReimports` (Accept: E-MCAD-02)
  - Done when: same component count, positions within 1e-4 mm and colours after re-import

### S6.11 Reference scenario (T-14)

- [ ] **S6.11.1: Script reference-scenario steps 1 to 4 (build, link, mate, check)**
  - Requirements: T-14, X-01, X-02, X-03, X-05
  - Modules: tests
  - Depends on: S6.02.4, S6.04.3, S6.05.4, S6.06.1
  - Deliverables: `tests/data/scripts/reference_scenario.cmd.json` part 1 (50 × 40 × 1.6 mm 2-layer board, USB-C, four M3 holes, 12 mm capacitor; base and lid as two parts per ADR 0032; cavity, bosses, cut-out and lid height linked; board mated DOF 0)
  - Tests: T-14 `Integration.Tests ReferenceScenario.Steps1to4` (interference-free, DRC clean)
  - Done when: test green
- [ ] **S6.11.2: Script reference-scenario steps 5 to 7 (width change, connector move, lid lowered)**
  - Requirements: T-14, X-02, X-04, X-05, X-06
  - Modules: tests
  - Depends on: S6.11.1
  - Deliverables: scenario part 2 with expected values
  - Tests: T-14 `Integration.Tests ReferenceScenario.Steps5to7`
  - Done when: cavity and bosses update at 60 mm; cut-out moves 5 mm with 0.3 mm kept; clash reported by both checks
- [ ] **S6.11.3: Script reference-scenario steps 8 and 9 (outputs, "no USB" configuration) and make T-14 a release gate**
  - Requirements: T-14, E-OUT-01, M-3DP-01, X-08
  - Modules: tests, scripts
  - Depends on: S6.11.2, S6.09.4, S6.07.2
  - Deliverables: scenario part 3; `scripts/test.ps1` fails the run if T-14 fails; T-14 added to the offline smoke set
  - Tests: T-14 `Integration.Tests ReferenceScenario.EndToEnd`; T-07 outputs validated by the test-side readers; enclosure 3MF watertight
  - Done when: full scenario green (S6 exit criterion)
- [ ] **S6.11.4: Meet P-17 (save or load a 50 MB project under 5 s)**
  - Requirements: P-17, F-02
  - Modules: doc, tests
  - Depends on: S6.11.3
  - Deliverables: generated 50 MB project (reference scenario scaled up); `Bench.SaveLoad50MB` for JSON and BSON; decision on embedded vs external mesh blobs recorded in `docs/format/blobs.md`
  - Tests: T-08 `Bench.SaveLoad50MB` under 5 s
  - Done when: target met or a defect recorded per spec 3.2

### S6.12 Integration user interface

- [ ] **S6.12.1: Link manager panel; UI replay of link operations**
  - Requirements: X-10, T-05
  - Modules: ui, tests
  - Depends on: S6.02.4, S5.15.1
  - Deliverables: panel listing driver, driven, quantity, direction, current value, status and where used, with all link actions; `tests/ui/replays/link_manager.journal.jsonl`
  - Tests: T-05 `Ui.Tests Replay.LinkCreateLockBreakRetarget` (Accept: X-10a)
  - Done when: replay green
- [ ] **S6.12.2: Cross-domain changes panel with review acceptance**
  - Requirements: X-09
  - Modules: ui
  - Depends on: S6.12.1
  - Deliverables: changes panel (old, new, affected objects), accept and acknowledge actions
  - Tests: T-05 `Ui.Tests Links.ReviewAcceptFromPanel`
  - Done when: tests green
- [ ] **S6.12.3: Linked-value indicators and cross-probing across 3D, PCB and tree**
  - Requirements: X-06, U-06, E-PRJ-01
  - Modules: ui
  - Depends on: S6.12.1, S6.06.1
  - Deliverables: link icon and driver name in the properties inspector; violation selection highlights in 3D, PCB editor and mechanical tree
  - Tests: T-05 `Ui.Tests CrossProbe.ViolationHighlightsEverywhere`
  - Done when: tests green

### S6.99 Stage gate

- [ ] **S6.99.1: Run the full S6 test suite, all checks and the reference scenario**
  - Requirements: T-13, T-14, T-11, P-03
  - Modules: all
  - Depends on: S6.12.3, S6.12.2, S6.11.4, S6.10.1, S6.08.2
  - Deliverables: `scripts/ci-local.ps1 -Nightly` results (runs `scripts/test.ps1 -Coverage`, `-Bench`, `-Fuzz` and `scripts/check-selftest.ps1`); offline smoke including T-14
  - Tests: T-13 all suites; T-14 green; T-09 links loader fuzz target
  - Done when: everything green
- [ ] **S6.99.2: Run the S6 benchmarks (P-17 at 50 MB, regressions)**
  - Requirements: P-17, T-08
  - Modules: tests
  - Depends on: S6.99.1
  - Deliverables: `bench/history.csv` rows
  - Tests: T-08 `Bench.SaveLoad50MB` and earlier benchmarks
  - Done when: results recorded
- [ ] **S6.99.3: Update traceability and documentation for S6**
  - Requirements: T-13
  - Modules: docs
  - Depends on: S6.99.2
  - Deliverables: `docs/traceability.csv`, `docs/stages/S6.md`, `docs/format/links.md`, `docs/commands/`
  - Tests: `/check-traceability` reports no gaps for S6; `scripts/docs.ps1 -FailOnWarning` clean (needs Doxygen, see S0.14.1)
  - Done when: S6 requirements updated
- [ ] **S6.99.4: Human review checkpoint: stop and wait for approval**
  - Requirements: all S6 requirements (see `docs/stages/S6.md`)
  - Modules: none
  - Depends on: S6.99.3
  - Deliverables: S6 summary for the owner
  - Tests: none (review)
  - Done when: the owner approves S6 in writing

## Stage S7: Documentation and manufacturing

Stage plan: `docs/stages/S7.md`. Exit: golden drawings match snapshots; Gerber/Excellon/IPC-2581 outputs pass
conformance (T-07); enclosure parts export as valid watertight 3MF.

### S7.01 Drawing views

- [ ] **S7.01.1: Drawing document with sheets, templates and title blocks**
  - Requirements: M-DRW-01, F-01
  - Modules: drw, libraries, docs
  - Depends on: S6.99.4
  - Deliverables: `docs/format/schemas/drawing.schema.json`, `docs/format/drawing.md`; sheets ISO A0–A4, ANSI A–E, custom; first/third-angle per sheet; templates with property-bound title blocks in `libraries/templates/` (original designs)
  - Tests: T-03 `Format.Tests RoundTrip` over drawing goldens; T-01 `Drw.Tests Sheets.*`
  - Done when: tests green
- [ ] **S7.01.2: Model and projected views with hidden-line removal and display modes**
  - Requirements: M-DRW-01
  - Modules: drw, geom
  - Depends on: S7.01.1
  - Deliverables: `src/drw/views/ViewGenerator.{h,cpp}` (kernel hidden-line removal behind `geom`); named and projected views; hidden lines removed/visible, shaded, shaded with edges; tangent edges visible/hidden/phantom; per-component visibility; configuration choice; runs as jobs
  - Tests: T-04 `Golden.Tests DrawingViews.Projected*` (edge counts)
  - Done when: tests green
- [ ] **S7.01.3: Section views with hatching**
  - Requirements: M-DRW-01
  - Modules: drw, geom
  - Depends on: S7.01.2
  - Deliverables: full, half, offset/stepped and aligned sections with hatch patterns
  - Tests: T-04 `Golden.Tests DrawingViews.Section*`
  - Done when: tests green
- [ ] **S7.01.4: Detail, broken, auxiliary, cropped and broken-out views**
  - Requirements: M-DRW-01
  - Modules: drw
  - Depends on: S7.01.2
  - Deliverables: remaining view types
  - Tests: T-04 `Golden.Tests DrawingViews.Detail*`, `Broken*`, `Auxiliary*`, `Cropped*`, `BrokenOut*`
  - Done when: tests green
- [ ] **S7.01.5: Exploded drawing views with explode lines**
  - Requirements: M-DRW-01, M-ASM-06
  - Modules: drw, asm
  - Depends on: S7.01.2, S3.07.3
  - Deliverables: exploded view type using the assembly's explode steps and lines
  - Tests: T-06 `Golden.Tests Snapshot.ExplodedDrawingViewShowsLines` (Accept: M-ASM-06b)
  - Done when: test green
- [ ] **S7.01.6: Keep views and annotations associative through model changes**
  - Requirements: M-DRW-01, A-06, A-07
  - Modules: drw
  - Depends on: S7.01.3, S7.01.4
  - Deliverables: views as regeneration nodes; annotations re-attach via TopoRef or report "reference lost"
  - Tests: T-02 `Integration.Tests Drawings.HoleMoveUpdatesSectionAndDetailViews`
  - Done when: test green; dimension values are covered in S7.02.1
- [ ] **S7.01.7: Vector PDF, SVG and PNG output**
  - Requirements: M-DRW-01, M-IO-01
  - Modules: drw, build
  - Depends on: S7.01.2
  - Deliverables: PDF writer (decision in `docs/adr/NNNN-pdf-output.md`: in-house vector writer or a GPL-compatible library); SVG and PNG writers; command `drw.export`
  - Tests: T-07 `Golden.Tests DrawingOutput.PdfStructure`, `SvgStructure`, `PngSnapshot`
  - Done when: tests green; PDF text and vectors parse with the test-side reader
- [ ] **S7.01.8: DXF export of drawings**
  - Requirements: M-IO-01, M-DRW-01
  - Modules: drw
  - Depends on: S7.01.2
  - Deliverables: `src/drw/out/DxfWriter.{h,cpp}` (layers, linetypes, text, hatches)
  - Tests: T-07 `Golden.Tests DrawingOutput.DxfStructure`
  - Done when: tests green (re-import tested in S7.07.7)
- [ ] **S7.01.9: Snapshot tests for 10 golden drawings**
  - Requirements: M-DRW-01, T-06
  - Modules: tests
  - Depends on: S7.01.7, S7.01.5
  - Deliverables: `tests/data/scripts/drawing_01..10.cmd.json`; reference images in `tests/data/snapshots/`
  - Tests: T-06 `Golden.Tests Snapshot.Drawings01..10` (Accept: M-DRW-01a)
  - Done when: all 10 match (S7 exit criterion)

### S7.02 Annotations

- [ ] **S7.02.1: Dimensions and model-dimension insertion (ADR 0019)**
  - Requirements: M-DRW-02, M-DRW-01
  - Modules: drw, sketch
  - Depends on: S7.01.6
  - Deliverables: ADR 0019 applied (`drw` → `sketch`); linear, aligned, ordinate, baseline and chain, angular, radius, diameter, chamfer, arc length; insert model (driving) dimensions; driven dimensions are reference
  - Tests: T-01 `Drw.Tests Dimensions.*`; T-02 `Integration.Tests Drawings.HoleMoveUpdatesSectionDetailAndDims` (Accept: M-DRW-01b)
  - Done when: tests green
- [ ] **S7.02.2: Tolerances and the ISO 286 limits-and-fits table**
  - Requirements: M-DRW-02, CR-05
  - Modules: drw, libraries, docs
  - Depends on: S7.02.1
  - Deliverables: symmetric, bilateral, limit, basic, min/max, fit designations; user-editable fits table in `libraries/drafting-standards/` with source row in `docs/data-sources.md` or marked user-supplied
  - Tests: T-01 `Drw.Tests Tolerances.*`
  - Done when: tests green
- [ ] **S7.02.3: GD&T frames, datum features and datum targets**
  - Requirements: M-DRW-02
  - Modules: drw
  - Depends on: S7.02.1
  - Deliverables: feature control frames with ASME Y14.5 / ISO 1101 symbol sets (original glyph drawings), composite frames, datums, datum targets
  - Tests: T-01 `Drw.Tests Gdt.*`
  - Done when: tests green
- [ ] **S7.02.4: Other symbols, notes, leaders, revision clouds, balloons and thread callouts**
  - Requirements: M-DRW-02
  - Modules: drw
  - Depends on: S7.02.1
  - Deliverables: surface finish, weld symbols, centre marks, centrelines, notes with property links, leaders, revision clouds, balloons (manual and auto), thread callouts from cosmetic-thread metadata
  - Tests: T-01 `Drw.Tests Symbols.*`
  - Done when: tests green
- [ ] **S7.02.5: ISO and ANSI drafting presets as open JSON**
  - Requirements: M-DRW-02, F-07
  - Modules: drw, libraries
  - Depends on: S7.02.1
  - Deliverables: `libraries/drafting-standards/{iso,ansi}.json` (text height, arrows, units, decimals); user-editable
  - Tests: T-01 `Drw.Tests Presets.*`
  - Done when: tests green
- [ ] **S7.02.6: Annotation snapshot tests**
  - Requirements: M-DRW-02, T-06
  - Modules: tests
  - Depends on: S7.02.2, S7.02.3, S7.02.4, S7.02.5
  - Deliverables: annotated golden drawings and reference images
  - Tests: T-06 `Golden.Tests Snapshot.Annotations*` (Accept: M-DRW-02a)
  - Done when: tests green

### S7.03 Drawing tables

- [ ] **S7.03.1: Provider interfaces for BOM rows, view types and tables**
  - Requirements: M-DRW-03, A-11
  - Modules: drw
  - Depends on: S7.01.1
  - Deliverables: `drw::IBomProvider`, `drw::IViewTypeProvider`, `drw::ITableProvider` registries
  - Tests: T-01 `Drw.Tests Providers.*`
  - Done when: tests green
- [ ] **S7.03.2: BOM table with balloon synchronisation**
  - Requirements: M-DRW-03
  - Modules: drw, asm
  - Depends on: S7.03.1, S7.02.4
  - Deliverables: top-level, parts-only and indented; property-bound columns; sort and grouping; item numbers synchronised with balloons; CSV export
  - Tests: T-04 `Golden.Tests BomTable.ExpectedRows` (Accept: M-DRW-03a); T-02 `Integration.Tests BomTable.AssemblyChangeUpdatesBomAndBalloons` (Accept: M-DRW-03b)
  - Done when: tests green
- [ ] **S7.03.3: Hole, revision and general tables with CSV export**
  - Requirements: M-DRW-03
  - Modules: drw
  - Depends on: S7.03.1
  - Deliverables: hole table from an origin (tag, position, size), revision table, general table; cut-list and bend tables come through providers in S8
  - Tests: T-04 `Golden.Tests Tables.Hole*`, `Revision*`, `General*`
  - Done when: tests green

### S7.04 Manufacturing outputs (full E-OUT-01)

- [ ] **S7.04.1: Complete Gerber X2 for every layer type**
  - Requirements: E-OUT-01
  - Modules: ecad/out
  - Depends on: S6.09.2
  - Deliverables: all copper, mask, paste, silkscreen, outline and mechanical layers with full attribute sets; job file complete
  - Tests: T-07 `Golden.Tests Gerber.AllLayers*`
  - Done when: tests green
- [ ] **S7.04.2: Complete drill outputs (slots, backdrill spans, Gerber X2 drill option)**
  - Requirements: E-OUT-01, E-PCB-08
  - Modules: ecad/out
  - Depends on: S6.09.3
  - Deliverables: Excellon slots and backdrill spans; Gerber X2 drill files as an option
  - Tests: T-07 `Golden.Tests Drill.SlotsBackdrill*`
  - Done when: tests green
- [ ] **S7.04.3: Complete IPC-2581 full-board export**
  - Requirements: E-OUT-01
  - Modules: ecad/out
  - Depends on: S6.09.4
  - Deliverables: full content (components, nets, layers, stack, drill, BOM section)
  - Tests: T-07 `Golden.Tests Ipc2581.FullBoard*`
  - Done when: tests green
- [ ] **S7.04.4: Configurable, variant-aware pick-and-place**
  - Requirements: E-OUT-01, E-PRJ-03
  - Modules: ecad/out
  - Depends on: S6.09.5
  - Deliverables: origin, units and rotation convention options
  - Tests: T-07 `Golden.Tests Pnp.Options*`
  - Done when: tests green
- [ ] **S7.04.5: IPC-D-356 netlist with a test-side reader**
  - Requirements: E-OUT-01
  - Modules: ecad/out, tests
  - Depends on: S6.09.1
  - Deliverables: `src/ecad/out/ipcd356/`; `tests/support/fab/IpcD356Reader.{h,cpp}`
  - Tests: T-07 `Golden.Tests IpcD356.MatchesNetlist`
  - Done when: tests green
- [ ] **S7.04.6: Output configurations run from UI or CLI**
  - Requirements: E-OUT-01, A-02
  - Modules: ecad/out, cli
  - Depends on: S7.04.1, S7.04.2, S7.04.3, S7.04.4, S7.04.5
  - Deliverables: named output jobs saved in the project; command `output.job.run` writing a dated folder with optional zip
  - Tests: T-02 `Integration.Tests Outputs.JobRunsHeadless`
  - Done when: tests green
- [ ] **S7.04.7: T-07 conformance for fabrication outputs**
  - Requirements: E-OUT-01, T-07
  - Modules: tests
  - Depends on: S7.04.6
  - Deliverables: conformance suite over golden boards with volatile fields masked
  - Tests: T-07 `Golden.Tests FabConformance.GerberCountsAndPositionsExact` (Accept: E-OUT-01)
  - Done when: suite green (S7 exit criterion)

### S7.05 Bills of materials

- [ ] **S7.05.1: Complete the basic parts list**
  - Requirements: E-BOM-01
  - Modules: ecad/out
  - Depends on: S6.09.5, S7.03.1
  - Deliverables: grouping by library item and revision or value + footprint + MPN; columns incl. range-compressed designators; variant-aware with DNP list; CSV and JSON; BOM rows provided to drawings
  - Tests: T-04 `Golden.Tests Bom.RowsPerVariant` (Accept: E-BOM-01)
  - Done when: tests green
- [ ] **S7.05.2: Combined product BOM through drw::IBomProvider in link**
  - Requirements: X-07, M-DRW-03
  - Modules: link, drw
  - Depends on: S7.05.1, S7.03.2
  - Deliverables: ADR 0018 applied for `link` → `drw`; board components as an indented level under each board or flattened, with mechanical parts and fasteners
  - Tests: T-04 `Golden.Tests Bom.ProductBomQuantities` (Accept: X-07)
  - Done when: test green

### S7.06 PCB fabrication and assembly drawings

- [ ] **S7.06.1: PCB view types (board, drill drawing and table, stack and impedance tables)**
  - Requirements: E-DOC-01
  - Modules: ecad/out, drw
  - Depends on: S7.03.1, S7.04.2
  - Deliverables: view types registered through `IViewTypeProvider`: board views per layer set, drill drawing with automatic symbol per hole size and drill table, stack table, impedance table, board dimensions, fabrication notes
  - Tests: T-02 `Integration.Tests PcbDrawings.NewHoleSizeAddsRowAndSymbol` (Accept: E-DOC-01b)
  - Done when: test green
- [ ] **S7.06.2: Assembly views, edge details and isometric view**
  - Requirements: E-DOC-01, E-PRJ-03
  - Modules: ecad/out, drw
  - Depends on: S7.06.1, S7.05.1
  - Deliverables: top/bottom assembly views with designators (variant-aware) and BOM table; board-edge detail and section views; isometric 3D view
  - Tests: T-04 `Golden.Tests PcbDrawings.AssemblyViewVariant*`
  - Done when: tests green
- [ ] **S7.06.3: Snapshot tests for golden fabrication and assembly drawings**
  - Requirements: E-DOC-01, T-06
  - Modules: tests
  - Depends on: S7.06.2
  - Deliverables: reference images
  - Tests: T-06 `Golden.Tests Snapshot.FabAndAssemblyDrawings` (Accept: E-DOC-01a)
  - Done when: tests green

### S7.07 Neutral formats (full M-IO-01)

- [ ] **S7.07.1: DXF import into sketches and drawings (ADR 0020)**
  - Requirements: M-IO-01, A-10
  - Modules: io, sketch, drw
  - Depends on: S7.01.8
  - Deliverables: ADR 0020 applied (`io` → `sketch`, `drw`); `src/io/dxf/DxfImporter.{h,cpp}` as a background job
  - Tests: T-07 `Io.Tests DxfImport.*`
  - Done when: tests green
- [ ] **S7.07.2: DXF export of sketches and PCB layers**
  - Requirements: M-IO-01
  - Modules: io, ecad/out
  - Depends on: S7.07.1
  - Deliverables: sketch DXF export in `io`; PCB layer DXF export in `ecad/out`
  - Tests: T-07 `Io.Tests DxfExport.Sketch*`; `Golden.Tests DxfExport.PcbLayers*`
  - Done when: tests green
- [ ] **S7.07.3: IGES import and export**
  - Requirements: M-IO-01
  - Modules: io, geom
  - Depends on: S6.99.4
  - Deliverables: `src/io/iges/`; commands `io.import.iges`, `io.export.iges`
  - Tests: T-07 `Io.Tests Iges.*`
  - Done when: tests green
- [ ] **S7.07.4: STL and OBJ import as mesh bodies; OBJ export**
  - Requirements: M-IO-01, M-3DP-01
  - Modules: io, part
  - Depends on: S6.99.4
  - Deliverables: mesh body type in `part`; importers and OBJ exporter
  - Tests: T-07 `Io.Tests StlImport.*`, `Obj.*`
  - Done when: tests green
- [ ] **S7.07.5: glTF 2.0/GLB and VRML 2.0 export**
  - Requirements: M-IO-01
  - Modules: io
  - Depends on: S6.99.4
  - Deliverables: appearances mapped to PBR materials for glTF
  - Tests: T-07 `Io.Tests Gltf.*`, `Vrml.*` (structural parsers in `tests/support`)
  - Done when: tests green
- [ ] **S7.07.6: 3MF import (should) and the background import queue**
  - Requirements: M-IO-01, A-10
  - Modules: io, ui
  - Depends on: S7.07.4
  - Deliverables: 3MF import through lib3mf; import queue for multiple files with progress
  - Tests: T-07 `Io.Tests ThreeMfImport.*`; T-01 `Io.Tests ImportQueue.*`
  - Done when: tests green
- [ ] **S7.07.7: Round-trip tests for every export with an import counterpart**
  - Requirements: M-IO-01, T-07
  - Modules: tests
  - Depends on: S7.07.2, S7.07.3, S7.07.6
  - Deliverables: round-trip suite STEP, IGES, STL, 3MF, OBJ, DXF
  - Tests: T-07 `Golden.Tests IoRoundTrip.AllFormats` (Accept: M-IO-01a)
  - Done when: suite green

### S7.08 3D printing export (full M-3DP-01) and board exchange

- [ ] **S7.08.1: Full 3D-print export options**
  - Requirements: M-3DP-01
  - Modules: io
  - Depends on: S7.07.4
  - Deliverables: body, part, selected components or assembly; one file or one per body/component with filename template; 3MF per-object colour/material and build items; output frame, units, scale, place on build plane
  - Tests: T-07 `Golden.Tests Print.Options*`
  - Done when: tests green
- [ ] **S7.08.2: Batch export of printable parts and PCB fit-check mock-ups**
  - Requirements: M-3DP-01, E-MCAD-02
  - Modules: io, link
  - Depends on: S7.08.1
  - Deliverables: "export all printable parts" (excludes purchased parts, fasteners and boards unless selected); board plus simplified component bodies mock-up
  - Tests: T-07 `Golden.Tests Print.BatchAndFitCheck`; enclosure parts watertight 3MF (S7 exit criterion)
  - Done when: tests green
- [ ] **S7.08.3: IDF 3.0 export and board STL/3MF**
  - Requirements: E-MCAD-02
  - Modules: io, link
  - Depends on: S7.08.2
  - Deliverables: IDF board and component outlines with heights; IDX recorded as Future if not done
  - Tests: T-07 `Golden.Tests Idf.Structure`
  - Done when: tests green
- [ ] **S7.08.4: Complete mesh validation (self-intersections, degenerate triangles)**
  - Requirements: M-3DP-01
  - Modules: io
  - Depends on: S7.08.1
  - Deliverables: per-body report covering every check, with "export anyway"
  - Tests: T-07 `Io.Tests MeshValidation.AllChecks`
  - Done when: tests green

### S7.09 Drawing and output user interface

- [ ] **S7.09.1: Drawing editor with view placement**
  - Requirements: M-DRW-01, U-04
  - Modules: ui
  - Depends on: S7.01.9
  - Deliverables: `src/ui/drawing/` (sheet tree, view palette, properties, printing)
  - Tests: T-05 `Ui.Tests Drawing.PlaceProjectedView`
  - Done when: tests green
- [ ] **S7.09.2: Annotation and table tools**
  - Requirements: M-DRW-02, M-DRW-03, U-07
  - Modules: ui
  - Depends on: S7.09.1, S7.02.6, S7.03.3
  - Deliverables: dimension, GD&T and symbol tools; table insertion and CSV export
  - Tests: T-05 `Ui.Tests Replay.AnnotateDrawing`
  - Done when: tests green
- [ ] **S7.09.3: Output job editor**
  - Requirements: E-OUT-01
  - Modules: ui
  - Depends on: S7.04.6
  - Deliverables: output configuration panel with run and open-folder actions
  - Tests: T-05 `Ui.Tests Outputs.RunJobFromUi`
  - Done when: tests green

### S7.99 Stage gate

- [ ] **S7.99.1: Run the full S7 test suite, all checks and the reference scenario**
  - Requirements: T-13, T-14, T-11, P-03
  - Modules: all
  - Depends on: S7.09.3, S7.09.2, S7.08.4, S7.08.3, S7.07.7, S7.06.3, S7.05.2, S7.04.7
  - Deliverables: `scripts/ci-local.ps1 -Nightly` results (runs `scripts/test.ps1 -Coverage`, `-Bench`, `-Fuzz` and `scripts/check-selftest.ps1`); offline smoke
  - Tests: T-13 all suites; T-14 green; T-09 fuzz targets for DXF, IGES, STL, OBJ, 3MF importers
  - Done when: everything green
- [ ] **S7.99.2: Run the benchmark regressions**
  - Requirements: T-08
  - Modules: tests
  - Depends on: S7.99.1
  - Deliverables: `bench/history.csv` rows
  - Tests: T-08 all existing benchmarks
  - Done when: results recorded
- [ ] **S7.99.3: Update traceability and documentation for S7**
  - Requirements: T-13
  - Modules: docs
  - Depends on: S7.99.2
  - Deliverables: `docs/traceability.csv`, `docs/stages/S7.md`, `docs/format/drawing.md`, `docs/commands/`, `docs/data-sources.md`
  - Tests: `/check-traceability` reports no gaps for S7; `scripts/docs.ps1 -FailOnWarning` clean (needs Doxygen, see S0.14.1)
  - Done when: S7 requirements updated
- [ ] **S7.99.4: Human review checkpoint: stop and wait for approval**
  - Requirements: all S7 requirements (see `docs/stages/S7.md`)
  - Modules: none
  - Depends on: S7.99.3
  - Deliverables: S7 summary for the owner
  - Tests: none (review)
  - Done when: the owner approves S7 in writing

## Stage S8: Advanced modeling

Stage plan: `docs/stages/S8.md`. Exit: golden sheet-metal flat patterns match analytic lengths; golden weldment cut
list matches; surfacing golden tests pass.

### S8.01 Multibody parts

- [ ] **S8.01.1: Combine and split bodies**
  - Requirements: M-PRT-05
  - Modules: part, geom
  - Depends on: S7.99.4
  - Deliverables: commands `part.body.combine` (add, subtract, common), `part.body.split` (by plane, surface or sketch; optional save-to-new-part)
  - Tests: T-04 `Golden.Tests Multibody.CombineSplit*`
  - Done when: tests green
- [ ] **S8.01.2: Move, copy, delete and keep bodies; body folders**
  - Requirements: M-PRT-05
  - Modules: part
  - Depends on: S8.01.1
  - Deliverables: commands `part.body.move`, `.copy`, `.delete`, `.keep`, `part.bodyFolder.*`
  - Tests: T-01 `Part.Tests Bodies.*`
  - Done when: tests green
- [ ] **S8.01.3: Derived parts linked to a master body**
  - Requirements: M-PRT-05, A-07
  - Modules: part
  - Depends on: S8.01.1
  - Deliverables: insert another part as a derived body with update on master change
  - Tests: T-02 `Integration.Tests Multibody.DerivedPartUpdates`
  - Done when: test green
- [ ] **S8.01.4: Golden enclosure split into base and lid; add the split variant to T-14**
  - Requirements: M-PRT-05, T-14
  - Modules: tests
  - Depends on: S8.01.3, S6.11.3
  - Deliverables: `tests/data/scripts/enclosure_split.cmd.json`; `reference_scenario_split.cmd.json` variant (ADR 0032, C-02)
  - Tests: T-04 `Golden.Tests Multibody.EnclosureSplitTwoBodiesTwoDerived` (Accept: M-PRT-05); T-14 `Integration.Tests ReferenceScenario.SplitVariant`
  - Done when: tests green

### S8.02 Surface modeling

- [ ] **S8.02.1: Extruded, revolved, swept, lofted, planar, ruled and offset surfaces**
  - Requirements: M-PRT-06
  - Modules: part, geom
  - Depends on: S7.99.4
  - Deliverables: surface body type; surface features with commands `part.surface.*.create`
  - Tests: T-04 `Golden.Tests Surfaces.Basic*`
  - Done when: tests green
- [ ] **S8.02.2: Filled n-sided patches and mid-surface (should)**
  - Requirements: M-PRT-06
  - Modules: part, geom
  - Depends on: S8.02.1
  - Deliverables: filled surface with tangency/curvature conditions; mid-surface as a should
  - Tests: T-04 `Golden.Tests Surfaces.Filled*`
  - Done when: tests green
- [ ] **S8.02.3: Knit, trim, untrim and extend**
  - Requirements: M-PRT-06
  - Modules: part, geom
  - Depends on: S8.02.1
  - Deliverables: knit with "try to form solid"; standard and mutual trim; untrim; extend
  - Tests: T-04 `Golden.Tests Surfaces.KnitTrim*`
  - Done when: tests green
- [ ] **S8.02.4: Thicken, delete face, replace face and cut with surface**
  - Requirements: M-PRT-06
  - Modules: part, geom
  - Depends on: S8.02.3
  - Deliverables: surface operations producing solids
  - Tests: T-04 `Golden.Tests Surfaces.Thicken*`, `ReplaceFace*`, `CutWithSurface*`
  - Done when: tests green
- [ ] **S8.02.5: Zebra stripes and curvature map (should)**
  - Requirements: M-PRT-06
  - Modules: render, ui
  - Depends on: S8.02.1
  - Deliverables: viewport analysis shaders
  - Tests: T-06 `Ui.Tests Snapshot.ZebraStripes`
  - Done when: tests green, or the slip is recorded
- [ ] **S8.02.6: Complete the boundary feature (must by S8)**
  - Requirements: M-PRT-01
  - Modules: part, geom
  - Depends on: S8.02.1, S2.03.7
  - Deliverables: boundary feature for solids and surfaces with tangency/curvature conditions
  - Tests: T-04 `Golden.Tests PartGolden.BoundaryComplete*`
  - Done when: tests green
- [ ] **S8.02.7: Golden surface-to-solid models**
  - Requirements: M-PRT-06
  - Modules: tests
  - Depends on: S8.02.4
  - Deliverables: `tests/data/scripts/surface_knit_solid.cmd.json`, `surface_thicken_loft.cmd.json`
  - Tests: T-04 `Golden.Tests Surfaces.KnitFormsValidSolid` (Accept: M-PRT-06a); `Surfaces.ThickenedLoftVolume` (Accept: M-PRT-06b)
  - Done when: tests green (S8 exit criterion)

### S8.03 Direct editing

- [ ] **S8.03.1: Move/rotate faces and offset faces as parametric features**
  - Requirements: M-PRT-07
  - Modules: part, geom
  - Depends on: S7.99.4
  - Deliverables: direct-edit features recorded in the tree; work on native and imported bodies
  - Tests: T-04 `Golden.Tests DirectEdit.MoveOffset*`
  - Done when: tests green
- [ ] **S8.03.2: Delete faces with healing, replace faces and resize fillets**
  - Requirements: M-PRT-07
  - Modules: part, geom
  - Depends on: S8.03.1
  - Deliverables: remaining direct edits
  - Tests: T-04 `Golden.Tests DirectEdit.DeleteReplaceResize*`
  - Done when: tests green
- [ ] **S8.03.3: Golden imported STEP bracket edits**
  - Requirements: M-PRT-07
  - Modules: tests
  - Depends on: S8.03.2
  - Deliverables: `tests/data/import/step/bracket.step` (licence noted); `tests/data/scripts/direct_edit_bracket.cmd.json`
  - Tests: T-04 `Golden.Tests DirectEdit.BracketMoveHoleDeleteFilletOffset` (Accept: M-PRT-07)
  - Done when: all bodies valid after each edit

### S8.04 Sheet metal

- [ ] **S8.04.1: Sheet-metal parameters and gauge tables**
  - Requirements: M-SHM-01, CR-05
  - Modules: part, libraries, docs
  - Depends on: S7.99.4
  - Deliverables: per part or body thickness, bend radius, bend allowance method (K-factor, allowance, deduction, table), relief; `libraries/gauges/` with sources in `docs/data-sources.md`
  - Tests: T-01 `Part.Tests SheetMetalParams.*`
  - Done when: tests green
- [ ] **S8.04.2: Base flange/tab, edge flange and miter flange**
  - Requirements: M-SHM-01
  - Modules: part, geom
  - Depends on: S8.04.1
  - Deliverables: `src/part/sheetmetal/` flange features; bends carry radius and angle metadata
  - Tests: T-04 `Golden.Tests SheetMetal.Flanges*`
  - Done when: tests green
- [ ] **S8.04.3: Hems, jogs, sketched bends and lofted bends**
  - Requirements: M-SHM-01
  - Modules: part, geom
  - Depends on: S8.04.2
  - Deliverables: hem types (closed, open, teardrop, rolled); jog; sketched bend; lofted bend (formed and bent)
  - Tests: T-04 `Golden.Tests SheetMetal.HemsJogsBends*`
  - Done when: tests green
- [ ] **S8.04.4: Closed corners, corner relief and corner treatments**
  - Requirements: M-SHM-01
  - Modules: part, geom
  - Depends on: S8.04.2
  - Deliverables: closed corner; relief types; break and weld corners
  - Tests: T-04 `Golden.Tests SheetMetal.Corners*`
  - Done when: tests green
- [ ] **S8.04.5: Forming tools, unfold/fold and convert-to-sheet-metal (should)**
  - Requirements: M-SHM-01
  - Modules: part, geom
  - Depends on: S8.04.3
  - Deliverables: forming tools from a user library; unfold/fold for cuts across bends; conversion of a shelled solid as a should
  - Tests: T-04 `Golden.Tests SheetMetal.FormingUnfoldFold*`
  - Done when: tests green
- [ ] **S8.04.6: Golden sheet-metal parts rebuild with bend metadata**
  - Requirements: M-SHM-01
  - Modules: tests
  - Depends on: S8.04.4, S8.04.5
  - Deliverables: `tests/data/scripts/sheetmetal_*.cmd.json`
  - Tests: T-04 `Golden.Tests SheetMetal.GoldenRebuildWithBendMetadata` (Accept: M-SHM-01)
  - Done when: tests green

### S8.05 Flat pattern

- [ ] **S8.05.1: Automatically maintained flat pattern as a derived configuration**
  - Requirements: M-SHM-02
  - Modules: part
  - Depends on: S8.04.6
  - Deliverables: flat pattern with bend lines, direction, angle, radius, order notes and flat size
  - Tests: T-01 `Part.Tests FlatPattern.*`
  - Done when: tests green
- [ ] **S8.05.2: Flat lengths match the analytic K-factor calculation**
  - Requirements: M-SHM-02
  - Modules: tests
  - Depends on: S8.05.1
  - Deliverables: 10 golden bent parts; analytic oracle in `tests/support/oracles/BendAllowance.{h,cpp}`
  - Tests: T-04 `Golden.Tests FlatPattern.LengthsWithin0_01mm` (Accept: M-SHM-02a)
  - Done when: all 10 within 0.01 mm (S8 exit criterion)
- [ ] **S8.05.3: Flat-pattern DXF export with layer mapping**
  - Requirements: M-SHM-02, M-IO-01
  - Modules: io, part
  - Depends on: S8.05.1, S7.07.2
  - Deliverables: configurable layer mapping (outline, interior cuts, bend-up, bend-down, etch)
  - Tests: T-07 `Golden.Tests FlatPattern.DxfReimportsWithLayers` (Accept: M-SHM-02b)
  - Done when: test green
- [ ] **S8.05.4: Bend table provider for drawings**
  - Requirements: M-SHM-02, M-DRW-03
  - Modules: part, drw
  - Depends on: S8.05.1, S7.03.1
  - Deliverables: sheet-metal bend table through `drw::ITableProvider`
  - Tests: T-04 `Golden.Tests Tables.BendTable`
  - Done when: tests green

### S8.06 Weldments and structural frames

- [ ] **S8.06.1: Structural profile library**
  - Requirements: M-WLD-01, CR-05
  - Modules: part, libraries, docs
  - Depends on: S7.99.4
  - Deliverables: `libraries/profiles/` parametric cross-sections (square/rectangular tube, round tube, pipe, angle, channel, I/H-beam, T-section, flat bar, round bar) from public dimensional data; user sketch profiles; `docs/data-sources.md` rows
  - Tests: T-01 `Part.Tests Profiles.SeedLoads`
  - Done when: tests green
- [ ] **S8.06.2: Structural members along 2D and 3D sketch segments**
  - Requirements: M-WLD-01, M-SKT-03
  - Modules: part
  - Depends on: S8.06.1
  - Deliverables: `src/part/weldments/` members with profile rotation, pierce point and groups
  - Tests: T-04 `Golden.Tests Weldments.HandrailStructureMembers` (Accept: M-SKT-03c)
  - Done when: tests green
- [ ] **S8.06.3: Corner treatments at joints**
  - Requirements: M-WLD-01
  - Modules: part, geom
  - Depends on: S8.06.2
  - Deliverables: miter, butt (either member trimmed), trim/extend to faces or bodies with gap control
  - Tests: T-04 `Golden.Tests Weldments.Corners*`
  - Done when: tests green
- [ ] **S8.06.4: End caps, gussets and weld beads**
  - Requirements: M-WLD-01
  - Modules: part, geom
  - Depends on: S8.06.3
  - Deliverables: end caps; triangular and polygonal gussets; cosmetic weld beads with optional simple geometry
  - Tests: T-04 `Golden.Tests Weldments.CapsGussetsBeads*`
  - Done when: tests green
- [ ] **S8.06.5: Golden rectangular frame with mitred corners**
  - Requirements: M-WLD-01
  - Modules: tests
  - Depends on: S8.06.4
  - Deliverables: `tests/data/scripts/weldment_frame.cmd.json`
  - Tests: T-04 `Golden.Tests Weldments.FrameLengthsAndMass` (Accept: M-WLD-01a); T-02 `Integration.Tests Weldments.Sketch3dChangeUpdatesMembers` (Accept: M-WLD-01b)
  - Done when: tests green
- [ ] **S8.06.6: Cut list with CSV export and drawing table**
  - Requirements: M-WLD-02, M-DRW-03
  - Modules: part, drw
  - Depends on: S8.06.5, S7.03.1
  - Deliverables: automatic cut list (item, quantity, profile, material, length, end angles both ends and both planes, mass, custom properties); CSV; table via `drw::ITableProvider`
  - Tests: T-04 `Golden.Tests Weldments.CutListMatches` (Accept: M-WLD-02)
  - Done when: test green (S8 exit criterion)

### S8.07 Full use of 3D sketches

- [ ] **S8.07.1: Use 3D sketches as loft guide curves and future routing paths**
  - Requirements: M-SKT-03
  - Modules: part, sketch
  - Depends on: S8.06.2, S2.03.6
  - Deliverables: loft guide curves from 3D sketches; connection-point metadata on parts for future pipe routing (SW-RTE-01 seam)
  - Tests: T-04 `Golden.Tests Sketch3d.LoftGuides`
  - Done when: tests green

### S8.08 Advanced modeling user interface

- [ ] **S8.08.1: Multibody and surface tools**
  - Requirements: M-PRT-05, M-PRT-06
  - Modules: ui
  - Depends on: S8.01.4, S8.02.7
  - Deliverables: body folders in the tree; surface tool panels with preview
  - Tests: T-05 `Ui.Tests Replay.SplitEnclosure`
  - Done when: tests green
- [ ] **S8.08.2: Direct-edit handles in the viewport**
  - Requirements: M-PRT-07, U-07
  - Modules: ui, render
  - Depends on: S8.03.3
  - Deliverables: face drag and offset handles that run direct-edit commands
  - Tests: T-05 `Ui.Tests Replay.DirectEditMoveFace`
  - Done when: tests green
- [ ] **S8.08.3: Sheet-metal and weldment tools**
  - Requirements: M-SHM-01, M-SHM-02, M-WLD-01, M-WLD-02
  - Modules: ui
  - Depends on: S8.05.3, S8.06.6
  - Deliverables: flange and bend tools; flat-pattern toggle; member placement tool; cut-list panel
  - Tests: T-05 `Ui.Tests Replay.EdgeFlange`, `Replay.PlaceMembers`
  - Done when: tests green

### S8.99 Stage gate

- [ ] **S8.99.1: Run the full S8 test suite, all checks and the reference scenario**
  - Requirements: T-13, T-14, T-11, P-03
  - Modules: all
  - Depends on: S8.08.3, S8.08.2, S8.08.1, S8.07.1, S8.05.4, S8.05.2, S8.02.6, S8.02.5
  - Deliverables: `scripts/ci-local.ps1 -Nightly` results (runs `scripts/test.ps1 -Coverage`, `-Bench`, `-Fuzz` and `scripts/check-selftest.ps1`); offline smoke
  - Tests: T-13 all suites; T-14 both variants green
  - Done when: everything green
- [ ] **S8.99.2: Run the benchmark regressions (incl. P-12 with advanced features)**
  - Requirements: P-12, T-08
  - Modules: tests
  - Depends on: S8.99.1
  - Deliverables: `bench/history.csv` rows
  - Tests: T-08 all existing benchmarks
  - Done when: results recorded
- [ ] **S8.99.3: Update traceability and documentation for S8**
  - Requirements: T-13
  - Modules: docs
  - Depends on: S8.99.2
  - Deliverables: `docs/traceability.csv`, `docs/stages/S8.md`, `docs/format/part.md`, `docs/commands/`, `docs/data-sources.md`
  - Tests: `/check-traceability` reports no gaps for S8; `scripts/docs.ps1 -FailOnWarning` clean (needs Doxygen, see S0.14.1)
  - Done when: S8 requirements updated
- [ ] **S8.99.4: Human review checkpoint: stop and wait for approval**
  - Requirements: all S8 requirements (see `docs/stages/S8.md`)
  - Modules: none
  - Depends on: S8.99.3
  - Deliverables: S8 summary for the owner
  - Tests: none (review)
  - Done when: the owner approves S8 in writing

## Stage S9: Simulation and analysis

Stage plan: `docs/stages/S9.md`. Exit: T-10 motion, tolerance and SPICE verification suites pass.

### S9.01 Motion analysis (rigid-body dynamics)

- [ ] **S9.01.1: Add Project Chrono behind sim::dynamics (ADRs 0007, 0021, 0022)**
  - Requirements: M-SIM-04, A-01, L-02
  - Modules: sim, build
  - Depends on: S8.99.4
  - Deliverables: vcpkg manifest entry for Chrono (or an overlay port in `third_party/` if the registry lacks a fitting version, recorded in `docs/provenance.md`); ADR 0022 applied for Eigen in `sim`; ADR 0021 applied for `sim` → `part`; `src/sim/dynamics/` wrapper; Chrono headers confined to `sim`
  - Tests: T-01 `Sim.Tests Dynamics.FreeFallMatchesAnalytic`; `scripts/check-layering.ps1` green
  - Done when: tests and checks green
- [ ] **S9.01.2: Convert mates to joints; user joints, bushings (should) and rigid groups**
  - Requirements: M-SIM-04
  - Modules: sim, asm
  - Depends on: S9.01.1
  - Deliverables: mate → joint mapping table; joint overrides; rigid groups; bushings as a should
  - Tests: T-01 `Sim.Tests Joints.FromMates*`
  - Done when: tests green
- [ ] **S9.01.3: Loads: gravity, forces, torques, springs, dampers and motors**
  - Requirements: M-SIM-04, M-PRT-10
  - Modules: sim
  - Depends on: S9.01.2
  - Deliverables: constant or expression loads (time and state); linear and torsional springs/dampers; speed-, position- and torque-driven motors with profiles; mass properties from materials
  - Tests: T-10 `Sim.Tests MotionBench.Pendulum`, `MassSpringDamper`
  - Done when: tests green
- [ ] **S9.01.4: Solid-body contact with friction and restitution**
  - Requirements: M-SIM-04
  - Modules: sim, geom
  - Depends on: S9.01.3
  - Deliverables: convex decomposition or mesh collision shapes from tessellation; static/dynamic friction; restitution
  - Tests: T-10 `Sim.Tests MotionBench.BouncingBallRestitution`
  - Done when: tests green
- [ ] **S9.01.5: Solver settings, motion study documents and results with CSV export**
  - Requirements: M-SIM-04, F-01
  - Modules: sim, doc
  - Depends on: S9.01.3
  - Deliverables: duration, frame rate, integrator accuracy; motion study saved in the assembly (schema update and migration); results for displacement, velocity, acceleration, reaction forces, motor torque and power, contact forces, energy; trace paths; interference over time; CSV export; runs as a job
  - Tests: T-03 `Format.Tests RoundTrip.MotionStudy`; T-01 `Sim.Tests Results.Csv*`
  - Done when: tests green
- [ ] **S9.01.6: T-10 motion benchmark suite**
  - Requirements: M-SIM-04, T-10
  - Modules: tests
  - Depends on: S9.01.4, S9.01.5
  - Deliverables: analytic benchmarks (pendulum, slider-crank dynamics, four-bar, mass-spring-damper) in `tests/data/scripts/` with analytic oracles in `tests/support/oracles/`
  - Tests: T-10 `Sim.Tests MotionBench.AllWithin0_5Percent` (Accept: M-SIM-04a)
  - Done when: suite green
- [ ] **S9.01.7: Golden cam-follower with contact**
  - Requirements: M-SIM-04
  - Modules: tests
  - Depends on: S9.01.6
  - Deliverables: `tests/data/scripts/cam_follower.cmd.json`
  - Tests: T-10 `Sim.Tests MotionBench.CamFollowerStaysInContact` (Accept: M-SIM-04b)
  - Done when: test green

### S9.02 Tolerance stack-up analysis

- [ ] **S9.02.1: Stack-up definition and contributor chains (ADR 0021 sim → drw)**
  - Requirements: M-SIM-05, M-DRW-02
  - Modules: sim, drw, asm
  - Depends on: S8.99.4
  - Deliverables: ADR 0021 applied for `sim` → `drw`; measurement gap between two faces; contributor chain read from model and drawing tolerances or entered directly; chain proposal from mates as a should
  - Tests: T-01 `Sim.Tests Tolerance.ChainFromDrawing*`
  - Done when: tests green
- [ ] **S9.02.2: Worst-case, RSS and Monte Carlo methods**
  - Requirements: M-SIM-05
  - Modules: sim
  - Depends on: S9.02.1
  - Deliverables: methods with normal or uniform distributions; seeded random generator from `core` for deterministic tests
  - Tests: T-10 `Sim.Tests Tolerance.WorstCase*`, `Rss*`, `MonteCarlo*`
  - Done when: tests green
- [ ] **S9.02.3: Stack-up report with PDF and CSV export; 2D/3D vector loops (should)**
  - Requirements: M-SIM-05
  - Modules: sim, drw
  - Depends on: S9.02.2, S7.01.7
  - Deliverables: nominal, min, max, sigma, % contribution, sensitivity ranking, pass/fail; PDF via the drawing PDF writer; CSV; vector-loop stacks as a should
  - Tests: T-07 `Golden.Tests Tolerance.ReportStructure`
  - Done when: tests green
- [ ] **S9.02.4: T-10 tolerance verification suite**
  - Requirements: M-SIM-05, T-10
  - Modules: tests
  - Depends on: S9.02.3
  - Deliverables: textbook stack-up cases with published answers cited in `tests/data/expected/`
  - Tests: T-10 `Sim.Tests ToleranceSuite.All` (Accept: M-SIM-05)
  - Done when: suite green

### S9.03 Circuit simulation (SPICE)

- [ ] **S9.03.1: Add the ngspice port, confirm it matches the recorded licence, and load it dynamically through platform**
  - Requirements: E-SIM-01, L-02, L-05
  - Modules: sim, platform, docs
  - Depends on: S8.99.4
  - Deliverables: `ngspice` in `vcpkg.json`. The licence was verified and accepted by the owner on 2026-11-11 (ADR 0008, `THIRD_PARTY_NOTICES.md`). This step confirms that the pinned port version and its `COPYING` still match that record, decides whether the `codemodels` feature (GPL-2.0-or-later `table.cm`) ships, and updates the notices. Code: `src/sim/spice/NgspiceEngine.{h,cpp}` using the shared-library API through `platform` dynamic loading (S0.11.5); a missing library gives a clear message
  - Tests: T-01 `Sim.Tests Spice.LoadsEngine`, `Spice.MissingLibraryMessage`
  - Done when: tests green; port version and `COPYING` match `THIRD_PARTY_NOTICES.md`, or the notices and ADR 0008 are updated
- [ ] **S9.03.2: Netlist export from the schematic with the active variant**
  - Requirements: E-SIM-01, E-LIB-01, E-PRJ-03
  - Modules: sim, ecad/sch, ecad/lib
  - Depends on: S9.03.1
  - Deliverables: SPICE netlister using component simulation models and pin mappings; user model files; DNP handling
  - Tests: T-04 `Golden.Tests Spice.NetlistGolden*`
  - Done when: tests green
- [ ] **S9.03.3: Analyses: operating point, DC sweep, transient, AC, noise**
  - Requirements: E-SIM-01
  - Modules: sim
  - Depends on: S9.03.2
  - Deliverables: analysis setup objects saved in the project; runs as jobs with cancel
  - Tests: T-10 `Sim.Tests SpiceBench.RcTransient`, `RcAcCorner`, `DiodeOp`
  - Done when: tests green
- [ ] **S9.03.4: Temperature, parameter and Monte Carlo sweeps; all source types**
  - Requirements: E-SIM-01
  - Modules: sim
  - Depends on: S9.03.3
  - Deliverables: sweeps using component tolerances; DC, pulse, sine, PWL, exponential and file-driven PWL sources
  - Tests: T-10 `Sim.Tests SpiceBench.Sweeps*`, `Sources*`
  - Done when: tests green
- [ ] **S9.03.5: Probes, measurements and CSV export**
  - Requirements: E-SIM-01
  - Modules: sim
  - Depends on: S9.03.3
  - Deliverables: net and pin voltage, current, power probes; rise/fall, overshoot, RMS, average, peak, frequency; math expressions; CSV export
  - Tests: T-01 `Sim.Tests Measurements.*`
  - Done when: tests green
- [ ] **S9.03.6: Map engine errors back to schematic objects**
  - Requirements: E-SIM-01, U-10
  - Modules: sim, ecad/sch
  - Depends on: S9.03.2
  - Deliverables: error parser with object mapping into the messages panel
  - Tests: T-01 `Sim.Tests Spice.ErrorMapping*`
  - Done when: tests green
- [ ] **S9.03.7: T-10 SPICE verification suite**
  - Requirements: E-SIM-01, T-10
  - Modules: tests
  - Depends on: S9.03.4, S9.03.5, S9.03.6
  - Deliverables: analytic circuits (RC/RL/RLC, divider, diode, op-amp gain) with expected waveforms
  - Tests: T-10 `Sim.Tests SpiceSuite.All` (Accept: E-SIM-01)
  - Done when: suite green

### S9.04 Future FEA seam

- [ ] **S9.04.1: Define the sim mesh and field-result seams**
  - Requirements: A-11, M-PRT-10
  - Modules: sim
  - Depends on: S9.01.1
  - Deliverables: `sim::IMesher`, `sim::FieldResult` interfaces and documentation for future FEA (SW-SIM-01…03); no solver
  - Tests: T-01 `Sim.Tests Seams.FieldResultRoundTrip`
  - Done when: tests green

### S9.05 Simulation user interface

- [ ] **S9.05.1: Motion study panel with plots and playback**
  - Requirements: M-SIM-04, U-04
  - Modules: ui
  - Depends on: S9.01.7
  - Deliverables: `src/ui/sim/` motion study editor, plots, animated playback
  - Tests: T-05 `Ui.Tests Replay.RunMotionStudy`
  - Done when: tests green
- [ ] **S9.05.2: Tolerance stack-up panel**
  - Requirements: M-SIM-05
  - Modules: ui
  - Depends on: S9.02.4
  - Deliverables: chain picker and report viewer
  - Tests: T-05 `Ui.Tests Replay.DefineStackUp`
  - Done when: tests green
- [ ] **S9.05.3: Simulation setup and waveform viewer**
  - Requirements: E-SIM-01, U-04
  - Modules: ui
  - Depends on: S9.03.7
  - Deliverables: analysis setup dialog, probes on schematic, waveform viewer with multiple plots and cursors
  - Tests: T-05 `Ui.Tests Replay.RunTransient`; T-06 `Ui.Tests Snapshot.WaveformViewer`
  - Done when: tests green

### S9.99 Stage gate

- [ ] **S9.99.1: Run the full S9 test suite, all checks and the reference scenario**
  - Requirements: T-13, T-14, T-11, P-03
  - Modules: all
  - Depends on: S9.05.3, S9.05.2, S9.05.1, S9.04.1
  - Deliverables: `scripts/ci-local.ps1 -Nightly` results (runs `scripts/test.ps1 -Coverage`, `-Bench`, `-Fuzz` and `scripts/check-selftest.ps1`); offline smoke
  - Tests: T-13 all suites; T-10 suites; T-14 green
  - Done when: everything green (S9 exit criterion)
- [ ] **S9.99.2: Run the benchmark regressions**
  - Requirements: T-08
  - Modules: tests
  - Depends on: S9.99.1
  - Deliverables: `bench/history.csv` rows
  - Tests: T-08 all existing benchmarks
  - Done when: results recorded
- [ ] **S9.99.3: Update traceability and documentation for S9**
  - Requirements: T-13
  - Modules: docs
  - Depends on: S9.99.2
  - Deliverables: `docs/traceability.csv`, `docs/stages/S9.md`, `docs/commands/`, `docs/provenance.md`
  - Tests: `/check-traceability` reports no gaps for S9; `scripts/docs.ps1 -FailOnWarning` clean (needs Doxygen, see S0.14.1)
  - Done when: S9 requirements updated
- [ ] **S9.99.4: Human review checkpoint: stop and wait for approval**
  - Requirements: all S9 requirements (see `docs/stages/S9.md`)
  - Modules: none
  - Depends on: S9.99.3
  - Deliverables: S9 summary for the owner
  - Tests: none (review)
  - Done when: the owner approves S9 in writing

## Stage S10: Scale and polish

Stage plan: `docs/stages/S10.md`. Exit: P-14 met; fasteners auto-insert into golden hole series; renders pass
snapshot tests; inspection report exports.

### S10.01 Spike SP-8: large-assembly performance

- [ ] **S10.01.1: Spike SP-8b: profile the generated 2,000-part assembly (time box 2 days)**
  - Requirements: M-ASM-03, P-14, P-11
  - Modules: asm, render, tests
  - Depends on: S9.99.4, S3.02.4
  - Deliverables: profile of load time, memory and frame rate against the S3.02.4 baseline; candidate techniques (instancing, LOD, lazy B-rep, background loading, memory-mapped mesh cache) measured in throwaway prototypes outside `src/`
  - Tests: T-08 `Bench.OpenLightweight2000`, `Bench.Viewport5MTriangles` re-run with the prototype numbers recorded
  - Done when: measurements recorded in the spike notes
- [ ] **S10.01.2: Spike SP-8c: decide the large-assembly strategy in ADR 0041**
  - Requirements: M-ASM-03, P-14, P-11
  - Modules: docs
  - Depends on: S10.01.1
  - Deliverables: `docs/adr/0041-large-assembly-strategy.md` with chosen techniques and memory budget
  - Tests: none (decision)
  - Done when: ADR Accepted by the owner

### S10.02 Large assembly handling

- [ ] **S10.02.1: Lightweight and review-only load modes with on-demand resolution**
  - Requirements: M-ASM-03
  - Modules: asm, doc
  - Depends on: S10.01.2
  - Deliverables: display mesh, bounding volumes and cached mass properties loaded; B-rep resolved on demand for edit and measure; review-only mode with measure, section, hide/show and interference preview
  - Tests: T-02 `Integration.Tests LargeAsm.EditLightweightResolves` (Accept: M-ASM-03b)
  - Done when: test green
- [ ] **S10.02.2: Selective loading with placeholders**
  - Requirements: M-ASM-03
  - Modules: asm
  - Depends on: S10.02.1
  - Deliverables: filters by sub-assembly, named selection, spatial box and property query; placeholders for unloaded components
  - Tests: T-01 `Asm.Tests SelectiveLoad.*`
  - Done when: tests green
- [ ] **S10.02.3: Simplified representations**
  - Requirements: M-ASM-03
  - Modules: asm, part, geom
  - Depends on: S10.02.1
  - Deliverables: defeaturing (internal faces, small features under a threshold, envelope); per-component switch between full and simplified
  - Tests: T-04 `Golden.Tests Simplified.*`
  - Done when: tests green
- [ ] **S10.02.4: Geometry instancing, level of detail and background loading**
  - Requirements: M-ASM-03, P-11
  - Modules: render, asm
  - Depends on: S10.02.1
  - Deliverables: instanced draws in `render::Device`; LOD selection; background loading with progress
  - Tests: T-06 `Ui.Tests Snapshot.LodTransitions`; T-08 `Bench.Viewport5MTriangles`
  - Done when: tests green
- [ ] **S10.02.5: Meet P-14 on the generated 2,000-part assembly**
  - Requirements: P-14, M-ASM-03
  - Modules: tests, asm
  - Depends on: S10.02.2, S10.02.3, S10.02.4
  - Deliverables: tuning per ADR 0041
  - Tests: T-08 `Bench.OpenLightweight2000` under 20 s on the reference machine (Accept: M-ASM-03a)
  - Done when: benchmark meets P-14 (S10 exit criterion)
- [ ] **S10.02.6: Meet P-11 at 5 million triangles shaded with edges**
  - Requirements: P-11
  - Modules: render
  - Depends on: S10.02.4
  - Deliverables: edge rendering and culling tuned
  - Tests: T-08 `Bench.Viewport5MTriangles` at least 60 fps on the reference machine
  - Done when: benchmark meets P-11

### S10.03 Fasteners and smart components

- [ ] **S10.03.1: Fastener and hardware library seeded from size tables**
  - Requirements: M-ASM-05, CR-05
  - Modules: part, libraries, docs
  - Depends on: S9.99.4
  - Deliverables: `libraries/fasteners/` parametric generators: socket, button, pan, countersunk and hex screws; hex nuts; washers; dowel pins; heat-set inserts; PCB standoffs and spacers; metric and inch series from public dimensional data with `docs/data-sources.md` rows
  - Tests: T-01 `Part.Tests Fasteners.GenerateAllSizes`
  - Done when: tests green
- [ ] **S10.03.2: Detect aligned hole series through stacked components**
  - Requirements: M-ASM-05, X-08
  - Modules: asm
  - Depends on: S10.03.1
  - Deliverables: hole series detection from Hole-tool metadata and geometry, including linked board holes
  - Tests: T-01 `Asm.Tests HoleSeries.*`
  - Done when: tests green
- [ ] **S10.03.3: Auto-fastening with size and stocked-length selection**
  - Requirements: M-ASM-05
  - Modules: asm
  - Depends on: S10.03.2
  - Deliverables: command `asm.fastener.autoInsert`; size from hole metadata; length from stack thickness rounded up to stocked length; optional washers and nuts
  - Tests: T-04 `Golden.Tests Fasteners.EnclosureFourSeries` (Accept: M-ASM-05a)
  - Done when: test green (S10 exit criterion)
- [ ] **S10.03.4: Automatic re-sizing with no-fit warnings**
  - Requirements: M-ASM-05, A-06
  - Modules: asm
  - Depends on: S10.03.3
  - Deliverables: fasteners as regeneration nodes that re-size on hole or stack changes
  - Tests: T-02 `Integration.Tests Fasteners.LidThicknessUpdatesLengths` (Accept: M-ASM-05b)
  - Done when: test green
- [ ] **S10.03.5: Smart components with associated features**
  - Requirements: M-ASM-05
  - Modules: asm, part
  - Depends on: S10.03.4
  - Deliverables: associated features applied to mated parts (heat-set insert pilot hole into a boss)
  - Tests: T-04 `Golden.Tests SmartComponents.HeatSetPilotHole`
  - Done when: tests green
- [ ] **S10.03.6: Board-to-enclosure auto-fastening with standoffs**
  - Requirements: M-ASM-05, X-08
  - Modules: link, asm
  - Depends on: S10.03.4
  - Deliverables: linked board holes drive standoff and screw insertion
  - Tests: T-02 `Integration.Tests Fasteners.BoardStandoffsFromLinkedHoles`
  - Done when: test green

### S10.04 Photorealistic rendering

- [ ] **S10.04.1: PBR appearance library with textures, decals and overrides**
  - Requirements: M-VIS-01, CR-05
  - Modules: render, libraries
  - Depends on: S9.99.4
  - Deliverables: `libraries/appearances/` metallic-roughness starter set (metals, plastics, rubber, glass, emissive, electronics finishes); textures, projected decals, per-face overrides; same appearances in the viewport approximation
  - Tests: T-06 `Ui.Tests Snapshot.ViewportAppearances`
  - Done when: tests green
- [ ] **S10.04.2: Environments, lights and cameras**
  - Requirements: M-VIS-01, CR-05
  - Modules: render, libraries
  - Depends on: S10.04.1
  - Deliverables: `libraries/environments/` HDR images and studio presets created for the product or permissively licensed (`docs/provenance.md`); ground shadows and reflections; directional, point, spot, area lights; perspective, orthographic and depth-of-field cameras
  - Tests: T-01 `Render.Tests Scene.*`
  - Done when: tests green
- [ ] **S10.04.3: Progressive CPU path tracer with Embree (ADR 0014)**
  - Requirements: M-VIS-01, L-02
  - Modules: render, build
  - Depends on: S10.04.2
  - Deliverables: `src/render/pathtracer/` using Embree from vcpkg; optional denoiser behind a seam; PNG and EXR output up to 8K; runs as a job with cancel
  - Tests: T-06 `Golden.Tests Render.FiveGoldenScenes` (Accept: M-VIS-01a)
  - Done when: tests green with tolerance-based image comparison
- [ ] **S10.04.4: Turntable, exploded and motion-study animations (MP4 seam)**
  - Requirements: M-VIS-01, M-ASM-06, M-SIM-04
  - Modules: render, asm, sim
  - Depends on: S10.04.3
  - Deliverables: image sequences for turntable, exploded and motion-study animations; MP4 through the optional encoder seam from S3.08.3
  - Tests: T-06 `Golden.Tests Render.TurntableFrames`
  - Done when: tests green
- [ ] **S10.04.5: Meet the preview render time**
  - Requirements: M-VIS-01
  - Modules: render, tests
  - Depends on: S10.04.3
  - Deliverables: `Bench.RenderPreview1080p`
  - Tests: T-08 `Bench.RenderPreview1080p` under 60 s on the reference machine (Accept: M-VIS-01b)
  - Done when: benchmark meets the target
- [ ] **S10.04.6: Render settings dialog and progressive preview window**
  - Requirements: M-VIS-01, U-04
  - Modules: ui
  - Depends on: S10.04.4
  - Deliverables: `src/ui/render/` settings, progressive preview, output
  - Tests: T-05 `Ui.Tests Replay.RenderScene`
  - Done when: tests green

### S10.05 Inspection documentation

- [ ] **S10.05.1: Inspection project with auto-ballooned characteristics**
  - Requirements: M-INS-01
  - Modules: drw
  - Depends on: S9.99.4
  - Deliverables: inspection document linked to a drawing; auto-balloon every characteristic in a configurable style without altering the drawing; schema and `docs/format/` update
  - Tests: T-03 `Format.Tests RoundTrip.Inspection`; T-01 `Drw.Tests Inspection.Balloons*`
  - Done when: tests green
- [ ] **S10.05.2: Characteristic table, measured values and pass/fail**
  - Requirements: M-INS-01
  - Modules: drw
  - Depends on: S10.05.1
  - Deliverables: number, type, nominal, limits, units, sheet and zone, method; measured value entry
  - Tests: T-04 `Golden.Tests Inspection.FortyCharacteristicsTable` (Accept: M-INS-01a)
  - Done when: test green
- [ ] **S10.05.3: Stable numbering across drawing revisions**
  - Requirements: M-INS-01, M-DRW-02, M-SIM-05, A-07
  - Modules: drw, sim
  - Depends on: S10.05.2
  - Deliverables: characteristic identity tied to annotation ObjectIds; new characteristics take new numbers
  - Tests: T-02 `Integration.Tests Inspection.RevisionKeepsNumbers` (Accept: M-INS-01b); T-02 `Integration.Tests Tolerances.DrawingToStackUpAndInspection` (Accept: M-DRW-02b)
  - Done when: tests green
- [ ] **S10.05.4: Ballooned PDF and characteristic report export**
  - Requirements: M-INS-01
  - Modules: drw
  - Depends on: S10.05.3
  - Deliverables: PDF of ballooned drawing; CSV and PDF report through a configurable template
  - Tests: T-07 `Golden.Tests Inspection.ReportExports`
  - Done when: tests green (S10 exit criterion)

### S10.06 Interaction and polish completion

- [ ] **S10.06.1: Complete in-canvas editing and live preview**
  - Requirements: U-07
  - Modules: ui, render
  - Depends on: S10.05.4
  - Deliverables: double-click editing of dimensions and parameters in every viewport; live preview in every feature dialog
  - Tests: T-05 `Ui.Tests InCanvas.AllFeatureDialogsPreview`
  - Done when: tests green
- [ ] **S10.06.2: Complete navigation (view cube, mouse presets, 2D cross-probe)**
  - Requirements: U-08
  - Modules: ui, render
  - Depends on: S10.06.1
  - Deliverables: configurable mouse presets (original, no vendor UI copied); view cube completion; zoom-to-cursor everywhere; 2D pan/zoom/fit/cross-probe parity
  - Tests: T-05 `Ui.Tests Navigation.*`
  - Done when: tests green
- [ ] **S10.06.3: Complete selection (lasso, all filters, select other)**
  - Requirements: U-09
  - Modules: ui, render
  - Depends on: S10.06.1
  - Deliverables: lasso; filters for face, edge, vertex, body, component, net, pad, track; "select other"
  - Tests: T-05 `Ui.Tests Selection.*`
  - Done when: tests green
- [ ] **S10.06.4: Accessibility audit and fixes**
  - Requirements: U-12
  - Modules: ui
  - Depends on: S10.06.3
  - Deliverables: keyboard navigation of all panels and dialogs; accessible names; colour-blind-safe default palettes; 4.5:1 contrast checks
  - Tests: T-05 `Ui.Tests Accessibility.KeyboardReachAll`, `Accessibility.NamesPresent`, `Accessibility.Contrast`
  - Done when: tests green
- [ ] **S10.06.5: Localisation readiness audit**
  - Requirements: U-13
  - Modules: ui, scripts
  - Depends on: S10.06.4
  - Deliverables: every user-visible string externalised; `src/ui/translations/` English file complete; check for untranslated literals added to the static checks
  - Tests: T-05 `Ui.Tests Localisation.NoUnwrappedStrings`
  - Done when: tests and checks green
- [ ] **S10.06.6: Start page with recent projects, templates and tutorials**
  - Requirements: U-14
  - Modules: ui, libraries
  - Depends on: S10.06.5
  - Deliverables: start page; templates (blank part, blank assembly, PCB-in-enclosure) in `libraries/templates/`; links to local tutorials
  - Tests: T-05 `Ui.Tests StartPage.OpenTemplates`
  - Done when: tests green
- [ ] **S10.06.7: Offline help with context F1**
  - Requirements: U-15, P-03
  - Modules: ui, docs
  - Depends on: S10.06.6
  - Deliverables: `docs/user-guide/` bundled with the build; F1 maps the focused panel or command to a page; no network access
  - Tests: T-05 `Ui.Tests Help.F1OpensLocalPage`
  - Done when: tests green
- [ ] **S10.06.8: Automatic re-pour on edit (should)**
  - Requirements: E-PCB-07, P-15
  - Modules: ecad/pcb
  - Depends on: S10.06.1
  - Deliverables: incremental re-pour triggered by edits as a background job, keeping P-15 interactivity
  - Tests: T-02 `Integration.Tests Pours.AutoRepourAfterMove`; T-08 `Bench.PourReflow`
  - Done when: tests green

### S10.99 Stage gate

- [ ] **S10.99.1: Run the full S10 test suite, all checks and the reference scenario**
  - Requirements: T-13, T-14, T-11, P-03
  - Modules: all
  - Depends on: S10.06.8, S10.06.7, S10.06.2, S10.05.4, S10.04.6, S10.04.5, S10.03.6, S10.03.5, S10.02.6, S10.02.5
  - Deliverables: `scripts/ci-local.ps1 -Nightly` results (runs `scripts/test.ps1 -Coverage`, `-Bench`, `-Fuzz` and `scripts/check-selftest.ps1`); offline smoke
  - Tests: T-13 all suites; T-14 both variants green; full fuzz run
  - Done when: everything green
- [ ] **S10.99.2: Run every performance target**
  - Requirements: P-10, P-11, P-12, P-13, P-14, P-15, P-16, P-17, T-08
  - Modules: tests
  - Depends on: S10.99.1
  - Deliverables: `bench/history.csv` rows; summary table of every P target against the reference machine
  - Tests: T-08 all benchmarks
  - Done when: every P target met or a deviation recorded for the owner
- [ ] **S10.99.3: Update traceability and documentation for S10**
  - Requirements: T-13
  - Modules: docs
  - Depends on: S10.99.2
  - Deliverables: `docs/traceability.csv` (every requirement done or with a recorded deviation), `docs/stages/S10.md`, `docs/user-guide/`, `docs/commands/`
  - Tests: `/check-traceability` reports no gaps; `scripts/docs.ps1 -FailOnWarning` clean (needs Doxygen, see S0.14.1)
  - Done when: all requirements updated
- [ ] **S10.99.4: Human review checkpoint: stop and wait for approval**
  - Requirements: all S10 requirements (see `docs/stages/S10.md`)
  - Modules: none
  - Depends on: S10.99.3
  - Deliverables: S10 and release summary for the owner
  - Tests: none (review)
  - Done when: the owner approves S10 in writing

## Future

These items from spec section 14 are **not** built in S0 … S10 and have no checkboxes. Their seams are requirements
now and are built in the steps listed. When a future item is scheduled, it gets its own stage, ADR and steps.

| Source ID | Capability | Seam | Built in step(s) |
|---|---|---|---|
| SW-SIM-01, SW-SIM-02, SW-SIM-03 | Structural FEA (linear static, frequency, buckling, thermal, fatigue, nonlinear, dynamics) | Materials carry mechanical and thermal properties; `sim` mesh and result-field interfaces; field-plot viewer later. Candidate solvers must pass the licence policy (GPL-3.0-compatible) | S2.08.1, S9.04.1 |
| SW-RTE-01 | Pipe and tube routing | 3D sketches and connection points on parts | S1.05.1 … S1.05.3, S8.07.1 |
| SW-RTE-02 + AD-HAR-01 | Electrical harness routing and harness design, as **one** cable/harness module | Connectors carry pin data in the library; cross-board connectivity model | S4.02.3, S6.08.1 |
| SW-CAM-01 | CAM toolpaths (2.5-axis milling, turning) | Semantic hole and feature metadata | S2.04.8 |
| SW-PDM-02 | Document workflow and lifecycle | Reusable lifecycle state machine from library items; projects work in Git | S4.01.2 |
| SW-API-01 + AD-API-01 | Automation API (front end to be decided) | Stable documented command names and JSON Schemas on the command bus | S0.04.1 … S0.04.5, S0.14.2 |
| AD-PCB-06 | Guided routing, BGA fanout, autorouter | Router path-finding and shove exposed as services | S5.09.1 |
| AD-PCB-09 | Rigid-flex | Board regions referencing stack definitions | S5.02.1 |
| AD-PCB-10 | Wire bonding, chip-on-board, printed electronics | Non-copper conductive and dielectric print layers in the stack | S5.03.1 |
| AD-SI-01 | Signal integrity (reflection, crosstalk, IBIS) | Impedance profiles and stack data; routed-geometry extraction API | S5.03.3, S5.10.3 |
| AD-PI-01 | Power integrity (DC IR drop, current density) | Per-net copper polygons with layer and thickness | S5.10.3 |
| AD-OUT-03 | CAM data viewer/editor | Test-side Gerber, Excellon and IPC-D-356 readers become the viewer's basis | S6.09.1, S7.04.5 |
| (new) | Online supplier data, pricing and stock | Optional opt-in module only; default build stays offline (P-03) | S0.02.3 |

Other deferred items recorded during planning:

- IDX (incremental ECAD/MCAD exchange) export, if not done with IDF in S7.08.3.
- Hosted CI (open decision, ADR 0033) beyond the first workflow added in S1.08.1.
- MP4 encoder bundling (spec Q-08), seam in S3.08.3 and S10.04.4.
- A cross-platform build system and non-Windows builds (spec D-02 plans the move later; ADR 0003). Portability checks
  start with the GCC/Clang probe in SP-1 (ADR 0034).

## Coverage

Generated during planning from the steps above and the spec, and kept current by `/check-traceability`. Every
requirement ID in the spec appears in at least one step's "Requirements" field and in `docs/traceability.csv`. "Accept tests" lists the tests tagged `(Accept: …)`; requirements without an
`Accept:` line in the spec show "n/a (no Accept line)".

<!-- coverage:start -->
| Requirement | Title | Steps | Accept tests |
|---|---|---|---|
| CR-01 | No proprietary sources or materials | S0.02.6, S0.13.3 | n/a (no Accept line) |
| CR-02 | Generic functional terms, no vendor trademarks | S0.02.6 | n/a (no Accept line) |
| CR-03 | No proprietary native file formats | S0.02.6 | n/a (no Accept line) |
| CR-04 | Reuse of GPL-compatible code allowed, fresh preferred | S0.02.1 | n/a (no Accept line) |
| CR-07 | Attribution and provenance | S0.02.1, S0.02.2 | CR-07a: S0.02.1 T-11 scripts/check-selftest.ps1 cases: a source without an SPDX header fails; CR-07b: S0.02.1 a third-party-marked file without a provenance row, and one named only in prose or in an incomplete row, fail |
| CR-05 | Standards data: calculators and public factual data only | S0.02.6, S2.04.7, S2.08.1, S4.03.6, S4.04.1, S5.03.4, S7.02.2, S8.04.1, S8.06.1, S10.03.1, S10.04.1, S10.04.2 | n/a (no Accept line) |
| CR-06 | No telemetry or phone-home | S0.02.3, S0.02.6 | n/a (no Accept line) |
| P-01 | Desktop application | S0.12.1, S0.13.4 | n/a (no Accept line) |
| P-02 | Initial target | S0.01.1 | n/a (no Accept line) |
| P-03 | Fully offline | S0.02.3, S0.08.7, S0.99.1, S1.99.1, S2.99.1, S3.99.1, S4.99.1, S5.99.1, S6.99.1, S7.99.1, S8.99.1, S9.99.1, S10.06.7, S10.99.1 | P-03a: S0.08.7 T-11 offline smoke of cli, ui (start and close) and Integration.Tests with no errors and no outbound connection attempts; P-03b: S0.02.3 network APIs in src/ fail; third-party network imports such as Qt6Core's ws2_32.dll are allowed, noted and summarised |
| P-04 | Portability | S0.01.3, S0.03.13, S0.11.1, S0.11.5, S0.11.6, S0.99.1 | P-04: S0.01.3 T-11 check passes on the solution and fails on a fixture project that links Qt6Widgets |
| P-05 | Language | S0.01.2 | n/a (no Accept line) |
| P-06 | Units | S0.03.4, S0.03.5 | n/a (no Accept line) |
| P-07 | Precision | S0.03.4, S5.02.2 | n/a (no Accept line) |
| P-10 | Cold start to usable empty window | S0.13.15, S0.99.2, S10.99.2 | n/a (no Accept line) |
| P-11 | Viewport frame rate, 5 million triangles, shaded with edges | S0.12.1, S0.12.2, S3.02.4, S3.99.2, S10.01.1, S10.01.2, S10.02.4, S10.02.6, S10.99.2 | n/a (no Accept line) |
| P-12 | Full rebuild of a 200-feature part | S2.10.1, S2.99.2, S8.99.2, S10.99.2 | n/a (no Accept line) |
| P-13 | Sketch solve, 500 entities / 800 constraints, interactive drag | S1.01.2, S1.01.3, S1.03.11, S1.99.2, S10.99.2 | n/a (no Accept line) |
| P-14 | Open a 2,000-part assembly in lightweight mode | S3.02.4, S3.99.2, S10.01.1, S10.01.2, S10.02.5, S10.99.2 | n/a (no Accept line) |
| P-15 | PCB with 2,000 components, 5,000 nets: interactive routing response | S5.01.2, S5.01.3, S5.02.3, S5.09.10, S5.99.2, S10.06.8, S10.99.2 | n/a (no Accept line) |
| P-16 | Batch DRC, 8-layer board, 2,000 components | S5.02.3, S5.08.6, S5.99.2, S10.99.2 | n/a (no Accept line) |
| P-17 | Save or load a 50 MB project | S0.08.4, S0.99.2, S6.11.4, S6.99.2, S10.99.2 | n/a (no Accept line) |
| P-20 | Crash safety | S0.11.2, S0.11.3, S0.13.13 | n/a (no Accept line) |
| P-21 | No silent data loss | S0.05.4, S2.01.2, S2.03.8, S6.02.3 | n/a (no Accept line) |
| P-22 | Undo/redo | S0.04.3 | n/a (no Accept line) |
| P-23 | Logging | S0.03.10, S0.11.3, S0.11.4, S0.13.13 | n/a (no Accept line) |
| P-30 | Licence GPL-3.0-or-later | S0.01.6, S0.02.2, S0.13.12 | n/a (no Accept line) |
| B-01 | Solution | S0.01.1, S0.01.4 | n/a (no Accept line) |
| B-02 | One project per module | S0.01.4 | n/a (no Accept line) |
| B-03 | Shared settings | S0.01.4 | n/a (no Accept line) |
| B-04 | Configurations | S0.01.3 | n/a (no Accept line) |
| B-05 | Dependencies | S0.01.1, S0.01.6 | n/a (no Accept line) |
| B-06 | Command-line build | S0.01.1 | n/a (no Accept line) |
| B-07 | Migration-ready | S0.01.4 | n/a (no Accept line) |
| L-01 | Standard switch | S0.01.1, S0.01.2 | n/a (no Accept line) |
| L-02 | Supported features only | S0.01.2, S9.01.1, S9.03.1, S10.04.3 | n/a (no Accept line) |
| L-03 | Portable subset | S0.01.2, S0.01.5 | n/a (no Accept line) |
| L-04 | Feature-test macros | S0.01.2, S0.01.5 | n/a (no Accept line) |
| L-05 | Modules | S0.01.2, S0.01.5, S9.03.1 | n/a (no Accept line) |
| A-01 | Layering | S0.01.3, S0.02.7, S0.03.1, S0.09.4, S0.10.3, S1.01.1, S3.01.1, S5.01.1, S9.01.1 | A-01: S0.09.4 T-11 fixture header outside geom that includes TopoDS_Shape.hxx fails |
| A-02 | Headless first | S0.03.2, S0.05.5, S0.10.1, S0.10.4, S0.11.6, S7.04.6 | n/a (no Accept line) |
| A-03 | Command bus | S0.03.2, S0.03.9, S0.04.1, S0.04.2, S0.04.3, S0.04.4, S0.04.5, S0.05.5, S0.14.2, S1.04.8 | n/a (no Accept line) |
| A-04 | Stable identities | S0.03.3, S0.05.1, S0.05.2, S0.09.7, S6.02.1 | n/a (no Accept line) |
| A-05 | Kernel adapter | S0.09.1, S0.09.2, S0.09.3, S0.09.4 | n/a (no Accept line) |
| A-06 | Topological naming | S0.09.5, S0.09.6, S0.09.7, S2.01.3, S2.06.1, S2.06.2, S3.03.1, S6.03.1, S7.01.6, S10.03.4 | A-06: S2.06.2 T-04 Golden.Tests TopoNaming.Case01..20 |
| A-07 | Regeneration engine | S0.03.8, S0.03.9, S0.05.3, S0.05.4, S0.10.1, S1.06.2, S2.01.2, S2.07.3, S2.10.1, S6.01.1, S6.01.2, S6.02.3, S7.01.6, S8.01.3, S10.05.3 | n/a (no Accept line) |
| A-08 | Expression engine | S0.03.6, S0.03.7, S0.03.8, S0.05.2, S0.13.8, S1.03.9, S1.06.4, S5.05.1, S6.02.2 | n/a (no Accept line) |
| A-09 | PCB coordinate model | S0.03.5, S4.02.2, S5.01.1, S5.02.1, S5.02.2, S5.13.1, S6.02.2 | n/a (no Accept line) |
| A-10 | Threading | S0.03.11, S0.05.4, S0.13.11, S2.09.2, S5.15.3, S7.07.1, S7.07.6 | n/a (no Accept line) |
| A-11 | Extensibility seam | S0.03.12, S0.04.1, S0.10.1, S0.10.2, S3.02.2, S4.07.1, S5.08.1, S6.09.2, S7.03.1, S9.04.1 | n/a (no Accept line) |
| F-01 | Open and documented | S0.04.2, S0.06.1, S0.14.1, S0.14.2, S1.02.1, S2.01.1, S3.02.1, S4.01.1, S5.02.1, S6.02.1, S7.01.1, S9.01.5 | n/a (no Accept line) |
| F-02 | Two equivalent encodings | S0.06.4, S0.06.6, S0.06.7, S0.06.11, S0.10.5, S6.11.4 | F-02: S0.06.11 T-03 Format.Tests RoundTrip.JsonBsonJsonIsByteIdentical |
| F-03 | Deterministic output | S0.06.2, S0.06.3 | n/a (no Accept line) |
| F-04 | Parametric source of truth | S0.05.1, S0.06.6, S0.06.9, S3.02.3 | n/a (no Accept line) |
| F-05 | Versioning and migration | S0.06.8, S2.01.1 | n/a (no Accept line) |
| F-06 | Blobs | S0.06.6, S4.05.4 | n/a (no Accept line) |
| F-07 | Library files | S0.06.10, S0.07.1, S2.04.7, S2.08.1, S4.01.1, S7.02.5 | n/a (no Accept line) |
| F-08 | Robust loading | S0.06.3, S0.08.5 | F-08: S0.08.5 T-09 each target runs 10 minutes without crash or hang |
| F-09 | Magic value (file identification) | S0.06.1, S0.06.2, S0.06.4, S0.06.5, S0.06.10, S0.06.11, S0.10.5 | F-09a: S0.06.11 Magic.BsonPrefixPerDocumentType; F-09b: S0.06.11 Magic.JsonStartsWithMagicKey; F-09c: S0.06.11 Magic.RejectsWrongOrMissingMagic |
| F-10 | Library scopes (global seed, per-user, per-project) | S0.03.13, S0.07.1, S0.07.2, S0.07.3, S0.11.1, S4.03.4, S4.03.5, S4.10.2 | F-10a: S0.07.1 T-01 Doc.Tests LibraryResolver.ProjectShadowsUserShadowsGlobal and FallsBackWhenProjectItemDeleted; F-10b: S4.03.4 T-02 Integration.Tests Library.OpensIdenticallyOnCleanMachine; F-10c: S4.03.5 T-02 Integration.Tests Library.SeedUpgradeThreeWayReview |
| T-01 | Unit tests | S0.08.1 | n/a (no Accept line) |
| T-02 | Integration tests | S0.04.5, S0.08.2, S0.10.4, S0.10.5 | n/a (no Accept line) |
| T-03 | Format round-trip tests | S0.06.11, S0.08.2, S0.10.5 | n/a (no Accept line) |
| T-04 | Geometry golden tests | S0.08.1, S0.08.2, S0.08.3, S2.03.9, S2.04.10, S2.06.2, S2.08.4, S3.03.7 | n/a (no Accept line) |
| T-05 | UI replay tests | S0.13.14, S1.07.7, S2.11.4, S4.10.5, S6.12.1 | n/a (no Accept line) |
| T-06 | Visual snapshot tests | S0.12.2, S0.13.14, S7.01.9, S7.02.6, S7.06.3 | n/a (no Accept line) |
| T-07 | Output conformance tests | S0.08.3, S6.09.1, S7.04.7, S7.07.7 | n/a (no Accept line) |
| T-08 | Benchmarks | S0.08.4, S0.99.2, S1.99.2, S2.99.2, S3.99.2, S4.99.2, S5.99.2, S6.99.2, S7.99.2, S8.99.2, S9.99.2, S10.99.2 | n/a (no Accept line) |
| T-09 | Fuzzing | S0.08.5 | n/a (no Accept line) |
| T-10 | Domain verification suites | S0.08.3, S1.03.10, S4.07.4, S5.08.5, S9.01.6, S9.02.4, S9.03.7 | n/a (no Accept line) |
| T-11 | Static checks | S0.02.3, S0.02.4, S0.02.5, S0.02.7, S0.08.7, S0.14.1, S0.15.1, S0.99.1, S1.99.1, S2.99.1, S3.99.1, S4.99.1, S5.99.1, S6.99.1, S7.99.1, S8.99.1, S9.99.1, S10.99.1 | n/a (no Accept line) |
| T-12 | Coverage | S0.08.6 | n/a (no Accept line) |
| T-13 | One-command run | S0.02.7, S0.08.2, S0.15.1, S0.99.1, S0.99.3, S1.08.1, S1.99.1, S1.99.3, S2.99.1, S2.99.3, S3.99.1, S3.99.3, S4.99.1, S4.99.3, S5.99.1, S5.99.3, S6.99.1, S6.99.3, S7.99.1, S7.99.3, S8.99.1, S8.99.3, S9.99.1, S9.99.3, S10.99.1, S10.99.3 | n/a (no Accept line) |
| T-14 | Reference scenario test | S6.11.1, S6.11.2, S6.11.3, S6.99.1, S7.99.1, S8.01.4, S8.99.1, S9.99.1, S10.99.1 | n/a (no Accept line) |
| U-01 | Modern visual design | S0.13.1, S0.13.3 | n/a (no Accept line) |
| U-02 | High-DPI and multi-monitor | S0.13.2 | n/a (no Accept line) |
| U-03 | Workspace layout | S0.13.2, S2.11.1, S3.09.1 | n/a (no Accept line) |
| U-04 | Contextual command bar | S0.13.6, S1.07.1, S7.09.1, S9.05.1, S9.05.3, S10.04.6 | n/a (no Accept line) |
| U-05 | Command palette | S0.13.7 | n/a (no Accept line) |
| U-06 | Properties inspector | S0.13.8, S1.07.4, S1.07.5, S2.11.5, S6.12.3 | n/a (no Accept line) |
| U-07 | In-canvas editing | S1.07.4, S2.11.2, S7.09.2, S8.08.2, S10.06.1 | n/a (no Accept line) |
| U-08 | Navigation | S0.12.4, S0.12.5, S0.13.4, S3.09.5, S4.10.1, S5.15.1, S10.06.2 | n/a (no Accept line) |
| U-09 | Selection | S0.12.3, S0.13.5, S3.09.2, S10.06.3 | n/a (no Accept line) |
| U-10 | Keyboard | S0.03.13, S0.13.9, S9.03.6 | n/a (no Accept line) |
| U-11 | Messages | S0.13.10, S1.07.6, S2.11.3, S4.10.4, S5.15.4 | n/a (no Accept line) |
| U-12 | Accessibility | S0.13.1, S5.15.1, S10.06.4 | n/a (no Accept line) |
| U-13 | Localisation-ready | S0.13.1, S10.06.5 | n/a (no Accept line) |
| U-14 | Start page | S10.06.6 | n/a (no Accept line) |
| U-15 | Offline help | S10.06.7 | n/a (no Accept line) |
| M-SKT-01 | Sketch entities and tools | S1.02.1, S1.02.2, S1.02.3, S1.02.4, S1.02.5, S1.02.6, S1.04.1, S1.04.2, S1.04.3, S1.04.4, S1.04.5, S1.04.6, S1.04.7, S1.04.8, S1.04.9, S1.07.1, S1.07.7 | M-SKT-01a: S1.04.8 T-01 Sketch.Tests CommandCoverage.AllEntitiesAndTools; M-SKT-01b: S1.04.8 T-04 Golden.Tests SketchTools.TrimExtendOffset01..15; M-SKT-01c: S1.04.7 T-04 Golden.Tests SketchRegions.Nested01..10 |
| M-SKT-02 | Sketch constraints and dimensions | S1.01.2, S1.01.3, S1.03.1, S1.03.2, S1.03.3, S1.03.4, S1.03.5, S1.03.6, S1.03.7, S1.03.8, S1.03.9, S1.03.10, S1.03.11, S1.03.12, S1.04.9, S1.07.2, S1.07.3, S1.07.6, S1.07.7 | M-SKT-02a: S1.03.10 T-10 Golden.Tests SketchSolverSuite.*; M-SKT-02b: S1.03.11 T-08 Bench.SketchDrag500x800 median under 16 ms; M-SKT-02c: S1.03.12 T-04 Golden.Tests SketchDof.* |
| M-SKT-03 | 3D sketching | S1.05.1, S1.05.2, S1.05.3, S2.03.5, S8.06.2, S8.07.1 | M-SKT-03a: S1.05.3 T-04 Golden.Tests Sketch3d.HandrailFullyDefined; M-SKT-03b: S2.03.5 Golden.Tests Sketch3d.HandrailDrivesSweep; M-SKT-03c: S8.06.2 T-04 Golden.Tests Weldments.HandrailStructureMembers |
| M-SKT-04 | Equations and global parameters | S1.03.9, S1.06.1, S1.06.2, S1.06.3, S1.06.4, S1.07.5, S3.05.5 | M-SKT-04a: S3.05.5 T-02 Integration.Tests Params.WallThicknessRegeneratesAll; M-SKT-04b: S1.06.2 T-01 Doc.Tests ParameterCycle.RejectsAandB |
| M-PRT-10 | Reference geometry | S2.02.1, S2.02.2, S2.02.3, S9.01.3, S9.04.1 | M-PRT-10: S2.02.3 T-04 Golden.Tests RefGeometry.* |
| M-PRT-01 | Solid features | S2.01.2, S2.03.1, S2.03.2, S2.03.3, S2.03.4, S2.03.5, S2.03.6, S2.03.7, S2.03.8, S2.03.9, S2.11.2, S8.02.6 | M-PRT-01a: S2.03.9 T-04 Golden.Tests PartGolden.SolidFeatureMatrix; M-PRT-01b: S2.03.8 T-04 Golden.Tests PartErrors.OpenProfile, PartErrors.SelfIntersectingResult |
| M-PRT-02 | Applied features | S2.04.1, S2.04.2, S2.04.3, S2.04.4, S2.04.5, S2.04.6, S2.04.7, S2.04.8, S2.04.9, S2.04.10, S2.11.2 | M-PRT-02a: S2.04.10 T-04 Golden.Tests PartGolden.AppliedFeatureMatrix; M-PRT-02b: S2.04.8 Golden.Tests Hole.M6TappedMatchesTable |
| M-PRT-03 | Patterns and mirrors | S2.05.1, S2.05.2, S2.05.3, S2.05.4, S2.05.5 | M-PRT-03: S2.05.5 T-04 Golden.Tests Patterns.Suite incl. skipped instances and a 10-instance variable pattern |
| M-PRT-04 | Feature tree | S2.01.1, S2.07.1, S2.07.2, S2.07.3, S2.11.1, S2.11.3, S2.11.4 | M-PRT-04a: S2.11.4 T-05 Ui.Tests Replay.FeatureTreeReorder, Replay.RollbackInsert, Replay.DeleteWithDependants; M-PRT-04b: S2.07.2 T-01 Part.Tests FeatureTree.ReorderRefusedWithExplanation |
| M-PRT-05 | Multibody parts | S8.01.1, S8.01.2, S8.01.3, S8.01.4, S8.08.1 | M-PRT-05: S8.01.4 T-04 Golden.Tests Multibody.EnclosureSplitTwoBodiesTwoDerived |
| M-PRT-06 | Surface modeling | S8.02.1, S8.02.2, S8.02.3, S8.02.4, S8.02.5, S8.02.7, S8.08.1 | M-PRT-06a: S8.02.7 T-04 Golden.Tests Surfaces.KnitFormsValidSolid; M-PRT-06b: S8.02.7 Surfaces.ThickenedLoftVolume |
| M-PRT-07 | Direct editing | S8.03.1, S8.03.2, S8.03.3, S8.08.2 | M-PRT-07: S8.03.3 T-04 Golden.Tests DirectEdit.BracketMoveHoleDeleteFilletOffset |
| M-PRT-09 | Materials and mass properties | S2.08.1, S2.08.2, S2.08.3, S2.08.4, S2.11.5 | M-PRT-09a: S2.08.2 T-04 Golden.Tests MassProps.PrimitivesWithin1e-6; M-PRT-09b: S2.08.4 T-04 Golden.Tests GoldenParts.MassWithin1e-4 |
| M-PRT-11 | Configurations | S3.04.1, S3.04.2, S3.04.3, S3.04.4, S3.09.3, S6.07.2 | M-PRT-11a: S3.04.4 T-04 Golden.Tests Configs.Bolt12MassValues; M-PRT-11b: S3.04.4 Configs.AssemblySwapsComponentConfigs |
| M-SHM-01 | Sheet metal features | S8.04.1, S8.04.2, S8.04.3, S8.04.4, S8.04.5, S8.04.6, S8.08.3 | M-SHM-01: S8.04.6 T-04 Golden.Tests SheetMetal.GoldenRebuildWithBendMetadata |
| M-SHM-02 | Flat pattern | S8.05.1, S8.05.2, S8.05.3, S8.05.4, S8.08.3 | M-SHM-02a: S8.05.2 T-04 Golden.Tests FlatPattern.LengthsWithin0_01mm; M-SHM-02b: S8.05.3 T-07 Golden.Tests FlatPattern.DxfReimportsWithLayers |
| M-WLD-01 | Weldments and structural frames | S8.06.1, S8.06.2, S8.06.3, S8.06.4, S8.06.5, S8.08.3 | M-WLD-01a: S8.06.5 T-04 Golden.Tests Weldments.FrameLengthsAndMass; M-WLD-01b: S8.06.5 T-02 Integration.Tests Weldments.Sketch3dChangeUpdatesMembers |
| M-WLD-02 | Cut lists | S8.06.6, S8.08.3 | M-WLD-02: S8.06.6 T-04 Golden.Tests Weldments.CutListMatches |
| M-ASM-01 | Mates | S3.01.2, S3.01.3, S3.02.1, S3.02.2, S3.03.1, S3.03.2, S3.03.3, S3.03.4, S3.03.5, S3.03.6, S3.03.7, S3.09.1, S3.09.2 | M-ASM-01: S3.03.7 T-04 Golden.Tests Mechanisms.DofAndDragReplay01..15 |
| M-ASM-02 | Top-down (in-context) design | S3.05.1, S3.05.2, S3.05.3, S3.05.4 | M-ASM-02a: S3.05.4 T-02 Integration.Tests InContext.LidFollowsBaseRim; M-ASM-02b: S3.05.4 InContext.LockStopsUpdate; M-ASM-02c: S3.05.4 InContext.ExternalRefListStatus |
| M-ASM-03 | Large assembly handling | S3.02.3, S3.02.4, S10.01.1, S10.01.2, S10.02.1, S10.02.2, S10.02.3, S10.02.4, S10.02.5 | M-ASM-03a: S10.02.5 T-08 Bench.OpenLightweight2000 under 20 s on the reference machine; M-ASM-03b: S10.02.1 T-02 Integration.Tests LargeAsm.EditLightweightResolves |
| M-ASM-04 | Interference and clearance detection | S3.06.1, S3.06.2, S3.06.3, S3.09.3, S6.06.1 | M-ASM-04: S3.06.3 T-04 Golden.Tests Interference.SeededExactly |
| M-ASM-05 | Smart components and fasteners | S10.03.1, S10.03.2, S10.03.3, S10.03.4, S10.03.5, S10.03.6 | M-ASM-05a: S10.03.3 T-04 Golden.Tests Fasteners.EnclosureFourSeries; M-ASM-05b: S10.03.4 T-02 Integration.Tests Fasteners.LidThicknessUpdatesLengths |
| M-ASM-06 | Exploded views and assembly animation | S3.07.1, S3.07.2, S3.07.3, S3.09.4, S7.01.5, S10.04.4 | M-ASM-06a: S3.07.3 T-02 Integration.Tests Explode.SaveReloadReplayIdentical; M-ASM-06b: S7.01.5 T-06 Golden.Tests Snapshot.ExplodedDrawingViewShowsLines |
| M-ASM-07 | Basic motion and animation | S3.01.2, S3.08.1, S3.08.2, S3.08.3, S3.08.4, S3.09.4 | M-ASM-07: S3.08.4 T-10 Golden.Tests Motion.SliderCrankWithin0_1Percent |
| M-DRW-01 | Drawing views | S7.01.1, S7.01.2, S7.01.3, S7.01.4, S7.01.5, S7.01.6, S7.01.7, S7.01.8, S7.01.9, S7.02.1, S7.09.1 | M-DRW-01a: S7.01.9 T-06 Golden.Tests Snapshot.Drawings01..10; M-DRW-01b: S7.02.1 T-02 Integration.Tests Drawings.HoleMoveUpdatesSectionDetailAndDims |
| M-DRW-02 | Annotations | S7.02.1, S7.02.2, S7.02.3, S7.02.4, S7.02.5, S7.02.6, S7.09.2, S9.02.1, S10.05.3 | M-DRW-02a: S7.02.6 T-06 Golden.Tests Snapshot.Annotations*; M-DRW-02b: S10.05.3 T-02 Integration.Tests Tolerances.DrawingToStackUpAndInspection |
| M-DRW-03 | Drawing tables | S7.03.1, S7.03.2, S7.03.3, S7.05.2, S7.09.2, S8.05.4, S8.06.6 | M-DRW-03a: S7.03.2 T-04 Golden.Tests BomTable.ExpectedRows; M-DRW-03b: S7.03.2 T-02 Integration.Tests BomTable.AssemblyChangeUpdatesBomAndBalloons |
| M-SIM-04 | Motion analysis (rigid-body dynamics) | S9.01.1, S9.01.2, S9.01.3, S9.01.4, S9.01.5, S9.01.6, S9.01.7, S9.05.1, S10.04.4 | M-SIM-04a: S9.01.6 T-10 Sim.Tests MotionBench.AllWithin0_5Percent; M-SIM-04b: S9.01.7 T-10 Sim.Tests MotionBench.CamFollowerStaysInContact |
| M-SIM-05 | Tolerance stack-up analysis | S9.02.1, S9.02.2, S9.02.3, S9.02.4, S9.05.2, S10.05.3 | M-SIM-05: S9.02.4 T-10 Sim.Tests ToleranceSuite.All |
| M-INS-01 | Inspection documentation | S10.05.1, S10.05.2, S10.05.3, S10.05.4 | M-INS-01a: S10.05.2 T-04 Golden.Tests Inspection.FortyCharacteristicsTable; M-INS-01b: S10.05.3 T-02 Integration.Tests Inspection.RevisionKeepsNumbers |
| M-VIS-01 | Photorealistic rendering | S10.04.1, S10.04.2, S10.04.3, S10.04.4, S10.04.5, S10.04.6 | M-VIS-01a: S10.04.3 T-06 Golden.Tests Render.FiveGoldenScenes; M-VIS-01b: S10.04.5 T-08 Bench.RenderPreview1080p under 60 s on the reference machine |
| E-PRJ-01 | Unified project and compilation | S4.01.1, S4.06.1, S4.06.4, S4.10.4, S5.06.1, S5.06.2, S5.15.2, S6.12.3 | E-PRJ-01a: S4.06.4 T-02 Integration.Tests Netlist.GoldenProjectCompiles; E-PRJ-01b: S5.06.2 T-02 Integration.Tests Sync.BothDirectionsZeroDifferences |
| E-PRJ-03 | Assembly variants | S6.07.1, S6.09.5, S7.04.4, S7.06.2, S9.03.2 | E-PRJ-03: S6.09.5 T-04 Golden.Tests Variants.ThreeVariantsBomAndPnp |
| E-PRJ-04 | Design reuse | S4.09.1, S5.14.1 | E-PRJ-04: S5.14.1 T-02 Integration.Tests Reuse.BuckRegulatorBlock |
| E-LIB-01 | Component model | S4.02.1, S4.02.2, S4.02.3, S4.02.4, S4.03.6, S4.10.3, S5.13.2, S9.03.2 | E-LIB-01a: S4.02.4 T-04 Golden.Tests Components.ValidateCleanly; E-LIB-01b: S4.02.4 Components.MismatchedPinPadReported |
| E-LIB-02 | Local library management | S4.01.2, S4.03.1, S4.03.2, S4.03.3, S4.10.2 | E-LIB-02a: S4.03.2 T-02 Integration.Tests Library.EditReleasedCreatesRevB; E-LIB-02b: S4.03.2 Library.ProjectReportsAndAppliesUpdate |
| E-LIB-03 | Footprint generator | S4.04.1, S4.04.2, S4.04.3, S4.04.4, S4.04.5, S4.04.6, S4.04.7, S4.04.8, S4.10.3 | E-LIB-03: S4.04.8 T-04 Golden.Tests FpGen.Cases01..20 |
| E-SCH-01 | Schematic capture | S4.05.1, S4.05.2, S4.05.3, S4.05.4, S4.05.5, S4.10.1, S4.10.5 | E-SCH-01: S4.10.5 T-05 Ui.Tests Replay.Schematic2SheetCompiles |
| E-SCH-02 | Hierarchy and multi-channel design | S4.06.1, S4.06.2, S4.06.3 | E-SCH-02a: S4.06.2 T-04 Golden.Tests Netlist.Hierarchical4Sheet; E-SCH-02b: S4.06.3 T-04 Golden.Tests Netlist.EightChannel |
| E-SCH-03 | Electrical rules check (ERC) | S4.07.1, S4.07.2, S4.07.3, S4.07.4, S4.10.4 | E-SCH-03: S4.07.4 T-04 Golden.Tests Erc.Seeded12Exactly |
| E-SCH-04 | Annotation | S4.08.1, S5.06.2 | E-SCH-04a: S4.08.1 T-04 Golden.Tests Annotation.ExpectedDesignators; E-SCH-04b: S5.06.2 Annotation.BackAnnotateByPosition |
| E-SCH-05 | Schematic directives | S4.08.2, S5.06.3 | E-SCH-05: S5.06.3 T-02 Integration.Tests Directives.UsbPairCreatesDiffPairClass |
| E-SIM-01 | Circuit simulation | S9.03.1, S9.03.2, S9.03.3, S9.03.4, S9.03.5, S9.03.6, S9.03.7, S9.05.3 | E-SIM-01: S9.03.7 T-10 Sim.Tests SpiceSuite.All |
| E-PCB-01 | Layer stack | S5.03.1, S5.03.2, S5.03.3, S5.03.4, S5.10.3, S5.15.6 | E-PCB-01a: S5.03.3 T-04 Golden.Tests Stack.FourAndEightLayerThickness; E-PCB-01b: S5.03.3 Golden.Tests Impedance.Within1Percent |
| E-PCB-02 | Board shape and regions | S5.02.1, S5.04.1, S5.04.2, S5.13.4, S5.15.1, S6.04.2, S6.05.2 | E-PCB-02: S5.13.4 T-06 Golden.Tests Snapshot.BoardCutoutsKeepouts2d3d |
| E-PCB-03 | Component placement | S5.07.1, S5.07.2, S5.07.3, S5.07.4, S5.15.2, S6.05.3 | E-PCB-03a: S5.15.2 T-05 Ui.Tests Replay.PlaceGoldenBoard; E-PCB-03b: S5.07.4 T-04 Golden.Tests Placement.RoomCopy4Channels |
| E-PCB-04 | Interactive routing | S5.01.2, S5.01.3, S5.09.1, S5.09.2, S5.09.3, S5.09.4, S5.09.5, S5.09.6, S5.09.7, S5.09.8, S5.09.9, S5.09.10, S5.15.3 | E-PCB-04a: S5.09.10 T-08 Bench.RouteMouseMove2000x5000; E-PCB-04b: S5.15.3 T-05 Ui.Tests Replay.RouteGoldenBoard; E-PCB-04c: S5.09.9 T-04 Golden.Tests Router.ShoveSuiteZeroDrc |
| E-PCB-05 | High-speed routing | S5.01.3, S5.10.1, S5.10.2, S5.10.3, S5.10.4, S5.10.5, S5.15.4 | E-PCB-05a: S5.10.5 T-04 Golden.Tests HighSpeed.DdrTunedWithin0_1mm; E-PCB-05b: S5.10.5 HighSpeed.ReturnPathFindsSeeded |
| E-PCB-07 | Copper pours | S5.01.4, S5.01.5, S5.11.1, S5.11.2, S5.11.3, S5.11.4, S5.11.5, S5.15.6, S10.06.8 | E-PCB-07a: S5.11.5 T-04 Golden.Tests Pours.AreaWithin0_1Percent; E-PCB-07b: S5.11.5 Pours.ThermalReliefGeometry; E-PCB-07c: S5.11.5 Pours.NoClearanceViolations |
| E-PCB-08 | HDI and fabrication features | S5.03.2, S5.12.1, S5.12.2, S5.12.3, S5.12.4, S6.09.4, S7.04.2 | E-PCB-08: S6.09.4 Golden.Tests Hdi.DrillSpansAndStackData |
| E-RUL-01 | Design rules | S5.03.4, S5.05.1, S5.05.2, S5.05.3, S5.06.3, S5.07.3, S5.08.2, S5.08.3, S5.09.5, S5.15.4 | E-RUL-01: S5.05.2 T-04 Golden.Tests Rules.Resolution30Cases |
| E-RUL-02 | Design rule check (DRC) | S5.02.3, S5.08.1, S5.08.2, S5.08.3, S5.08.4, S5.08.5, S5.08.6, S5.13.3, S5.15.4, S6.06.1 | E-RUL-02a: S5.08.5 T-10 Golden.Tests Drc.SeededSuite; E-RUL-02b: S5.08.6 T-08 Bench.BatchDrc8Layer2000 under 15 s |
| E-3D-01 | 3D board model | S5.13.1, S5.13.2, S5.13.3, S5.13.4, S5.15.5, S6.03.1 | E-3D-01a: S5.15.5 T-06 Ui.Tests Snapshot.Board3dMatchesStack; E-3D-01b: S5.13.1 T-04 Golden.Tests Board3d.ThicknessAndHolesExact |
| E-DOC-01 | Fabrication and assembly drawings | S7.06.1, S7.06.2, S7.06.3 | E-DOC-01a: S7.06.3 T-06 Golden.Tests Snapshot.FabAndAssemblyDrawings; E-DOC-01b: S7.06.1 T-02 Integration.Tests PcbDrawings.NewHoleSizeAddsRowAndSymbol |
| E-OUT-01 | Manufacturing outputs | S6.09.1, S6.09.2, S6.09.3, S6.09.4, S6.09.5, S6.11.3, S7.04.1, S7.04.2, S7.04.3, S7.04.4, S7.04.5, S7.04.6, S7.04.7, S7.09.3 | E-OUT-01: S7.04.7 T-07 Golden.Tests FabConformance.GerberCountsAndPositionsExact |
| E-BOM-01 | Basic parts list | S6.09.5, S7.05.1 | E-BOM-01: S7.05.1 T-04 Golden.Tests Bom.RowsPerVariant |
| E-MB-01 | Multi-board systems | S6.08.1, S6.08.2 | E-MB-01a: S6.08.2 T-02 Integration.Tests MultiBoard.TwoBoardStackCompiles; E-MB-01b: S6.08.2 MultiBoard.SwappedPinPairReported |
| X-01 | Board as an assembly component | S6.03.1, S6.03.2, S6.11.1 | X-01: S6.03.2 T-02 Integration.Tests Links.BoardMatesDof0 |
| X-02 | Board outline linking | S6.02.2, S6.04.1, S6.04.2, S6.04.3, S6.11.1, S6.11.2 | X-02a: S6.04.1 T-02 Integration.Tests Links.BoardWidthResizesCavity; X-02b: S6.04.2 T-02 Integration.Tests Links.WallResizesBoardAndDrcReportsOutside |
| X-03 | Mounting holes and fixings | S6.05.1, S6.11.1 | X-03: S6.05.1 T-02 Integration.Tests Links.HoleMoveMovesBoss3mm |
| X-04 | Height limits and keepouts from the enclosure | S6.05.2, S6.11.2 | X-04: S6.05.2 T-02 Integration.Tests Links.LidLowered3mmCapViolatesHeight |
| X-05 | Mechanically constrained components | S6.05.3, S6.05.4, S6.11.1, S6.11.2 | X-05: S6.05.4 T-02 Integration.Tests Links.ConnectorMove5mmMovesCutout |
| X-06 | Unified interference and clearance | S6.06.1, S6.11.2, S6.12.3 | X-06: S6.06.1 T-02 Integration.Tests Links.CapVsRibFoundByBothChecks |
| X-07 | Combined product BOM | S7.05.2 | X-07: S7.05.2 T-04 Golden.Tests Bom.ProductBomQuantities |
| X-08 | Configurations and variants | S6.07.2, S6.11.3, S10.03.2, S10.03.6 | X-08: S6.07.2 T-02 Integration.Tests Links.NoUsbConfigRemovesConnectorAndCutout |
| X-09 | Cross-domain change propagation and review | S6.01.1, S6.01.2, S6.02.4, S6.12.2 | X-09: S6.02.4 T-02 Integration.Tests Links.ReviewModeHoldsUntilAccepted |
| X-10 | Link manager | S6.01.1, S6.01.2, S6.02.1, S6.02.3, S6.12.1 | X-10a: S6.12.1 T-05 Ui.Tests Replay.LinkCreateLockBreakRetarget; X-10b: S6.02.1 Link.Tests Cycles.RejectedAtCreation |
| E-MCAD-02 | 3D export of boards and products | S6.10.1, S7.08.2, S7.08.3 | E-MCAD-02: S6.10.1 T-07 Golden.Tests StepExport.ProductReimports |
| M-IO-01 | Neutral file formats | S2.09.1, S2.09.2, S2.09.3, S2.11.6, S6.10.1, S7.01.7, S7.01.8, S7.07.1, S7.07.2, S7.07.3, S7.07.4, S7.07.5, S7.07.6, S7.07.7, S8.05.3 | M-IO-01a: S2.09.2 StepRoundTrip.PartMassPreserved, S7.07.7 T-07 Golden.Tests IoRoundTrip.AllFormats; M-IO-01b: S2.09.3 T-07 Golden.Tests StepImport.ThirdParty01..20 |
| M-3DP-01 | Export for 3D printing | S0.10.2, S2.09.4, S2.09.5, S2.11.6, S6.11.3, S7.07.4, S7.08.1, S7.08.2, S7.08.4 | M-3DP-01a: S2.09.5 T-07 Golden.Tests Print.GoldenPartsWatertight; M-3DP-01b: S2.09.5 Print.MeshVolumeWithin0_1Percent; M-3DP-01c: S2.09.5 Print.ThreeMfValidatesWithLib3mf |

**Uncovered requirements:** none.

Totals: 160 requirements, 160 covered; 547 steps; 134 Accept-tagged tests over 133 Accept criteria in 80 requirements.
<!-- coverage:end -->
