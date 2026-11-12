# Provenance of reused code {#provenance}

This log records every file in the repository that contains reused or adapted third-party code (spec CR-04, CR-07).
Fresh implementation is preferred; reuse is allowed only from GPL-3.0-or-later-compatible sources (spec section 4.3)
and only when it saves substantial effort or brings a proven, hard-to-get-right algorithm.

Rules:

- Add a row **before** committing the reused code. `scripts/check-spdx.ps1` fails the build when a third-party file
  (under the third-party folder, marked "Adapted from", or with another licence or copyright holder) is not named in
  the "Destination file(s)" column of a complete row: by its repository-relative path, by a folder that contains it
  (ending in `/`), or by its third-party project folder. A row is complete when Project, URL, Version / commit,
  Original path(s), Licence, Destination file(s) and Step ID are filled in. Mentions in prose do not count. Separate
  several destinations with commas.
- Every such file keeps its original copyright and licence notice and an `SPDX-License-Identifier` header.
- Libraries consumed through vcpkg are listed in `THIRD_PARTY_NOTICES.md`, not here, unless their code is copied or
  patched into the repository (for example an overlay port).
- Never remove a row; if the code is later removed, add a note in "Changes made".

| Date | Project | URL | Version / commit | Original path(s) | Licence | Destination file(s) | Changes made | Step ID |
|---|---|---|---|---|---|---|---|---|
