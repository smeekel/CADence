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

## Licence verification (S0.01.6, 2026-11-12)

Checked against the vcpkg port metadata (`ports/<port>/vcpkg.json`) and portfile at the pinned baseline
`177e8175` (read from vcpkg's registry cache), the upstream licence file (GitHub licence API) and, for installed
ports, the installed `share/<port>/copyright` file. Spec section 4.3 is the yardstick.

| Library | Port and version (pinned) | Licence (SPDX) | Evidence | Status |
|---|---|---|---|---|
| Qt Base (Core, Gui, Widgets) | `qtbase` 6.11.3#1, features `gui`, `widgets` | LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only (used under LGPL-3.0-only, dynamically linked) | Port metadata has `"license": null`; the installed copyright file (261 KB) holds the LGPL-3.0, GPL-2.0 and GPL-3.0 texts, the commercial-licence reference and the permissive licences of bundled third-party code | Allowed (4.3: LGPL-3.0) |
| PCRE2 | `pcre2` 10.49#2 | BSD-3-Clause WITH PCRE2-exception | Port metadata; installed copyright | Allowed |
| double-conversion | `double-conversion` 3.4.0 | BSD-3-Clause | Port metadata has no licence; portfile installs upstream `LICENSE`; GitHub reports BSD-3-Clause; installed copyright is the BSD text | Allowed |
| zlib | `zlib` 1.3.2#3 | Zlib | Port metadata; installed copyright | Allowed |
| MD4C | `md4c` 0.6.0 | MIT | Port metadata; installed copyright | Allowed |
| OpenGL registry headers | `opengl-registry` 2026-08-03, `egl-registry` 2025-05-27, `opengl` (Windows SDK) | MIT (GL headers, per-file SPDX), Apache-2.0 (EGL registry); `opengl32` is a Windows system library | Installed copyright notes and header SPDX lines | Allowed (headers only, build time) |

Qt Advanced Docking System (`qt-advanced-docking-system` 4.5.0, LGPL-2.1-only, dynamically linked) is verified
when ADR 0030 is Accepted (S0.13.2).

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
