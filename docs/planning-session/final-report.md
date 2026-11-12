# Planning session: final report {#planning-final-report}

Date: 2026-11-11. This is the report that `CURSOR_PLAN_PROMPT.md` section 5 step 7 asks for. It closes the planning
session. Phase 3 re-ran the section 6 self-check independently across the whole repository, fixed small
inconsistencies, and re-ran the build, the tests and the checks on this machine. **The session stops here for human
review.**

After Phase 3 the owner made four decisions: fabrication outputs in S6, the repository, the Doxygen install and the
P-03 severity. A follow-up run applied them, installed Doxygen, re-ran everything and pushed the repository.
Section 12 covers it. Sections 1, 5, 7 and 9 show the state after the follow-up.

## 1. Outcome

- **Self-check:** every item in prompt section 6 passes, now including the Doxygen part of the scaffold acceptance
  (section 4.2): `scripts/docs.ps1` builds the HTML manual with **0 Doxygen warnings**.
- **Build:** the Debug and Headless clean rebuilds succeed with **0 warnings**.
- **Tests:** the Headless run passes 17 test executables (21 tests). The Debug run passes all 18 executables
  (24 tests), including `Ui.Tests`.
- **Checks:** the three checks pass on the repository. `scripts/check-selftest.ps1` confirms that each check fails on
  its deliberate violations (9 cases) and that an allowed third-party network import passes and is reported (1 case).
- **Network imports:** the only third-party network import is `Qt6Cored.dll` → `WS2_32.dll` (`WSAAsyncSelect`,
  `WSACleanup`, `WSAStartup`). It is allowed under the owner's P-03 decision and listed in
  [`docs/network-imports.md`](../network-imports.md).
- **Coverage:** all **160** spec requirement IDs are covered, in `docs/traceability.csv` and in at least one of the
  **547** steps. All 80 requirements that have an `Accept:` line have tagged tests.
- **Fixes:** Phase 3 made four small fixes (section 4). The largest adds the 66 sub-folders that the steps use to the
  `docs/FOLDER_STRUCTURE.md` tree. They take 62 new tree lines.

## 2. What was created in this session

All paths are relative to `C:\dev\CADence`.

| Phase | Output |
|---|---|
| 1: understanding | `docs/planning-session/phase1-understanding.md` covers the input summary, the 28 conflicts and ambiguities C-01 … C-28, the requirement inventory, the toolchain check and the questions. It also wrote `docs/FOLDER_STRUCTURE.md`. |
| 2a: scaffold | 200 files: `APP.slnx`, `Directory.Build.props/.targets`, `vcpkg.json`, `vcpkg-configuration.json` (baseline `177e8175121536d414f405ed8b3099b325a42638`), `LICENSE` (GPL-3.0 text, 35,149 bytes), `README.md`, `THIRD_PARTY_NOTICES.md`, `.gitignore`, `.gitattributes`, `build/props/*.props` (6 files), 18 module folders in `src/` (project, filters, `Module.h/.cpp`, README), 18 test projects in `tests/`, `tests/data/scripts/README.md`, 18 seed library kind folders in `libraries/`, 8 scripts in `scripts/`, plus `docs/mainpage.md`, `docs/images/.gitkeep`, `docs/format/README.md` and `docs/commands/README.md`. Report: `docs/planning-session/scaffold-report.md`. |
| 2b: planning docs | `docs/IMPLEMENTATION_CHECKLIST.md` (547 steps), `docs/traceability.csv` (160 rows), `docs/stages/S0.md` … `S10.md`, `docs/RISKS.md` (24 risks), `docs/cpp-features.md`, `docs/provenance.md`, `docs/data-sources.md`, `docs/adr/0000` … `0033` (34 ADRs). Report: `docs/planning-session/planning-docs-report.md`. |
| 3: verification | `docs/planning-session/checks/check_plan.py` (an independent plan checker; run it with `python docs/planning-session/checks/check_plan.py`) and this report. |

## 3. Self-check results (prompt section 6), re-run independently

