# ADR 0032: Planning assumptions bundle {#adr-0032}

- **Status:** Proposed
- **Date:** 2026-11-11

## Context

Rule 080 asks for an ADR for every resolved ambiguity. Phase 1 (`docs/planning-session/phase1-understanding.md`,
section 3) found 28 conflicts. Those with architectural weight have their own ADRs (0015 … 0031). This ADR records the
remaining assumptions in one place so each can be accepted or overturned individually.

## Decision

Each item below is in effect until the owner decides otherwise.

| ID | Assumption | Applied in |
|---|---|---|
| C-02 | T-14 step 2 uses two separate parts (base and lid) at S6. The split-body variant is added to T-14 in S8. | S6.11.1, S8.01.4 |
| C-03 | X-07 (combined product BOM) is delivered in S7, as its heading and spec section 15 say; S6 delivers X-01 … X-06 and X-08 … X-10. | S7.05.2 |
| C-04 | S0 delivers B-01 … B-07, L-01 … L-05, CR-01 … CR-07 (checks and docs) and F-01 … F-10. F-10 in S0 means scope resolution with an empty seed; self-contained project copies and the three-way seed-upgrade review come in S4. | S0.07.*, S4.03.4, S4.03.5 |
| C-05 | S0 builds the benchmark harness and meets P-10 and basic P-17. Other targets belong to their feature stage: P-13 S1, P-12 S2, P-15/P-16 S5, P-17 (50 MB) S6, P-11/P-14 S10 with early measurement in S3 (SP-8a). P-01 … P-07, P-20 … P-23, P-30 are S0. | S0.08.4, S3.02.4, every `Sx.99.2` |
| C-06 | Requirements that span stages are split into steps per stage; `docs/traceability.csv` lists all stages (for example `S2;S7`). U-12/U-13 conventions (accessible names, contrast tokens, `tr()`) start in S0. | traceability |
| C-13 | Module `asm`: folder `src/asm/`, project `Asm`, namespace `app::assembly` (`asm` is a C++ keyword). | all `asm` steps |
| C-14 | Test projects are `tests/<module>/<Module>.Tests.vcxproj` (subfolder form). | S0.08.* |
| C-15 | Every project, tests included, builds into `out/<Configuration>/<Project>/`; `scripts/test.ps1` finds `*.Tests.exe` recursively. | scaffold |
| C-16 | Benchmark project in `tests/bench/`; history in top-level `bench/history.csv`. | S0.08.4 |
| C-17 | `docs/traceability.csv` uses the columns `id,title,source,stage,status,steps,tests,notes`. | traceability |
| C-18 | The spec is not edited by the implementer. Questions are recorded in ADRs and reports and proposed to the owner for spec section 17. | all |
| C-19 | Library kinds `layerStack` and `environment` are added (`libraries/layer-stacks/`, `libraries/environments/`). | S0.06.10, S5.03.4, S10.04.2 |
| C-22 | No installer is planned. The application seeds `%ProgramData%\APP\libraries\` on first run and runs the seed-upgrade review itself. | S0.07.2, S4.03.5 |
| C-23 | Scripts target PowerShell 7 but stay runnable on Windows PowerShell 5.1 (no `&&`, ternary or pwsh-only parameters; ASCII-only files), so P-02 holds with only Visual Studio and Git installed. | all scripts |
| C-26 (rest) | Notepad "(proposed)" items accepted as Proposed: rule priority "lower number = higher priority", the rule query language, schema IDs `urn:app:schema:<type>:<major>`, UI design tokens and the naming scheme. `TopoRef` is decided by SP-2 (ADR 0035). Docking (ADR 0030) and logging (ADR 0031) have their own ADRs. | S0.06.1, S0.13.1, S5.05.1 |
| C-27 | Parameters carry an ObjectId (A-04: every persistent object); the spec section 6.2 example is illustrative. | S1.06.1 |
| C-28 | ID gaps (E-PRJ-02, E-PCB-06, M-PRT-08, M-SIM-01 … 03) are not invented. | traceability |
| Q-09 | The global library folder is writable by default, as the F-10 table says. | S0.07.2 |
| Q3 | Doxygen and PowerShell 7 are **not installed**. `scripts/docs.ps1` fails cleanly until Doxygen is installed; steps that need it (S0.14.1, every `Sx.99.3`) say so. | S0.14.1, S0.15.1 |
| Naming | The product keeps the placeholder name `APP`; `CADence` is only the workspace folder name (a third-party trademark must not become the product name). | all |

## Consequences

- Each row can be overturned by a superseding ADR without reopening the others.
- Rows marked in the checklist header's "Open decisions" table are the ones most likely to change.

## Alternatives considered

- One ADR per row: many near-empty documents for editorial choices.
- Leaving editorial items unrecorded: violates rule 080.

## Related requirements

T-14, X-07, B-*, L-*, CR-*, F-01 … F-10, P-*, U-12, U-13, A-04; ADRs 0015 … 0031, 0035.
