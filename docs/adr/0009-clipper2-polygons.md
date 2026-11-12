# ADR 0009: Clipper2 for PCB polygon operations (D-08) {#adr-0009}

- **Status:** Accepted (spec decision D-08)
- **Date:** 2026-11-11

## Context

Spec D-08 chooses Clipper2 (Boost Software License 1.0) for robust integer polygon booleans and offsets. Its 64-bit
integer coordinates fit the PCB nanometre grid (A-09), and it is the basis for clearance shapes, DRC and copper pours.

## Decision

- Clipper2 is added through vcpkg in step S5.01.1 and used only in `src/ecad/pcb/`. The `$ThirdParty` table in
  `scripts/check-layering.ps1` already confines `clipper2/` includes to `ecad/pcb`; S5.01.1 adds the manifest entry
  and the `THIRD_PARTY_NOTICES.md` row.
- Arcs are approximated with a documented maximum chord error; exact arcs are kept in the board database.
- The pour engine design (incremental re-pour, islands, thermal reliefs) is decided by spike SP-6 in ADR 0039.

## Licence verification (S0.01.6, 2026-11-12)

Checked against the vcpkg port metadata (`ports/<port>/vcpkg.json`) and portfile at the pinned baseline
`177e8175` (read from vcpkg's registry cache), the upstream licence file (GitHub licence API) and, for installed
ports, the installed `share/<port>/copyright` file. Spec section 4.3 is the yardstick.

`clipper2` 2.0.1 at the baseline: `BSL-1.0` in the port metadata; GitHub `AngusJohnson/Clipper2` reports
BSL-1.0. Allowed (4.3: Boost). Not in `vcpkg.json` yet; pinned and re-verified when added in S5.

## Consequences

- Integer arithmetic makes clearance results exact on the grid (P-07).
- Pour robustness and performance remain risks owned by SP-6 (`docs/RISKS.md`).

## Alternatives considered

- Boost.Geometry: slower for large polygon sets and floating-point by default.
- CGAL: GPL/LGPL mix and heavy build.

## Related requirements

D-08, A-09, P-07, E-PCB-07, E-RUL-02; ADR 0039; steps S5.01.1, S5.01.4, S5.01.5, S5.11.*.
