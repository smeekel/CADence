# Commands {#commands}

Every user action that changes data is a **command** on the command bus (spec A-03): a name, a JSON parameter object,
and an execute/undo pair. Commands are journaled, give undo and redo, drive UI replay tests (T-05) and the headless
`cli` (A-02), and become the future public API. This page fixes the naming and documentation convention; the
registry table below grows with each command.

## Naming

- Form: `<domain>.<object>.<verb>`, lower case, words separated by dots; more levels are allowed where an object has
  sub-kinds: `sketch.entity.line.add`, `part.feature.extrude.create`.
- Domains match modules: `project`, `doc`, `sketch`, `part`, `asm`, `drw`, `lib`, `sch`, `pcb`, `out`, `link`, `sim`,
  `io`, `view` (UI-only state such as camera and selection, never journaled as data changes).
- Verbs, in this order of preference: `create`, `add`, `edit`, `delete`, `move`, `set`, `run`, `export`, `import`.
  Use `create` for objects that own a history entry (features, documents) and `add` for members of a collection
  (entities, constraints, mates).
- Names are **stable**: once released, a command is never renamed. A changed meaning gets a new name; the old one is
  kept and marked deprecated.

Examples: `sketch.constraint.add`, `part.feature.fillet.create`, `asm.mate.add`, `pcb.track.route`, `link.create`,
`io.export.step`, `project.save`.

## Documentation of a command

Each command, in the same change that adds it:

1. a parameter schema `docs/commands/<name>.schema.json` (JSON Schema draft 2020-12), with a description and units
   for every field;
2. a row in the registry table below;
3. the `@cmd <name>` tag on the C++ class or function that implements it, plus `@req` with the requirement IDs;
4. registration in the module's `registerCommands( core::CommandRegistry& )` function;
5. a headless test (T-01 or T-02) that runs it through the command bus, including undo.

## Parameters and results

- Parameters are one JSON object (`nlohmann::ordered_json`), validated against the schema before `execute()`.
- Lengths carry explicit units or follow the document units; board coordinates are integer nanometres (A-09).
- Objects are referenced by ObjectId, never by name or index (A-04).
- Errors are returned as `app::Error` values (code, message, detail, objects); commands never throw across the bus.

## Journal

The session journal is a JSON Lines file. Its first line is a header object with `$magic`, `formatVersion` and
`type` `journal`; each following line is one command with a relative timestamp, its name and its parameters.
`cli replay <journal>` re-executes a journal headless.

## Registry

| Command | Module | Undo | Schema | Requirements | Since |
|---|---|---|---|---|---|
| *(none yet; the first commands arrive in Stage 0)* | | | | | |
