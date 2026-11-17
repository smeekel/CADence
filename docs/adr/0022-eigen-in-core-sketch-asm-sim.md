# ADR 0022: Eigen allowed in `core`, `sketch`, `asm` and `sim` (C-08) {#adr-0022}

- **Status:** Accepted (owner decision, 2026-11-16; Phase 1 conflict C-08)
- **Date:** 2026-11-11

## Context

Rule 030 lists Eigen (MPL-2.0) only for `core`. Spec D-05 requires Eigen in the sketch solver, the mate solver needs
the same linear algebra (D-06), and Chrono integration in `sim` exchanges Eigen-compatible matrices.

## Decision

- Eigen may be used in the internal code of `core`, `sketch`, `asm` and `sim`. Public APIs prefer `core` math types
  (`Vec2`, `Vec3`, `Mat3`, `Transform`, `Box3` in `src/core/math/`) so callers do not depend on Eigen. These types
  store Eigen values and expose them through `eigen()` for the modules allowed to use Eigen.
- Applied per module when first needed, each time adding the module to an Eigen entry in the `$ThirdParty` table of
  `scripts/check-layering.ps1` and the rule 030 table: `sketch` in **S1.01.1**, `asm` in **S3.01.1**, `sim` in
  **S9.01.1**. `core` uses it from **S0.03.5** (math primitives), where `eigen3` 5.0.1 is added to `vcpkg.json`, the
  licence manifest and `THIRD_PARTY_NOTICES.md`, and `check-layering.ps1` gains the Eigen entry for `core` with a
  self-test case that fails when `ecad/pcb` includes `<Eigen/Core>`.
- The include directory `include/eigen3` is added for all projects in `build/props/Vcpkg.props`; the layering check,
  not the include path, decides which modules may include Eigen.

## Consequences

- Eigen includes elsewhere (for example `ecad/pcb`) fail the layering check (tested in S1.01.1).
- MPL-2.0 is file-level copyleft and compatible with GPL-3.0-or-later; Eigen headers are not modified.

## Licence verification (S0.03.5)

`eigen3` 5.0.1 at baseline `177e8175` (upstream tag 5.0.1 from gitlab.com/libeigen/eigen). The port metadata says
MPL-2.0. The installed copyright file combines COPYING.README, COPYING.APACHE, COPYING.BSD, COPYING.MINPACK and
COPYING.MPL2. In the headers APP compiles in (`Eigen/Core`, `Eigen/Geometry`), `Core/arch/Default/BFloat16.h` is
Apache-2.0 and `Geometry/AlignedBox.h` and the LAPACKE/MKL bindings are BSD-3-Clause; the Minpack code is only under
`unsupported/`, which APP does not use. Recorded as `MPL-2.0 AND Apache-2.0 AND BSD-3-Clause` in
`docs/licence-manifest.json`; all three are allowed by spec section 4.3 and compatible with GPL-3.0-or-later.

## Alternatives considered

- Own small dense solver: slower and riskier for sparse Jacobians.
- Eigen in every module: loses control over where heavy templates compile.

## Related requirements

D-05, D-06, A-01; ADRs 0006, 0007; steps S1.01.1, S3.01.1, S9.01.1.
