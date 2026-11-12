# Stage roadmap

Spec: section 9 (stages), section 14 (future), section 15 (traceability). Each stage ends with green tests, updated traceability and a human review checkpoint. The reference scenario (T-14) must pass from S6 onward.

| Stage | Name | Requirements | Exit criteria (summary; full text in spec section 9) |
|---|---|---|---|
| S0 | Foundation | P-*, A-01…A-11, B-01…B-07, L-01…L-05, F-01…F-10, T-01…T-13 scaffolding, U-01…U-06, U-08/U-09 basic, U-10, U-11, CR-01…CR-07 | Clean-clone build in VS 2026; themed shell + viewport with kernel box; box → JSON → BSON → STL round-trip test with magic checks; library scopes resolve; CLI runs scripts headless |
| S1 | Sketcher | M-SKT-01…04 | 10 golden profiles fully constrained; conflict diagnosis; P-13; global parameters drive dimensions |
| S2 | Part modeling core | M-PRT-01…04, M-PRT-09, M-PRT-10, M-IO-01 (STEP), M-3DP-01 (basic) | 30 golden parts' mass properties; correct re-referencing after early edits (A-06) |
| S3 | Assemblies core | M-ASM-01, 02, 04, 06, 07, M-PRT-11 | Golden mechanism DOF; in-context update; interference finds seeded clashes; exploded/animation export |
| S4 | Libraries + schematic | E-PRJ-01, E-PRJ-04, E-LIB-01…03, E-SCH-01…05 | 4-sheet hierarchical netlist; ERC seeded errors; 20 footprint cases |
| S5 | PCB layout | E-PCB-01…05, 07, 08, E-RUL-01, 02, E-3D-01 | Golden board placed/routed by replay; DRC exact; 3D matches stack |
| S6 | MCAD↔ECAD | X-01…X-10, E-MB-01, E-PRJ-03, E-MCAD-02 | **Reference scenario passes end to end** |
| S7 | Docs + manufacturing | M-DRW-01…03, E-DOC-01, E-OUT-01, E-BOM-01, M-IO-01 full, M-3DP-01 full, X-07 | Drawing snapshots; Gerber/drill/IPC-2581 conformance; watertight 3MF |
| S8 | Advanced modeling | M-PRT-05, 06, 07, M-SHM-01, 02, M-WLD-01, 02 | Flat lengths analytic; cut list; surfacing golden |
| S9 | Simulation | M-SIM-04, M-SIM-05, E-SIM-01 | T-10 motion, tolerance, SPICE suites |
| S10 | Scale + polish | M-ASM-03, M-ASM-05, M-VIS-01, M-INS-01, U-07…09, U-12…15 complete | P-14; auto-fasteners; render snapshots; inspection export |

## Required spikes (time-boxed, each ends with an ADR before dependent work)

| Spike | When | Question to answer | Feeds |
|---|---|---|---|
| SP-1 Toolchain | S0 start | Do VS 2026 + vcpkg (manifest) + Qt 6 + OCCT + GoogleTest build and run together with `/std:c++latest`? Which C++26 features are usable on MSVC, GCC and Clang today? | B-*, L-*, `docs/cpp-features.md` |
| SP-2 Topological naming | S0 end / S1 | Which `TopoRef` scheme (role + index + signature, or OCCT naming services) survives the 20 breakage cases in A-06? | A-06, M-PRT-* |
| SP-3 Sketch solver | S1 start | Decomposition + LM solver meets P-13 and reports minimal conflict sets? | M-SKT-02 |
| SP-4 Mate solver | S3 start | Rigid-cluster solver handles the 15 golden mechanisms with correct DOF and drag stability? | M-ASM-01, M-ASM-07 |
| SP-5 Router | S5 start | Push-and-shove approach (fresh design, or adapted from a compatible open project under CR-07) meets P-15? | E-PCB-04, E-PCB-05 |
| SP-6 Pours | S5 | Clipper2-based pour engine: robustness and speed on a dense board? | E-PCB-07 |
| SP-7 Links | S6 start | Link evaluation inside the regeneration graph across documents, with review mode, without cycles? | X-* |
| SP-8 Large assemblies | S3 (early measure) / S10 | Lightweight loading and instancing meet P-14 and P-11? | M-ASM-03 |

## Future (spec section 14): seams built now

| Future item | Seam and where it is built |
|---|---|
| SW-SIM-01, SW-SIM-02, SW-SIM-03 FEA | Material model with mechanical/thermal properties (S2); generic mesh + result-field viewer interface in `sim` (S9) |
| SW-RTE-01 piping; SW-RTE-02 + AD-HAR-01 harness | 3D sketches (S1/S8); connector pin data in library (S4); multi-board connectivity (S6) |
| SW-CAM-01 CAM | Semantic hole/feature metadata (S2) |
| SW-PDM-02 workflow | Library lifecycle state machine reusable for documents (S4) |
| SW-API-01 + AD-API-01 | Stable command names and schemas from S0 |
| AD-PCB-06 autorouting | Router path-finding and shove as services (S5) |
| AD-PCB-09 rigid-flex | Board regions referencing stack definitions (S5) |
| AD-PCB-10 wire bonding / printed electronics | Layer model allows non-copper conductive and print layers (S5) |
| AD-SI-01 / AD-PI-01 | Stack + impedance model (S5); copper geometry export per net (S5) |
| AD-OUT-03 CAM viewer | Test-side Gerber/Excellon/IPC-2581 readers (S0–S7) |
