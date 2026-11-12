# ADR 0018: Layering: `link` references `ecad/lib`, `ecad/sch` and `drw` {#adr-0018}

- **Status:** Proposed (Phase 1 conflict C-07)
- **Date:** 2026-11-11

## Context

Rule 030 lets `link` use `core`, `doc`, `geom`, `sketch`, `part`, `asm` and `ecad/pcb`. Several cross-domain
requirements need more:

- Board 3D models need component 3D bodies and mating envelopes from the library (E-3D-01, X-05) → `ecad/lib`.
- Multi-board connectivity across connectors (E-MB-01) needs the schematic netlist → `ecad/sch`.
- The combined product BOM (X-07) implements `drw::IBomProvider`; the architecture notepad already names it
  "implemented in `link/`" → `drw`.

## Decision

- `link` may reference `ecad/lib`, `ecad/sch` and `drw`. The graph stays acyclic: none of these modules references
  `link`.
- Applied in three steps, each updating rule 030, `$Allowed['link']` in `scripts/check-layering.ps1` and
  `Link.vcxproj` together after this ADR is Accepted:
  - `link` → `ecad/lib` in **S5.13.1** (board 3D builder, ADR 0025);
  - `link` → `ecad/sch` in **S6.08.1** (multi-board connectivity);
  - `link` → `drw` in **S7.05.2** (product BOM provider).

## Consequences

- `link` is the one module that sees both domains, as intended by the integration notepad.
- Builds of `link` take longer; it is near the top of the graph already.

## Alternatives considered

- Provider interfaces in `ecad/pcb` for every library or schematic query: many thin interfaces with a single
  implementation.
- Putting the product BOM in `drw`: `drw` would need `ecad/*`, a larger change.

## Related requirements

A-01, E-3D-01, X-05, E-MB-01, X-07, M-DRW-03; ADR 0025; steps S5.13.1, S6.08.1, S7.05.2.
