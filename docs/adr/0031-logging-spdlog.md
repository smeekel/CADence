# ADR 0031: Logging with spdlog (C-26) {#adr-0031}

- **Status:** Accepted (owner decision, 2026-11-16; Phase 1 conflict C-26; new dependency)
- **Date:** 2026-11-11

## Context

P-23 requires structured logs in a local rotating file and a "Copy diagnostics" command. The architecture notepad
proposes spdlog (MIT), which provides rotating file sinks and asynchronous logging.

## Decision

- `core` exposes a small logging facade (`app::core::Log` in `core/Log.h`); spdlog is used behind it in `core` only,
  added in step **S0.03.10**. Add the vcpkg dependency, a `THIRD_PARTY_NOTICES.md` row and a `$ThirdParty` entry
  (module `core`).
- Log records are structured (JSON lines with time, level, module, message, optional ObjectId); files rotate in the
  injected local folder (ADR 0024).
- No network sinks are compiled in (P-03).

## Consequences

- Domain modules log through the facade, so the backend can change without touching them.
- spdlog is installed without its default features, so it builds on `std::format` (`SPDLOG_USE_STD_FORMAT`) and
  brings no `fmt`; `scripts/check-layering.ps1` confines `spdlog/` includes to `core` (S0.03.10).
- The file sink is a small rotating sink in `core/Log.cpp` on `std::filesystem` paths: spdlog's own sink opens files
  through narrow paths in the ANSI code page, which fails for user folders with characters outside it. spdlog still
  provides the logger, levels and sink locking.

## Alternatives considered

- Own logger: more code for rotation and asynchronous writes.
- Qt logging: Qt is confined to `ui` (ADR 0023).

## Related requirements

P-23, P-03, A-01; ADRs 0023, 0024; steps S0.03.10, S0.11.4.
