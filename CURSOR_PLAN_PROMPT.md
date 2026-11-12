# Planning Prompt: Scaffold and Implementation Plan for APP

> **How to use:** copy this kit into the empty repository root, open the folder in Cursor, open an Agent chat and run `/plan-implementation` (or paste this file and @-mention it). This is a **planning and scaffolding session**. Do not implement product features in this session.

---

## 1. Your role

You are the lead engineer and planner for **APP** (working name), a clean-room, offline, Windows-first desktop CAD application. It combines parametric 3D mechanical design (sketches, parts, assemblies, drawings, sheet metal, weldments, motion simulation) with electronics design (libraries, schematic, SPICE, PCB layout, manufacturing outputs) in **one shared parametric model**. A PCB and its enclosure are designed together and stay fitted when either changes.

Your job in this session:

1. Read the specification and notepads listed in section 2.
2. Produce the **folder structure** document.
3. Produce the **repository scaffold**: solution, projects, shared build settings, scripts, docs skeleton. It must compile.
4. Produce the **implementation checklist**: a complete, ordered, Markdown checklist of implementation steps covering every requirement.
5. Produce the supporting planning files (traceability, stage plans, ADRs, risk register).
6. Verify, report and **stop for human review**.

---

## 2. Inputs (read all of these before writing anything)

| Input | Purpose |
|---|---|
| `docs/spec/cad-functional-requirements.md` | **Authoritative** functional requirements (v0.2). Wins on any conflict. |
| `docs/notepads/00_Index.md` | Map of the notepads and when to use each |
| `docs/notepads/Architecture_and_Build.md` | Modules, layering, Visual Studio solution layout, dependencies |
| `docs/notepads/Data_Model_and_File_Format.md` | Document model, JSON/BSON format, magic UUID, schemas |
| `docs/notepads/Command_Bus_and_Module_Contracts.md` | Command bus, error model, module interfaces |
| `docs/notepads/Mechanical_Modeling_Rules.md` | Sketch, part, assembly, drawing, motion rules |
| `docs/notepads/Electronics_Design_Rules.md` | Library, schematic, PCB, rules/DRC, outputs |
| `docs/notepads/MCAD_ECAD_Integration_Rules.md` | Cross-domain link model and the reference scenario |
| `docs/notepads/Libraries_and_Seed_Data.md` | Global/user/project library scopes and seed data |
| `docs/notepads/UI_UX_Guidelines.md` | Interface and visual design |
| `docs/notepads/Testing_Strategy.md` | Test harness, golden data, acceptance tests |
| `docs/notepads/Licensing_and_Provenance.md` | GPL, SPDX headers, provenance, clean-room rules |
| `docs/notepads/Code_Style_and_Documentation.md` | Formatting and Doxygen documentation |
| `docs/notepads/Stage_Roadmap.md` | Stages S0–S10, exit criteria, required spikes |
| `.cursor/rules/*.mdc` | Rules you must follow while writing any file |

If the spec and a notepad disagree, follow the spec and list the conflict in your final report.

---

## 3. Hard constraints for this session

- **No feature implementation.** Only code needed for the scaffold to compile, link and run trivial tests. Placeholder classes are allowed, but they must not pretend to work: no fake results, no `return true; // TODO` in logic that tests rely on.
- **Build system:** Visual Studio 2026 solution `APP.slnx` + one `.vcxproj` per module + shared `.props` files. **No CMake.** Platform x64. Configurations `Debug`, `Release`, `Headless`.
- **Language:** C++26 via `/std:c++latest`, limited to features MSVC already supports and that GCC/Clang also support (spec 4.5).
- **Dependencies:** vcpkg manifest mode through MSBuild integration. Pin a current vcpkg baseline commit; look up the current one, do not invent it.
- **Licence:** GPL-3.0-or-later. Every source file starts with the SPDX header (rule `070`).
- **Style:** every C++ file follows rule `010` and the provided `.clang-format`; every public declaration has Doxygen comments (rule `020`).
- **Offline product:** no runtime network code anywhere in `src/`.
- **Clean room:** spec section 2. Write fresh code.

---

## 4. Deliverables (exact paths)

### 4.1 Folder structure document — `docs/FOLDER_STRUCTURE.md`

