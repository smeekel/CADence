# ADR 0008: ngspice as a shared library (D-07) {#adr-0008}

- **Status:** Accepted (spec decision D-07); licence verification pending
- **Date:** 2026-11-11

## Context

Spec D-07 chooses ngspice, used through its documented shared-library API, for circuit simulation (E-SIM-01, stage
S9). ngspice is mostly under a modified BSD licence, but parts of its history (for example XSPICE and numparam
components) carried other terms. The Phase 1 toolchain check found that the vcpkg `ngspice` port (version 41 in the
local clone) declares **CC-BY-SA-4.0** in its metadata, which is not a software licence APP can rely on and may refer
only to documentation. The exact licence of the build APP would ship is therefore unconfirmed.

## Decision

- ngspice is loaded at run time as a shared library through the `platform` dynamic-loading interface (S0.11.5), and
  used only inside `src/sim/spice/` (step S9.03.1).
- Before any ngspice code is linked or shipped, S9.03.1 verifies the licence of the exact version used (vcpkg port
  metadata if a port is used, plus the `COPYING` file of the source) and records it in `THIRD_PARTY_NOTICES.md` and,
  if built outside vcpkg, in `docs/provenance.md`. If any part is not GPL-3.0-compatible, that part is excluded from
  the build or a superseding ADR is written.
- A missing engine library produces a clear message; the rest of the application works without it.

## Consequences

- The licence check is a gate on S9.03.*; it is listed as an open item in `docs/RISKS.md`.
- Dynamic loading keeps S0 … S8 builds free of ngspice.
- The simulator's error output is parsed and mapped back to schematic objects (S9.03.6).

## Alternatives considered

- Xyce (GPL-3.0): larger and harder to embed as a library.
- Writing a SPICE engine: out of scope.

## Related requirements

D-07, E-SIM-01, L-02, CR-07, P-30; steps S0.11.5, S9.03.1 … S9.03.7.
