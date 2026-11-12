# ADR 0008: ngspice as a shared library (D-07) {#adr-0008}

- **Status:** Accepted (spec decision D-07; licence confirmed by the owner, 2026-11-11)
- **Date:** 2026-11-11

## Context

Spec D-07 chooses ngspice, used through its documented shared-library API, for circuit simulation (E-SIM-01, stage
S9). The Phase 1 toolchain check found that the vcpkg `ngspice` port declares **CC-BY-SA-4.0** in its metadata.

The licence was verified on 2026-11-11 against the port `ngspice` 41 at the pinned baseline and the upstream
`ngspice-41.tar.gz`, whose SHA512 matches the port. Its `COPYING` is identical to the file at upstream tag
`ngspice-41`. ngspice's code is under the "Modified BSD" licence (`BSD-3-Clause`). CC-BY-SA-4.0 covers only the
licensing document itself and the ngspice manual, so the vcpkg metadata is wrong for the code. Some parts are under
other licences. These are compiled into `ngspice.dll`:

- `numparam`: LGPL-2.0-or-later;
- `osdi` and the bundled `cppduals` headers: MPL-2.0;
- `maths/sparse`: an MIT-style licence;
- XSPICE: public domain.

The `table` code model is GPL-2.0-or-later. The `tclspice`, ADMS and `ndev` parts are not built by the port.
`THIRD_PARTY_NOTICES.md` lists them all.

## Decision

- **Resolved by owner decision (2026-11-11):** ngspice is BSD-licensed and acceptable. Every part listed above can be
  combined with GPL-3.0-or-later under spec section 4.3, so no part is excluded.
- ngspice is loaded at run time as a shared library through the `platform` dynamic-loading interface (S0.11.5), and
  used only inside `src/sim/spice/` (step S9.03.1).
- `THIRD_PARTY_NOTICES.md` records the licence, the parts under other licences and the obligations that come from
  the LGPL and MPL-2.0 parts: a replaceable DLL and the source offer. S9.03.1 checks that the port version it adds
  still matches that record. If the port version or its `COPYING` changes, the check is repeated.
- A missing engine library produces a clear message; the rest of the application works without it.

## Consequences

- The licence question is closed. S9.03.1 only confirms that the pinned port still matches the recorded licence.
- Dynamic loading keeps S0 … S8 builds free of ngspice.
- The simulator's error output is parsed and mapped back to schematic objects (S9.03.6).

## Alternatives considered

- Xyce (GPL-3.0): larger and harder to embed as a library.
- Writing a SPICE engine: out of scope.

## Related requirements

D-07, E-SIM-01, L-02, CR-07, P-30; steps S0.11.5, S9.03.1 … S9.03.7.