- A full tree of the repository as it will look by Stage 10, with a one-line purpose per folder.
- Mark which folders this scaffold creates now and which appear in later stages.
- Must agree with spec section 5.1 and `Architecture_and_Build.md`.

### 4.2 Repository scaffold

Create, at minimum:

```
APP.slnx
Directory.Build.props
vcpkg.json
vcpkg-configuration.json
LICENSE                       (official GPL-3.0 text, unmodified)
README.md                     (what APP is, how to build, how to test, how to build docs)
THIRD_PARTY_NOTICES.md
.clang-format  .editorconfig  Doxyfile   (already provided: keep them)
build/props/
  Common.props                (output/intermediate dirs, x64, character set, /utf-8, /MP)
  Cpp.props                   (/std:c++latest, /permissive-, /Zc:__cplusplus, /W4 /WX, /external:W0)
  Vcpkg.props                 (manifest mode, triplet x64-windows)
  Qt.props                    (moc/uic/rcc custom build rules; UI projects only)
  Headless.props              (APP_HEADLESS define, excludes UI)
  Tests.props                 (GoogleTest include/lib, test output dir)
src/<module>/                 one folder + one .vcxproj per module in spec 5.1:
                              core doc geom sketch part asm drw ecad/lib ecad/sch ecad/pcb ecad/out
                              link sim io render ui cli platform
                              each containing: <Module>.vcxproj, Module.h, Module.cpp
                              (a documented `moduleInfo()` returning name + version),
                              and a README.md stating the module's purpose and allowed dependencies
tests/<module>/               <Module>.Tests.vcxproj + one passing GoogleTest per module
tests/data/scripts/           (empty, with README explaining golden-data scripts)
libraries/                    global seed library folders per kind (F-10), each with README.md
scripts/
  build.ps1                   (msbuild APP.slnx /m with -Configuration parameter)
  test.ps1                    (builds Headless, runs all *.Tests.exe, writes JUnit XML)
  docs.ps1                    (creates out/docs, runs doxygen; -Pdf runs make.bat in out/docs/latex)
  check-layering.ps1          (A-01: parses .vcxproj ProjectReferences + #includes)
  check-spdx.ps1              (CR-07)
  check-network-apis.ps1      (P-03)
docs/
  mainpage.md                 (Doxygen main page)
  images/                     (diagrams for the docs; keep with .gitkeep)
  FOLDER_STRUCTURE.md
  IMPLEMENTATION_CHECKLIST.md
  traceability.csv
  RISKS.md
  cpp-features.md             (initial allow-list of C++23/26 features; see spec L-03)
  provenance.md               (empty table with header)
  data-sources.md             (empty table with header)
  format/README.md            (skeleton of the format spec, incl. the F-09 magic section)
  commands/README.md          (command naming and documentation convention)
  adr/0000-template.md
  adr/0001-record-architecture-decisions.md
  adr/0002 … one ADR per spec decision D-01 … D-13 (status: Accepted, from the spec)
  stages/S0.md … S10.md       (stage plans; see 4.4)
```

Scaffold acceptance (check each one yourself):

- `scripts/build.ps1 -Configuration Debug` and `-Configuration Headless` succeed with zero warnings.
- `scripts/test.ps1` runs every test project and all tests pass.
- `scripts/check-layering.ps1`, `check-spdx.ps1` and `check-network-apis.ps1` pass on the scaffold, and each fails on a deliberately bad input (show this in the report).
- The `ui` project shows an empty main window titled "APP". The `cli` project prints its version and exits 0.
- `scripts/docs.ps1` generates HTML docs with no Doxygen warnings for scaffold code.
- Project references follow the layering in `Architecture_and_Build.md`. No module references `ui`.

If you cannot run a build in this environment, say so in the report and list the exact commands for the user to run.

### 4.3 Implementation checklist — `docs/IMPLEMENTATION_CHECKLIST.md`

This is the main planning output. Requirements:

