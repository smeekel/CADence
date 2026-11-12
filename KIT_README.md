# Cursor planning kit for APP

This kit sets up a Cursor workspace so the Cursor Agent can create the repository scaffold and the implementation plan for APP from the functional requirements (v0.2).

## Contents

```
CURSOR_PLAN_PROMPT.md            The planning prompt: what the agent must produce and how
KIT_README.md                    This file
.cursor/
  rules/                         Project rules (loaded automatically by Cursor)
    000-project-core.mdc         Always on: project facts and non-negotiables
    010-cpp-code-style.mdc       C++ files: braces, spacing, alignment, naming, language usage
    020-documentation-doxygen.mdc C++ and docs: Doxygen requirements (HTML + PDF)
    030-architecture-layering.mdc src/: module dependencies and core patterns
    040-build-msbuild.mdc        .slnx/.vcxproj/.props/vcpkg/scripts: build rules (no CMake)
    050-file-format.mdc          File-format code, schemas and libraries
    060-testing.mdc              tests/: GoogleTest, golden data, acceptance tests
    070-licensing-provenance.mdc GPL, SPDX headers, reuse and provenance (agent-requested)
    080-planning-checklist.mdc   Checklist, traceability, ADR and stage-plan formats
  commands/                      Slash commands for the Agent chat
    plan-implementation.md       /plan-implementation: run the planning session
    next-step.md                 /next-step: implement the next checklist step
    check-traceability.md        /check-traceability: audit coverage and status
docs/
  spec/cad-functional-requirements.md   Authoritative requirements (v0.2)
  notepads/                      Modular context packs (see 00_Index.md)
.clang-format                    Enforces the code style (Visual Studio uses it natively)
.editorconfig                    Encoding, line endings, indentation
Doxyfile                         Documentation build (HTML + LaTeX/PDF), offline
```

## How to use

1. Create an empty Git repository and copy everything in this kit into its root, including the hidden `.cursor` folder, `.clang-format` and `.editorconfig`.
2. Open the folder in Cursor. In **Settings → Rules**, check that the project rules are listed. Rule `000` should always be on, and the others attach when matching files are open.
3. Open an Agent chat and run **`/plan-implementation`**. Alternatively, paste `@CURSOR_PLAN_PROMPT.md` and ask the agent to execute it.
4. Review what it produces: `docs/FOLDER_STRUCTURE.md`, the scaffold, `docs/IMPLEMENTATION_CHECKLIST.md`, `docs/stages/`, `docs/traceability.csv`, `docs/RISKS.md` and `docs/adr/`. Then run `/check-traceability`.
5. Build on your machine with Visual Studio 2026 (`APP.slnx`), or run `pwsh scripts/build.ps1` and `pwsh scripts/test.ps1`.
6. Implement step by step with **`/next-step`**. The agent stops at every stage gate for your review.

## Notes

- Notepads were removed from Cursor in version 2.0, so the "notepads" here are Markdown files you @-mention (for example `@docs/notepads/Electronics_Design_Rules.md`). Mention only the ones a step needs.
- In Visual Studio, enable ClangFormat support under **Tools → Options → Text Editor → C/C++ → Code Style → Formatting**, so formatting matches the rules.
- Documentation: `pwsh scripts/docs.ps1` writes HTML to `out/docs/html`. Add `-Pdf` to build `refman.pdf` (needs MiKTeX or another LaTeX distribution). The `@req` tags produce a "Requirements Traceability" page automatically.
- Items marked **(proposed)** in the notepads are design suggestions beyond the spec. The planning session confirms or replaces each one with an ADR.
