# MCAD ↔ ECAD integration rules

Spec: section 1 (reference scenario), section 12 (X-01 … X-10, E-MCAD-02), M-ASM-02, A-07. This is APP's main differentiator: the board and the enclosure are one parametric model, not two programs exchanging files.

## Link model

A **link** connects a quantity owned by one domain (the **driver**) to a quantity in the other domain (the **driven**).

```json
{
  "id": "<ObjectId>",
  "name": "Enclosure cavity follows board outline",
  "driver": { "document": "<board id>", "object": "outline", "quantity": "Outline.Curves" },
  "driven": { "document": "<part id>", "object": "<sketch id>", "quantity": "ProjectedCurves" },
  "mapping": { "expr": "offset( driver, 0.5 mm )" },
  "mode": "live | review | locked",
  "lastAcknowledgedValue": { },
  "status": "ok | outOfDate | broken | locked | cycle"
}
```

Rules:

1. **Exactly one driver per link.** A quantity can be driven by at most one link.
2. Links are **regeneration-graph nodes** (A-07). Cycles across documents are rejected at creation (X-10).
3. **Units cross at the link:** board int64 nm ↔ mechanical mm double. Conversion happens only in `link/`.
4. Modes: `live` (default) propagates on regeneration; `review` holds the driven value until the user accepts (X-09); `locked` freezes the last value.
5. A broken link never deletes geometry. The driven feature goes to `Error` with "link broken" (P-21).
6. Every value change since the last acknowledgement is listed in the **cross-domain changes** panel (X-09).
7. Link targets use `TopoRef` and `ObjectId`s, never names.

## Linkable quantities (minimum set)

| Quantity | Board side | Mechanical side | Requirement |
|---|---|---|---|
| Board placement in assembly | Board faces, hole axes, component frames as mate references | Mates | X-01 |
| Board outline + cutouts | `outline`, `cutouts[]` | Sketch curves / projected face boundary / offset wall | X-02 |
| Board size parameters | `Board.Width`, `Board.Height`, `Board.Thickness` | Parameters and expressions | X-02 |
| Mounting holes | Hole position, diameter, fastener metadata | Bosses, standoffs, insert pilots, clearance holes | X-03 |
| Height limits and keepouts | Height regions, keepouts | Projected enclosure interior (with clearance expression) | X-04 |
| Constrained components | Component position, rotation, side | Mechanical reference (e.g. opening centre) or component-driven cut feature | X-05 |
| Variants ↔ configurations | Active variant, DNP set | Assembly configuration, feature suppression | X-08 |

## Behaviours

- **X-01** Boards are assembly components; their 3D (E-3D-01) is their geometry.
- **X-02** Outline can be mechanical-driven or board-driven; either direction works; DRC reports components left outside a shrunken board.
- **X-03** Moving a board hole moves the linked boss (and vice versa). From S10, auto-fastening (M-ASM-05) uses linked holes.
- **X-04** Height regions = distance from board surface to nearest enclosure face above/below, minus clearance; keepouts under ribs, bosses and snaps; auto-updated; used by placement rules and DRC.
- **X-05** Mechanically constrained components are locked in the PCB editor. The **component-driven cut** feature subtracts the component body or its library "mating envelope" body plus a clearance from enclosure walls.
- **X-06** Boards and components join interference/clearance checks; PCB DRC can include enclosure bodies; violations cross-probe between 3D, PCB and the mechanical tree.
- **X-07** Assembly BOMs can include board components (indented or flattened) with mechanical parts and fasteners (via `drw::IBomProvider`).
- **X-08** Configurations select variants; DNP parts hidden and excluded from interference; features can depend on variant.
- **X-09** Changes panel; review mode per link or per project.
- **X-10** Link manager panel: list, create, edit, re-target, lock, unlock, break, reverse.

## Reference scenario (T-14, release gate from S6)

Build this as `tests/data/scripts/reference_scenario.cmd.json` and keep it green in every later stage:

1. Create a 50 × 40 mm, 1.6 mm, 2-layer board with a USB-C connector on one edge, four M3 mounting holes and a tall electrolytic capacitor (12 mm).
2. Create an enclosure base and lid (two bodies via split, M-PRT-05; or two parts) whose cavity is linked to the board outline + 0.5 mm, bosses linked to the mounting holes, a connector cut-out driven by the USB-C mating envelope + 0.3 mm, and lid height linked to the tallest component + 1 mm.
3. Mate the board into the base on two hole axes and the bottom face (DOF 0).
4. Check: interference-free; DRC clean.
5. Change `Board.Width` 50 → 60 mm. Expect: cavity and bosses update, still interference-free, DRC clean.
6. Move the connector 5 mm along the edge. Expect: cut-out moves 5 mm, clearance 0.3 mm kept.
7. Lower the lid by 3 mm (break the height link or override it). Expect: assembly interference and PCB DRC both report the capacitor clash.
8. Export Gerber/drill/IPC-2581 and enclosure 3MF. Expect: outputs regenerate and validate (T-07), and enclosure meshes are watertight.
9. Switch to the "no USB" configuration. Expect: connector hidden, cut-out suppressed.
