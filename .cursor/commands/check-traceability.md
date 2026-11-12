# Check traceability

Compare @docs/spec/cad-functional-requirements.md, @docs/traceability.csv and @docs/IMPLEMENTATION_CHECKLIST.md, then report:

1. Requirement IDs in the spec that are missing from the CSV or from every checklist step.
2. IDs in the CSV or checklist that no longer exist in the spec.
3. Steps marked done whose CSV status is not `done`, or the reverse.
4. `Accept:` criteria in the spec with no named test in any step.
5. Steps that depend on later steps.

Propose exact edits to fix each issue. Do not apply them until asked.
