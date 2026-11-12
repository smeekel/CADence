# ADR 0005: Open CASCADE Technology behind `geom` (D-04) {#adr-0005}

- **Status:** Accepted (spec decision D-04; licence confirmed by the owner, 2026-11-11)
- **Date:** 2026-11-11

## Context

Spec D-04 chooses OCCT as the B-rep kernel because it is the only mature open kernel with booleans, fillets, offsets,
sweeps, lofts and STEP/IGES. A-05 requires all kernel calls to go through `geom::` interfaces, with no OCCT type in
any header outside `geom/`.

Phase 1 conflict C-25: spec section 4.3 lists OCCT under "LGPL-2.1-or-later, LGPL-3.0". The actual licence is
**`LGPL-2.1-only WITH OCCT-exception-1.0`**. This was verified on 2026-11-11 against the vcpkg port `opencascade`
8.0.1 at the pinned baseline, which installs `LICENSE_LGPL_21.txt` and `OCCT_LGPL_EXCEPTION.txt`, and against the
upstream `README.md` and file headers at tag `V8_0_1`. LGPL-2.1-only is not compatible with GPL-3.0-or-later for
*copying code*, but spec section 4.3 permits *linking* an LGPL-2.1-only library.

## Decision

- **Resolved by owner decision (2026-11-11):** OCCT under LGPL is acceptable. The spec section 4.3 wording
  ("LGPL-2.1-or-later") is a known discrepancy, and it is closed by this decision. The spec is not edited (C-18).
- OCCT is used only inside `src/geom/`, linked dynamically as the vcpkg port builds it. `scripts/check-layering.ps1`
  fails on any OCCT include outside `geom` (A-01 Accept, step S0.09.4).
- No OCCT source is copied into APP files. Studying OCCT algorithms is allowed; adapted code is not.
- `THIRD_PARTY_NOTICES.md` records the licence, the upstream URL and the use, together with the LGPL obligations:
  dynamic linking, a replaceable DLL and the source offer. It also carries the prominent notice that the OCCT
  exception requires.
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
