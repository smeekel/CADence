# Implement the next checklist step

1. Open @docs/IMPLEMENTATION_CHECKLIST.md and find the first unchecked step whose dependencies are all checked. If a `Human review checkpoint` is the next unchecked item, stop and ask for approval instead.
2. Read the requirements it lists in @docs/spec/cad-functional-requirements.md and the relevant notepads in @docs/notepads.
3. State the plan for this step in 3–6 bullets, including the tests you will write.
4. Implement it following `.cursor/rules/`: command first, then headless tests, then UI (if any). Write fresh code; if you reuse GPL-compatible code, add the SPDX notice and a `docs/provenance.md` entry in the same change.
5. Run `scripts/build.ps1 -Configuration Headless` (and `Debug` if UI changed), `scripts/test.ps1` and the check scripts. Fix until green.
6. Update: the step's checkbox, `docs/traceability.csv` (status, tests), format docs and schemas if data changed, and Doxygen comments.
7. Report: what changed, tests added, results, and anything deferred. Suggest a commit message that starts with the step ID and lists requirement IDs.

Never tick a step whose `Done when` criteria are not all met. Mark it in progress with a note instead.
