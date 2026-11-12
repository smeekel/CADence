# Golden-data command scripts

This folder holds the **command scripts** that build golden models (spec T-02, T-04; rule 060). It is empty in the
scaffold; the first scripts arrive with the command bus and the headless `cli` in Stage 0.

## Why scripts

Golden data is generated, never hand-made. Each golden model (part, assembly, schematic, board, project) is built by
running a script of commands through the command bus, exactly as a user or the UI would. The model therefore stays
reproducible when the format or the code changes, and the script documents how it was made.

## Files

| File | Content |
|---|---|
| `<name>.cmd.json` | A command script: an ordered list of commands, each `{ "name": "...", "params": { ... } }`, with a header carrying `$magic`, `formatVersion` and `type` `commandScript` |
| `<name>.expected.json` | Expected values, beside the script that builds the model: computed independently of the code under test: mass properties, topology counts, degrees of freedom, netlists, DRC results |

Names describe the case, for example `extrude_up_to_surface_block.cmd.json`.

## Running

- `Cli.exe run <script>` executes a script headless (Stage 0).
- Integration tests (T-02) run every script and compare the resulting model with the `*.expected.json` values,
  using explicit tolerances (relative 1e-6 for geometry by default).
- Committed golden outputs are regenerated only with `scripts/test.ps1 -UpdateGolden -Reason "<why>"`, and the
  reason goes into the commit message.

## Rules

- Deterministic: fixed seeds, no clock, no network, temporary folders only.
- Expected values come from analytic formulas, independent calculations or hand-checked references, never from
  the code under test.
- Third-party input files (STEP, IGES, DXF) do not belong here; they go to `tests/data/import/` with licence notes.