| Check | Result | Evidence |
|---|---|---|
| Every spec requirement ID is in `traceability.csv` and in at least one step | **Pass** | `check_plan.py` parses the spec bullets, the headings and the P-target table and finds exactly 160 IDs. All 160 have a CSV row and appear in at least one step's Requirements line. No step cites an ID the spec lacks. |
| Every step has the Requirements, Modules, Depends on, Deliverables, Tests and Done-when lines | **Pass** | All 547 steps have all six lines, non-empty and in rule-080 order. Step IDs are unique and match their stage and work package. |
| No step depends on a later step | **Pass** | Every "Depends on" ID exists and appears earlier in the document. Each step either lists IDs or says `none`. |
| Every stage ends with an `Sx.99` gate that includes the human review checkpoint | **Pass** | In all 11 stages, the last work package is `Sx.99 Stage gate`. Each gate covers the full test suite, benchmarks, traceability and docs. Each gate's last step is `Human review checkpoint: stop and wait for approval`. |
| C++ files: SPDX header, Doxygen file comment, `.clang-format` and rule 010 | **Pass** | All 62 C++ files in `src/` and `tests/` were checked. Each starts with both SPDX lines and has an `@file` comment and `#pragma once` (headers). Each is clean under `clang-format --dry-run --Werror --style=file` (clang-format 22.1.3, bundled with VS 2026). Extra rule-010 greps found no `if (`, no K&R braces, no lines over 120 columns, no tabs, no CR characters and no runs of 3 or more blank lines. One file was also reviewed by eye. |
| No CMake files, no runtime network code, no third-party code without provenance | **Pass** | No `CMakeLists.txt`, `*.cmake` or `CMakePresets.json` exists outside `vcpkg_installed/` and `out/`. The network grep of `src/` matches only the MSBuild XML namespace URL in project files. The only non-GPL SPDX identifier or "Adapted from" marker is a test fixture string inside `check-selftest.ps1`. `third_party/` does not exist, and `docs/provenance.md` holds only its header. `vcpkg.json` (gtest, plus `qtbase` with `gui` and `widgets` only) matches `THIRD_PARTY_NOTICES.md`. |
| Build, tests and checks green | **Pass** (Doxygen included since the follow-up run) | See section 5. |
| Stage plans meet section 4.4 | **Pass** | Each of the 11 plans has the goal (1 or 2 sentences), the requirements, the work packages, the entry criteria, the exit criteria, the key risks and the golden data. Work-package IDs and titles match the checklist exactly. The exit criteria are copied verbatim from spec section 9. The step counts in the headers match the checklist. |
| `RISKS.md` meets section 4.6 | **Pass** | The table has the required columns (ID, risk, likelihood, impact, mitigation, owner). All 11 required topics are present (R-01 … R-07, R-09, R-10, R-11, R-13), along with 13 more. Every owning step ID and ADR number exists. |
| Deliverable paths match `docs/FOLDER_STRUCTURE.md` | **Pass after fix 3** | Before the fix, 66 sub-folders named in step deliverables were missing from the tree. Every top-level folder was already listed. The checker skips the project-relative paths `links/`, `blobs/` and `cache/` and the module names such as `ecad/pcb`. |
| Extra: Accept criteria → tests; coverage table; cross-references | **Pass** | All 80 requirements with an `Accept:` line have `(Accept: …)` tests, 134 tags in total. The coverage table matches the steps exactly, and its "uncovered" list is empty. Every step ID and ADR number cited in the checklist, the CSV, the risks, the stage plans, the ADRs and the folder doc exists. ADRs 0034 … 0041 are reserved for the spikes, and each spike pair ends in its ADR. UI-only steps depend on the headless step that delivers their command or test. |

## 4. Fixes made in Phase 3

1. **T-07 and T-10 had no S0 step, although the CSV and `docs/stages/S0.md` plan them in S0.** Spec section 9 puts
   "T-01…T-13 scaffolding" in S0. Step S0.08.3 builds the mesh validator, which is T-07's structural checker for STL,
   and the analytic oracles that T-10 compares against. The fix adds `T-07, T-10` to S0.08.3's Requirements line and
   adds S0.08.3 to the T-07 and T-10 rows of `docs/traceability.csv` and of the checklist coverage table.
