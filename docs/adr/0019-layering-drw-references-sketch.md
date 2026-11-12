# ADR 0019: Layering: `drw` references `sketch` {#adr-0019}

- **Status:** Proposed (Phase 1 conflict C-07)
- **Date:** 2026-11-11

## Context

M-DRW-02 requires inserting model (driving) dimensions into drawings and keeping them associative. Driving dimensions
are sketch constraints, owned by `sketch`. Rule 030 lets `drw` use `core`, `doc`, `geom`, `part` and `asm`, but not
`sketch`, although `part` already depends on `sketch`.

## Decision

- `drw` may reference `sketch` to read sketch dimensions and constraint values, and to issue sketch edit commands
  when a driving dimension is edited from a drawing.
- Applied in step **S7.02.1**: rule 030 table, `$Allowed['drw']` in `scripts/check-layering.ps1` and `Drw.vcxproj`
  change together after this ADR is Accepted.

## Consequences

- No cycle: `sketch` never references `drw`.

## Alternatives considered

- Exposing sketch dimensions through `part`: `part` would re-export another module's types.

## Related requirements

A-01, M-DRW-02, M-DRW-01; step S7.02.1.
