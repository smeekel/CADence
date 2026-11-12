# Libraries and seed data

Spec: F-07, F-10, E-LIB-01 … E-LIB-03, M-PRT-09, M-ASM-05, M-WLD-01, M-SHM-01, M-DRW-02, CR-05, Q-09.

## Scopes and resolution (F-10)

| Scope | Default location | Seeded from | Editable |
|---|---|---|---|
| Global | `%ProgramData%\APP\libraries\` | Repo `/libraries` at install/first run | Yes (permissions: open question Q-09) |
| User | `%APPDATA%\APP\libraries\` | Empty | Yes |
| Project | `<project>/libraries/` | Copies of used items + project-only items | Yes |

- Resolution: **project → user → global**. A narrower scope shadows a wider one when the library ID matches. The browser shows each item's scope and marks shadowed items.
- "Override into user/project" copies an item and records its origin (`originScope`, `originId`, `originRevision`).
- On save, every used item is copied into the project scope (default on), so projects are self-contained.
- Extra read-only library folders (team share, Git checkout) can be added in Preferences.
- `doc::LibraryResolver` implements resolution. It is pure and fully unit-tested, and never touches the network.

## Seed upgrades

- `/libraries/seed-manifest.json` carries `seedVersion` and a content hash per item.
- On upgrade: unchanged global items update silently; items changed only locally stay; items changed both locally and in the new seed go to a **three-way review** (old seed / new seed / local). Nothing is overwritten without the user's decision.

## Library item envelope

```json
{
  "$magic": "d9a3877a-3f27-4df8-9528-f8dfde59773d",
  "formatVersion": "1.0.0",
  "type": "library",
  "kind": "material",
  "items": [
    {
      "id": "<ObjectId>",
      "libraryId": "material.aluminium-6061-t6",
      "revision": "A",
      "lifecycle": "Released",
      "name": "Aluminium 6061-T6",
      "source": "docs/data-sources.md#materials-aluminium",
      "payload": { }
    }
  ]
}
```

`libraryId` is a stable, human-readable dotted key used for shadowing across scopes. `id` identifies a specific revision instance.

## Seed content by stage (initial, minimal, grows later)

| Kind | Needed by | Initial seed (examples) |
|---|---|---|
| Drafting standards | S1/S7 | ISO, ANSI presets |
| Materials + appearances | S2 | Common steels, stainless, aluminium alloys, brass, copper, ABS, PC, PLA, PETG, PA, POM, FR-4, glass, rubber |
| Hole and thread tables | S2 | ISO metric coarse/fine M1.6–M24; UNC/UNF #2–1/2"; clearance close/normal/loose |
| Sheet templates and title blocks | S7 | ISO A4–A0, ANSI A–E (original designs) |
| Symbols and components | S4 | Generic R, C, L, diode, LED, BJT, MOSFET, op-amp, regulator, connector headers, USB-C receptacle, test point, mounting hole, fiducial |
| Footprint generator parameters | S4 | Default fillet-goal and tolerance tables with documented, non-paywalled sources; users may enter licensed IPC values |
| Rule sets | S5 | "Generic 2-layer 6/6 mil", "Generic 4-layer 5/5 mil", "HDI 3/3 mil" (original values, labelled generic) |
| Layer stacks | S5 | 2-, 4-, 6-, 8-layer generic stacks |
| Gauge tables | S8 | Steel, stainless, aluminium sheet gauges |
| Structural profiles | S8 | Square/rect tube, round tube, angle, channel, I-beam, flat bar (common metric and inch sizes) |
| Fasteners and standoffs | S10 | Socket/button/pan/countersunk/hex screws, nuts, washers, dowels, heat-set inserts, M2.5/M3/#4-40 standoffs |
| Render environments | S10 | Original studio HDRs or permissively licensed ones |

## Data source rules (CR-05)

- Every seed table has an entry in `docs/data-sources.md`: kind, source (title, publisher, URL or citation), licence/terms, date retrieved, notes on transformation.
- Use factual dimensional data from public sources (manufacturer catalogues, government or open data). Never reproduce paid standards documents. When unsure, leave the table for the user to fill and mark it "user-supplied".
- Component and footprint seeds are generic. No vendor logos, trademarks or copied datasheet drawings.