2. **40 Markdown files had CRLF line endings**: `docs/RISKS.md`, `cpp-features.md`, `provenance.md`,
   `data-sources.md`, all 34 ADRs and two planning-session reports. `.editorconfig` and `.gitattributes` require LF
   for these files. The fix converts them to LF; the content is unchanged.
3. **`docs/FOLDER_STRUCTURE.md` was missing 66 sub-folders that the steps create.** Examples are
   `src/core/command/`, `src/ecad/pcb/router/`, `src/ui/shell/`, `tests/support/oracles/` and five
   `tests/<module>/spikes/` folders. The fix adds 62 tree lines: 61 sub-folders, plus one `<module>/spikes/`
   placeholder for the five spike folders. Each line has a stage marker (the stage of the first step that creates the
   folder) and a one-line purpose that names that step's requirement IDs. A legend sentence explains where these
   sub-folders come from. The existing descriptions are unchanged. Two existing lines were redrawn only for the tree
   glyphs: the `translations/` line and the continuation line under `support/`.
4. **A false cross-reference was ruled out, and nothing was changed.** `S2.03.4a` in the checklist header is the
   rule-080 example of a letter suffix, not a step reference.

## 5. Build, test and check results (this machine, follow-up run)

Toolchain: VS 2026 Community 18.10 (v145, MSVC 19.51), MSBuild 18, the VS-bundled vcpkg and Doxygen 1.18.0. All
commands ran in Windows PowerShell 5.1 as `powershell -NoProfile -ExecutionPolicy Bypass -File <script>`. Logs are in
`out/logs/final-*.txt`. The Phase 3 logs (`phase3-*.txt`) are kept beside them.

| Command | Result |
|---|---|
| `scripts\build.ps1 -Configuration Debug -Rebuild` | `build: OK (Debug, 0 warnings)`, exit 0; the log has no `warning` or `error` lines |
| `scripts\build.ps1 -Configuration Headless -Rebuild` | `build: OK (Headless, 0 warnings)`, exit 0; no `warning` or `error` lines |
| `scripts\test.ps1` (Headless) | 17 executables, 21 tests, 0 failures; CLI smoke `APP 0.0.0` (exit 0); the three checks OK, and the network check reports 0 allowed third-party imports (no Qt in Headless); `test: OK`, exit 0 |
| `scripts\test.ps1 -Configuration Debug -NoBuild` | 18 executables, 24 tests (including `Ui.Tests`), 0 failures; the network check lists 1 allowed import; `test: OK`, exit 0 |
| `scripts\check-layering.ps1` | `OK (36 projects, 62 C++ files)`, exit 0 |
| `scripts\check-spdx.ps1` | `OK (115 files)`, exit 0 |
| `scripts\check-network-apis.ps1` | `OK (42 source files, 44 build files, vcpkg.json; allow-list: 0 entries)`, exit 0. With `-BinaryDir out\Debug,out\Headless`: `OK (..., 38 executables, 92 third-party binaries, allowed third-party network imports: 1; ...)`, and the summary line `allowed: Qt6Cored.dll -> WS2_32.dll: WSAAsyncSelect (ordinal 101), WSACleanup (ordinal 116), WSAStartup (ordinal 115) [out/Debug/Ui, out/Debug/Ui.Tests]`. Written to [`docs/network-imports.md`](../network-imports.md). |
| `scripts\check-selftest.ps1` | `OK (3 checks pass on the clean copy; 9 deliberate violations detected; 1 allowed case passes and is reported)`, exit 0 |
| `scripts\docs.ps1` | `docs: HTML OK -> out\docs\html\index.html`, exit 0; `out/docs/doxygen-warnings.log` is empty |
| `scripts\docs.ps1 -Pdf` | Stops before running Doxygen with `docs: pdflatex not found ...`, exit 1. This is expected, because LaTeX is not installed by decision. |
| `python docs\planning-session\checks\check_plan.py` | `check_plan: OK` |

