# ADR 0016: Layering: `core` may use nlohmann/json {#adr-0016}

- **Status:** Proposed (Phase 1 conflict C-07)
- **Date:** 2026-11-11

## Context

Rule 030 lists only the standard library and Eigen as third-party code for `core`, and nlohmann/json only for `doc`.
The command bus lives in `core` (A-03), and the command-bus notepad gives `Command::params()` and the session journal
the type `nlohmann::ordered_json` (ADR 0010). Without this change every command would need a second parameter model
and a conversion layer.

## Decision

- `core` may include `<nlohmann/json.hpp>` and use `ordered_json` for command parameters, results and journal
  records. `doc` keeps using it for documents.
- No other module gains it implicitly; modules that build command parameters use `core`'s alias
  `app::core::Json`.
- Applied in step **S0.03.1**: rule 030 table, a new nlohmann entry in the `$ThirdParty` table of
  `scripts/check-layering.ps1` (modules `core`, `doc`), and `Core.vcxproj` change together, only after this ADR is
  Accepted.

## Consequences

- One JSON model from the command bus through journals to files; schema validation (S0.04.2) works on the same type.
- `core` gains a header-only dependency (MIT) that it already shares with `doc`.

## Alternatives considered

- A neutral variant type in `core` converted to JSON in `doc`: duplicates the model and costs a conversion per
  command.
- Moving the command bus to `doc`: `doc` would become the root of most modules' dependencies for an unrelated reason.

## Related requirements

A-01, A-03, F-02; ADR 0010; rule 030; steps S0.03.1, S0.04.1 … S0.04.5.
