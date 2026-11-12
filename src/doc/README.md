# Module doc {#module-doc}

**Purpose.** Document model and project container, JSON and BSON encodings, magic-UUID sniffing, migrations and library-scope resolution.

| Item | Value |
|---|---|
| Folder | `src/doc/` |
| Project | `Doc.vcxproj` (static library) |
| Namespace | `app::doc` |
| Tests | `tests/doc/Doc.Tests.vcxproj` |
| Content from stage | S0 |
| Main requirements | F-01 ... F-10 |

## Allowed dependencies

- Application modules: `core`
- Third-party libraries: nlohmann/json (`ordered_json`).

The table in `.cursor/rules/030-architecture-layering.mdc` is authoritative and `scripts/check-layering.ps1` enforces
it on project references and `#include` paths. It changes only together with an ADR.

## Main types

`moduleInfo()` (scaffold). Planned: `Project`, `Document`, `ProjectStore`, `FileSniffer`, `LibraryResolver`.
