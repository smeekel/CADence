# ADR 0010: nlohmann/json `ordered_json` for JSON and BSON (D-09) {#adr-0010}

- **Status:** Accepted (spec decision D-09)
- **Date:** 2026-11-11

## Context

Spec D-09 chooses nlohmann/json (MIT) because one library writes both encodings (F-02) from the same in-memory tree.
The `ordered_json` variant keeps key order, which deterministic output (F-03) and the `$magic` first key (F-09) need;
the default `json` type sorts keys alphabetically.

## Decision

- `nlohmann::ordered_json` is the only JSON type used for documents, libraries, command parameters and journals.
- It is used in `doc` (format) and, per ADR 0016, in `core` (command parameters, journal).
- Determinism rules on top of the library (schema key order, shortest round-trip floats, LF endings) live in
  `src/doc/format/` (steps S0.06.2, S0.06.3). UUID encoding in BSON follows ADR 0028; unknown fields follow ADR 0027.

## Licence verification (S0.01.6, 2026-11-12)

Checked against the vcpkg port metadata (`ports/<port>/vcpkg.json`) and portfile at the pinned baseline
`177e8175` (read from vcpkg's registry cache), the upstream licence file (GitHub licence API) and, for installed
ports, the installed `share/<port>/copyright` file. Spec section 4.3 is the yardstick.

`nlohmann-json` 3.12.0#2 at the baseline: `MIT` in the port metadata; GitHub `nlohmann/json` reports MIT
(`LICENSE.MIT`). Allowed. Not in `vcpkg.json` yet; pinned when added in S0.03.1 (ADR 0016).

## Consequences

- Single dependency for both encodings; byte-identical JSON → BSON → JSON round trips are testable (F-02 Accept,
  S0.06.11).
- Large files (P-17, 50 MB) may need streaming; measured in S6.11.4.

## Alternatives considered

- RapidJSON plus a separate BSON library: two in-memory models, harder to keep equivalent.
- simdjson: read-only.

## Related requirements

D-09, F-02, F-03, F-08, F-09, P-17; ADRs 0016, 0027, 0028; steps S0.03.1, S0.06.*.
