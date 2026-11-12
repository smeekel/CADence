# Global seed libraries

This folder holds the **global seed** libraries that ship with APP (spec F-10, notepad
`docs/notepads/Libraries_and_Seed_Data.md`). There is one folder per library kind; each starts empty in the scaffold
and receives content in the stage named in its README.

## Scopes

| Scope | Default location | Seeded from | Editable |
|---|---|---|---|
| Global | `%ProgramData%\APP\libraries\` | this folder, on first run | Yes |
| User | `%APPDATA%\APP\libraries\` | empty | Yes |
| Project | `<project>.apj/libraries/` | copies of used items and project-only items | Yes |

Resolution order is project → user → global: a narrower scope shadows a wider one when the library ID matches.
Normal operation never writes into the global seed except through an explicit library edit command.

## Kinds

| Folder | Kind | First content |
|---|---|---|
| `components/` | `component` | S4 |
| `symbols/` | `symbol` | S4 |
| `footprints/` | `footprint` | S4 |
| `models-3d/` | `model3d` | S4 |
| `reuse-blocks/` | `reuseBlock` | S4/S5 |
| `snippets/` | `snippet` | S4/S5 |
| `materials/` | `material` | S2 |
| `appearances/` | `appearance` | S2/S10 |
| `profiles/` | `profile` | S8 |
| `gauges/` | `gauge` | S8 |
| `hole-tables/` | `holeTable` | S2 |
| `fasteners/` | `fastener` | S10 |
| `footprint-params/` | `footprintParams` | S4 |
| `drafting-standards/` | `draftingStandard` | S1/S7 |
| `rule-sets/` | `ruleSet` | S5 |
| `templates/` | `template` | S4/S7 |
| `layer-stacks/` | `layerStack` (proposed) | S5 |
| `environments/` | `environment` (proposed) | S10 |

## Rules for seed data

- Factual data from public sources only; never reproduce paid standards (CR-05). Record every source in
  `docs/data-sources.md`.
- Generic components and footprints only: no vendor logos, trademarks or copied datasheet drawings.
- `seed-manifest.json` (Stage 0) records the seed version and a content hash per item for upgrade review.
