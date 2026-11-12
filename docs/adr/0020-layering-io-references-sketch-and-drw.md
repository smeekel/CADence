# ADR 0020: Layering: `io` references `sketch` and `drw` {#adr-0020}

- **Status:** Proposed (Phase 1 conflict C-07)
- **Date:** 2026-11-11

## Context

M-IO-01 requires DXF import into sketches and drawings and DXF export of sketches. Rule 030 lets `io` use `core`,
`doc`, `geom`, `part`, `asm` and `ecad/pcb`, but not `sketch` or `drw`.

## Decision

- `io` may reference `sketch` (create sketch entities from DXF, export sketches) and `drw` (import DXF into drawing
  sheets). Drawing DXF *export* stays in `drw` (S7.01.8); PCB layer DXF export stays in `ecad/out`.
- Applied in step **S7.07.1**: rule 030 table, `$Allowed['io']` in `scripts/check-layering.ps1` and `Io.vcxproj`
  change together after this ADR is Accepted.

## Consequences

- No cycle: neither `sketch` nor `drw` references `io`.

## Alternatives considered

- Putting DXF import in `sketch` and `drw` separately: two DXF parsers.

## Related requirements

A-01, M-IO-01; steps S7.07.1, S7.07.2.
