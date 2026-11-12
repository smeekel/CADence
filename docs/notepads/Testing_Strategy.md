# Testing strategy

Spec: section 7 (T-01 … T-14), P-1x targets, every `Accept:` line in sections 10–13.

## Layout

```
tests/
  <module>/<Module>.Tests.vcxproj     GoogleTest unit + module integration tests (T-01)
  integration/Integration.Tests.vcxproj  command-script tests through cli (T-02)
  format/Format.Tests.vcxproj        round-trip, magic, migration tests (T-03)
  golden/Golden.Tests.vcxproj        geometry/netlist/output golden tests (T-04, T-07)
  ui/Ui.Tests.vcxproj                off-screen replay + snapshots (T-05, T-06)
  bench/Bench.vcxproj                benchmarks (T-08)
  fuzz/                              fuzz targets (T-09) - Clang/libFuzzer; may run under WSL until a cross-platform build exists
  data/
    scripts/*.cmd.json               command scripts that build golden models
    expected/*.expected.json         expected values (mass props, counts, netlists, DOF…)
    format/                          fixtures for every formatVersion (migrations)
    import/                          third-party STEP/IGES/DXF test files (with licence notes)
    snapshots/                       reference images for T-06
```

## Categories and what they prove

| ID | Category | Proves | Typical assertion |
|---|---|---|---|
| T-01 | Unit | Each class/function works in isolation | Solver converges, expression evaluates, rule resolves |
| T-02 | Integration | Commands compose correctly headless | Script → model state matches expected JSON |
| T-03 | Format round-trip | Encodings are lossless and deterministic | `json→bson→json` byte-identical; magic bytes exact |
| T-04 | Geometry golden | Modeling results are correct and stable | Volume, area, CoM, bbox, topology counts, DOF within tolerance |
| T-05 | UI replay | UI drives commands correctly | Journal + Qt events → expected model and widget state |
| T-06 | Visual snapshot | Rendering hasn't regressed | SSIM ≥ 0.98 vs reference |
| T-07 | Output conformance | Generated files are valid and faithful | Re-parsed Gerber/drill/IPC-2581/STEP/3MF match database |
| T-08 | Benchmarks | P-10…P-17 targets met | Median of N runs under target |
| T-09 | Fuzz | Loaders/parsers never crash | No crash/hang in nightly budget |
| T-10 | Domain verification | Engines are numerically right | Analytic benchmarks: sketch, motion (0.5 %), SPICE (1 %), DRC (exact), stack-up |
| T-11 | Static checks | Repo rules hold | Warnings, layering, network ban, SPDX, licences |
| T-12 | Coverage | Enough of the code is exercised | ≥ 80 % lines for core, doc, sketch, pcb rules/DRC |
| T-13 | One command | Everyone runs the same suite | `scripts/test.ps1` |
| T-14 | Reference scenario | The product goal works end to end | Scenario in `MCAD_ECAD_Integration_Rules.md` passes |

## Rules

- **Accept → test.** Each `Accept:` criterion in the spec maps to one or more named tests, annotated `// Accept: <ID>` and listed in `docs/traceability.csv`.
- **Golden data from scripts.** Golden models are built by command scripts, never hand-edited binaries. Regenerate expected outputs only with `scripts/test.ps1 -UpdateGolden -Reason "<why>"`.
- **Tolerances explicit.** Default relative 1e-6 for geometry; state others in the test.
- **Determinism.** Fixed seeds; no clock dependence; temporary directories; no network.
- **Independent oracles.** Expected values come from analytic formulas, independent calculations in the test, or hand-checked references, never from the code under test.
- **Speed.** Unit tests < 1 s each; the full T-13 suite should stay under ~10 minutes. Tag slow tests `*_Slow` and run them in the stage gate.
- **Never weaken.** Do not skip, disable or loosen a failing test to pass a step.

## Test oracles to build early (S0–S1)

- Gerber X2 / Excellon / IPC-D-356 readers (test-side) for T-07. They later become the basis of the CAM viewer (AD-OUT-03).
- Mesh validator (watertight, manifold, normals) for M-3DP-01.
- Analytic mass-property functions for primitives (box, cylinder, cone, sphere, torus).
- SPICE analytic references (RC, RLC, diode, op-amp gain).
- Motion analytic references (pendulum, slider-crank, free fall, spring-mass).
- Seeded-violation board generator for DRC (T-10).

## Running

- Visual Studio: Test Explorer discovers GoogleTest executables through the bundled adapter.
- Command line: `pwsh scripts/test.ps1` (builds `Headless`, runs all `*.Tests.exe` with `--gtest_output=xml:out/test-results/<name>.xml`).
- Switches: `-Configuration`, `-Filter <gtest filter>`, `-Bench`, `-Fuzz`, `-UpdateGolden -Reason "…"`, `-Coverage` (uses the Visual Studio code coverage tooling where available).
