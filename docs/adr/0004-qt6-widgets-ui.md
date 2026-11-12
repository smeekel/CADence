# ADR 0004: Qt 6 Widgets for the UI (D-03) {#adr-0004}

- **Status:** Accepted (spec decision D-03)
- **Date:** 2026-11-11

## Context

Spec D-03 chooses Qt 6 Widgets with a custom theme: mature, portable, high-DPI aware and usable under the LGPL.
Widgets suit dense CAD tool UIs better than QML. A-01 and P-04 forbid Qt Widgets in domain modules and require a
`Headless` configuration without any UI dependency.

## Decision

- The `ui` executable uses Qt 6 Widgets, pulled through vcpkg (`qtbase` with `default-features: false` and only the
  needed features, see ADR 0026). `moc`, `uic` and `rcc` run through custom build rules defined once in
  `build/props/qt.props`.
- Qt is confined to `ui` (ADR 0023). Domain public APIs use standard library types only.
- The theme (light and dark, accent colour, tokens) lives in `src/ui/theme/`; strings go through `tr()` from S0 (U-13).

## Consequences

- Qt Core types never cross into domain modules, so headless tests and the CLI do not link Qt.
- Docking uses Qt Advanced Docking System (ADR 0030), also LGPL-2.1-only; linking is allowed, copying code is not.
- Applied in S0.13.1 … S0.13.15 and every later UI work package.

## Alternatives considered

- QML / Qt Quick: less suited to dense property panels and tree views (D-03).
- Native Win32 or WinUI: not portable (P-04).
- Dear ImGui: immediate-mode UI is weak at accessibility (U-12) and docking persistence.

## Related requirements

D-03, A-01, P-04, B-04, B-05, U-01 … U-15; ADRs 0023, 0026, 0030.
