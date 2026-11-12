# Permitted C++23/26 features {#cpp-features}

APP compiles with `/std:c++latest` (spec L-01) and uses a C++23/26 feature only when MSVC implements it (L-02) and
current GCC and Clang support it too (L-03). Where support may be missing on another compiler, code checks the
feature-test macro and provides a fallback (L-04); `src/core/Compat.h` (step S0.01.5) collects these helpers. Named
modules are not used (L-05). Adding a feature to this list, or moving one between sections, requires an ADR.

**Status of this list.** Verified on MSVC 19.51 (VS 2026, `/std:c++latest`, `__cplusplus` = 202400) by spike SP-1
(step S0.01.2, ADR 0034, Accepted). By owner decision APP is built and tested only with Visual Studio on Windows 11
for now, so the "GCC / Clang" column still records the expected status from public conformance tables; it is
measured with `scripts/probe-cpp-features.ps1` when other platforms come into scope. The list stays conservative in
the meantime. `scripts/check-cpp-features.ps1` (S0.01.5) fails the build on items in "Not allowed".

## Allowed

| Feature | Header / form | Feature-test macro | MSVC (probe) | GCC / Clang (to confirm in SP-1) |
|---|---|---|---|---|
| Concepts and constrained templates | `<concepts>`, `requires` | `__cpp_concepts` | yes | yes |
| Ranges (C++20) | `<ranges>` | `__cpp_lib_ranges` | yes | yes |
| `std::format` | `<format>` | `__cpp_lib_format` | yes | yes |
| `std::expected` | `<expected>` | `__cpp_lib_expected` | yes | yes (GCC 12+, Clang 16+ with libc++ 17) |
| `std::print` / `std::println` | `<print>` | `__cpp_lib_print` | yes | yes (GCC 14+, libc++ 18+) |
| Explicit object parameter ("deducing `this`") | `this auto& self` | `__cpp_explicit_this_parameter` | yes | yes (GCC 14+, Clang 18+) |
| `std::ranges::to` | `<ranges>` | `__cpp_lib_ranges_to_container` | yes | yes (GCC 14+, libc++ 17+) |
| `std::to_chars` / `from_chars` for floating point | `<charconv>` | `__cpp_lib_to_chars` | yes | yes |
| `std::span`, `std::byte`, `std::bit_cast` | `<span>`, `<cstddef>`, `<bit>` | `__cpp_lib_span`, `__cpp_lib_bit_cast` | yes | yes |
| `std::jthread`, `std::stop_token` | `<thread>`, `<stop_token>` | `__cpp_lib_jthread` | yes | yes (libc++ 18+) |
| `std::source_location` | `<source_location>` | `__cpp_lib_source_location` | yes | yes |
| `[[nodiscard]]`, `[[maybe_unused]]`, `[[likely]]` | attributes | n/a | yes | yes |

`src/core/Compat.h` stops the build with an `#error` when one of these feature-test macros is missing or older than
the version APP relies on.

## Allowed with a feature-test check and fallback (L-04)

Code uses these features only through the wrappers in `src/core/Compat.h`, which pick the standard facility when its
feature-test macro is defined and a tested fallback otherwise. The check fails on any direct use outside
`src/core/Compat.h`. A feature without a wrapper yet (`std::mdspan`) gets one, with a fallback and tests, in the step
that first needs it.

| Feature | Header | Macro | Why conditional | Use through | Detected by (regular expressions) |
|---|---|---|---|---|---|
| `std::views::zip`, `zip_transform`, `enumerate` | `<ranges>` | `__cpp_lib_ranges_zip`, `__cpp_lib_ranges_enumerate` | Present on MSVC; library support on libc++ arrived late | `compat::zip`, `compat::zipTransform`, `compat::enumerate` | `\bviews::zip\b`, `\bviews::zip_transform\b`, `\bviews::enumerate\b` |
| `std::mdspan` | `<mdspan>` | `__cpp_lib_mdspan` | Present on MSVC; GCC support is recent | no wrapper yet | `#\s*include\s*<mdspan>`, `\bstd::mdspan\b` |
| `std::flat_map` / `std::flat_set` | `<flat_map>`, `<flat_set>` | `__cpp_lib_flat_map`, `__cpp_lib_flat_set` | Present on MSVC; GCC and libc++ support is recent | `compat::FlatMap`, `compat::FlatSet` | `#\s*include\s*<flat_map>`, `#\s*include\s*<flat_set>`, `\bstd::flat_\w+` |

