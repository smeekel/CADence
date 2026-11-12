# Third-party notices {#third-party-notices}

APP is licensed under the GNU General Public License, version 3 or (at your option) any later version
(`GPL-3.0-or-later`, see `LICENSE`). It links the third-party libraries listed below. Each keeps its own licence;
the full licence texts are installed by vcpkg in `vcpkg_installed/<tree>/x64-windows/share/<port>/copyright` and
will ship with the application.

This file is required by P-30 and CR-07. It lists **every linked library** and **every reused project**. Today it
is maintained by hand from `vcpkg.json` (baseline in `vcpkg-configuration.json`) and `docs/provenance.md`;
generating it automatically from those two sources is planned for Stage 0.

## Libraries linked by the scaffold

vcpkg baseline `177e8175121536d414f405ed8b3099b325a42638` (microsoft/vcpkg, 2026-11-12), triplet `x64-windows`.

| Library | vcpkg port and version | Licence (SPDX) | Used by | Shipped |
|---|---|---|---|---|
| Qt Base (Core, Gui, Widgets) | `qtbase` 6.11.3#1, features `gui`, `widgets`; no Network | LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only (used under LGPL-3.0-only / GPL-3.0-only) | `ui`, `tests/ui` | Yes (DLLs) |
| PCRE2 | `pcre2` 10.49 (Qt dependency) | BSD-3-Clause WITH PCRE2-exception | `ui` via Qt Core | Yes (DLL) |
| double-conversion | `double-conversion` 3.4.0 (Qt dependency) | BSD-3-Clause | `ui` via Qt Core | Yes (DLL) |
| zlib | `zlib` 1.3.2 (Qt dependency) | Zlib | `ui` via Qt Core | Yes (DLL) |
| MD4C | `md4c` 0.6.0 (Qt dependency) | MIT | `ui` via Qt Gui | Yes (DLL) |
| GoogleTest and GoogleMock | `gtest` 1.18.0 | BSD-3-Clause | `tests/*` only | No (test only) |

Build-time only, not linked into APP: `vcpkg-cmake`, `vcpkg-cmake-config` (MIT, vcpkg build helpers),
`opengl-registry` and `egl-registry` (Khronos API headers, Apache-2.0 and MIT), `opengl` (Windows SDK system
library). The `Headless` configuration links GoogleTest only.

## Libraries planned by the specification (not linked yet)

Added to `vcpkg.json` in the checklist step that first uses them, after a licence check against spec section 4.3
and an ADR (rule 070):

| Library | Decision | Expected licence |
|---|---|---|
| Open CASCADE Technology | D-04 | LGPL-2.1-only with OCCT exception (link only, no code copied) |
| Eigen | D-05, D-06 | MPL-2.0 |
| Project Chrono | D-06 | BSD-3-Clause |
| ngspice | D-07 | Mostly BSD-3-Clause upstream; the vcpkg port metadata says CC-BY-SA-4.0. Verify before adding. |
| Clipper2 | D-08 | BSL-1.0 |
| nlohmann/json | D-09 | MIT |
| lib3mf | D-10 | BSD-2-Clause |
| Intel Embree | D-13 | Apache-2.0 |

## Reused source code

None. When code is reused under CR-04, each project is listed here with its licence, and the details go to
`docs/provenance.md` (CR-07).
