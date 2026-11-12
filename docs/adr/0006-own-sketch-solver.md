# ADR 0006: Own 2D sketch constraint solver (D-05) {#adr-0006}

- **Status:** Accepted (spec decision D-05)
- **Date:** 2026-11-11

## Context

Spec D-05 calls for an own sketch solver (Newton / Levenberg–Marquardt with degree-of-freedom analysis, Eigen for
linear algebra) because the solver is a core differentiator that must be fast (P-13: under 16 ms per drag update for
500 entities and 800 constraints) and debuggable. CR-04 prefers fresh implementation; SolveSpace's solver or FreeCAD's
PlaneGCS may serve as references or fallbacks under CR-07.

## Decision

- The solver lives in `src/sketch/solver/` and is implemented fresh from published algorithms.
- Its detailed design (decomposition, damping, redundancy and conflict detection, drag strategy) is decided by spike
  SP-3 in ADR 0036 (steps S1.01.2 and S1.01.3) before solver work starts.
- Eigen is allowed in `sketch` internals (ADR 0022); public APIs use `core` math types.
- Any reuse of third-party solver code needs a CR-07 provenance entry and a new ADR.

## Consequences

- Full control over diagnostics (DOF colouring, conflict reports) and performance tuning.
- Solver convergence is a high risk (`docs/RISKS.md`); the T-10 solver suite (S1.03.10) and the P-13 benchmark
  (S1.03.11) guard it.

## Alternatives considered

- Linking SolveSpace's solver library (GPL-3.0): possible fallback under CR-07; less control over diagnostics.
- PlaneGCS from FreeCAD (LGPL-2.1-or-later): tied to FreeCAD types; fallback only.

## Related requirements

D-05, M-SKT-02, P-13, CR-04, CR-07; ADRs 0022, 0036; steps S1.01.1 … S1.03.12.