Not re-verified in the follow-up run:

- `out\Debug\Ui\Ui.exe` by hand: it showed the window title `APP` and closed with exit 0 in Phase 3. `Ui.Tests` passed
  in this run.
- The vcpkg baseline commit: the scaffold worker confirmed it through the GitHub API.
- The Release configuration: it was green in Phase 2a and was not requested here.
- A clean-clone build: this is step S0.01.1.

## 6. Coverage statistics

| Measure | Value |
|---|---|
| Requirement IDs in the spec | 160: P 20, A 11, B 7, L 5, F 10, U 15, T 14, CR 7, M 34, E 27, X 10 |
| Covered by at least one step / rows in the CSV | 160 / 160 |
| Requirements with an `Accept:` line / tagged Accept tests | 80 / 134 tags over 133 clauses |
| Steps / work packages / spike steps | 547 / 119 / 17 (spikes SP-1 … SP-8, plus SP-8a) |
| ADRs | 34 written (0000 … 0033); 0034 … 0041 reserved for spikes |
| Risks | 24 (R-01 … R-24) |

Two requirement counts are given per stage. "Planned in stage" counts the CSV rows whose `stage` lists that stage.
26 rows list more than one stage, so the column sums to more than 160. "Cited by steps" also counts regression, gate
and extension steps.

| Stage | Name | Steps | Work packages | Requirements planned in stage | IDs cited by steps |
|---|---|---|---|---|---|
| S0 | Foundation | 102 | 16 | 79 | 81 |
| S1 | Sketcher | 49 | 9 | 6 | 20 |
| S2 | Part modeling core | 55 | 12 | 11 | 28 |
| S3 | Assemblies core | 42 | 10 | 9 | 23 |
| S4 | Electronics libraries and schematic | 45 | 11 | 11 | 25 |
| S5 | PCB layout | 69 | 16 | 16 | 33 |
| S6 | MCAD ↔ ECAD integration | 37 | 13 | 17 | 42 |
| S7 | Documentation and manufacturing | 48 | 10 | 11 | 30 |
| S8 | Advanced modeling | 38 | 9 | 10 | 20 |
| S9 | Simulation and analysis | 26 | 6 | 3 | 20 |
| S10 | Scale and polish | 36 | 7 | 15 | 35 |
| **Total** | | **547** | **119** | | **160** |

## 7. Open questions for the owner

The owner resolved the fabrication outputs at S6 (ADR 0015), the P-03 wording (ADR 0026), the repository and the
Doxygen install. Section 12 lists these decisions, and they are no longer open. The remaining questions:

1. **Continuous integration (ADR 0033, open).**
   - **Question:** which service to use: GitHub Actions on hosted Windows runners, a self-hosted runner on this
     machine, or local runs only. The repository is now https://github.com/smeekel/CADence. ADR 0033 (local-first)
     is unchanged.
   - **Current plan:** `scripts/test.ps1` and `scripts/ci-local.ps1` (S0.15.1) meet "CI runs all tests" locally, and
     hosted CI is step S1.08.1.
2. **Tool installs.**
   - **PowerShell 7:** optional. Rule 040 asks for it, but the scripts stay compatible with Windows PowerShell 5.1 so
     that P-02 holds, and they have been run only under 5.1.
   - **Optional extras:** a Clang/GCC toolchain (the VS "C++ Clang tools" or WSL 2 compilers) for SP-1's L-03 matrix,
     and MiKTeX for the PDF manual (`scripts/docs.ps1 -Pdf`). LaTeX is not installed, by decision.
3. **Licence corrections.**
   - **OCCT:** OCCT is LGPL-2.1-only (with the OCCT exception) in the vcpkg metadata, while spec section 4.3 says
     "LGPL-2.1-or-later, LGPL-3.0". Linking stays allowed, but the spec should be corrected (ADR 0005). Qt Advanced
     Docking System is also LGPL-2.1-only (ADR 0030).
   - **ngspice:** the vcpkg port metadata declares CC-BY-SA-4.0. The actual licence must be verified in S9.03.1
     before linking (ADR 0008, R-12).
