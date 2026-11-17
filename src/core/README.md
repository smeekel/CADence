# Module core {#module-core}

**Purpose.** Foundation services: math, units, the unit-aware expression engine, ObjectId, events, the command bus, undo, jobs and logging.

| Item | Value |
|---|---|
| Folder | `src/core/` |
| Project | `Core.vcxproj` (static library) |
| Namespace | `app::core` |
| Tests | `tests/core/Core.Tests.vcxproj` |
| Content from stage | S0 |
| Main requirements | A-03, A-04, A-08, A-10 |

## Allowed dependencies

- Application modules: Nothing in the application (A-01).
- Third-party libraries: Standard library; Eigen (ADR 0022); nlohmann/json through `core/Json.h` (rule 030, ADR 0016).

The table in `.cursor/rules/030-architecture-layering.mdc` is authoritative and `scripts/check-layering.ps1` enforces
it on project references and `#include` paths. It changes only together with an ADR.

## Main types

`ModuleInfo` and the product version constants (scaffold). `Compat.h`: required feature-test macros and the
`compat::` wrappers for conditional C++23/26 features (`enumerate`, `zip`, `zipTransform`, `FlatMap`, `FlatSet`; L-04).
`Error.h` and `Result.h` (namespace `app`, ADR 0042): `ErrorCode`, `Error`, `fail()`, `Result<T>` and `ok()`; every
function that can fail across a module boundary returns `Result<T>`. `Json.h`: `app::core::Json`. `ObjectId.h`:
`ObjectId` (UUID version 7, RFC 4122 byte order, lowercase text) and `ObjectIdGenerator` (injectable clock and random
source, `makeDeterministic()` for tests; `newObjectId()` uses the process-wide generator). `Units.h`, `Quantity.h`,
`Tolerance.h`: display units with exact conversion ratios, `Quantity` (value in mm or rad plus a `Dimension`, with
unit-checked `add` and `subtract`), locale-independent formatting and parsing, and `kConfusionTolerance` (1e-7 mm).
`math/` (namespace `app::core::math`): `Vec2`, `Vec3`, `Mat3`, `Transform` and `Box3`, thin value types over Eigen
storage (ADR 0022) whose API does not need Eigen except through `eigen()`; and `IntGeom.h`, the integer-nanometre board
types `Nm`, `NmPoint`, `NmVector` and `NmRect` with overflow-checked arithmetic returning `Result` (A-09; no floating
point).
`expr/` (namespace `app::core::expr`): the expression lexer, parser and evaluator (`tokenize`, `parseExpression`,
syntax tree in `Ast.h`, `evaluate` and `evaluateExpression` with unit checking and the built-in functions,
`referencedNames`, and parameter scopes: `IParameterScope`, `lookup`, `resolverFor` and the expression-backed
`ParameterScope`; grammar and rules in `docs/format/expressions.md`, A-08).
Planned: `Command`, `CommandBus`, `UndoStack`, `JobSystem`, `EventBus`, `ExpressionEngine`.
