# ADR 0025: Board 3D model and 3D DRC in `link` through providers (C-10) {#adr-0025}

- **Status:** Proposed (Phase 1 conflict C-10)
- **Date:** 2026-11-11

## Context

E-3D-01 (S5) needs a B-rep board with placed component models, and E-RUL-02 (S5) needs 3D body clearance checks.
But `ecad/pcb` may not use `geom`, and nanometre to millimetre conversion is allowed only in `link` and `render`
(A-09). E-LIB-03 must produce a native 3D body for generated footprints without depending on `part`. STEP export of
products (E-MCAD-02) must reach board geometry from `io`.

## Decision

- The 3D board builder and the 3D DRC checks live in `link`. `link` work therefore starts in S5 (step S5.13.1), before
  the cross-domain link model of S6.
- They register through provider interfaces (A-11): `ecad/pcb` defines `IBoard3dProvider` and a DRC check factory
  entry for 3D checks; `asm` defines `app::assembly::IComponentGeometryProvider` (S3.02.2), which `link` implements
  for boards (S6.03.1).
- E-LIB-03's footprint generator emits a parametric part document (JSON) for the 3D body; `part` regenerates it
  (S4.04.7).
- Requires `link` → `ecad/lib` (ADR 0018), applied in S5.13.1.

## Consequences

- `ecad/pcb` stays free of `geom`; the integer board model is unchanged.
- Running 3D DRC from the PCB editor requires `link` to be loaded, which `ui` and `cli` always do.

## Alternatives considered

- Let `ecad/pcb` depend on `geom`: breaks the separation and A-09.
- Build the board 3D model in `render` only: not usable for DRC or STEP export.

## Related requirements

E-3D-01, E-RUL-02, E-LIB-03, E-MCAD-02, A-09, A-11; ADR 0018; steps S3.02.2, S4.04.7, S5.12.4, S5.13.1 … S5.13.4,
S6.03.1.
