# Provenance of reused code {#provenance}

This log records every file in the repository that contains reused or adapted third-party code (spec CR-04, CR-07).
Fresh implementation is preferred; reuse is allowed only from GPL-3.0-or-later-compatible sources (spec section 4.3)
and only when it saves substantial effort or brings a proven, hard-to-get-right algorithm.

Rules:

- Add a row **before** committing the reused code. `scripts/check-spdx.ps1` fails the build when a third-party file
  (under the third-party folder, marked "Adapted from", or with another licence or copyright holder) has no row here
  naming its repository-relative path or its third-party project folder.
- Every such file keeps its original copyright and licence notice and an `SPDX-License-Identifier` header.
- Libraries consumed through vcpkg are listed in `THIRD_PARTY_NOTICES.md`, not here, unless their code is copied or
  patched into the repository (for example an overlay port).
- Never remove a row; if the code is later removed, add a note in "Changes made".

| Date | Project | URL | Version / commit | Original path(s) | Licence | Destination file(s) | Changes made | Step ID |
|---|---|---|---|---|---|---|---|---|
