# ADR 0005: Open CASCADE Technology behind `geom` (D-04) {#adr-0005}

- **Status:** Accepted (spec decision D-04)
- **Date:** 2026-11-11

## Context

Spec D-04 chooses OCCT as the B-rep kernel because it is the only mature open kernel with booleans, fillets, offsets,
sweeps, lofts and STEP/IGES. A-05 requires all kernel calls to go through `geom::` interfaces, with no OCCT type in
any header outside `geom/`.

Phase 1 conflict C-25: the vcpkg port metadata gives OCCT's licence as **LGPL-2.1-only** (with the OCCT exception),
while spec section 4.3 lists it under "LGPL-2.1-or-later, LGPL-3.0". LGPL-2.1-only is not compatible with
GPL-3.0-or-later for *copying code*, but spec section 4.3 permits *dynamic linking* of LGPL-2.1-only libraries.

## Decision

- OCCT is used only inside `src/geom/`, linked dynamically as the vcpkg port builds it. `scripts/check-layering.ps1`
  fails on any OCCT include outside `geom` (A-01 Accept, step S0.09.4).
- No OCCT source is copied into APP files. Studying OCCT algorithms is allowed; adapted code is not.
- The licence is recorded as LGPL-2.1-only with exception in `THIRD_PARTY_NOTICES.md`, and section 4.3 of the spec
  should be corrected by the owner (proposed, not edited by the implementer).
- Kernel failures (fillet, shell, boolean) map to feature error states (P-21) with messages, never crashes.

## Consequences

- The kernel stays replaceable and testable through `geom` interfaces and test oracles (S0.08.3, S0.09.2).
- Fillet and shell robustness is a known risk (`docs/RISKS.md` R-02); golden tests cover failure messages.
- Topological naming is built on top of OCCT history (SP-2, ADR 0035).

## Alternatives considered

- Commercial kernels (Parasolid, ACIS): not open, licence servers violate P-03.
- Writing a B-rep kernel: far beyond the scope of S0 … S10.

## Related requirements

D-04, A-05, A-06, P-21, CR-04, P-30; ADR 0035; steps S0.09.1 … S0.09.7, S2.03.*, S2.04.*.
