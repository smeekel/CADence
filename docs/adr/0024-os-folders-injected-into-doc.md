# ADR 0024: OS folders injected into `doc` through `core` interfaces (C-11) {#adr-0024}

- **Status:** Proposed (Phase 1 conflict C-11)
- **Date:** 2026-11-11

## Context

`doc` must resolve library folders in `%ProgramData%` and `%APPDATA%` (F-10), and the session journal lives in
`%LOCALAPPDATA%` (A-03). Only `platform` may call the OS (P-04), and neither `doc` nor `core` may depend on
`platform`.

## Decision

- `core` defines an `IKnownFolders` interface (global library root, user library root, journal folder, recovery
  folder, temporary folder). `platform` implements it with Win32 known-folder APIs.
- `doc::LibraryResolver`, the journal writer and autosave take folder paths as injected configuration. `ui` and `cli`
  create the `platform` implementation at start-up and pass it in.
- Tests inject temporary folders, so library-scope tests (F-10) run without touching the user's profile.
- Applied in step **S0.03.13** (interface) and S0.11.1 (Win32 implementation).

## Consequences

- `doc` stays portable and testable; a future Linux or macOS `platform` supplies XDG or macOS folders.

## Alternatives considered

- `doc` reads environment variables directly: hidden OS dependency, hard to test.
- `doc` depends on `platform`: breaks the layering table.

## Related requirements

F-10, A-03, P-04, P-20, U-10; ADR 0017; steps S0.03.13, S0.07.1 … S0.07.3, S0.11.1.
