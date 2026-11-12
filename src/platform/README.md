# Module platform {#module-platform}

**Purpose.** The only place for operating-system specific code: known folders, DPI, crash handler, dynamic loading, system information.

| Item | Value |
|---|---|
| Folder | `src/platform/` |
| Project | `Platform.vcxproj` (static library) |
| Namespace | `app::platform` |
| Tests | `tests/platform/Platform.Tests.vcxproj` |
| Content from stage | S0 |
| Main requirements | P-04 |

## Allowed dependencies

- Application modules: `core`
- Third-party libraries: Win32 and other OS APIs (P-04).

The table in `.cursor/rules/030-architecture-layering.mdc` is authoritative and `scripts/check-layering.ps1` enforces
it on project references and `#include` paths. It changes only together with an ADR.

## Main types

`moduleInfo()` (scaffold). Planned: known-folder provider, crash handler, dynamic library loader.
