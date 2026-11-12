# Permitted C++23/26 features {#cpp-features}

APP compiles with `/std:c++latest` (spec L-01) and uses a C++23/26 feature only when MSVC implements it (L-02) and
current GCC and Clang support it too (L-03). Where support may be missing on another compiler, code checks the
feature-test macro and provides a fallback (L-04); `src/core/Compat.h` (step S0.01.5) collects these helpers. Named
modules are not used (L-05). Adding a feature to this list, or moving one between sections, requires an ADR.

**Status of this list.** Initial version from the Phase 1 MSVC probe (VS 2026, MSVC 19.51, `/std:c++latest`,
`__cplusplus` = 202400). GCC and Clang status is **pending spike SP-1** (step S0.01.2, ADR 0034): no native Clang or
GCC is installed yet, so the "GCC / Clang" column records the expected status from public conformance tables and must
be confirmed by compiling a probe (for example in WSL 2). `scripts/check-cpp-features.ps1` (S0.01.5) fails the build
on items in "Not allowed".

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

## Allowed with a feature-test check and fallback (L-04)

| Feature | Header | Macro | Why conditional |
|---|---|---|---|
| `std::views::zip`, `zip_transform`, `enumerate` | `<ranges>` | `__cpp_lib_ranges_zip`, `__cpp_lib_ranges_enumerate` | Present on MSVC; library support on libc++ arrived late |
| `std::mdspan` | `<mdspan>` | `__cpp_lib_mdspan` | Present on MSVC; GCC support is recent |
| `std::flat_map` / `std::flat_set` | `<flat_map>`, `<flat_set>` | `__cpp_lib_flat_map` | Present on MSVC; GCC and libc++ support is recent |

## Not allowed (checked by `scripts/check-cpp-features.ps1`)

| Feature | Reason |
|---|---|
| Named modules (`import`, `export module`, `import std;`) | L-05: not reliable across compilers and the future build system |
| `std::generator` (`<generator>`) | Available on MSVC but not portable enough yet; revisit with an ADR |
| `std::stacktrace` (`<stacktrace>`) outside `src/platform/` | Limited portability; crash reports use `platform` (minidumps) |
| `std::move_only_function` | Uneven library support; revisit with an ADR |
| Pack indexing (`Ts...[N]`) | Not implemented by MSVC (probe) |
| `_` placeholder variables | Not implemented by MSVC (probe) |
| `= delete("reason")` | Not implemented by MSVC (probe) |
| Saturation arithmetic (`std::add_sat` …) | Not implemented by MSVC (probe) |
| `std::inplace_vector` | Not implemented by MSVC (probe) |
| Static reflection | Not implemented by MSVC (probe); L-02 forbids requiring it |
| Contracts (`pre`, `post`, `contract_assert`) | Not implemented by MSVC (probe) |

## Change log

| Date | Change | ADR |
|---|---|---|
| 2026-11-11 | Initial list from the Phase 1 MSVC probe; GCC/Clang to be confirmed | 0034 (to be written in S0.01.2) |
