# ADR 0002: C++26 for core and UI code (D-01) {#adr-0002}

- **Status:** Accepted (spec decision D-01)
- **Date:** 2026-11-11

## Context

The geometry kernel (OCCT), physics engine (Chrono) and SPICE engine (ngspice) that APP builds on are C/C++. Spec
D-01 and P-05 choose C++26 for all core and UI code, within the language policy L-01 … L-05: compile with
`/std:c++latest`, use only features MSVC implements (L-02) that GCC and Clang also support (L-03), guard optional
features with feature-test macros (L-04), and do not use named modules (L-05).

## Decision

- All APP code is C++26 compiled with `/std:c++latest` (set once in `build/props`). Switch to `/std:c++26` when MSVC
  provides it.
- Permitted C++23/26 features are listed in `docs/cpp-features.md`. Adding a feature requires an ADR; the first
  revision of the list comes from spike SP-1 (ADR 0034, step S0.01.2).
- C# is not used for the core.

## Consequences

- Wrapping C/C++ libraries is direct; no interop layer.
- The portable subset is narrower than what MSVC alone allows; `scripts/` static checks (S0.01.5) flag disallowed
  features.
- Applied in S0.01.2 (feature matrix) and S0.01.5 (allow-list check).

## Alternatives considered

- C# core with native interop: rejected by D-01 because wrapping OCCT, Chrono and ngspice costs more than it saves.
- C++20 only: would forgo `std::expected`, `<print>` and deducing `this`, which MSVC, GCC and Clang already support.

## Related requirements

D-01, P-05, L-01 … L-05; ADR 0034; steps S0.01.2, S0.01.5, S0.01.6.
