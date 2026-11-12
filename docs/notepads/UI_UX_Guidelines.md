# UI and UX guidelines

Spec: section 8 (U-01 … U-15), A-02, A-10, CR-01, CR-02.

## Principles

1. **Commands first.** Every UI action calls a command (A-03). Widgets never change documents directly.
2. **Never block.** Long work runs as jobs with progress and cancel; the UI thread never waits > 100 ms (A-10).
3. **Live preview.** Feature dialogs and tools preview the result before committing (U-07).
4. **Modeless by default.** Property panels and in-canvas editing instead of modal dialogs. Modal dialogs only for destructive confirmation (U-11).
5. **Original look** (CR-01/02). Do not imitate any vendor's layout, icon set or branded tool names.

## Shell layout (U-03, U-04, U-14)

```
┌──────────────────────────────────────────────────────────────────────────────┐
│ Title bar · Workspace switcher (Part · Assembly · Drawing · Schematic · PCB · Simulation) │
│ Contextual command bar (tabbed groups for the active workspace)              │
├────────────┬───────────────────────────────────────────────┬─────────────────┤
│ Model tree │                                               │ Properties      │
│ / Project  │            Viewport / canvas                  │ (inspector)     │
│ navigator  │   (view cube, breadcrumbs, selection filter)  │                 │
│            │                                               │ Libraries       │
├────────────┴───────────────────────────────────────────────┴─────────────────┤
│ Messages · Nets/Layers · Changes (cross-domain) · Plots      | Status bar: units, snap, DOF, job progress │
└──────────────────────────────────────────────────────────────────────────────┘
```

- Docking: all panels dockable, tabbable and floatable onto other monitors (U-02, U-03). Layout saved per workspace; "Reset layout" restores the default.
- Start page (U-14): recent projects, templates (blank part, blank assembly, PCB-in-enclosure), local tutorials.
- Command palette `Ctrl+K` (U-05): fuzzy search across commands, recent files and settings; shows each command's shortcut.

## Visual design tokens (U-01) (proposed)

Defined once in `src/ui/theme/tokens.json` and compiled into a Qt style sheet + palette at start-up. Light and dark themes share tokens.

| Token | Value |
|---|---|
| Spacing grid | 4 px base; padding steps 4, 8, 12, 16, 24 |
| Corner radius | 4 px controls, 8 px panels/popovers |
| UI font | Platform UI font (Segoe UI Variable on Windows), 9 pt base, 11 pt headers |
| Monospace | Cascadia Mono / platform monospace for numeric fields and code |
| Icon size | 16 px in trees/tables, 20 px command bar, 32 px large buttons; SVG source, single colour + accent |
| Accent | One accent colour per theme, used sparingly for selection and primary actions |
| Motion | 120–180 ms ease-out for panel and hover transitions; none for data updates |
| Contrast | Text ≥ 4.5:1 against its background in both themes (U-12) |

Viewport colours: neutral gradient background, a selection colour distinct from the accent, a pre-highlight colour, and constraint-state colours for sketches (under-defined, fully defined, over-defined, error). PCB layer colours come from a colour-blind-safe default palette (U-12) and are user-editable.

## Interaction rules

- **Navigation (U-08):** orbit, pan, zoom-to-cursor, view cube, standard views, section view; mouse presets (default + alternatives selectable in Preferences). 2D: pan, zoom, fit, cross-probe.
- **Selection (U-09):** click, window/crossing box, lasso, filters (face, edge, vertex, body, component, net, pad, track), "select other" cycling for occluded items; Esc clears.
- **In-canvas editing (U-07):** double-click a dimension to edit; Enter commits; Esc cancels; expressions accepted anywhere (A-08).
- **Properties (U-06):** every numeric field accepts expressions with units, shows the evaluated value, and marks linked (X-) values with a link icon and the driver's name.
- **Shortcuts (U-10):** every command bindable; bindings stored as open JSON in the user settings folder.
- **Messages (U-11):** non-modal panel; click to locate; group by document and severity.
- **Accessibility (U-12):** full keyboard navigation, accessible names on every control, focus indicators, colour never the only carrier of meaning.
- **Localisation (U-13):** all strings through `tr()`; no concatenated sentences; English only initially.
- **Help (U-15):** F1 opens the bundled offline guide at the relevant page.

## Workspaces (minimum panels)

| Workspace | Panels |
|---|---|
| Part | Feature tree, properties, parameters table, messages |
| Assembly | Assembly tree (components, mates), properties, interference results, configurations, motion timeline |
| Drawing | Sheet tree, properties, view palette, tables |
| Schematic | Project navigator, properties, library browser, ERC results, simulation plots |
| PCB | Layers, nets/classes, rules/constraint grid, DRC results, 3D toggle, length/match panel |
| Simulation | Study tree, plots, results tables |
| All | Command palette, messages, cross-domain changes (X-09), link manager (X-10), job progress |
