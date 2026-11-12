# ADR 0029: vcpkg root selection (C-24) {#adr-0029}

- **Status:** Accepted (verified by spike SP-1a, step S0.01.1, 2026-11-12; confirmed by the owner, 2026-11-12)
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
- Pinned baseline: `177e8175121536d414f405ed8b3099b325a42638` of https://github.com/microsoft/vcpkg (default git
  registry in `vcpkg-configuration.json`).
- Binary caching uses vcpkg's default local cache (`%LOCALAPPDATA%\vcpkg\archives`); nothing in the repository
  configures it.
- Clean clones and scratch builds must not live under `%TEMP%`: MSBuild then reports warning MSB8029 (output under
  the temporary directory), which the zero-warning rule turns into a build failure.

## Verification (spike SP-1a, step S0.01.1, 2026-11-12)

Machine: Windows 11 x64, Visual Studio Community 2026 18.10.1 (v145, MSVC 19.51), bundled vcpkg
2026-07-27-98d7cb0c, Git 2.51.2. Only Visual Studio's MSBuild, its bundled vcpkg and Git were used by the build.

- **Baseline:** commit `177e8175…` exists upstream (GitHub API, 2026-11-12: "[libressl] Update to 4.4.1 (#55013)")
  and matches `vcpkg-configuration.json`.
- **Clean clone** of `stage/s0` from https://github.com/smeekel/CADence.git into a scratch folder outside `%TEMP%`
  (`C:\dev\cadence-s0-cleanclone`), then `scripts\build.ps1` per configuration and `scripts\test.ps1 -NoBuild`:

  | Run | vcpkg binary cache | Headless | Debug | Release | Tests (Headless) |
  |---|---|---|---|---|---|
  | Clean clone, warm cache | default (local archives) | OK, 0 warnings, 13 s | OK, 0 warnings, 26 s | OK, 0 warnings, 12 s | OK, 3 s |
  | Clean clone, cold (`VCPKG_BINARY_SOURCES=clear`) | disabled | OK, 0 warnings, 24 s | OK, 0 warnings, 490 s (qtbase built from source in 7 min) | OK, 0 warnings, 14 s | not run |

  The cold run reused source archives already in vcpkg's download cache; on a machine that has never fetched them,
  add the download time.

  A first attempt in `%TEMP%\cadence-clean` failed on MSB8029 (see Decision).
- **DLL imports** (`dumpbin /dependents`, clean-clone build):

  | Binary | Imported DLLs (CRT and `api-ms-win-crt-*` omitted) |
  |---|---|
  | `Qt6Core.dll` / `Qt6Cored.dll` | `KERNEL32`, `ntdll`, `ADVAPI32`, `AUTHZ`, `USERENV`, `USER32`, `SHELL32`, `ole32`, `OLEAUT32`, `VERSION`, `WINMM`, `MPR`, `NETAPI32`, **`WS2_32`**, `icuuc`, `z`/`zd`, `double-conversion`, `pcre2-16`/`pcre2-16d`, `api-ms-win-core-synch`, `api-ms-win-core-winrt*` |
  | `Ui.exe` (Debug, Release) | `Qt6Core`, `Qt6Widgets`, `KERNEL32`, `SHELL32` |
  | `Cli.exe` (Debug, Release, Headless) | `KERNEL32` |
  | `*.Tests.exe` (Headless) | `gtest`, `gtest_main`, `KERNEL32` |

  The only network DLL imported by any binary is `WS2_32.dll`, imported by Qt6Core in both Debug and Release. The
  scaffold's `docs/network-imports.md` listed only the Debug `Qt6Cored.dll`; it was regenerated from `out/Debug`,
  `out/Release` and `out/Headless` and now lists both. `MPR.dll` (network drive names) and `NETAPI32.dll`
  (account and share information) are Windows management libraries, not socket or HTTP libraries; they are noted
  here and are not in the network-DLL list of ADR 0026.
- **OCCT `TKernel`:** OCCT is not in `vcpkg.json` until step S0.09.1, so its import list is recorded there.

## Consequences

- Builds are reproducible across machines; a developer can still point `VCPKG_ROOT` at their own clone.
- The first build needs network access to fetch ports (build time only; the product stays offline, P-03).

## Alternatives considered

- Rely on `vcpkg integrate install`: machine-dependent.
- Vendor binaries in the repository: large and hard to update.

## Related requirements

P-02, B-05, B-06; ADR 0003; step S0.01.1.
