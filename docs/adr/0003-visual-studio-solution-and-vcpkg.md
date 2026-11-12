# ADR 0003: Visual Studio solution with vcpkg manifest mode (D-02) {#adr-0003}

- **Status:** Accepted (spec decision D-02)
- **Date:** 2026-11-11

## Context

Spec D-02 and B-01 … B-07 choose a Visual Studio 2026 solution (`APP.slnx`) with one `.vcxproj` per module and vcpkg
manifest mode through the MSBuild integration. P-02 requires that a clean clone builds after installing only Visual
Studio (C++ desktop workload) and Git. A cross-platform build system is planned for later (B-07).

## Decision

- One `APP.slnx` at the root; solution folders mirror the module layout; one `.vcxproj` per module; tests are one
  GoogleTest executable per module.
- All settings live in `build/props/*.props` imported through `Directory.Build.props`. Project files hold only file
  lists, references and module-specific overrides.
- Configurations `Debug`, `Release` and `Headless` (no Qt Widgets, no UI project); platform x64.
- vcpkg manifest mode with a pinned baseline in `vcpkg-configuration.json`; the vcpkg root is chosen as in ADR 0029.
- No CMake files. No build logic in source code (no `#pragma comment(lib, …)`).

## Licence verification (S0.01.6, 2026-11-12)

Checked against the vcpkg port metadata (`ports/<port>/vcpkg.json`) and portfile at the pinned baseline
`177e8175` (read from vcpkg's registry cache), the upstream licence file (GitHub licence API) and, for installed
ports, the installed `share/<port>/copyright` file. Spec section 4.3 is the yardstick.

| Item | Port and version | Licence (SPDX) | Linked or shipped | Status |
|---|---|---|---|---|
| vcpkg tool | Visual Studio 2026 18.10 bundled, 2026-07-27 | MIT | No (build tool) | Allowed |
| `vcpkg-cmake`, `vcpkg-cmake-config` | 2025-08-07, 2026-07-21 | MIT | No (port build helpers) | Allowed |

Exact versions of every linked port are pinned with `overrides` in `vcpkg.json` (S0.01.6), so a baseline update
cannot change a shipped library without a manifest edit and a licence re-check.

## Consequences

- Visual Studio and `msbuild APP.slnx /m` build the same way; `scripts/build.ps1` and `scripts/test.ps1` wrap them.
- First builds are slow while vcpkg compiles Qt and OCCT (see `docs/RISKS.md`, Qt + vcpkg build times); binary
  caching is configured in SP-1 (S0.01.1).
- Moving to a cross-platform generator later maps one project to one target (B-07).

## Alternatives considered

- CMake with presets: explicitly excluded for now by P-02 and D-02.
- Per-project vcpkg integration through the user-wide `vcpkg integrate install`: not reproducible (ADR 0029).

## Related requirements

D-02, P-02, B-01 … B-07; ADR 0029; steps S0.01.1, S0.01.3, S0.01.4, S0.01.6.
