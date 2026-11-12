# ADR 0029: vcpkg root selection (C-24) {#adr-0029}

- **Status:** Proposed (Phase 1 conflict C-24)
- **Date:** 2026-11-11

## Context

P-02 expects automatic dependency restore with only Visual Studio and Git installed. The reference machine has a
user-wide vcpkg integration pointing at an old standalone clone (`C:\vcpkg`), which would make builds depend on
machine state.

## Decision

- `build/props/Vcpkg.props` picks the vcpkg root explicitly: `$(VCPKG_ROOT)` if set, otherwise the Visual
  Studio-bundled `$(VsInstallRoot)\VC\vcpkg`.
- Per-user integration is disabled (`VCPkgLocalAppDataDisabled`) so the user-wide `C:\vcpkg` integration is ignored.
- Manifest mode with a pinned baseline in `vcpkg-configuration.json`; Headless installs only test dependencies
  (`vcpkg_installed/headless/`), Debug and Release add the `ui` feature (`vcpkg_installed/full/`), as the scaffold
  does.
- Binary caching location and clean-clone timing are verified in spike SP-1 (step **S0.01.1**).

## Consequences

- Builds are reproducible across machines; a developer can still point `VCPKG_ROOT` at their own clone.
- The first build needs network access to fetch ports (build time only; the product stays offline, P-03).

## Alternatives considered

- Rely on `vcpkg integrate install`: machine-dependent.
- Vendor binaries in the repository: large and hard to update.

## Related requirements

P-02, B-05, B-06; ADR 0003; step S0.01.1.
