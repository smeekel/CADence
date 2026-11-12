# ADR 0007: Own mate solver; Project Chrono for dynamics (D-06) {#adr-0007}

- **Status:** Accepted (spec decision D-06)
- **Date:** 2026-11-11

## Context

Spec D-06 chooses an own assembly mate solver for kinematics, because mates need tight integration with the
regeneration graph and drag interaction, and Project Chrono (BSD-3-Clause) for rigid-body dynamics in M-SIM-04, which
needs contacts, friction and integrators.

## Decision

- The mate solver lives in `src/asm/` (namespace `app::assembly`); its design is decided by spike SP-4 in ADR 0037
  (S3.01.2, S3.01.3). Eigen is allowed in `asm` internals (ADR 0022).
- Chrono is used only inside `src/sim/dynamics/` (step S9.01.1), from the vcpkg port `chronoengine` (8.0.0 in the
  local clone at Phase 1) if it builds with the pinned baseline, otherwise as an overlay port recorded in
  `docs/provenance.md`. The `$ThirdParty` table in `scripts/check-layering.ps1` already confines `chrono/` includes
  to `sim`.
- Mates convert to Chrono joints in `sim` (S9.01.2); kinematic playback in S3 (M-ASM-07) uses the mate solver only.

## Licence verification (S0.01.6, 2026-11-12)

Checked against the vcpkg port metadata (`ports/<port>/vcpkg.json`) and portfile at the pinned baseline
`177e8175` (read from vcpkg's registry cache), the upstream licence file (GitHub licence API) and, for installed
ports, the installed `share/<port>/copyright` file. Spec section 4.3 is the yardstick.

| Library | Port and version at baseline | Licence (SPDX) | Evidence | Status |
|---|---|---|---|---|
| Project Chrono | `chronoengine` 8.0.0#2 | BSD-3-Clause | Port metadata; GitHub `projectchrono/chrono` reports BSD-3-Clause | Allowed |
| oneTBB (Chrono dependency) | `tbb` | Apache-2.0 | Port metadata | Allowed |
| Eigen (Chrono dependency) | `eigen3` 5.0.1 | MPL-2.0 | Port metadata (see ADR 0006) | Allowed |

Not in `vcpkg.json` yet; pinned and re-verified when added in S9.

## Consequences

- Chrono's build size and vcpkg availability are integration risks (`docs/RISKS.md`); they are checked at the start
  of S9, not earlier, so S0 … S8 do not pay the build cost.
- The mate solver's performance on large assemblies is a risk owned by SP-4 and SP-8.

## Alternatives considered

- Using Chrono for kinematics too: heavyweight for interactive drag and harder to integrate with mates and the
  regeneration graph.
- Other physics engines (Bullet, MuJoCo): weaker for engineering joints and reaction-force outputs.

## Related requirements

D-06, M-ASM-01, M-ASM-07, M-SIM-04; ADRs 0021, 0022, 0037; steps S3.01.*, S3.03.*, S9.01.*.
