# ADR 0030: Docking with Qt Advanced Docking System (C-26) {#adr-0030}

- **Status:** Proposed (Phase 1 conflict C-26; new dependency)
- **Date:** 2026-11-11

## Context

U-03 requires dockable, floatable, tabbable panels with per-workspace layouts. The UI notepad proposes Qt Advanced
Docking System (ADS), which is available as the vcpkg port `qt-advanced-docking-system`. Its licence is
**LGPL-2.1-only** (C-25): linking is allowed by spec section 4.3, copying its code is not.

## Decision

- Use ADS in `ui` only, linked dynamically from vcpkg, added in step **S0.13.2**. Add rows to
  `THIRD_PARTY_NOTICES.md` and to the `$ThirdParty` table of `scripts/check-layering.ps1` (module `ui`).
- Layout persistence uses ADS state blobs stored in user settings per workspace.
- No ADS source is copied or modified in the repository.

## Consequences

- Rich docking without writing it; one more LGPL-2.1-only dependency to track.
- If the port lags behind the Qt version, `QDockWidget` is the fallback (with fewer features).

## Alternatives considered

- Plain `QDockWidget`: no tabbed floating groups or auto-hide; weaker U-03.
- KDDockWidgets (GPL-2.0-only or commercial): GPL-2.0-only is not compatible with GPL-3.0-or-later.

## Related requirements

U-03, D-03, P-30, CR-07; ADR 0004; step S0.13.2.
