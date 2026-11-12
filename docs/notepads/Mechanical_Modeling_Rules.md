# Mechanical modeling rules

Spec: section 10 (M-SKT, M-PRT, M-SHM, M-WLD, M-ASM, M-DRW, M-SIM, M-INS, M-VIS) and section 13 (M-IO-01, M-3DP-01). Read the spec requirement for the full `Accept:` criteria before implementing a step.

## Global behaviour rules

1. **History is the source of truth.** A part is its ordered feature list; geometry is derived by regeneration (A-07). Edits change inputs, never cached geometry.
2. **Failures never destroy data** (P-21). A failing feature goes to `Error` with a message, keeps its definition, and leaves downstream features `OutOfDate` (or rebuilt against the last valid upstream state if possible).
3. **References are persistent** (A-06). Faces, edges and vertices are referenced through `TopoRef`. If re-matching is ambiguous or fails, report "reference lost" and highlight; never silently pick another entity.
4. **Every number is an expression** (A-08) with units. Dimensions can reference parameters (`Board.Width`), other named dimensions, and (from S6) linked values.
5. **Kernel isolation:** all geometry operations go through `geom::Kernel`; feature code never includes OCCT headers.
6. **Determinism:** regenerating identical inputs yields identical topology order and `TopoRef` resolution.

## Sketcher (S1)

| ID | Key rules |
|---|---|
| M-SKT-01 | Entities: point, line, construction line, arcs (centre/3-point/tangent), circle, ellipse, partial ellipse, splines (interpolating, control-point), slots, rectangles, polygon, text→outline. Tools: trim (incl. drag-trim), extend, split, offset, mirror, linear/circular pattern, fillet, chamfer, project edges, intersection curves, construction toggle. Automatic closed-region detection, incl. nested and touching contours; per-region selection. |
| M-SKT-02 | Constraints: coincident, horizontal, vertical, parallel, perpendicular, tangent, G2, concentric, coradial, equal, collinear, midpoint, symmetric, fix, on-curve, pierce, merge. Dimensions: linear (aligned/H/V), angular, radius, diameter, arc length, point-to-curve; driving or driven. Auto-inference while drawing (suppressible). Colour by state (under/fully/over/unsolvable) and show DOF. Over-definition → minimal conflicting set + offer delete or convert to driven. Interactive drag solve, < 16 ms at 500/800 (P-13). |
| M-SKT-03 | 3D sketch: points, lines, arcs, splines, projected curves; constraints along X/Y/Z, parallel to plane, coincident, tangent, perpendicular, equal, fixed; dimensions. Used by sweeps, lofts, structures and future routing. |
| M-SKT-04 | Parameters at part, assembly and project scope, in a table (value, expression, unit, scope, where used, status). Cycles rejected naming all parameters involved. CSV/JSON import/export. |

**Solver design rules (D-05):** graph-decompose the constraint system into independently solvable clusters; Newton with Levenberg–Marquardt damping; numerical rank analysis for DOF and redundancy; drag uses a soft "follow the cursor" constraint; never move fixed or fully-defined geometry during a drag.

## Part modeling (S2, S8)

| ID | Key rules |
|---|---|
| M-PRT-10 | Reference planes (offset, angle, 3-point, normal-to-curve, mid-plane, tangent), axes, points, coordinate systems. Parametric, in the tree. |
| M-PRT-01 | Extrude (blind, through all, up to next/vertex/surface, offset from surface, mid-plane, 2 directions, draft, thin, start offset); revolve; sweep (guides, twist); loft (guides, centreline, end conditions); boundary. Boss or cut; new body or merge; feature scope. |
| M-PRT-02 | Fillet (constant, variable, face, full-round; setback); chamfer (d-d, angle-d, vertex); shell (per-face thickness); draft (neutral plane, parting line); rib; hole tool (simple, counterbore, countersink, counterdrill, tapped, pipe-tapped) from editable JSON tables; cosmetic thread metadata (designation, length, class) used by drawings and fasteners. Modelled threads: should. |
| M-PRT-03 | Linear (2-dir, skip), circular, curve-driven, sketch-driven, table-driven (CSV), variable pattern; mirror features/faces/bodies; "geometry pattern" fast mode. |
| M-PRT-04 | Feature tree: edit, rename, suppress, delete with dependants (list + cascade), reorder with dependency validation, folders, rollback bar, state icons, "what's wrong" panel, parent/child view. |
| M-PRT-05 | Multibody: combine (add/subtract/common), split (save bodies to new parts), move/copy, delete/keep, folders, derived (linked) part insertion. |
| M-PRT-06 | Surfaces: extruded, revolved, swept, lofted, boundary, filled, offset, ruled, planar, mid-surface (should). Ops: knit (+form solid), trim (standard/mutual), untrim, extend, thicken, delete face, replace face, cut with surface. Zebra/curvature analysis: should. |
| M-PRT-07 | Direct edits on native or imported bodies (move/rotate face, offset face, delete face + heal, replace face, resize fillet), recorded as parametric features. |
| M-PRT-09 | Material library (density, E, ν, yield, UTS, k, cp, α, appearance); assign per part/body; mass properties (mass, volume, area, CoM, inertia tensor, principal axes) about a chosen frame; mass override; AABB and minimum OBB exposed as parameters. |
| M-PRT-11 | Configurations vary parameters, suppression, material, properties; for assemblies component suppression/visibility/configuration and mate suppression. In-app configuration table with CSV import/export. Derived configurations: should. Link to board variants in S6 (X-08). |

## Sheet metal and weldments (S8)

