# ADR 0022: Eigen allowed in `core`, `sketch`, `asm` and `sim` (C-08) {#adr-0022}

- **Status:** Proposed (Phase 1 conflict C-08)
- **Date:** 2026-11-11

## Context

Rule 030 lists Eigen (MPL-2.0) only for `core`. Spec D-05 requires Eigen in the sketch solver, the mate solver needs
the same linear algebra (D-06), and Chrono integration in `sim` exchanges Eigen-compatible matrices.

## Decision

- Eigen may be used in the internal code of `core`, `sketch`, `asm` and `sim`. Public APIs prefer `core` math types
  (`Vec3`, `Mat4`, `Transform`) so callers do not depend on Eigen.
- Applied per module when first needed, each time adding the module to an Eigen entry in the `$ThirdParty` table of
  `scripts/check-layering.ps1` and the rule 030 table: `sketch` in **S1.01.1**, `asm` in **S3.01.1**, `sim` in
  **S9.01.1**. `core` uses it from S0.03.4 if needed.

## Consequences

- Eigen includes elsewhere (for example `ecad/pcb`) fail the layering check (tested in S1.01.1).
- MPL-2.0 is file-level copyleft and compatible with GPL-3.0-or-later; Eigen headers are not modified.

## Alternatives considered

- Own small dense solver: slower and riskier for sparse Jacobians.
- Eigen in every module: loses control over where heavy templates compile.

## Related requirements

D-05, D-06, A-01; ADRs 0006, 0007; steps S1.01.1, S3.01.1, S9.01.1.
