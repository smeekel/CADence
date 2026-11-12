# ADR 0012: GoogleTest and GoogleMock (D-11) {#adr-0012}

- **Status:** Accepted (spec decision D-11)
- **Date:** 2026-11-11

## Context

Spec D-11 chooses GoogleTest (BSD-3-Clause) with GoogleMock because Visual Studio's Test Explorer runs GoogleTest
projects through its included adapter, which suits a solution-based build without CMake/CTest.

## Decision

- One GoogleTest executable per module, `tests/<module>/<Module>.Tests.vcxproj` (C-14), plus `Format.Tests`,
  `Golden.Tests`, `Integration.Tests`, `Ui.Tests`, benchmark (`*.Bench.exe`) and fuzz (`*.Fuzz.exe`) executables.
- gtest is linked as a DLL from vcpkg (`x64-windows`, `GTEST_LINKED_AS_SHARED_LIBRARY=1`), as the scaffold does.
- `scripts/test.ps1` discovers `*.Tests.exe`, writes JUnit XML to `out/test-results/<Configuration>/`, and offers
  `-Filter`, `-Coverage`, `-Bench`, `-Fuzz` and `-UpdateGolden -Reason`.
- Tests that encode spec Accept criteria carry `// Accept: <ID>` comments (rule 060).

## Consequences

- Test Explorer and the command line run the same tests.
- Slow tests are named `*_Slow` and run at stage gates.

## Alternatives considered

- Catch2 and doctest: weaker Visual Studio integration out of the box.
- Microsoft C++ Unit Test Framework: Windows-only, conflicts with P-04.

## Related requirements

D-11, T-01 … T-13; steps S0.08.1 … S0.08.6.
