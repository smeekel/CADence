# ADR 0023: Qt confined to `ui`; Win32-only `platform`; Qt-free `render` (C-09) {#adr-0023}

- **Status:** Proposed (Phase 1 conflict C-09)
- **Date:** 2026-11-11

## Context

Rule 010 allows Qt in `ui`, `render` and `platform`, and the scaffold's `check-layering.ps1` encodes that (its `Qt`
entry lists all three). But A-01 and P-04 require the Headless configuration to build every domain module and the CLI
with no Qt Widgets or Windows UI dependency, `render` must build in Headless for snapshot tests and CLI image export,
and the architecture notepad puts file dialogs in `platform`, which `cli` uses (ADR 0017).

## Decision

- Only `ui` uses Qt.
- `platform` is Win32-only: known folders, DPI queries, crash dumps, dynamic library loading, system information and
  an off-screen OpenGL context. File dialogs live in `ui`.
- `render` has no Qt. `ui` creates the window and hands `render` a current OpenGL context; headless rendering uses
  the off-screen context from `platform` (ADR 0013).
- Applied in step **S0.11.6** (off-screen GL context): the `Qt` entry in `scripts/check-layering.ps1` is narrowed to
  `ui`, rule 010 and rule 030 are updated, and the Headless check (S0.01.3) confirms `render` and `platform` link no
  Qt library.

## Consequences

- Headless builds never compile or link Qt (the scaffold already installs only gtest for Headless).
- `render` needs a small context abstraction instead of `QOpenGLContext`.

## Alternatives considered

- Keep Qt Core/Gui in `render` and `platform`: Headless would need Qt, breaking B-04 and slowing CI.

## Related requirements

A-01, P-04, B-04, D-03, D-12; ADRs 0004, 0013, 0017; steps S0.01.3, S0.11.1 … S0.11.6, S0.12.1.
