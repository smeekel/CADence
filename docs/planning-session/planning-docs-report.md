# Planning documents report (Phase 2b) {#planning-docs-report}

Date: 2026-11-11. Scope: prompt sections 4.3 to 4.6, written in parallel with the scaffold worker. No build or code
files were added to the repository, and nothing was committed or pushed.

## Outcome

The implementation plan is complete and passes its self-checks:

- All **160** requirement IDs in the spec are covered. Each appears in at least one step and in
  `docs/traceability.csv`, and the "uncovered" list is empty.
- All **133** Accept criteria (in the 80 requirements that have an `Accept:` line) map to named, tagged tests.
- No step depends on a later step.
- The plan has **547 steps** in S0 … S10. Every stage ends with an `Sx.99` gate that includes "Human review
  checkpoint: stop and wait for approval".

## Files written

| File | Content |
|---|---|
| `docs/IMPLEMENTATION_CHECKLIST.md` | Header with conventions and open decisions; stages S0 … S10 with work packages and steps; Future section; generated coverage table |
| `docs/traceability.csv` | 160 rows, columns `id,title,source,stage,status,steps,tests,notes`; `steps` separated by `;`; status `not-started` |
| `docs/stages/S0.md` … `S10.md` | 11 stage plans: goal, requirements, work packages, entry criteria, exit criteria (spec section 9), key risks, golden and seed data, notes |
| `docs/RISKS.md` | 24 risks (R-01 … R-24) with likelihood, impact, mitigation and owning step or spike |
| `docs/cpp-features.md` | Allowed, conditional and not-allowed C++23/26 features from the Phase 1 MSVC probe; GCC/Clang to be confirmed in SP-1 |
| `docs/provenance.md` | Empty provenance table with header (read by `scripts/check-spdx.ps1`) |
| `docs/data-sources.md` | Empty data-source table with header |
| `docs/adr/0000` … `0033` | 34 ADR files (template, index, 13 spec decisions, 20 Phase 1 assumptions); 0034 … 0041 reserved for spikes |
| `docs/planning-session/planning-docs-report.md` | This report |

## Coverage statistics

| Measure | Value |
|---|---|
| Requirement IDs in the spec | 160 (P 20, A 11, B 7, L 5, F 10, U 15, T 14, CR 7, M 34, E 27, X 10) |
| Covered by at least one step | 160 |
| Present in `traceability.csv` | 160 |
| Requirements with an `Accept:` line | 80 |
| Accept criteria (clauses) | 133, all tagged; 134 tagged tests (M-IO-01a has two) |
| Steps | 547 |
| Spike steps | 17, across 8 spikes (SP-1 … SP-8, with SP-8a as an early measurement in S3) |
| ADRs written | 34 (0000 … 0033) |

Steps, work packages and requirements per stage. A requirement can be planned in more than one stage (for example
`S2;S7`), so the requirement column adds up to more than 160.

| Stage | Name | Steps | Work packages | Requirements planned in stage |
|---|---|---|---|---|
| S0 | Foundation | 102 | 16 | 79 |
| S1 | Sketcher | 49 | 9 | 6 |
| S2 | Part modeling core | 55 | 12 | 11 |
| S3 | Assemblies core | 42 | 10 | 9 |
| S4 | Electronics: libraries and schematic | 45 | 11 | 11 |
| S5 | PCB layout | 69 | 16 | 16 |
| S6 | MCAD ↔ ECAD integration | 37 | 13 | 17 |
| S7 | Documentation and manufacturing | 48 | 10 | 11 |
| S8 | Advanced modeling | 38 | 9 | 10 |
| S9 | Simulation and analysis | 26 | 6 | 3 |
| S10 | Scale and polish | 36 | 7 | 15 |
| **Total** | | **547** | **119** | |

Spikes. Each spike is time-boxed and ends in an ADR that must be Accepted before dependent work starts.

| Spike | Topic | Steps | ADR |
|---|---|---|---|
| SP-1 | Toolchain and C++ feature matrix | S0.01.1, S0.01.2 | 0034 |
| SP-2 | Topological naming (`TopoRef`) | S0.09.5, S0.09.6 | 0035 |
| SP-3 | Sketch solver | S1.01.2, S1.01.3 | 0036 |
| SP-4 | Mate solver | S3.01.2, S3.01.3 | 0037 |
| SP-8a | Early large-assembly measurement | S3.02.4 | (feeds 0041) |
| SP-5 | Push-and-shove router | S5.01.2, S5.01.3 | 0038 |
| SP-6 | Polygon pours | S5.01.4, S5.01.5 | 0039 |
| SP-7 | Cross-domain links | S6.01.1, S6.01.2 | 0040 |
| SP-8 | Large-assembly strategy | S10.01.1, S10.01.2 | 0041 |

