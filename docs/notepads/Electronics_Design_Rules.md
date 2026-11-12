# Electronics design rules

Spec: section 11 (E-PRJ, E-LIB, E-SCH, E-SIM, E-PCB, E-RUL, E-3D, E-DOC, E-OUT, E-BOM, E-MB) plus E-MCAD-02 in section 12. Read the spec requirement for full `Accept:` criteria.

## Global behaviour rules

1. **One compiled connectivity model.** The schematic compiles to components, pins, nets, net classes and directives. The board implements that model. Differences are shown as a change list (ECO) the user accepts or rejects item by item, in both directions (E-PRJ-01).
2. **Integer board geometry.** All board coordinates and sizes are `int64_t` nanometres in the board frame (A-09). Conversions to mm happen only in `link` and `render`. DRC uses exact integer geometry; Clipper2 handles polygon booleans and offsets.
3. **Rules are data.** Every routing, placement, DRC and pour decision reads the resolved rule for the objects involved (E-RUL-01). No hard-coded clearances.
4. **Library items are revisioned.** Projects record library ID + revision for every component used and keep a copy in the project scope (F-10).
5. **Variants are first class** (E-PRJ-03): BOM, pick-and-place, assembly drawings, 3D and simulation always use the active variant.
6. **Offline:** no supplier, pricing or stock lookups.

## Project, libraries, reuse (S4)

| ID | Key rules |
|---|---|
| E-PRJ-01 | Project holds schematics, boards, libraries, outputs, links. Compile messages with click-to-locate. Synchronise via change list (component add/remove/modify, footprint change, net add/remove/rename, pin/gate swap, class/rule changes) both ways. Navigator; cross-probe and cross-select across schematic, PCB, 3D. |
| E-PRJ-03 | Named variants: fitted / DNP / alternate component; parameter overrides; affects schematic display, BOM, P&P, assembly drawings, 3D. Linked to mechanical configurations (X-08). (S6) |
| E-PRJ-04 | Reuse blocks (schematic + placed/routed layout), snippets; stored in libraries with revisions; instances record source and revision; re-annotation without clashes. |
| E-LIB-01 | Component = symbol(s) with typed pins and swap groups (should), footprint(s) with padstacks (SMD/THT, shapes incl. custom polygon, per-layer, mask/paste expansion, round/slot, plated/NPTH), silk, courtyard, assembly outline, fiducials, text; 3D model (native part preferred, or STEP) with offset/rotation; SPICE model + pin map; parameters (value, tolerance, ratings, description, manufacturer, MPN, local datasheet path, custom); validated pin–pad map. |
| E-LIB-02 | Library manager across scopes; search by any parameter; revisions; lifecycle (Draft → Released → Deprecated → Obsolete, configurable); released revisions immutable; "newer revision available" + update with preview; where-used across configured folders. |
| E-LIB-03 | Footprint wizard for chip, MELF, SOD, SOT variants, SOIC, SSOP, TSSOP, QFP, QFN/DFN (exposed pad + paste subdivision), BGA (full/perimeter/staggered/depopulated), LGA, PLCC, DPAK/D2PAK, DIP, headers, radial/axial. Pads from toe/heel/side fillet goals + tolerances at least/nominal/most density. **All goal and tolerance tables user-editable; no paid-standard text reproduced** (CR-05). Generates courtyard, silk, assembly outline, pin-1 marker and a parametric native 3D body. |

## Schematic (S4, S9)

| ID | Key rules |
|---|---|
| E-SCH-01 | Sheets with templates; place parts and gates; wires with auto-junctions and rubber-banding; buses (`D[0..7]`) and entries; net labels; power ports bound to net names; sheet ports; no-connect; graphics and images; copy/paste with designator handling; find/replace across sheets. |
| E-SCH-02 | Hierarchical sheet symbols to any depth; flat multi-sheet with ports/global labels; documented, selectable net-scope rules; multi-channel repeat with channel naming for designators/nets and per-channel rooms; signal bundles expanded at each end. |
| E-SCH-03 | User-editable pin-type connection matrix; checks: unconnected inputs, multiple drivers, single-pin nets, undriven power, duplicate designators, conflicting net names, missing footprint/model, unmatched ports, floating labels, off-grid; violation browser; per-violation waivers with comments. |
| E-SCH-04 | Auto-annotate (order pattern; scope project/sheets/selection; keep/reset; channel-aware); back-annotate by board position and pin/gate swaps. |
| E-SCH-05 | Directives on wires/nets/buses/areas: net class, differential pair (auto `_P`/`_N`), match group, impedance profile, rule parameters, ERC suppression; become PCB classes and rules on sync. |
| E-SIM-01 | ngspice (S9): OP, DC (nested), transient, AC, noise, temperature, parameter sweep, Monte Carlo from tolerances; sources DC/pulse/sine/PWL/exp/file-PWL; probes V/I/P; waveform viewer with cursors, measurements, math, CSV; models from library and user files; saved setups; engine errors mapped to schematic objects; variant-aware. |

## PCB layout (S5)