1. **Structure:** `## Stage` → `### Work package` → checklist steps. Stages are S0–S10 from `Stage_Roadmap.md`, in order. The section 9 table in the spec and `Stage_Roadmap.md` give the content of each stage.
2. **Step format** (use exactly this shape):

   ```markdown
   - [ ] **S2.03.4: Implement extrude end conditions "up to surface" and "offset from surface"**
     - Requirements: M-PRT-01
     - Modules: part, geom
     - Depends on: S2.03.1, S2.02.2
     - Deliverables: `src/part/features/ExtrudeFeature.{h,cpp}`, command `part.feature.extrude.create` params extended
     - Tests: T-04 golden parts `extrude_up_to_surface_*`; T-02 command script `extrude_end_conditions.cmd.json`
     - Done when: all listed tests pass; golden mass properties within 1e-6; docs updated
   ```

3. **Step size:** each step is one focused change, about 0.5–2 days of work, ending with a green build. Split anything larger.
4. **Coverage:** every requirement ID in the spec (P, A, B, L, F, U, T, CR, M, E, X) appears in at least one step. Each `Accept:` criterion in the spec maps to at least one test named in a step.
5. **Order:** respect dependencies. Command bus, document model, file format and test harness come first (S0). The UI for a feature comes after its command and headless test.
6. **Spikes:** early, time-boxed spike steps for the high-risk areas listed in `Stage_Roadmap.md` (topological naming, sketch solver, mate solver, push-and-shove router, cross-domain links, large-assembly performance). Each spike ends in an ADR, decided before dependent work starts.
7. **Stage gates:** each stage ends with a work package `Sx.99 Stage gate` containing: run the full test suite, run benchmarks for that stage's P-targets, update traceability, update docs, and **"Human review checkpoint: stop and wait for approval"**.
8. **Coverage table:** the last section is a table `Requirement → Step IDs` generated from the steps, followed by a list of any requirement not covered (this list must be empty).
9. **Future:** a final non-checkbox section listing spec section 14 items, with the seams each relies on and the step IDs where each seam is built.

### 4.4 Stage plans — `docs/stages/S0.md` … `S10.md`

Each stage plan has: goal (1–2 sentences), the requirements it delivers, its work packages (titles only, matching the checklist), entry criteria, exit criteria (copied from the spec), key risks, and the golden test data it introduces.

### 4.5 Traceability — `docs/traceability.csv`

Columns: `id,title,source,stage,status,steps,tests,notes`. One row per requirement ID in the spec. Status `not-started` for all. `steps` lists checklist step IDs separated by `;`.

### 4.6 Risk register — `docs/RISKS.md`

Table: ID, risk, likelihood, impact, mitigation, owning step or spike. Include at least: topological naming robustness, OCCT fillet/shell failures, sketch solver convergence, mate solver performance, push-and-shove router complexity, polygon pour robustness, large-assembly memory, C++26 compiler support gaps, Qt + vcpkg build times, ngspice and Chrono integration, file-format evolution.

---

## 5. Process

1. Read every input in section 2. Summarise your understanding in 10 bullet points or fewer at the start of your reply.
2. List any ambiguities or conflicts you found, and for each, the assumption you will use. Record the assumptions in `docs/adr/` where they are architectural.
3. Write `docs/FOLDER_STRUCTURE.md`.
4. Create the scaffold, then run build, tests and checks. Fix until green.
5. Write `docs/IMPLEMENTATION_CHECKLIST.md`, stage plans, `traceability.csv` and `RISKS.md`.
6. Run the self-check (section 6).
7. Reply with a final report: what was created, build/test results, open questions, assumptions, coverage statistics (requirements, steps, steps per stage), and the proposed first five steps to execute next. Then **stop**.

---

## 6. Self-check before finishing

- [ ] Every requirement ID in the spec appears in `traceability.csv` and in at least one checklist step.
- [ ] Every checklist step has Requirements, Modules, Depends on, Deliverables, Tests and Done-when lines.
- [ ] No step depends on a later step.
- [ ] Every stage ends with a `Sx.99 Stage gate` including the human review checkpoint.
- [ ] All C++ files: SPDX header, Doxygen file comment, formatted with `.clang-format`, style rule `010` honoured (Allman braces, `if( a == b )` spacing, aligned `=` and declarations, blank lines between blocks).
- [ ] No CMake files, no runtime network code, no proprietary or third-party code copied without a provenance entry.
- [ ] Build, tests and checks are green, or the report states exactly why they could not be run.
