# Module ui {#module-ui}

**Purpose.** Qt 6 Widgets desktop application: main window, panels, tools and themes. The only module that uses Qt Widgets.

| Item | Value |
|---|---|
| Folder | `src/ui/` |
| Project | `Ui.vcxproj` (executable) |
| Namespace | `app::ui` |
| Tests | `tests/ui/Ui.Tests.vcxproj` |
| Content from stage | S0 |
| Main requirements | U-01 ... U-15 |

## Allowed dependencies

- Application modules: `core`, `platform`, `doc`, `geom`, `sketch`, `part`, `asm`, `drw`, `ecad/lib`, `ecad/sch`, `ecad/pcb`, `ecad/out`, `link`, `sim`, `io`, `render`
- Third-party libraries: Qt 6 (qtbase: Core, Gui, Widgets).

The table in `.cursor/rules/030-architecture-layering.mdc` is authoritative and `scripts/check-layering.ps1` enforces
it on project references and `#include` paths. It changes only together with an ADR.

The `Headless` configuration does not build this project (B-04, P-04); `build/props/Headless.props` stops the build if it is attempted.

## Main types

`moduleInfo()`, `MainWindow` (scaffold: an empty window titled "APP").
