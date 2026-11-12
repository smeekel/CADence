# ADR 0017: Layering: `cli` references `platform` and `render` {#adr-0017}

- **Status:** Proposed (Phase 1 conflict C-07)
- **Date:** 2026-11-11

## Context

Spec A-01 lets `cli` use `core` and the domain modules `doc` … `io`; the scaffold's `check-layering.ps1` encodes that
list, which excludes `platform` and `render`. But the CLI must obtain known folders (library scopes F-10, journal
location A-03) from `platform` (ADR 0024), load optional engines dynamically through `platform`, and export images
and run the path tracer headless through `render` (snapshot tests T-06, M-VIS-01 batch rendering).

## Decision

- `cli` may reference `platform` and `render`. It still never references `ui`, and both modules stay Qt-free
  (ADR 0023), so the Headless configuration is unaffected.
- Applied in step **S0.10.3**: rule 030 table, `$Allowed['cli']` in `scripts/check-layering.ps1` and
  `Cli.vcxproj` change together, after this ADR is Accepted.

## Consequences

- `cli` can do everything the UI can do except interactive editing (A-02 headless first).
- `check-layering.ps1` still fails if `cli` includes a `ui` header (verified in S0.10.3's test).

## Alternatives considered

- Duplicating folder lookup in `cli` with Win32 calls: violates P-04.
- Routing through a domain module: none of them may depend on `platform` or `render` for these purposes.

## Related requirements

A-01, A-02, P-04, F-10, T-06; ADRs 0023, 0024; step S0.10.3.
