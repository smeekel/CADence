# Notepads index

These notepads split the specification (`docs/spec/cad-functional-requirements.md`, v0.2) into focused context packs. @-mention only the ones a task needs, to keep the agent's context small. **The spec is authoritative**; notepads summarise it and add implementation-level contracts the spec leaves open. Where a notepad adds a design choice not found in the spec, it is marked **(proposed)**; the planning session should confirm or replace it with an ADR.

| Notepad | Use when working on | Spec sections |
|---|---|---|
| `Architecture_and_Build.md` | Modules, layering, solution/projects, dependencies, threading | 3, 4, 5 |
| `Data_Model_and_File_Format.md` | Document model, IDs, JSON/BSON, schemas, magic, migrations | 6 |
| `Command_Bus_and_Module_Contracts.md` | Commands, undo/redo, errors, events, module interfaces | 5 (A-02, A-03, A-05, A-07, A-11) |
| `Mechanical_Modeling_Rules.md` | Sketcher, parts, assemblies, drawings, sheet metal, weldments, motion, tolerance, rendering, inspection | 10, 13 |
| `Electronics_Design_Rules.md` | Libraries, schematic, ERC, SPICE, PCB, rules, DRC, outputs, BOM | 11 |
| `MCAD_ECAD_Integration_Rules.md` | Links between board and mechanical model; the reference scenario | 1, 12 |
| `Libraries_and_Seed_Data.md` | Global/user/project libraries, seed content, data sources | F-07, F-10, CR-05 |
| `UI_UX_Guidelines.md` | Shell, panels, theming, interaction design | 8 |
| `Testing_Strategy.md` | Test harness, golden data, acceptance tests, benchmarks | 7 |
| `Licensing_and_Provenance.md` | GPL, SPDX, reuse of outside code, clean-room rules | 2, 4.3 |
| `Code_Style_and_Documentation.md` | Formatting, naming, Doxygen | (owner's style guide) |
| `Stage_Roadmap.md` | Stage order, exit criteria, spikes, future seams | 9, 14 |

Typical combinations:

- **Planning session:** all notepads.
- **A sketch-solver step:** Mechanical_Modeling_Rules + Command_Bus + Testing_Strategy.
- **A PCB routing step:** Electronics_Design_Rules + Command_Bus + Testing_Strategy.
- **A file-format change:** Data_Model_and_File_Format + Testing_Strategy.
- **A UI panel:** UI_UX_Guidelines + Command_Bus.
