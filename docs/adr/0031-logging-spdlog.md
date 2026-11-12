# ADR 0031: Logging with spdlog (C-26) {#adr-0031}

- **Status:** Proposed (Phase 1 conflict C-26; new dependency)
- **Date:** 2026-11-11

## Context

P-23 requires structured logs in a local rotating file and a "Copy diagnostics" command. The architecture notepad
proposes spdlog (MIT), which provides rotating file sinks and asynchronous logging.

## Decision

- `core` exposes a small logging facade (`app::core::log`); spdlog is used behind it in `core` only, added in step
  **S0.03.10**. Add the vcpkg dependency, a `THIRD_PARTY_NOTICES.md` row and a `$ThirdParty` entry (module `core`).
- Log records are structured (JSON lines with time, level, module, message, optional ObjectId); files rotate in the
  injected local folder (ADR 0024).
- No network sinks are compiled in (P-03).

## Consequences

- Domain modules log through the facade, so the backend can change without touching them.
- spdlog brings `fmt`; APP code uses `std::format` and does not include `fmt` directly.

## Alternatives considered

- Own logger: more code for rotation and asynchronous writes.
- Qt logging: Qt is confined to `ui` (ADR 0023).

## Related requirements

P-23, P-03, A-01; ADRs 0023, 0024; steps S0.03.10, S0.11.4.
