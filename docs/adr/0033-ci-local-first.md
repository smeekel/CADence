# ADR 0033: Continuous integration: local scripts first, hosted CI later {#adr-0033}

- **Status:** Proposed (open decision, Phase 1 question Q2)
- **Date:** 2026-11-11

## Context

The S0 exit criterion says "CI runs all tests" (T-13). The workspace is not yet a Git repository, and the owner has
not chosen between GitHub Actions on hosted Windows runners, a self-hosted runner on the reference machine, or a local
scheduled run. Hosted Windows runners would need a vcpkg binary cache to keep Qt and OCCT build times reasonable.

## Decision

- Until the owner decides, CI means the **local** scripts:
  - `scripts/test.ps1` (from the scaffold: builds, runs every `*.Tests.exe`, the CLI smoke test and all checks, writes
    JUnit XML; switches `-Coverage`, `-Bench`, `-Fuzz`, `-UpdateGolden -Reason`);
  - `scripts/check-selftest.ps1` (each check fails on a deliberate violation);
  - `scripts/ci-local.ps1`, added in step **S0.15.1**, which runs all three configurations, the checks, static
    analysis, documentation (when Doxygen is installed) and the offline smoke run; `-Nightly` adds benchmarks and
    fuzzing.
- Every stage gate (`Sx.99.1`) runs `scripts/ci-local.ps1 -Nightly` and archives the results.
- Step **S1.08.1** adds hosted CI once the owner chooses a service; it updates this ADR to Accepted with the choice.
  That step is the only one blocked by the decision; no other step depends on it.

## Consequences

- The S0 exit criterion "CI runs all tests" is met by `scripts/ci-local.ps1` on the reference machine, recorded as a
  deviation in `docs/stages/S0.md` until S1.08.1 is done.
- Regressions between manual runs are caught later than with hosted CI (risk in `docs/RISKS.md`).

## Alternatives considered

- Block S0 on hosted CI: stalls work on a decision outside the implementer's control.
- Skip CI until later stages: violates T-13.

## Related requirements

T-13, T-11, T-08, T-09, T-12; steps S0.15.1, S1.08.1, every `Sx.99.1`.