4. **The six layering ADRs, 0016 … 0021 (Proposed).** Each adds an edge to the rule-030 layering table, and each must
   be Accepted before the step that applies it:
   - 0016: `core` uses nlohmann/json. Needed from S0.03.
   - 0017: `cli` references `platform` and `render`. Needed in S0.10.3.
   - 0018: `link` references `ecad/lib`, `ecad/sch` and `drw`. Needed from S5 to S7.
   - 0019: `drw` references `sketch`. Needed in S7.
   - 0020: `io` references `sketch` and `drw`. Needed in S7.
   - 0021: `sim` references `part` and `drw`. Needed in S9.
5. **The other Proposed ADRs, 0022 … 0033 except 0026 (Accepted).** They cover Eigen in `sketch`, `asm` and `sim`;
   Qt confined to `ui`; OS folders injected into `doc`; the 3D board in `link`; unknown-field preservation; UUIDs in
   BSON; vcpkg root selection; Qt ADS docking; spdlog; and the planning assumptions.
   - **Needed in S0:** 0029 before S0.01.1, then 0031, 0024, 0027, 0028, 0023 and 0030.
   - **Needed later:** 0022 from S1 (the sketch solver) and 0025 in S5 (the 3D board).
   - **Bundles:** 0032 records the planning assumptions, and 0033 records the open CI decision.
6. **Smaller items.**
   - **Spec Q-08 (MP4/FFmpeg):** the default is an optional, dynamically loaded encoder seam.
   - **Spec Q-09 (writable global library folder):** the default is writable.
   - **Product name:** keep the `APP` placeholder until the owner renames the software. "Cadence" is a third-party
     trademark.

## 8. Assumptions in effect

These are recorded in ADR 0032 or in their own ADRs, and come from Phase 1 conflicts C-01 … C-28:

- **Stage scope.**
  - S0 also delivers B-*, L-*, CR-*, F-09 and F-10 (C-04).
  - The later performance targets belong to their feature stages: P-13 in S1, P-12 in S2, P-15 and P-16 in S5, P-11
    and P-14 in S10 (C-05).
  - X-07 is delivered in S7 (C-03), and T-14 at S6 uses two separate parts (C-02).
- **Layering.**
  - The scaffold follows rule 030 verbatim. Each extension is applied only in the step that first needs it, after its
    ADR is Accepted (C-07, C-08).
  - Only `ui` uses Qt (C-09).
  - The 3D board and 3D DRC live in `link` behind provider interfaces (C-10).
  - OS folders are injected into `doc` (C-11).
- **Names and paths.**
  - The `asm` module uses namespace `app::assembly`, because `asm` is a C++ keyword (C-13).
  - Test projects follow `tests/<module>/<Project>.Tests.vcxproj` (C-14).
  - Every project, tests included, builds to `out/<Configuration>/<Project>/` (C-15).
  - Benchmark history is kept in `bench/history.csv` (C-16).
- **File format.**
  - Unknown keys stay in place, after the schema-known keys (C-20).
  - BSON stores ObjectIds as binary values, chosen by schema `format: uuid` (C-21).
  - Parameters carry ObjectIds (C-27).
