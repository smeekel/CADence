# Data model and file format

Spec: section 6 (F-01 … F-10), A-04, A-06, A-09. This notepad plays the role of the "database schema" for APP: there is no database, the open file format is the persistent schema.

## Principles

1. **Open and documented** (F-01): Markdown + JSON Schema (draft 2020-12) per document type, in `docs/format/`.
2. **Two encodings** (F-02): JSON project folder (`.apj`) and single-file BSON (`.apb`), lossless and byte-stable conversion.
3. **Deterministic** (F-03): `$magic` first, then schema key order; stable arrays; shortest round-trip floats; LF; 2-space indent.
4. **Design intent only** (F-04): derived data goes in `cache/`, is flagged non-authoritative and is safe to delete.
5. **Versioned** (F-05): `formatVersion` semver; forward migrations; newer minor → warning, newer major → refuse.
6. **Robust** (F-08): unknown fields preserved; errors carry file + JSON path + reason; fuzzed.

## Magic value (F-09)

- UUID `d9a3877a-3f27-4df8-9528-f8dfde59773d`, constant forever, shared by all document types and versions.
- JSON: first key `"$magic"`, lowercase string.
- BSON: first element `$magic`, binary subtype 0x04. Constant bytes at offsets 4–32:

```
05 24 6D 61 67 69 63 00 10 00 00 00 04 D9 A3 87 7A 3F 27 4D F8 95 28 F8 DF DE 59 77 3D
```

- `doc::FileSniffer::identify( path )` returns `{ encoding, type, formatVersion }` or an error, reading at most 4 KB.

## Identifiers

- `core::ObjectId`: 128-bit UUID v7, stored in JSON as a lowercase hyphenated string and in BSON as binary subtype 0x04. Never reused.
- Human-facing names (feature "Extrude3", designator "R12", net "VBUS") are separate fields, editable and not used as references.
- Topology references (A-06) are stored as `TopoRef` objects, never as kernel indices:

```json
{ "feature": "<ObjectId>", "role": "extrude.side", "index": 2,
  "signature": { "kind": "plane", "normal": [ 0, 1, 0 ], "centroid": [ 10.0, 0.0, 5.0 ], "area": 120.0 } }
```

`role` + `index` identify the face by how the feature generated it; `signature` disambiguates and allows re-matching after upstream edits. **(proposed: final scheme decided by the topological-naming spike ADR)**

## Project layout (text form)

```
MyProduct.apj/
  project.json
  parts/<id>.part.json
  assemblies/<id>.asm.json
  drawings/<id>.drw.json
  schematics/<id>.sch.json
  boards/<id>.pcb.json
  links/links.json
  libraries/            project-scope library items (F-10)
  blobs/<sha256>.bin    raw payloads, no magic
  cache/                optional, deletable
```

BSON form: one `.apb` file whose root holds `project` plus a `documents` array (each element is the same object as the corresponding JSON file) and a `blobs` map from hash to binary.

## Common envelope (every document)

```json
{
  "$magic": "d9a3877a-3f27-4df8-9528-f8dfde59773d",
  "formatVersion": "1.0.0",
  "type": "part | assembly | drawing | schematic | board | links | project | library",
  "id": "<ObjectId>",
  "name": "string",
  "units": { "length": "mm", "angle": "deg" },
  "properties": { "PartNumber": "…", "Revision": "A" },
  "parameters": [ { "id": "…", "name": "Width", "expr": "50 mm", "comment": "" } ],
  "configurations": [ ],
  "extensions": { }
}
```

`parameters[].expr` uses the shared expression language (A-08). `extensions` holds unknown or future fields and is round-tripped.

## Document types (key content)

| Type | Main collections | Notes |
|---|---|---|
| `project` | `documents[]` (id, type, path), `parameters[]` (project scope), `variants[]`, `outputJobs[]`, `librarySettings` | One per project |
| `part` | `referenceGeometry[]`, `sketches[]` (entities, constraints, dimensions), `features[]` (ordered; type, inputs, `TopoRef`s, state), `bodies[]` (names, folders), `material`, `sheetMetal`, `configurations[]` | Feature order = history order |
| `assembly` | `components[]` (id, source document, configuration, transform, fixed, state: full/lightweight/suppressed), `mates[]`, `explodedViews[]`, `motionStudies[]`, `stackUps[]`, `configurations[]` | Boards appear as components whose source is a `board` |
| `drawing` | `sheets[]` (format, template), `views[]` (type, source, orientation, scale, display), `annotations[]` (attached by `TopoRef`), `tables[]`, `inspection` | Associative to sources |
| `schematic` | `sheets[]`, `components[]` (library item + revision, designator, part/gate, params), `wires[]`, `buses[]`, `labels[]`, `ports[]`, `sheetSymbols[]`, `directives[]`, `waivers[]`, `simulations[]` | Compiles to netlist |
| `board` | `stack` (layers, dielectrics, via spans, impedance profiles), `outline`, `regions[]` (keepout, room, height), `components[]` (footprint ref, position nm, rotation, side, locked, linkId), `tracks[]`, `arcs[]`, `vias[]`, `pours[]`, `rules[]`, `ruleSets[]`, `diffPairs[]`, `matchGroups[]`, `signalPaths[]`, `waivers[]` | **All coordinates int64 nm** in board frame |
| `links` | `links[]` (id, driver ref, driven ref, quantity, direction, mode: live/review/locked, lastAcknowledgedValue) | Spec section 12 |
| `library` | `items[]` (id, kind, revision, lifecycle, payload), `scope` | Kinds: component, symbol, footprint, model3d, reuseBlock, snippet, material, appearance, profile, gauge, holeTable, fastener, footprintParams, draftingStandard, ruleSet, template |

## Numbers and units in files

- Mechanical values: numbers in document units (default mm/deg) **or** expressions `{ "expr": "BoardWidth + 2 mm" }`. Evaluated values may be cached under `cache/`.
- Board geometry: integers in nanometres (`"x": 25400000` = 25.4 mm). Expressions allowed only for rule values and linked quantities, never for raw primitives.

## Migrations

- `src/doc/migrations/Migrate_<from>_to_<to>.cpp`, each a pure function `ordered_json → Result<ordered_json>`, chained by the loader.
- Every migration has a fixture of the old version in `tests/data/format/` and a test.

## Library scopes (F-10)

Resolution order **project → user → global**. See `Libraries_and_Seed_Data.md`.

## Open items for the planning session

- Confirm `TopoRef` scheme (spike).
- Decide whether large meshes in BSON are embedded or always external blobs (performance test, P-17).
- Decide the JSON Schema `$id` URI scheme (offline: `urn:app:schema:<type>:<major>` **(proposed)**).
