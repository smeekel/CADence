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

## Licence-verified libraries (planned, not linked yet)

Both licences were verified on 2026-11-11 against the vcpkg port at the baseline above and against the upstream
files, and accepted by the owner. Each library is added to `vcpkg.json` in the step that first uses it: OCCT in
S0.09.1, ngspice in S9.03.1.

### Open CASCADE Technology (OCCT)

| Item | Value |
|---|---|
| Licence (SPDX) | `LGPL-2.1-only WITH OCCT-exception-1.0` |
| Upstream | https://github.com/Open-Cascade-SAS/OCCT (project site https://dev.opencascade.org) |
| Version verified | vcpkg port `opencascade` 8.0.1, built from upstream tag `V8_0_1` with five vcpkg build patches |
| How APP uses it | vcpkg builds it as DLLs (manifest mode, triplet `x64-windows`), and only `geom` links it dynamically (ADR 0005). No OCCT source is copied into APP. |
| Shipped | Yes (DLLs), from S0.09.1 |

- **vcpkg port:** the metadata says `LGPL-2.1-only` and leaves out the exception. The portfile installs both
  `LICENSE_LGPL_21.txt` and `OCCT_LGPL_EXCEPTION.txt` as the port's copyright file.
- **Upstream:** the `README.md` and the file headers (for example `Standard.hxx`) grant "the GNU Lesser General Public
  License version 2.1 ... with special exception defined in the file `OCCT_LGPL_EXCEPTION.txt`". They say nothing
  about later versions, so the licence is `-only`. OCCT is also offered under a commercial licence; APP uses the LGPL
  option.
- **The exception:** "Open CASCADE exception (version 1.0) to GNU LGPL version 2.1", SPDX `OCCT-exception-1.0`.
  Object code may include material from OCCT header files under terms of APP's choice, provided the supporting
  documentation gives prominent notice that the code uses OCCT.
- **Notice (OCCT exception):** APP makes use of, and is based on, facilities provided by the Open CASCADE Technology
  software.

### ngspice

| Item | Value |
|---|---|
| Licence (SPDX) | `BSD-3-Clause`, upstream's "Modified BSD" licence, for all code except the parts listed below |
| Upstream | https://ngspice.sourceforge.io (source: https://sourceforge.net/p/ngspice/ngspice/) |
| Version verified | vcpkg port `ngspice` 41. The tarball `ngspice-41.tar.gz` matches the port's SHA512, and its `COPYING` is identical to the one at upstream tag `ngspice-41`. vcpkg applies five build patches. |
| How APP uses it | vcpkg builds it as `ngspice.dll` (the `sharedspice` project). `sim` loads it at run time through the `platform` dynamic-loading interface (ADR 0008). It is not linked at build time, and no source is copied. |
| Shipped | Yes (DLL, plus the XSPICE code models if the port's default `codemodels` feature is kept), from S9.03.1 |

- **The vcpkg metadata is wrong:** it says `CC-BY-SA-4.0`, but that licence covers only the licensing document (`COPYING`
  says "license for this document: CC-BY-SA v4.0") and the ngspice manual. Neither is code. The port installs
  `COPYING` as its copyright file.
- **Parts not under BSD-3-Clause:** from `COPYING`, checked against the files that the port's Visual Studio projects
  compile:

| Part | Licence | In the vcpkg build |
|---|---|---|
| `src/frontend/numparam` | LGPL-2.0-or-later | Yes, in `ngspice.dll` |
| `src/osdi` | MPL-2.0 | Yes, in `ngspice.dll` |
| `src/include/cppduals` (not listed in `COPYING`) | MPL-2.0 | Yes, headers compiled into `ngspice.dll`; the port does not install them |
| `src/maths/sparse` | MIT-style licence without a name ("compatible to New BSD") | Yes, in `ngspice.dll` |
| `src/xspice`, except `icm/table` | Public domain (Georgia Tech Research Corporation) | Yes, in `ngspice.dll` and the code models |
| `src/xspice/icm/table` | GPL-2.0-or-later | Only in `table.cm`, built by the default `codemodels` feature |
| `src/tclspice.c` | LGPL v2 (`COPYING` gives no "only" or "or later") | No (Tcl build only) |
| `src/spicelib/devices/adms/admst` | LGPL-2.1 | No |
| `src/spicelib/devices/ndev` | Public domain | No |
| `m4` | Unnamed, DFSG-compatible | No (build scripts) |
| ngspice manual | CC-BY-SA-4.0 | No (not shipped) |

All of these may be combined with GPL-3.0-or-later under spec section 4.3. Because `ngspice.dll` contains LGPL and
MPL-2.0 code, the obligations below apply to it as well.

## Obligations for LGPL libraries

These obligations apply to Qt (LGPL-3.0-only), to OCCT (`LGPL-2.1-only WITH OCCT-exception-1.0`) and to the LGPL
parts of `ngspice.dll`. APP's own licence already requires its complete source to be offered. In addition:

1. **Dynamic linking.** Each LGPL library is a separate DLL from the vcpkg `x64-windows` triplet, and ngspice is
   loaded at run time. APP never links them statically and never copies their code into APP files (spec section 4.3,
   rule 070).
2. **Ability to replace the library.** A user must be able to replace each of these DLLs with a modified,
   interface-compatible build and run APP with it. APP ships them as separate files next to its executables, and it
   neither embeds them nor rejects a replaced copy. Modifying APP and reverse engineering it to debug such
   modifications are permitted (LGPL-2.1 section 6, LGPL-3.0 section 4).
3. **Offering the source.** Every distribution of APP includes these licence texts and either of the following:
   - the corresponding source of each library's exact version, meaning the upstream release plus the vcpkg port
     patches and build scripts at the pinned baseline;
   - a written offer of that source, valid for at least three years (LGPL-2.1 section 6(c)).

   The same source offer covers ngspice's MPL-2.0 files (MPL-2.0 section 3.2) and, if `table.cm` is shipped, its
   GPL-2.0-or-later code.
4. **Notices.** Each library's copyright and licence text ships with APP from vcpkg's
   `share/<port>/copyright`, together with the OCCT notice above.

## Libraries planned by the specification (not licence-verified yet)

Added to `vcpkg.json` in the checklist step that first uses them, after a licence check against spec section 4.3
and an ADR (rule 070):

| Library | Decision | Expected licence |
|---|---|---|
| Eigen | D-05, D-06 | MPL-2.0 |
| Project Chrono | D-06 | BSD-3-Clause |
| Clipper2 | D-08 | BSL-1.0 |
| nlohmann/json | D-09 | MIT |
| lib3mf | D-10 | BSD-2-Clause |
| Intel Embree | D-13 | Apache-2.0 |

## Reused source code

None. When code is reused under CR-04, each project is listed here with its licence, and the details go to
`docs/provenance.md` (CR-07).