- **Libraries.**
  - Two extra kinds, `layerStack` and `environment`, are added (C-19).
  - The app seeds `%ProgramData%\APP\libraries\` on first run. Installer technology is out of scope (C-22).
- **Build.**
  - The scripts stay Windows PowerShell 5.1 compatible and ASCII-only (C-23).
  - The vcpkg root is chosen explicitly, and the user-wide integration is ignored (C-24, ADR 0029).
- **Traceability.**
  - The CSV `stage` column names the delivering stage or stages. Steps in other stages may also cite an ID as
    regression, gate or extension work. Phase 3 checked that every listed stage has at least one step citing the ID.
  - The spec is not edited. Proposed additions to spec section 17 go to the owner (C-18).

## 9. Conflicts between the spec and the notepads or rules

The spec wins in every case, except where it contradicts itself or the facts; those cases are marked **owner**.

| ID | Conflict | Resolution |
|---|---|---|
| C-01 | T-14 at S6 needs E-OUT-01 outputs from S7 | **owner**: minimal Gerber, drill, IPC-2581, BOM and pick-and-place writers in S6 (ADR 0015, Accepted) |
| C-03 | X-07: S7 in its heading and in section 15, but inside "X-01…X-10" in the S6 row of section 9 | S7, which the roadmap supports |
| C-04 | Spec S0 row versus `Stage_Roadmap.md` (B, L, CR, F-09, F-10) | Roadmap scope, which the S0 exit criteria imply |
| C-07, C-08 | Rule 030 and the notepads disagree on dependencies that the requirements need (for example `drw::IBomProvider` in `link`, and Eigen) | ADRs 0016 … 0022 |
| C-09 | Rule 010 allows Qt in `render` and `platform`; A-01, P-04 and B-04 require a Headless build without Qt | Qt only in `ui` (ADR 0023) |
| C-12 | P-03 "linked" versus Qt6Core importing `ws2_32.dll` | **owner**: third-party network imports allowed, noted and summarised; APP code in `src/` fails (ADR 0026, Accepted) |
| C-14 | T-01 `tests/<module>.Tests.vcxproj` versus the subfolder form in rule 060 and the testing notepad | Subfolder form |
| C-15 | B-03 `out/<Configuration>/<Project>/` versus the notepad's `out/<Configuration>/tests/` | Spec form |
| C-16 | T-08 `bench/history.csv` versus the notepad's `tests/bench/` | Both: the project in `tests/bench/`, the history in `bench/` |
| C-17 | Section 16 traceability columns versus the prompt and rule 080 | The prompt's columns, a superset |
| C-19 | Seed notepad kinds (layer stacks, environments) missing from F-10 | Two kinds added (proposed) |
| C-20 | F-08 preserves unknown fields in place; the notepad moves them to `extensions` | Spec (ADR 0027) |
| C-21 | F-09: only `$magic` is a BSON UUID; the notepad makes every ObjectId binary | Schema-driven binary ObjectIds (ADR 0028) |
| C-23 | Rule 040 requires PowerShell 7; P-02 allows only VS and Git | Scripts compatible with 5.1 |
| C-25 | Spec section 4.3 OCCT licence versus the vcpkg metadata | **owner** (open question 3) |
| C-27 | The section 6.2 parameter example has no `id`; the notepad envelope has one | Parameters carry ObjectIds |

Phase 3 found no new conflict between the spec and the notepads.

## 10. Proposed first five steps

Each step depends only on S0.01.1 or on nothing, so all five can start once this plan and ADR 0029 are approved:

1. **S0.01.1: Spike SP-1a: verify the scaffold toolchain from a clean clone** (time box: 1 day).
   - Clean-clone build in all three configurations.
   - Confirm the vcpkg root and baseline (ADR 0029).
   - Record `dumpbin /imports` lists and compare them with `docs/network-imports.md` (ADR 0026).
2. **S0.01.2: Spike SP-1b: C++23/26 feature matrix on MSVC, GCC and Clang, ending in ADR 0034** (time box: 1 day).
   - Confirms `docs/cpp-features.md`.
   - Needs a Clang/GCC toolchain (open question 2).
3. **S0.01.3: Add the Headless dependency check** (`scripts/check-headless.ps1`). Covers P-04's Accept criterion.
4. **S0.01.4: Add the build-hygiene check for migration-ready projects** (`scripts/check-build-hygiene.ps1`). Covers
   B-01, B-02, B-03 and B-07.
5. **S0.02.1: Tighten the SPDX check's provenance rule to the provenance table columns.** Covers CR-07a and CR-07b
   through new `check-selftest.ps1` cases.

## 11. Files changed in Phase 3

- **Created:** `docs/planning-session/checks/check_plan.py` and `docs/planning-session/final-report.md`.
- **Edited:**
  - `docs/IMPLEMENTATION_CHECKLIST.md`: the Requirements line of S0.08.3, and the coverage-table rows for T-07 and T-10.
  - `docs/traceability.csv`: the T-07 and T-10 rows.
  - `docs/FOLDER_STRUCTURE.md`: the legend sentence and the `src/` and `tests/` tree blocks.
- **Line endings only (CRLF to LF):**
  - `docs/RISKS.md`, `docs/cpp-features.md`, `docs/provenance.md` and `docs/data-sources.md`.
  - `docs/adr/0000` … `0033` (34 files).
  - `docs/planning-session/scaffold-report.md` and `docs/planning-session/planning-docs-report.md`.
- **Build output and logs:** `out/logs/phase3-*.txt` and the refreshed `out/test-results/`. These are generated and
  ignored by git.

## 12. Owner decisions and follow-up run (2026-11-11)

### Decisions

1. **S6 outputs:** minimal Gerber, drill, IPC-2581, BOM and pick-and-place writers are built in S6 (S6.09.1 …
   S6.09.5). ADR 0015 is **Accepted**. It is removed from the open questions in the checklist, the stage plans
   (`S6.md`), the risks (R-24) and the planning reports.
2. **Repository:** the work is pushed to https://github.com/smeekel/CADence. The software keeps the `APP`
   placeholder, because it will be renamed later. CI is not decided, so ADR 0033 (local-first) is unchanged.
3. **Doxygen:** installed. LaTeX is not installed.
4. **P-03 severity reduced (ADR 0026, Accepted).** APP needs no network support.
   - A socket or network library that a third-party library imports, such as Qt6Core importing `ws2_32.dll`, is
     allowed. It is not a failure.
   - `scripts/check-network-apis.ps1` notes every such import and prints a summary of the library, the network DLL
     and the imported APIs. The summary is in [`docs/network-imports.md`](../network-imports.md).
   - These still fail: network APIs in APP's own code in `src/`, network link inputs in APP's projects, network ports
     or `qtbase` network features in `vcpkg.json`, and APP-built executables that import a network DLL.
   - The offline smoke run (S0.08.7) stays the runtime proof.

### Network-import summary

Generated from `out/Debug` and `out/Headless` (see [`docs/network-imports.md`](../network-imports.md)):

| Library | Network DLL | Imported APIs | Found in |
|---|---|---|---|
| `Qt6Cored.dll` | `WS2_32.dll` | `WSAAsyncSelect` (ordinal 101), `WSACleanup` (ordinal 116), `WSAStartup` (ordinal 115) | `out/Debug/Ui`, `out/Debug/Ui.Tests` |

Qt6Core imports the WinSock functions by ordinal only. The check translates the fixed WinSock 1.1 ordinals into
names. The Headless build has no third-party network import. No other third-party DLL imports a network library.

### Doxygen

- **Install:** `winget install DoxiGen.Doxygen` fails with "No package found matching input criteria". The winget id
  is `DimitriVanHeesch.Doxygen`, so `winget install --id DimitriVanHeesch.Doxygen -e` was used. It installed Doxygen
  1.18.0 from doxygen.nl, and winget verified the installer hash. The official installer was not needed.
- **First run:** 23 warning lines. Four fixes made it clean:
  1. **`IMPLICIT_DIR_DOCS = NO`.** Doxygen 1.18 turns each `src/<module>/README.md` into directory documentation
     by default, which drops its `{#module-...}` page anchor. Without this setting, 18 `@ref module-*` links in
     `docs/mainpage.md` were unresolved.
  2. **`EXCLUDE` replaces `EXCLUDE_PATTERNS`.** The pattern `*/out/*` also matched `src/ecad/out/`, so the whole
     `ecad/out` module was missing from the manual. The Doxyfile now excludes the root folders `tests`, `out`,
     `vcpkg_installed` and `third_party` by path.
  3. **`GENERATE_LATEX = NO` by default.** LaTeX output converts the built-in class diagram (`MainWindow` derives
     from `QMainWindow`) with `epstopdf`, which needs TeX. `scripts/docs.ps1 -Pdf` passes `GENERATE_LATEX = YES` on
     top of the Doxyfile through `doxygen -`. It also checks for `pdflatex` and `epstopdf` before running Doxygen.
  4. **ADR template heading.** `<short decision title>` in `docs/adr/0000-template.md` was read as an HTML tag. It is
     now "Short decision title".
- **Predictions from the scaffold report:**
  - **`docs/adr`:** the folder exists, so there was no warning and nothing to fix.
  - **`Q_OBJECT`:** gave no warning, because Doxygen handles it in the source listing. Because the Doxyfile uses
    `EXPAND_ONLY_PREDEF = YES`, `Q_OBJECT=` was still added to `PREDEFINED`, so that it never shows up as a member.

### Files changed in the follow-up run

- **Created:** `docs/network-imports.md`. It is generated by `scripts/check-network-apis.ps1 -SummaryPath`.
- **Edited:**
  - **Doxygen:** `Doxyfile`, `scripts/docs.ps1` and `docs/adr/0000-template.md`.
  - **Network checks:** `scripts/check-network-apis.ps1`, with the new rule, the import summary and the
    `-SummaryPath` switch. `-BinaryDir` now accepts several folders. `scripts/check-selftest.ps1` gains the
    `network-own-binary` failure case and the `network-third-party-import` allowed case.
  - **ADRs:** `docs/adr/0015-reference-scenario-outputs-in-s6.md` and `docs/adr/0026-network-api-check-scope.md`, both
    Accepted, plus the index in `docs/adr/0001-record-architecture-decisions.md`.
  - **Checklist:** `docs/IMPLEMENTATION_CHECKLIST.md`: the open-decisions table, now split into open items and items
    decided by the owner; steps S0.01.1, S0.02.3, S0.08.7, S0.14.1 and S0.99.1; the S6 introduction; and the P-03
    coverage row.
  - **Traceability:** `docs/traceability.csv`, in the P-03, T-14, E-PRJ-03, E-PCB-08 and E-OUT-01 rows.
  - **Risks:** `docs/RISKS.md`, in R-14, R-16 and R-24.
  - **Other docs:** `docs/stages/S0.md`, `docs/stages/S6.md`, `docs/FOLDER_STRUCTURE.md` (the `docs/` files
    paragraph), `docs/planning-session/planning-docs-report.md` (the open questions) and this report.
- **Logs:** `out/logs/final-*.txt`. They are ignored by git.

### Git

- **Before:** the workspace was already a Git repository on `main`, with `origin` set to
  https://github.com/smeekel/CADence.git. It had the remote's single commit `535926a` ("first commit", the
  one-line README) checked out, and all the work was uncommitted on top of it. No `git init`, merge or rebase was
  needed: the new commits sit directly on `535926a`, so the push is a fast-forward.
