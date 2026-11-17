# ADR 0027: Unknown fields may be dropped on load (C-20) {#adr-0027}

- **Status:** Accepted (owner decision, 2026-11-16; amended by the owner from "keep in place" to "may be dropped";
  deviates from spec F-08, recorded for the S0 gate)
- **Date:** 2026-11-11 (proposed), 2026-11-16 (amended and accepted)

## Context

F-08 says unknown fields are preserved on load and written back on save. The data-model notepad moves them into an
`extensions` object, which would change the file on a round trip. F-03 requires deterministic output with keys in
schema order. The first draft of this ADR kept unknown keys in place, per object and per array element (matched by
ObjectId), which needs an "unknown fields" bag on every in-memory object and a matching rule for array elements
without an ObjectId.

The owner decided on 2026-11-16 that unknown fields may be dropped instead of kept in place.

## Decision

- **Unknown keys are dropped on load.** The JSON reader (S0.06.3) and the BSON reader (S0.06.4) build the in-memory
  model from schema-known keys only. Every dropped key is reported as a warning carrying the file, the JSON path and
  the reason ("unknown field, dropped"), so the user sees what an older version did not understand. Loading still
  succeeds.
- **Save writes known keys only**, in schema order (F-03). Opening and saving a file that has unknown fields therefore
  removes them; the warning on load says so before any save.
- **Schemas stay open.** Format schemas do not set `additionalProperties: false` on document objects, so a file from a
  newer minor version (F-05) validates and loads with warnings instead of being refused.
- `extensions` remains a reserved object for deliberate add-ons; its content is defined by schema like any other
  field. The loader never moves unknown keys into it.
- Applied in steps **S0.06.3** (reader drops and reports unknown keys) and **S0.06.8** (newer minor version loads with
  a warning). Fuzz tests (F-08 Accept, S0.08.5) still cover malformed input.

## Consequences

- No per-object "unknown fields" bag and no matching rule for array elements, so the document model and the readers
  stay simpler.
- **Deviation from F-08.** A file written by a newer minor version loses the fields this version does not know when
  it is saved here. This is listed in the S0 gate report as a spec conflict for the owner. Forward compatibility is
  limited to "loads with warnings".
- Round-trip tests (F-02, F-03) use files that contain only known fields, so they stay byte-identical.
- ADR 0028 no longer needs a rule for unknown fields in BSON: the readers drop them before conversion.
- Settings files (S0.03.13) are not project format files and are outside this ADR.

## Alternatives considered

- Keep unknown keys in place (the first draft): meets F-08 but needs an unknown-field bag on every object and array
  matching rules; not chosen by the owner.
- Move unknown keys into `extensions`: changes files on round trip.
- Refuse files with unknown keys: breaks loading newer minor versions (F-05).

## Related requirements

F-03, F-05, F-08; ADR 0010, ADR 0028; steps S0.06.3, S0.06.8, S0.08.5.