| ID | Key rules |
|---|---|
| E-PCB-01 | Stack manager (copper, core/prepreg with thickness/Dk/Df/material, mask, paste, silk, mechanical); ≥ 64 signal layers (128 should); via spans; impedance profiles (microstrip, embedded microstrip, stripline, coplanar; single-ended and differential) solved with closed-form formulas (field solver: should); total thickness; stack table export. |
| E-PCB-02 | Outline (lines/arcs; or linked from mechanical in S6); cutouts/slots; keepouts per layer and object type; rooms; component height regions (manual or projected from enclosure, X-04); origin; multiple grids (polar: should); units. |
| E-PCB-03 | Placement from sync; move/rotate/flip/snap/align/distribute/arrange in room/cluster; live ratsnest; cross-select; room copy across channels; live placement rules; locks (incl. mechanically driven, X-05). |
| E-PCB-04 | Router modes walk-around, push-and-shove, hug-and-push, ignore (flagged); widths from rules (cycle min/pref/max); vias with layer change while routing; corners 45°/90°/arc/any-angle; loop removal; glossing; multi-track; drag tracks/vias with shove; unroute; auto-complete. P-15. |
| E-PCB-05 | Differential pairs (coupled, gap from rules/impedance, max uncoupled, intra-pair phase); length tuning patterns (accordion/trombone/sawtooth) with amplitude/spacing rules; match groups with tolerance; **signal paths** spanning series parts; length + delay (from Dk) panel; return-path check over plane splits/voids. |
| E-PCB-07 | Net-assigned pours on any copper layer; solid/hatched; rule-based clearance and connect style (direct/thermal/none with spokes count, width, gap, angle); island and neck removal; priority; cutouts; repour on demand (dynamic: should, must by S10); via stitching and shielding. |
| E-PCB-08 | Through/blind/buried/micro vias, stacked/staggered microvias, via-in-pad (fill/cap), tenting; backdrill (from surface to layer, stub rule) in drill outputs; teardrops as regenerable objects; embedded components with automatic cavity; cavities; castellated holes (should). Reflected in DRC, 3D and outputs. |

## Rules and DRC (S5)

**Rule resolution (E-RUL-01):**

- Each rule: `category`, `name`, `priority` (integer, lower = higher priority **(proposed)**), `scope` (one query, or two for pairwise rules such as clearance), `enabled`, `ruleSet`, values (numbers or expressions).
- Query language **(proposed)**, documented in `docs/rules-language.md`, built on the shared expression engine:

```
InNet('VBUS') || InNetClass('Power')
InDiffPair('USB_D') && OnLayer('Top')
IsPad() && HasFootprint('QFN-*')
InRoom('CH1') && !IsVia()
```

- For an object (or pair), the highest-priority enabled rule whose scope matches applies. The UI shows "applied rule" for any object.
- Rule sets group rules (e.g. per fabricator) and import/export as JSON. A constraint grid edits class × class clearance matrices.

**Categories:** electrical (clearance, short, unrouted, unconnected pin), routing (width, via style, layers, corners, diff-pair), SMT (pad entry, neck-down), mask/paste, pour/plane, manufacturing (annular ring, hole size, hole-to-hole, silk-to-mask, silk-to-silk, acute angle, sliver, outline clearance), high-speed (length, matched length, via count, parallel segment, stub), placement (clearance, height, side, orientation), impedance-driven width.

**DRC (E-RUL-02):** online markers while editing; batch report (HTML, JSON, CSV) + violation browser; waivers (comment, author, date); 3D body-to-body clearance and (S6) to enclosure; z-axis copper clearance (should). P-16. Verified by the seeded-violation board suite (T-10) with zero false positives.

## 3D board, documentation and outputs (S5, S7)

| ID | Key rules |
|---|---|
| E-3D-01 | Substrate with true thickness/outline/cutouts; plated holes; mask/silk/copper as textures (thin solids optional); component models from footprints (side, rotation); in-place 2D/3D switch; sections; measure; 3D clearance highlight. Same object as the mechanical board component (X-01). |
| E-DOC-01 | Uses `drw` with PCB view types: board views per layer set, drill drawing + automatic symbols + drill table, stack table, impedance table, dimensions, fab notes, assembly views (variant-aware) with BOM, edge details/sections, isometric. Associative. |
| E-OUT-01 | Gerber X2 per layer + Gerber job file; Excellon (PTH/NPTH split, slots, backdrill) or Gerber X2 drill; IPC-2581 (rev B+); P&P CSV (configurable origin/units/rotation; variant-aware); IPC-D-356. Saved output jobs run from UI or CLI into dated folders with optional zip. ODB++ out of scope. Conformance: re-parsed outputs match database counts and positions exactly. |
| E-BOM-01 | Grouped parts list (item, qty, range-compressed designators, value, footprint, description, manufacturer, MPN, chosen params); variant-aware with DNP list; CSV/JSON; drawing table. |
| E-MB-01 | Multi-board system = assembly with ≥ 2 boards; connector-pair pin mapping (direct or custom); cross-board connectivity compile; mismatch report (pin map, net names, unconnected mates, misaligned 3D mates). (S6) |
| E-MCAD-02 | Export board or product as STEP AP214/242 (colours, names, structure), IDF 3.0, STL/3MF fit-check; IDX should. (S6/S7) |
