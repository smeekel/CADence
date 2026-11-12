# Data sources for shipped library data {#data-sources}

This table records the source of every table or item of default data shipped under `libraries/` (spec CR-05, F-10).
Paid or copyrighted standards (for example IPC-7351 or ISO fastener standards) must not be reproduced verbatim.
Ship only factual dimensional data from publicly available sources, or original values created for this product,
and implement calculators and generators with user-editable parameter tables.

Rules:

- Add a row in the same step that adds or changes seed data. The clean-room check (`scripts/check-clean-room.ps1`,
  see `docs/clean-room.md`) fails when a data file under `libraries/` is not named in the "Table / item" column of a
  complete row (every column filled in): by its path, such as `libraries/materials/steel.json`, or by a folder that
  contains it, ending in `/`. Separate several paths with commas. `README.md` files and `libraries/seed-manifest.json`
  are not data.
- "Terms" states why the data may be shipped (public factual data, original work, permissive licence with its
  SPDX identifier).
- "Transformation" says how the data was turned into library files (for example "converted from mm table to JSON;
  values unchanged" or "original values").
- Data the user must supply (for example a licensed limits-and-fits table) is marked "user-supplied" and is not
  shipped.

| Library kind | Table / item | Source (title, publisher, URL or citation) | Terms | Retrieved | Transformation |
|---|---|---|---|---|---|
