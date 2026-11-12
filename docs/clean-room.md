# Clean room and data sources {#clean-room}

How APP stays independent of proprietary CAD products and records where its code and data come from (spec section 2,
CR-01 to CR-07). Every contributor, human or agent, follows the checklist below for every change. The term lists in
this page are the input of `scripts/check-clean-room.ps1`, which `scripts/test.ps1` runs.

**Status of the term lists:** proposed in step S0.02.6, awaiting review by the owner. Add or remove a term by editing
the tables below; the first column of each table is what the check matches.

## Contributor checklist

Before committing, confirm each item:

1. **CR-01 No proprietary material.** Nothing in the change was taken from SOLIDWORKS, Altium Designer or any other
   proprietary CAD product: no source code, binaries, SDK headers, API documentation, help text, icons, images,
   templates, sample files or UI layouts, and nothing decompiled. If the origin of something is uncertain, do not use
   it. Icons are drawn for APP (U-01).
2. **CR-02 Generic terms.** UI text uses generic engineering vocabulary (extrude, fillet, mate, netlist, via, design
   rule). Vendor product names and branded feature names (table "Vendor trademark terms") do not appear in UI
   strings, translations or library data.
3. **CR-03 Open formats only.** Importers and exporters handle open or neutral formats only (spec section 13: STEP,
   IGES, STL, 3MF, DXF, Gerber, ...). No native format of another CAD product is registered (table "Native CAD file
   extensions").
4. **CR-04 Fresh code preferred.** New code is written fresh. Reused open-source code is GPL-3.0-compatible (spec 4.3)
   and saves substantial effort on a hard problem.
5. **CR-05 Standards data.** Paid or copyrighted standards (IPC-7351, ISO fastener standards, ...) are never reproduced
   verbatim. Shipped data is factual dimensional data from public sources or original values, and every data file
   under `libraries/` has a complete row in `docs/data-sources.md`.
6. **CR-06 No telemetry.** No telemetry, analytics, usage statistics, licence servers, update checks or other
   "phone home" code (table "Telemetry and update-check keywords"); APP needs no network access (P-03, ADR 0026).
7. **CR-07 Attribution.** Reused code keeps its copyright and licence lines and SPDX header, has a complete row in
   `docs/provenance.md`, and `scripts/update-notices.ps1` has regenerated `THIRD_PARTY_NOTICES.md`.

## What the check enforces

`scripts/check-clean-room.ps1` fails on:

| Rule | Requirement | Scope |
|---|---|---|
| 1. Vendor trademark terms | CR-02 | String literals in C++ files under `src/` (comments and `#include` lines are skipped); Qt `.ui`, `.ts` and `.qml` files anywhere; JSON files under `libraries/` |
| 2. Native CAD file extensions | CR-03 | String literals in C++ files under `src/` and Qt `.ui` files, for example `"*.sldprt"` in a file-dialog filter or an importer registration |
| 3. Telemetry and update-check keywords | CR-06 | Code and string literals in C++ files under `src/` (comments are skipped), and every other text file under `src/` except Markdown documentation |
| 4. Undocumented data files | CR-05 | Every file under `libraries/` except `README.md` files and `libraries/seed-manifest.json` needs a complete row in `docs/data-sources.md` |

Matching is case-insensitive on whole words; a space in a term also matches no space, `_` or `-` (so "Hole Wizard"
also matches `HoleWizard` and `hole_wizard`). CR-01 and CR-04 cannot be checked automatically and rely on the
checklist and on review. There is no allow-list: rephrase the text instead.

## Vendor trademark terms (rule 1)

| Term | Vendor | Use instead |
|---|---|---|
| `SOLIDWORKS` | Dassault Systemes | "other CAD software" |
| `CATIA` | Dassault Systemes | "other CAD software" |
| `Altium` | Altium | "other EDA software" |
| `Autodesk` | Autodesk | "other CAD software" |
| `AutoCAD` | Autodesk | "other CAD software" |
| `Fusion 360` | Autodesk | "other CAD software" |
| `Onshape` | PTC | "other CAD software" |
| `Solid Edge` | Siemens | "other CAD software" |
| `OrCAD` | Cadence | "other EDA software" |
| `Hole Wizard` | Dassault Systemes | "Hole tool" |
| `FeatureManager` | Dassault Systemes | "Feature tree" |
| `PropertyManager` | Dassault Systemes | "Properties panel" |
| `CommandManager` | Dassault Systemes | "Command ribbon" |
| `ConfigurationManager` | Dassault Systemes | "Configurations panel" |
| `SmartMate` | Dassault Systemes | "Automatic mate" |
| `DimXpert` | Dassault Systemes | "Dimensioning assistant" |
| `Instant3D` | Dassault Systemes | "Drag editing" |
| `SpeedPak` | Dassault Systemes | "Simplified assembly" |
| `eDrawings` | Dassault Systemes | "Viewer export" |
| `PhotoView` | Dassault Systemes | "Rendering" |
| `ActiveRoute` | Altium | "Guided routing" |
| `Situs` | Altium | "Topological autorouter" |
| `xSignals` | Altium | "Signal path" |
| `ActiveBOM` | Altium | "BOM manager" |
| `Draftsman` | Altium | "Board drawing" |

## Native CAD file extensions (rule 2)

| Extension | Product format |
|---|---|
| `.sldprt` | SOLIDWORKS part |
| `.sldasm` | SOLIDWORKS assembly |
| `.slddrw` | SOLIDWORKS drawing |
| `.slddrt` | SOLIDWORKS sheet format |
| `.sldlfp` | SOLIDWORKS library feature |
| `.schdoc` | Altium schematic |
| `.pcbdoc` | Altium PCB |
| `.prjpcb` | Altium PCB project |
| `.schlib` | Altium schematic library |
| `.pcblib` | Altium footprint library |
| `.intlib` | Altium integrated library |
| `.dwg` | AutoCAD drawing (DXF is the open alternative) |
| `.ipt` | Inventor part |
| `.iam` | Inventor assembly |
| `.idw` | Inventor drawing |
| `.f3d` | Fusion 360 archive |
| `.catpart` | CATIA part |
| `.catproduct` | CATIA product |
| `.catdrawing` | CATIA drawing |
| `.prt` | Creo or NX part |
| `.par` | Solid Edge part |
| `.psm` | Solid Edge sheet-metal part |
| `.brd` | Allegro or EAGLE board |
| `.dsn` | OrCAD schematic |
| `.olb` | OrCAD library |

## Telemetry and update-check keywords (rule 3)

| Keyword | Why |
|---|---|
| `telemetry` | Telemetry of any kind (CR-06) |
| `analytics` | Usage analytics |
| `usage statistics` | Usage analytics |
| `usage stats` | Usage analytics |
| `phone home` | Phone-home code |
| `check for updates` | Update check |
| `update check` | Update check |
| `auto update` | Automatic update |
| `licence server` | Licence server |
| `license server` | Licence server |
| `crash upload` | Uploading crash reports (crash dumps stay local) |
| `Sentry` | Error-reporting service |
| `Mixpanel` | Analytics service |
| `AppCenter` | Analytics and update service |
