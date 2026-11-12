# APP developer manual {#mainpage}

APP (working name) is a clean-room, offline, Windows-first desktop CAD application that combines parametric 3D
mechanical design and electronics design (schematic, PCB, manufacturing outputs) in one shared parametric model.
This manual is generated offline by Doxygen from the repository (`scripts/docs.ps1`).

The authoritative requirements are in `docs/spec/cad-functional-requirements.md`; the implementation plan is in
`docs/IMPLEMENTATION_CHECKLIST.md`.

## Where to start

| Topic | Page |
|---|---|
| Repository layout, modules, projects and namespaces | @ref folder-structure |
| Open project file format (JSON and BSON, magic UUID) | @ref format |
| Command bus naming and documentation convention | @ref commands |
| Requirement tags in the code (`@req`) | the "Requirements Traceability" page |

## Modules

Each module is one folder under `src/` and one Visual Studio project. Its page states its purpose and the modules it
may depend on; `scripts/check-layering.ps1` enforces the dependencies.

| Area | Modules |
|---|---|
| Foundation | @ref module-core, @ref module-platform, @ref module-doc |
| Mechanical | @ref module-geom, @ref module-sketch, @ref module-part, @ref module-asm, @ref module-drw |
| Electronics | @ref module-ecad-lib, @ref module-ecad-sch, @ref module-ecad-pcb, @ref module-ecad-out |
| Cross-domain and services | @ref module-link, @ref module-sim, @ref module-io, @ref module-render |
| Applications | @ref module-cli, @ref module-ui |

## Building, testing and documentation

| Task | Command |
|---|---|
| Build | `scripts/build.ps1 -Configuration Debug` (or `Release`, `Headless`) |
| Test | `scripts/test.ps1` (Headless) or `scripts/test.ps1 -Configuration Debug` |
| Static checks | `scripts/check-layering.ps1`, `scripts/check-spdx.ps1`, `scripts/check-network-apis.ps1` |
| Documentation | `scripts/docs.ps1` (HTML), `scripts/docs.ps1 -Pdf` (PDF, needs LaTeX) |

## Licence

APP is free software under the GNU General Public License, version 3 or later. Third-party libraries and any reused
code are listed in `THIRD_PARTY_NOTICES.md` and `docs/provenance.md`.
