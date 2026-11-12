# ADR 0027: Preservation of unknown fields (C-20) {#adr-0027}

- **Status:** Proposed (Phase 1 conflict C-20)
- **Date:** 2026-11-11

## Context

F-08 requires unknown fields to be preserved on load and written back on save. The data-model notepad's envelope
moves them into an `extensions` object, which would change the file on a round trip. F-03 requires deterministic
output with keys in schema order.

## Decision

- Unknown keys stay in the object where they were found. On save they are written after the schema-known keys, in
  their original order.
- `extensions` is a reserved object for deliberate add-ons only; the loader never moves keys into it.
- Unknown keys inside arrays of objects are preserved per element, keyed by the element's ObjectId.
- Applied in step **S0.06.3** (JSON reader that keeps unknown keys in place, written back by the deterministic writer
  of S0.06.2); fuzz tests (F-08 Accept, S0.08.5) cover malformed input.

## Consequences

- A file written by a newer minor version round-trips unchanged through an older reader (apart from known-key order).
- In-memory objects carry an "unknown fields" bag that commands do not touch.

## Alternatives considered

- Move unknown keys into `extensions`: changes files on round trip.
- Drop unknown keys with a warning: violates F-08.

## Related requirements

F-03, F-05, F-08; ADR 0010; steps S0.06.3, S0.06.8, S0.08.5.
