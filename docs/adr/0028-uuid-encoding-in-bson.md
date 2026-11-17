# ADR 0028: UUID encoding in BSON decided by schema (C-21) {#adr-0028}

- **Status:** Accepted (owner decision, 2026-11-16, with the canonical lower-case UUID amendment; Phase 1 conflict C-21)
- **Date:** 2026-11-11

## Context

F-09 makes `$magic` a BSON UUID binary (subtype 0x04). The data-model notepad also stores every ObjectId as binary
subtype 0x04. F-02 requires a byte-identical `json → bson → json` round trip, so the converter must know which JSON
strings are UUIDs. Guessing from string content would turn a user-entered string that happens to look like a UUID
into binary.

## Decision

- ObjectIds and other UUID-valued fields are BSON binary subtype 0x04 when, and only when, the document's JSON Schema
  marks the field `format: uuid`. `$magic` is always binary.
- The converter never guesses from string content. Unknown fields (ADR 0027) are always kept as strings.
- **Canonical text form (amendment, 2026-11-16).** UUIDs are written only in the standard lower-case hyphenated
  form `xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx`. Every `format: uuid` field in a schema also carries
  `"pattern": "^[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}$"`, so upper-case, unhyphenated or braced
  UUIDs are rejected on load with file, JSON path and reason instead of being rewritten silently. `core::ObjectId::parse`
  accepts only this form. This keeps `json -> bson -> json` byte-identical, because binary UUIDs come back in exactly
  one text form.
- Applied in step **S0.06.4** (BSON writer and reader) and verified by the F-02 Accept test in S0.06.11.

## Consequences

- Schemas must mark every UUID field with `format: uuid` and the canonical `pattern`; `scripts/check-commands.ps1` and
  the schema tests (S0.06.1) check this. The pattern is applied from S0.06.1 (first schemas), S0.06.3 (reader rejects
  non-canonical UUIDs) and S0.06.4 (BSON writer and reader).
- BSON files are smaller and UUID comparisons faster.

## Alternatives considered

- Store all UUIDs as strings in BSON: simpler, but contradicts the notepad and wastes space.
- Detect UUIDs by pattern: unsafe for user data.

## Related requirements

F-02, F-09, A-04; ADRs 0010, 0027; steps S0.06.4, S0.06.11.
