# ADR 0001: Record architecture decisions {#adr-0001}

- **Status:** Accepted
- **Date:** 2026-11-11

## Context

APP is built step by step by an AI implementer under human review. Decisions made in one step must be visible to
every later step, and the spec (`docs/spec/cad-functional-requirements.md`) is authoritative. Rule 080 asks for an ADR
for every spike outcome, every new dependency, every deviation from the spec and every resolved ambiguity. Rule 000
says assumptions are recorded as ADRs, never as invented requirements.

## Decision

- Decisions are recorded as Markdown files `docs/adr/NNNN-kebab-title.md` using `0000-template.md`.
- Sections: Status, Context, Decision, Consequences, Alternatives considered, Related requirements. Each file has a
  Doxygen anchor `{#adr-NNNN}` so it appears in the generated docs.
- Status is Proposed, Accepted or Superseded by NNNN. Only the owner moves an ADR to Accepted. ADRs taken directly from
  spec decisions D-01 … D-13 are Accepted because the spec is authoritative.
- ADRs are never deleted. A changed decision gets a new ADR that supersedes the old one.
- Numbers are permanent. 0034 … 0041 are reserved for the time-boxed spikes SP-1 … SP-8 in
  `docs/IMPLEMENTATION_CHECKLIST.md`. Other new ADRs take the next free number from 0042 when written.

## Index

| ADR | Title | Status |
|---|---|---|
| 0000 | Template | n/a |
| 0001 | Record architecture decisions | Accepted |
| 0002 | C++26 for core and UI code (D-01) | Accepted |
| 0003 | Visual Studio solution with vcpkg manifest mode (D-02) | Accepted |
| 0004 | Qt 6 Widgets for the UI (D-03) | Accepted |
| 0005 | Open CASCADE Technology behind `geom` (D-04) | Accepted |
| 0006 | Own 2D sketch constraint solver (D-05) | Accepted |
| 0007 | Own mate solver; Project Chrono for dynamics (D-06) | Accepted |
| 0008 | ngspice as a shared library (D-07) | Accepted (licence check pending) |
| 0009 | Clipper2 for PCB polygon operations (D-08) | Accepted |
| 0010 | nlohmann/json `ordered_json` for JSON and BSON (D-09) | Accepted |
| 0011 | lib3mf for 3MF (D-10) | Accepted |
| 0012 | GoogleTest and GoogleMock (D-11) | Accepted |
| 0013 | OpenGL 4.5 behind `render::Device` (D-12) | Accepted |
| 0014 | Embree and an in-house path tracer (D-13) | Accepted |
| 0015 | Reference scenario fabrication outputs in S6 (C-01) | Accepted |
| 0016 | Layering: `core` may use nlohmann/json | Proposed |
| 0017 | Layering: `cli` references `platform` and `render` | Proposed |
| 0018 | Layering: `link` references `ecad/lib`, `ecad/sch`, `drw` | Proposed |
| 0019 | Layering: `drw` references `sketch` | Proposed |
| 0020 | Layering: `io` references `sketch` and `drw` | Proposed |
| 0021 | Layering: `sim` references `part` and `drw` | Proposed |
| 0022 | Eigen allowed in `core`, `sketch`, `asm`, `sim` (C-08) | Proposed |
| 0023 | Qt confined to `ui`; Win32-only `platform`; Qt-free `render` (C-09) | Proposed |
| 0024 | OS folders injected into `doc` through `core` interfaces (C-11) | Proposed |
| 0025 | Board 3D model and 3D DRC in `link` through providers (C-10) | Proposed |
| 0026 | Network-API check scope and third-party network imports (C-12) | Accepted |
| 0027 | Preservation of unknown fields (C-20) | Proposed |
| 0028 | UUID encoding in BSON decided by schema (C-21) | Proposed |
| 0029 | vcpkg root selection (C-24) | Proposed |
| 0030 | Docking with Qt Advanced Docking System (C-26) | Proposed |
| 0031 | Logging with spdlog (C-26) | Proposed |
| 0032 | Planning assumptions bundle | Proposed |
| 0033 | Continuous integration: local scripts first, hosted CI later | Proposed (open) |
| 0034 | Reserved: SP-1 toolchain and C++ feature matrix | to be written in S0.01.2 |
| 0035 | Reserved: SP-2 topological naming (`TopoRef`) | to be written in S0.09.6 |
| 0036 | Reserved: SP-3 sketch solver | to be written in S1.01.3 |
| 0037 | Reserved: SP-4 mate solver | to be written in S3.01.3 |
| 0038 | Reserved: SP-5 push-and-shove router | to be written in S5.01.3 |
| 0039 | Reserved: SP-6 polygon pours | to be written in S5.01.5 |
| 0040 | Reserved: SP-7 cross-domain links | to be written in S6.01.2 |
| 0041 | Reserved: SP-8 large-assembly strategy | to be written in S10.01.2 |

## Consequences

- Every step that changes architecture, adds a dependency or deviates from the spec names its ADR in Deliverables.
- `/check-traceability` and the stage gates check that Proposed ADRs blocking a step were Accepted before the step.

## Alternatives considered

- Decisions only in commit messages or chat: not durable and not reviewable.
- A single decisions log file: harder to supersede and link individual decisions.

## Related requirements

Rule 000, rule 080; spec section 16; all ADRs above.