- **`.gitignore`:** it already keeps out `/out/` (all build output and intermediates, because `OutDir` and `IntDir`
  are both under `out/`), `/vcpkg_installed/` and `.vs/`. The root-anchored `/out/` keeps the `src/ecad/out/` and
  `tests/ecad/out/` sources tracked.
- **Commits on `main`,** on top of `535926a` ("first commit"):

  | Commit | Content |
  |---|---|
  | `fed918f` | Project inputs: the spec, the notepads, `CURSOR_PLAN_PROMPT.md`, `KIT_README.md`, `.cursor/rules` and `.cursor/commands` (28 files) |
  | `4cd42ea` | S0 scaffold: the root configuration files, `README.md`, `LICENSE`, `Doxyfile`, `build/`, `src/`, `tests/`, `libraries/`, `scripts/` and the Doxygen pages in `docs/` (203 files) |
  | `9764bba` | Planning documents: the checklist, the traceability CSV, the stage plans, the risks, the ADRs, `FOLDER_STRUCTURE.md` and `network-imports.md` (53 files) |
  | `5596b9c` | Planning-session reports and `checks/check_plan.py` (5 files) |
  | the next commit | Records the push result in this section |

- **Push:** `git push -u origin main` gave `535926a..5596b9c  main -> main`, exit 0, with no force. `main` tracks
  `origin/main`, and 289 files are tracked.
