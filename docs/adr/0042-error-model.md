# ADR 0042: Error model and Result type {#adr-0042}

- **Status:** Proposed
- **Date:** 2026-11-16
- **Deciders:** project owner (approves), implementer (proposes)

## Context

Rule 010 asks every function that can fail across a module boundary to return `app::Result<T>`
(`std::expected<T, app::Error>`), with no exceptions crossing public APIs and `[[nodiscard]]` on every such function.
The command-bus notepad proposes the shape of `Error` (code, message, detail, objects to highlight). Step S0.03.2
implements it; `std::expected` is an "Allowed" feature (`docs/cpp-features.md`, ADR 0034). A-02 and A-03 need errors
that the CLI, the journal and the UI can all report the same way.

## Decision

- `src/core/Error.h` defines, in namespace `app`:
  - `enum class ErrorCode : std::uint8_t` with `InvalidArgument`, `NotFound`, `Conflict`, `GeometryFailure`,
    `FormatError`, `IoError`, `Unsupported`, `Cancelled` and `Internal`. This is the notepad's list plus `IoError`,
    because file and folder failures (S0.03.10 logging, S0.06 storage) are neither format errors nor internal.
  - `toString( ErrorCode )`: stable kebab-case names (`invalid-argument`, `io-error`, ...) used in CLI output,
    journals and test expectations. They are part of the public interface and do not change.
  - `struct Error { code; message; detail; objects; }`: `message` is a user-readable English sentence, `detail` is
    developer detail (file, JSON path, parameter name), `objects` are the `core::ObjectId`s the UI highlights.
    `describe()` gives `"<code>: <message> (<detail>)"`.
  - `fail( code, message, detail, objects )` returns `std::unexpected<Error>`, so `return app::fail( ... );` works for
    any `Result<T>`.
- `src/core/Result.h` defines `template <typename T = void> using Result = std::expected<T, Error>` and
  `ok()` for `Result<void>`. Callers chain with the standard `and_then`, `transform` and `or_else`.
- **Objects to highlight.** `Error.objects` was added in step S0.03.3, which delivers `ObjectId`. `Error.h` includes
  `ObjectId.h` and not the reverse, so `ObjectId::parse` returns `std::optional`; callers turn a failed parse into an
  `Error` with their own context (file, JSON path).
- `Error` lives in namespace `app`, not `app::core`, as rule 010 names it (`app::Result`, `app::Error`); the files are
  in the `core` module.
- Programmer errors (broken invariants) are not `Error` values; they use `APP_ASSERT` when that macro is added.
  `ErrorCode::Internal` is for defects that must still be reported without crashing, for example in the CLI.

## Consequences

- One error type from the kernel adapter to the CLI. The `Messages` panel (S0.13.10) and the CLI print
  `describe()`; the stable code names can be matched in integration tests.
- `[[nodiscard]]` is required on every function returning `Result` (rule 010, S0.03.2 "Done when"). Callers that drop a
  `Result` are also caught by clang-tidy `bugprone-unused-return-value`, configured in `.clang-tidy` to check
  `::std::expected`.
- Adding an error code is a compatible change; renaming or removing one needs a new ADR.

## Alternatives considered

- Exceptions across module APIs: forbidden by rule 010 and hard to journal.
- `std::error_code` with categories: no room for the message and detail the UI and the CLI need.
- A custom `Result` class: duplicates `std::expected`, which all three target compilers provide.

## Related requirements

A-02, A-03; rule 010; ADR 0034; steps S0.03.2, S0.03.3.
