# ADR 0015: Reference scenario fabrication outputs in S6 (C-01) {#adr-0015}

- **Status:** Accepted (owner decision, 2026-11-11; Phase 1 question Q1)
- **Date:** 2026-11-11

## Context

Spec T-14 requires the reference scenario (PCB in an enclosure) to pass end to end at the S6 exit. Its step 8 needs
Gerber, drill and IPC-2581 outputs (E-OUT-01), which the spec places in S7. Step 8 also lists the BOM and
pick-and-place for the active variant, and the "no USB" configuration (X-08) needs a variant-aware BOM. E-PRJ-03's
Accept line (three variants produce the expected BOMs and pick-and-place files) cannot pass without them.

## Decision

The owner decided that minimal Gerber, drill, IPC-2581, BOM and pick-and-place writers are built in S6.

- Minimal writers are built in S6 work package S6.09 so T-14 runs literally at the S6 gate:
  - S6.09.1 test-side Gerber X2 and Excellon readers (T-07 basis, and the future CAM viewer seam);
  - S6.09.2 Gerber X2 for copper, mask, paste, silkscreen and outline layers, with job file;
  - S6.09.3 Excellon drill with plated/non-plated split;
  - S6.09.4 IPC-2581 for the board stack and drill spans (E-PCB-08 Accept);
  - S6.09.5 minimal variant-aware BOM (CSV) and pick-and-place (CSV) (E-PRJ-03 Accept).
- Full E-OUT-01 options (every layer type, slots and backdrill, Gerber X2 drill, IPC-D-356, output jobs) and the full
  E-BOM-01 parts list stay in S7 (S7.04, S7.05).

## Consequences

- S6 grows by five steps; S7.04 and S7.05 extend these writers instead of creating them.
- The T-07 conformance suite starts in S6 with the test-side readers.
- T-14 step 8 and E-PRJ-03's Accept are checked at the S6 gate.

## Alternatives considered

- Keep all outputs in S7 and let T-14 pass only partially at S6: contradicts the spec's S6 exit criterion.
- Move T-14 to the S7 gate: changes the spec's stage plan.
- Build only Gerber, drill and IPC-2581 in S6 and move the BOM and pick-and-place (S6.09.5) to S7: T-14 step 8 at the
  S6 gate would check only fabrication files, and E-PRJ-03's Accept would move to S7. Not chosen by the owner.

## Related requirements

T-14, E-OUT-01, E-PCB-08, E-PRJ-03, E-BOM-01, X-08; steps S6.09.1 … S6.09.5, S6.11.3, S7.04.*, S7.05.1.
