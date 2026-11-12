# File format {#format}

This is the normative specification of the APP project file format (spec section 6, F-01 … F-10). It is a
**skeleton**: the sections exist now and are filled in by the Stage 0 steps that implement the format, together
with the JSON Schemas in `docs/format/schemas/`. Where this page and the spec disagree, the spec wins until this
page is updated.

Status: draft, `formatVersion` 1.0.0 not yet released.

## 1. Scope and design rules

- The file format is the persistent schema of APP. It stores **design intent** only (F-04); derived data (B-rep,
  meshes, DRC and simulation results) is an optional, deletable cache.
- The format is open: every document type has a Markdown page here and a JSON Schema (draft 2020-12) in
  `docs/format/schemas/` (F-01).
- File extensions `.apj` and `.apb` are placeholders until the product is named.

## 2. Encodings (F-02)

| Encoding | Form | Purpose |
|---|---|---|
| JSON | Project folder (`.apj`): one pretty-printed UTF-8 JSON file per document, plus `blobs/` | Git diffs and merges |
| BSON | Single file (`.apb`): the same tree, blobs embedded as BSON binary | Fast loading, single-file sharing |

Conversion between the two is lossless: `json → bson → json` is byte-identical for every golden project (T-03).

## 3. Magic value (F-09) {#format-magic}

Every project, document and library file carries one fixed format-identifier UUID:

    d9a3877a-3f27-4df8-9528-f8dfde59773d

It identifies the format family, is the same for every document type and every format version, and **never
changes**. Versions are carried by `formatVersion` (section 5), the document kind by `type`.

### 3.1 JSON

The first key of the root object is `$magic`, whose value is the UUID as a lowercase string:

    {
      "$magic": "d9a3877a-3f27-4df8-9528-f8dfde59773d",
      "formatVersion": "1.0.0",
      "type": "part",
      ...
    }

A reader recognises a file by parsing the first key; a quick check may look for the UUID string in the first 256
bytes. Output always starts with `{"$magic":` when whitespace is ignored.

### 3.2 BSON

The first element of the root document is `$magic`, encoded as BSON binary subtype `0x04` (UUID, RFC 4122 byte
order). Bytes 4 to 32 of every valid file are therefore constant:

| Offset | Bytes | Meaning |
|---|---|---|
| 0–3 | (varies) | int32 total document length |
| 4 | `05` | element type: binary |
| 5–11 | `24 6D 61 67 69 63 00` | element name `$magic` and terminating zero |
| 12–15 | `10 00 00 00` | binary length 16 (little-endian) |
| 16 | `04` | binary subtype: UUID |
| 17–32 | `D9 A3 87 7A 3F 27 4D F8 95 28 F8 DF DE 59 77 3D` | the magic UUID |

A reader compares those 29 bytes and then reads the file as ordinary BSON, so the files stay readable by any
standard BSON tool.

### 3.3 Rejection

Files whose magic is missing or different are rejected with "not an APP file". JSON files without `$magic` open only
through an explicit "import legacy/unknown JSON" path. Blob files under `blobs/` carry no magic; they are reached
only through hashed references from files that do. Tests: exact BSON prefix for every document type, JSON prefix,
and rejection of wrong or missing magic without a crash.

## 4. Document envelope and key order (F-03)

Every document starts with `$magic`, then `formatVersion`, then `type`, then the remaining keys in schema order.
Arrays keep a stable order; floating-point numbers use the shortest round-trip form; line endings are LF; JSON is
indented by 2 spaces. Saving an unchanged project produces byte-identical files.

*To be specified in Stage 0:* the common envelope keys (`id`, `name`, `units`, `properties`, `parameters`,
`configurations`, `extensions`) and their schema.

## 5. Versioning and migration (F-05)

`formatVersion` is a semantic version. Older versions load through forward migrations
(`src/doc/migrations/`), each logged. A newer minor version loads with a warning; a newer major version is refused
with a clear message. *Version history table: to be added with the first release of the format.*

## 6. Project layout (text form)

    MyProduct.apj/
      project.json            manifest: format version, units, document list, global parameters
      parts/<uuid>.part.json
      assemblies/<uuid>.asm.json
      drawings/<uuid>.drw.json
      schematics/<uuid>.sch.json
      boards/<uuid>.pcb.json
      links/links.json        cross-domain link definitions
      libraries/              project-scope library items (F-10)
      blobs/<sha256>.bin      raw payloads, no magic (F-06)
      cache/                  optional, deletable (F-04)

## 7. Document types

| `type` | File | Schema | Page | Stage |
|---|---|---|---|---|
| `project` | `project.json` | to be added | to be added | S0 |
| `part` | `parts/<uuid>.part.json` | to be added | to be added | S0/S2 |
| `assembly` | `assemblies/<uuid>.asm.json` | to be added | to be added | S3 |
| `drawing` | `drawings/<uuid>.drw.json` | to be added | to be added | S7 |
| `schematic` | `schematics/<uuid>.sch.json` | to be added | to be added | S4 |
| `board` | `boards/<uuid>.pcb.json` | to be added | to be added | S5 |
| `links` | `links/links.json` | to be added | to be added | S6 |
| `library` | library files (F-07) | to be added | to be added | S0/S4 |

## 8. Identifiers, units and numbers

*To be specified in Stage 0:* ObjectId (UUID v7, A-04) encoding in JSON and BSON; mechanical values in document
units or expressions (A-08); board geometry as int64 nanometres (A-09); topology references (`TopoRef`, A-06).

## 9. Robust loading (F-08)

Unknown fields are preserved on load and written back on save. Malformed files produce an error naming the file,
the JSON path and the reason, and never crash the application; every loader has a fuzz target (T-09).

## 10. Blobs (F-06)

Large binary payloads are stored once by SHA-256 content hash, as `blobs/<sha256>.bin` in the folder form and as
BSON binary in the single-file form.

## 11. Library files and scopes (F-07, F-10)

Library files use the same encodings and envelope with `type` `library`. Items resolve from three scopes in the
order project → user → global; a narrower scope shadows a wider one when the library ID matches. *Item envelope,
kinds and the seed manifest: to be specified in Stage 0.*
