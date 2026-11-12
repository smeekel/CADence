# ADR 0034: Spike SP-1: toolchain and C++23/26 feature matrix {#adr-0034}

- **Status:** Proposed (MSVC column verified; GCC and Clang columns blocked, see "Open items")
- **Date:** 2026-11-12
- **Deciders:** project owner (approves), implementer (proposes)

## Context

Spec L-01 … L-05 allow a C++23/26 feature only when MSVC implements it and current GCC and Clang support it too
(P-05, L-03), with feature-test macros and fallbacks where support may be missing (L-04) and no named modules
(L-05). `docs/cpp-features.md` was written from a Phase 1 MSVC-only probe; its GCC/Clang column holds expected
values from public conformance tables. Step S0.01.2 (spike SP-1b) verifies every row on MSVC v145, GCC 14 and
Clang 18 (WSL). SP-1a (step S0.01.1) verified the build toolchain itself; its record is in ADR 0029.

## Decision

- The probe is `tests/core/spikes/cpp_feature_probe.cpp`: one block per row of `docs/cpp-features.md`, each a small
  program that uses the feature and returns 0 when it behaves as specified. It belongs to no project.
- `scripts/probe-cpp-features.ps1` compiles, links and runs every block with MSVC (`/std:c++latest /permissive-
  /Zc:__cplusplus /Zc:preprocessor /utf-8 /EHsc /W4 /WX`) and, in WSL, with GCC 14 or later (`-std=c++2c -Wall
  -Wextra -Werror`) and Clang 18 or later (`-std=c++2c -stdlib=libc++`). It prints the matrix and writes it as
  Markdown with `-OutFile`.
- **Proposed:** keep `docs/cpp-features.md` as it is. The MSVC results below match every row. The decision is final
  only when the GCC and Clang columns are measured and confirm the "Allowed" rows, or the rows they contradict move
  to "Allowed with a feature-test check and fallback" or "Not allowed" through this ADR.

## Results

Run on 2026-11-12, Visual Studio Community 2026 18.10.1, `cl` 19.51.36257 for x64:

| Feature | Section | MSVC 19.51 | GCC 14 (WSL) | Clang 18 + libc++ (WSL) |
|---|---|---|---|---|
| Concepts and constrained templates | Allowed | yes | not run | not run |
| Ranges (C++20) | Allowed | yes | not run | not run |
| `std::format` | Allowed | yes | not run | not run |
| `std::expected` | Allowed | yes | not run | not run |
| `std::print` / `std::println` | Allowed | yes | not run | not run |
| Explicit object parameter (deducing `this`) | Allowed | yes | not run | not run |
| `std::ranges::to` | Allowed | yes | not run | not run |
| `std::to_chars` / `from_chars` for floating point | Allowed | yes | not run | not run |
| `std::span`, `std::byte`, `std::bit_cast` | Allowed | yes | not run | not run |
| `std::jthread`, `std::stop_token` | Allowed | yes | not run | not run |
| `std::source_location` | Allowed | yes | not run | not run |
| `[[nodiscard]]`, `[[maybe_unused]]`, `[[likely]]` | Allowed | yes | not run | not run |
| `std::views::zip`, `zip_transform`, `enumerate` | Conditional | yes | not run | not run |
| `std::mdspan` | Conditional | yes | not run | not run |
| `std::flat_map` / `std::flat_set` | Conditional | yes | not run | not run |
| Named modules (`import std;`) | Not allowed | no (C2230: could not find module 'std') | not run | not run |
| `std::generator` | Not allowed | yes | not run | not run |
| `std::stacktrace` | Not allowed | yes | not run | not run |
| `std::move_only_function` | Not allowed | yes | not run | not run |
| Pack indexing (`Ts...[N]`) | Not allowed | no (C2760) | not run | not run |
| `_` placeholder variables | Not allowed | no (C2374: redefinition) | not run | not run |
| `= delete("reason")` | Not allowed | no (C2143) | not run | not run |
| Saturation arithmetic (`std::add_sat`) | Not allowed | no (C2039) | not run | not run |
| `std::inplace_vector` | Not allowed | no (C1083: no header) | not run | not run |
| Static reflection | Not allowed | no (C1083: no `<meta>`) | not run | not run |
| Contracts (`pre`, `post`, `contract_assert`) | Not allowed | no (C3646) | not run | not run |

MSVC agrees with `docs/cpp-features.md` on every row: all "Allowed" and "Conditional" features build and run; the
library features listed as "Not allowed" for portability (`std::generator`, `std::stacktrace`,
`std::move_only_function`) do build on MSVC, which is why they are banned for portability rather than for MSVC; the
other "Not allowed" features fail on MSVC as recorded. `import std;` fails without a prebuilt standard-library
module, which L-05 rules out anyway.

## Open items

- **GCC 14 and Clang 18 are not installed.** WSL 2 has Ubuntu 26.04.1 with no C++ compiler; Visual Studio ships only
  `clang-format` and `clang-tidy`, not a Clang compiler. The packages are available from Ubuntu
  (`g++-14` 14.3.0, `clang-18` 18.1.8, `libc++-18-dev` 18.1.8). Installing them is a system change that needs the
  owner's approval:

  ```bash
  sudo apt install g++-14 clang-18 libc++-18-dev libc++abi-18-dev
  ```

  After that, `scripts\probe-cpp-features.ps1 -OutFile out\logs\cpp-feature-probe.md` fills both columns with no
  further changes.
- **Owner acceptance.** S0.01.2 is done only when this ADR is Accepted by the owner (checklist "Done when").

## Consequences

- Until this ADR is Accepted, step S0.01.2 stays in progress and the steps that depend on it (S0.01.5 and, through
  S0.02.7, all of S0.03 onward) cannot start.
- The probe and the script are kept, so the matrix can be re-run whenever a compiler is upgraded (R-09).
- `tests/core/spikes/cpp_feature_probe.cpp` uses banned features on purpose; `scripts/check-cpp-features.ps1`
  (S0.01.5) must exempt it.

## Alternatives considered

- Rely on published conformance tables without compiling: not a verification, and L-03 asks for current support.
- Compiler Explorer or another online compiler: needs network access and is not reproducible on the reference
  machine.
- The Visual Studio "C++ Clang tools for Windows" component: Clang with the MSVC standard library does not exercise
  libstdc++ or libc++, which are what Linux and macOS builds will use.

## Related requirements

P-05, L-01, L-02, L-03, L-04, L-05; ADR 0002, ADR 0029; steps S0.01.1, S0.01.2, S0.01.5; risk R-09.