## Not allowed (checked by `scripts/check-cpp-features.ps1`)

| Feature | Reason | Allowed in | Detected by (regular expressions) |
|---|---|---|---|
| Named modules (`import`, `export module`, `import std;`) | L-05: not reliable across compilers and the future build system | | `^\s*(export\s+)?import\s+[\w<":]`, `^\s*(export\s+)?module\s*([\w.:]+\s*)?;` |
| `std::generator` (`<generator>`) | Available on MSVC but not portable enough yet; revisit with an ADR | | `#\s*include\s*<generator>`, `\bstd::generator\b` |
| `std::stacktrace` (`<stacktrace>`) outside `src/platform/` | Limited portability; crash reports use `platform` (minidumps) | `src/platform/` | `#\s*include\s*<stacktrace>`, `\bstd::(basic_)?stacktrace\b` |
| `std::move_only_function` | Uneven library support; revisit with an ADR | | `\bstd::move_only_function\b` |
| Pack indexing (`Ts...[N]`) | Not implemented by MSVC (probe) | | `\.\.\.\s*\[` |
| `_` placeholder variables | Not implemented by MSVC (probe) | | `(?<![\w$])_(?![\w$])` |
| `= delete("reason")` | Not implemented by MSVC (probe) | | `=\s*delete\s*\(` |
| Saturation arithmetic (`std::add_sat` …) | Not implemented by MSVC (probe) | | `\bstd::\w+_sat\b`, `\bstd::saturate_cast\b` |
| `std::inplace_vector` | Not implemented by MSVC (probe) | | `#\s*include\s*<inplace_vector>`, `\bstd::inplace_vector\b` |
| Static reflection | Not implemented by MSVC (probe); L-02 forbids requiring it | | `#\s*include\s*<meta>`, `\^\^` |
| Contracts (`pre`, `post`, `contract_assert`) | Not implemented by MSVC (probe) | | `\bcontract_assert\b`, `\)\s*(const\s*)?(noexcept\s*)?(override\s*)?(pre\|post)\s*\(` |

### How the check reads these tables

`scripts/check-cpp-features.ps1` reads the last column of both tables above. Each code span in it is a .NET regular
expression matched against every line of the C++ files under `src/` and `tests/`, after comments and the contents
of string and character literals are blanked (so a comment or a message may name a banned feature). A `|` inside a
pattern is written `\|` so that it does not split the table cell. "Allowed in" lists repository path prefixes where
the row does not apply; rows of the conditional table apply everywhere except `src/core/Compat.h`. The spike probe
`tests/core/spikes/cpp_feature_probe.cpp` uses banned features on purpose and is the only file the check skips
(ADR 0034).

## Change log

| Date | Change | ADR |
|---|---|---|
| 2026-11-11 | Initial list from the Phase 1 MSVC probe; GCC/Clang to be confirmed | 0034 (to be written in S0.01.2) |
| 2026-11-12 | MSVC 19.51 column re-verified row by row with `scripts/probe-cpp-features.ps1` (no change); GCC/Clang still pending because no compiler is installed in WSL | 0034 (Proposed) |
| 2026-11-12 | Owner decision: Visual Studio on Windows 11 only for now; GCC/Clang measurement deferred, list unchanged | 0034 (Accepted) |
| 2026-11-12 | Step S0.01.5: detection patterns added for `scripts/check-cpp-features.ps1`; conditional features are used only through `src/core/Compat.h`; no feature moved between sections | 0034 (Accepted) |