These spikes cover the six high-risk areas in `Stage_Roadmap.md`: topological naming, sketch solver, mate solver,
push-and-shove router, cross-domain links and large-assembly performance. Two extra spikes cover the toolchain (SP-1)
and pours (SP-6).

## Self-check results (prompt section 6, for the planning documents)

| Check | Result | How it was checked |
|---|---|---|
| Every requirement ID is in `traceability.csv` and in at least one step | **Pass** (160/160) | Script: IDs parsed from spec headings, bullets and the P-target table (exactly 160), compared with each step's Requirements line and with the CSV after writing |
| Every step has Requirements, Modules, Depends on, Deliverables, Tests and Done-when lines | **Pass** (547/547) | Script: per-step field parser |
| No step depends on a later step | **Pass** | Script: every ID in "Depends on" must exist and come earlier in document order |
| Every stage ends with `Sx.99` including the human review checkpoint | **Pass** (11/11) | Script: last step of each stage is `Sx.99.*`, and one gate step is titled "Human review checkpoint: stop and wait for approval" |
| Every Accept criterion maps to a named test | **Pass** (133/133) | Script: `(Accept: <ID><clause>)` tags compared with a per-requirement clause count. The set of requirements with an `Accept:` line was re-derived from the spec and matched the count table (80) |
| Coverage table with an empty "uncovered" list | **Pass** | Generated into the checklist between `<!-- coverage:start -->` and `<!-- coverage:end -->` |
| Future section with seams and the steps that build them | **Pass** | 13 spec section 14 items, each mapped to existing step IDs |
| ADR structure | **Pass** (34 files) | Script: title with `{#adr-NNNN}` anchor, Status, and the five rule-080 sections; every "ADR NNNN" and step ID cited in the docs exists |
| Doxygen anchors on all written Markdown | **Pass** | Same script |
| Steps of 0.5 to 2 days | **Checked by hand, not by script** | Each step was written as one deliverable with its tests. Large items were split, for example routing into 10 steps and drawing views into 9. Step size cannot be machine-checked; rule 080 tells the implementer to split further with letter suffixes |
| C++ files, `.clang-format`, CMake, runtime network code | **Not applicable** | This worker wrote no code or build files |
| Build, tests and checks green | **Not run by this worker** | See `docs/planning-session/scaffold-report.md` |