| ID | Key rules |
|---|---|
| M-SHM-01 | Per-body parameters: thickness, bend radius, bend method (K-factor, allowance, deduction, table), relief. Gauge tables (JSON). Base flange/tab, edge flange, miter flange, hems, jog, sketched bend, lofted bend, closed corner, corner relief/treatments, forming tools, convert-to-sheet-metal (should), unfold/fold. |
| M-SHM-02 | Always-maintained flat pattern (derived configuration) with bend lines, direction, angle, radius, order notes and flat size; DXF export with layer mapping. Flat length = analytic K-factor result within 0.01 mm. |
| M-WLD-01 | Profile library (JSON, parametric sections + sketch profiles); members along 2D/3D sketch segments with rotation and pierce point; groups; corner treatments (miter, butt, trim/extend with gap); end caps, gussets, weld beads. |
| M-WLD-02 | Auto cut list grouping identical members: item, qty, profile, material, length, end angles (both ends, both planes), mass, custom properties; drawing table; CSV. |

## Assemblies (S3, S10)

| ID | Key rules |
|---|---|
| M-ASM-01 | Components = instances (part or sub-assembly + configuration), fixed or floating. Standard, advanced and mechanical mates (full list in spec). Solver reports DOF per component and minimal conflict set; drag respects mates; mates use `TopoRef`. |
| M-ASM-02 | In-context editing; external references tracked, listed, lockable, breakable; layout sketches and skeleton parts; assembly features: should. This is the mechanism under section 12 links. |
| M-ASM-03 | Load modes: full, lightweight (mesh + bounds + cached mass, resolve on demand), review-only. Selective loading by filter; simplified representations; instancing; LOD; background loading. P-14. |
| M-ASM-04 | Interference (pairs + volume), options (coincident faces, hidden, cosmetic threads, multibody), clearance threshold, hole alignment. Boards and components included from S6. |
| M-ASM-05 | Parametric fastener/hardware library (screws, nuts, washers, pins, heat-set inserts, PCB standoffs). Auto-fasten aligned hole series; length = stack thickness rounded up to stocked length; auto-resize; smart components carrying cut features into mated parts. |
| M-ASM-06 | Named exploded views per configuration (linear, radial, rotational steps), explode lines with jogs, animation export as image sequence. |
| M-ASM-07 | Keyframe timeline (position, camera, visibility, appearance); kinematic playback with motors (constant or profile), springs, gravity; optional stop-at-contact; PNG sequence; MP4 via optional encoder. |

**Mate solver rules (D-06):** solve rigid-body transforms with constraints expressed on `TopoRef` geometry; decompose into rigid clusters; detect redundancy numerically; keep the last solved pose on failure; dragging solves incrementally from the current pose.

## Drawings (S7)

| ID | Key rules |
|---|---|
| M-DRW-01 | Multi-sheet; ISO A0–A4, ANSI A–E, custom; templates with property-bound title blocks; 1st/3rd angle. Views: model, projected, section (full/half/offset/aligned), detail, broken, auxiliary, crop, broken-out, exploded. Display modes, tangent-edge modes, per-component visibility, configuration choice. Fully associative. PDF (vector), DXF, SVG, PNG, print. |
| M-DRW-02 | Dimensions (linear, aligned, ordinate, baseline/chain, angular, radial, diameter, chamfer, arc length); model dimension insertion; tolerances incl. ISO 286 fits table; GD&T (ASME Y14.5 / ISO 1101 symbols), datums, targets; surface finish, weld symbols, centre marks/lines, notes with property links, leaders, revision clouds, balloons, thread callouts. ISO and ANSI presets (JSON). |
| M-DRW-03 | BOM (top-level/parts-only/indented; property columns; balloon sync; electronics via provider, X-07), hole, revision, cut-list, bend and general tables; CSV export. |

## Motion, tolerance, inspection, rendering (S9, S10)

| ID | Key rules |
|---|---|
| M-SIM-04 | Mates → joints (overridable); gravity, forces/torques (constant or expression), springs/dampers, motors (speed/position/torque); contacts with friction and restitution (convex decomposition or mesh); Chrono solver; plots + CSV of kinematics, reactions, motor torque/power, contact forces, energy; trace paths; interference over time. Analytic benchmarks within 0.5 %. |
| M-SIM-05 | Stack-up from a gap and a contributor chain (proposed from mates: should); tolerances from model/drawing; worst case, RSS, Monte Carlo (normal/uniform); contribution and sensitivity; pass/fail; PDF/CSV. 1D required; 2D/3D should. |
| M-INS-01 | Inspection project from a drawing: auto-balloon all characteristics without altering the drawing; characteristic table; measured values with pass/fail; ballooned PDF and CSV/PDF report via template; numbers stable across revisions. |
| M-VIS-01 | PBR appearance library (incl. PCB finishes), textures, decals; HDR environments and studio presets (own or permissive assets); lights and cameras with DOF; progressive CPU path tracer (Embree) with optional denoise; PNG/EXR up to 8K; turntable/exploded/motion animations to frames; rasterised preview using same appearances. |

## Import/export and 3D printing (S2, S7)

- **M-IO-01:** STEP AP203/214/242 (assembly structure, names, colours; PMI should), IGES 5.3, STL, 3MF (export; import should), OBJ, glTF/GLB (export), VRML (export), DXF (import to sketches/drawings; export from drawings, flat patterns, sketches, PCB layers), PDF/SVG/PNG (drawings). Imported B-rep is checked and healed with a report; imports run in the background. Parasolid, ACIS, DWG, 3D PDF are out of scope.
- **M-3DP-01:** STL (bin/ASCII), 3MF (units, objects, colours, build items), OBJ; per body/part/component or combined; filename template; tessellation controls with preview and triangle count; validation (watertight, manifold, normals, self-intersections, degenerates) with per-body report; output frame, units, scale, place-on-plate; batch "all printable parts"; PCB fit-check mock-up export. Mesh volume within 0.1 % of B-rep at default settings.
