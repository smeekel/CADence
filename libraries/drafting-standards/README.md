# Global seed library: drafting-standards

Drafting presets (ISO and ANSI): line types, text heights, arrowheads, dimension styles.

| Item | Value |
|---|---|
| Library `kind` | `draftingStandard` |
| First content | S1/S7 |
| Requirements | M-DRW-02 |
| Data sources | Original presets; standards are referenced by name only. |

This folder is part of the **global seed** shipped with APP (F-10). On first run it is copied to
`%ProgramData%\APP\libraries\drafting-standards\`. Items resolve project first, then user, then global, so a project or user item with the
same library ID shadows the item here. Files use the open library format (F-07, `docs/format/README.md`) and start
with the `$magic` UUID (F-09). Every seed item is listed in `libraries/seed-manifest.json` (Stage 0).

Empty in the scaffold.