The checker scripts are outside the repository (no code files allowed here), in this agent's private store:
`C:\Users\steve\AppData\Local\Cursor\AgentStores\cursor_agent_stores\bc-0822bba3-cf29-5805-a540-0bd8d5a97966\files\`:

- `check-plan.py` assembles the checklist from part files, runs the checks, and writes the coverage table and
  `traceability.csv`.
- `gen-stages.py` writes the stage plans.
- `check-docs.py` checks the ADRs and cross-references.

Later changes to these documents are expected to go through `/check-traceability`, as the stage gates say.

## Scaffold findings folded into the plan

The scaffold worker finished while this plan was being written. Its report changed these parts of the plan:

- **P-03 and Qt6Core.**
  - **Finding:** Qt's own `Qt6Core(d).dll` imports `ws2_32.dll` even without Qt Network.
  - **S0.02.3:** the network check reports listed third-party DLL imports as `info`. It fails on unlisted ones and on
    APP code.
  - **S0.08.7:** the offline smoke run is the runtime proof, and it also fails on socket attempts.
  - **Also recorded in:** ADR 0026, risk R-14, and the checklist's open-decisions table.
  - **Later decision (2026-11-11):** the owner reduced the severity. Third-party network imports are allowed, noted
    and summarised in `docs/network-imports.md`; there is no reviewed list. Network APIs in APP's own code still
    fail (ADR 0026, Accepted).
- **Existing script switches.** The scaffold's `scripts/test.ps1` already has `-Filter`, `-NoBuild`, `-NoChecks`,
  `-Coverage`, `-Bench`, `-Fuzz` and `-UpdateGolden -Reason`, and `scripts/check-selftest.ps1` exists. Steps that
  run tests or checks now refer to them:
  - **S0.02.1, S0.02.3, S0.02.7:** extend `check-selftest.ps1` instead of creating a new self-test script.
  - **S0.08.2, S0.08.4, S0.08.5, S0.08.6:** use and extend the existing switches.
  - **S0.13.14:** snapshot updates go through `-UpdateGolden -Reason`, and the Qt `minimal` platform plugin is used.
  - **S0.15.1 and every stage gate:** run these switches and `check-selftest.ps1`.
- **Doxygen is not installed.**
  - **S0.14.1:** states the prerequisite and the install command, and says to mark the step blocked rather than
    stop all work.
  - **S0.14.2:** no longer depends on S0.14.1.
  - **`scripts/ci-local.ps1` (S0.15.1):** reports "skipped" when Doxygen is missing.
  - **Every `Sx.99.3` gate:** runs `scripts/docs.ps1 -FailOnWarning`, which needs Doxygen.
- **Namespace `app::assembly`.** Used for module `asm` throughout: checklist header, S3.02.2, ADRs 0007, 0025 and 0032.
- **Qt in `render` and `platform`.** The scaffold's layering check still allows Qt in `render` and `platform`.
  ADR 0023 narrows this to `ui` in step S0.11.6.

## Open questions for the owner

Resolved by the owner on 2026-11-11, and removed from this list:

- **Reference scenario at S6 (C-01):** minimal Gerber, drill, IPC-2581, BOM and pick-and-place writers are built in
  S6 (ADR 0015, Accepted).
- **Meaning of "linked" in P-03 (C-12):** third-party network imports are allowed, noted and summarised; network APIs
  in APP's own code fail (ADR 0026, Accepted).
- **Repository:** https://github.com/smeekel/CADence. The product keeps the `APP` placeholder.
- **Doxygen:** installed (1.18.0).
- **Licences (C-25, ADRs 0005 and 0008):** OCCT is `LGPL-2.1-only WITH OCCT-exception-1.0` and ngspice is
  `BSD-3-Clause` with some parts under other licences. Both are acceptable. The spec section 4.3 OCCT wording is
  resolved by this decision; the spec is not edited. Both are recorded in `THIRD_PARTY_NOTICES.md`.

1. **Hosted CI (ADR 0033, open).**
   - **Question:** which service to use (GitHub Actions hosted Windows, a self-hosted runner on this machine, or
     local only). The repository lives at https://github.com/smeekel/CADence.
   - **Current plan:** local `scripts/test.ps1` and `scripts/ci-local.ps1`; hosted CI is step S1.08.1.
2. **Tool installs (Q3).**
   - **PowerShell 7:** optional, since the scripts stay 5.1-compatible.
   - **A Clang/GCC toolchain:** WSL 2 compilers or the VS "C++ Clang tools", so SP-1 can confirm the L-03
     portable feature set.
   - **MiKTeX:** optional, for the PDF manual.
3. **Proposed ADRs 0016 … 0033, except 0026.**
   - **Needed first:** 0029 (vcpkg root, S0.01.1), 0016 (S0.03.1), 0031 (logging, S0.03.10),
     0024, 0027, 0028, 0023, 0030 and 0017, all in S0.
   - **Later:** the layering extensions 0018 … 0022 are needed from S1 onwards.
4. **Spec Q-08 (MP4/FFmpeg) and Q-09 (writable global library folder).**
   - **Q-08 default:** the encoder is an optional, dynamically loaded seam (S3.08.3, S10.04.4).
   - **Q-09 default:** the global folder is writable, as the F-10 table says (ADR 0032).
5. **Product name.** The plan keeps the placeholder `APP`. "Cadence" is a third-party trademark, so the workspace
   name should not become the product name (ADR 0032, R-22).

## Proposed first five steps

Each of these depends only on S0.01.1 or on nothing, so they can start as soon as this plan is approved:

1. **S0.01.1: Spike SP-1a: verify the scaffold toolchain from a clean clone** (time box 1 day). Clean-clone build in
   all three configurations, vcpkg root and binary cache per ADR 0029, and timing recorded. Needs ADR 0029 Accepted.
2. **S0.01.2: Spike SP-1b: C++23/26 feature matrix on MSVC, GCC and Clang, ending in ADR 0034** (time box 1 day).
   Confirms `docs/cpp-features.md`. Benefits from a Clang/GCC toolchain (open question 2).
3. **S0.01.3: Add the Headless dependency check.** Covers P-04's Accept criterion.
4. **S0.01.4: Add the build-hygiene check for migration-ready projects.** Covers B-01, B-02, B-03 and B-07.
5. **S0.02.1: Tighten the SPDX check's provenance rule to the provenance table columns.** Covers CR-07's Accept
   criteria through `scripts/check-selftest.ps1` cases.
