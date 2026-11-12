# ADR 0021: Layering: `sim` references `part` and `drw` {#adr-0021}

- **Status:** Proposed (Phase 1 conflict C-07)
- **Date:** 2026-11-11

## Context

Rule 030 lets `sim` use `core`, `doc`, `geom`, `asm` and `ecad/sch`. Motion analysis (M-SIM-04) needs part materials
and mass properties (M-PRT-09) from `part`, and tolerance stack-up (M-SIM-05) reads tolerances from drawing
dimensions (M-DRW-02) in `drw`.

## Decision

- `sim` may reference `part` and `drw`. No cycle: neither references `sim`.
- Applied in two steps, each updating rule 030, `$Allowed['sim']` in `scripts/check-layering.ps1` and `Sim.vcxproj`
  together after this ADR is Accepted: `sim` → `part` in **S9.01.1**; `sim` → `drw` in **S9.02.1**.

## Consequences

- Stack-ups and dynamics use the same data as the model, with no copies.

## Alternatives considered

- Reading mass properties through `asm` only: `asm` would re-export `part` types.
- Copying tolerances into an assembly-level table: duplicated data that can drift from drawings.

## Related requirements

A-01, M-SIM-04, M-SIM-05, M-DRW-02, M-PRT-09; steps S9.01.1, S9.02.1.
