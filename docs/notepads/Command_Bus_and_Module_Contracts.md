# Command bus and module contracts

Spec: A-02 (headless first), A-03 (command bus), A-05 (kernel adapter), A-07 (regeneration), A-10 (threading), A-11 (factories). This notepad plays the role of "API contracts": APP's internal API is the command bus plus each module's public interface. Signatures here are **(proposed)** starting points; the S0 steps finalise them with ADRs.

## Error model

```cpp
namespace app
{

enum class ErrorCode
{
    InvalidArgument,
    NotFound,
    Conflict,          // e.g. over-constrained sketch, link cycle
    GeometryFailure,   // kernel operation failed
    FormatError,       // file parse/validation
    Unsupported,
    Cancelled,
    Internal
};

struct Error
{
    ErrorCode                  code;
    std::string                message;   ///< User-readable, English.
    std::string                detail;    ///< Developer detail (path, kernel status, JSON path).
    std::vector<core::ObjectId> objects;  ///< Objects to highlight.
};

template<typename T>
using Result = std::expected<T, Error>;

} // namespace app
```

Rules: `Result` across module boundaries; no exceptions across public APIs; `[[nodiscard]]` on every function returning `Result`.

## Commands

```cpp
namespace app::core
{

/// Context handed to every command.
struct CommandContext
{
    doc::Project&  project;
    JobSystem&     jobs;
    EventBus&      events;
    Log&           log;
};

class Command
{
public:
    virtual ~Command() = default;

    [[nodiscard]] virtual std::string_view name() const = 0;        ///< e.g. "part.feature.extrude.create"
    [[nodiscard]] virtual nlohmann::ordered_json params() const = 0;  ///< Serialisable parameters.
    [[nodiscard]] virtual Result<void> execute( CommandContext& ctx ) = 0;
    [[nodiscard]] virtual Result<void> undo( CommandContext& ctx ) = 0;
    [[nodiscard]] virtual bool mergeWith( const Command& next );      ///< Optional: coalesce drags.
};

using CommandFactory = std::function<Result<std::unique_ptr<Command>>( const nlohmann::ordered_json& params )>;

class CommandRegistry
{
public:
    void registerCommand( std::string name, CommandFactory factory, std::string paramsSchemaPath );
    [[nodiscard]] Result<std::unique_ptr<Command>> create( std::string_view name, const nlohmann::ordered_json& params ) const;
};

class CommandBus
{
public:
    [[nodiscard]] Result<void> run( std::unique_ptr<Command> command );      ///< Executes, pushes undo, journals.
    [[nodiscard]] Result<void> run( std::string_view name, const nlohmann::ordered_json& params );
    [[nodiscard]] Result<void> undo();
    [[nodiscard]] Result<void> redo();
    void beginGroup( std::string label );                                     ///< Groups commands into one undo step.
    void endGroup();
};

} // namespace app::core
```

### Naming and documentation

- Names: `<domain>.<object>.<verb>` (lower case, dots): `sketch.entity.line.add`, `sketch.constraint.add`, `part.feature.fillet.create`, `asm.mate.add`, `pcb.track.route`, `link.create`, `io.export.step`, `project.save`.
- Every command has a parameter schema `docs/commands/<name>.schema.json` and is listed in `docs/commands/README.md`. Command names are **stable**: they become the future public API (spec section 14).
- Each module exposes `void registerCommands( core::CommandRegistry& )`.

### Journal

- Session journal: JSON Lines in `%LOCALAPPDATA%\APP\journals\<session>.jsonl`. The first line is a header object with `$magic`, `formatVersion` and `type: "journal"`, then one line per command `{ "t": <ms>, "name": "…", "params": { … } }`.
- `cli replay <journal>` re-executes a journal headless (used by T-02 and T-05).

## Events

- `EventBus` publishes `DocumentChanged { documentId, ChangeSet { added, modified, removed } }`, `RegenStateChanged`, `SelectionChanged` (UI only), `JobProgress`.
- Subscribers never mutate documents inside a handler; they post commands.

## Key module interfaces (proposed)

| Module | Interface | Responsibilities |
|---|---|---|
| `core` | `ExpressionEngine` | Parse/evaluate unit-aware expressions; dependency extraction; unit checking (A-08) |
| `core` | `JobSystem` | Thread pool, task graph, progress, cancellation tokens |
| `doc` | `ProjectStore` | Load/save JSON and BSON, sniff magic, migrate, preserve unknown fields |
| `doc` | `LibraryResolver` | Resolve library IDs across project → user → global scopes (F-10) |
| `geom` | `Kernel` | Primitives, extrude/revolve/sweep/loft/boundary, booleans, fillet/chamfer/shell/draft/offset, sewing/healing, mass properties, distance/interference, tessellation, STEP/IGES I/O |
| `geom` | `TopoNaming` | Create and resolve `TopoRef`s across regenerations |
| `sketch` | `SketchSolver` | Solve, drag, DOF analysis, conflict diagnosis |
| `part` | `FeatureRegistry`, `Feature` | Feature types with `regenerate( RegenContext& ) -> Result<Shape>` |
| `asm` | `MateSolver` | Solve component transforms; report DOF and conflicts; drag |
| `asm` | `InterferenceChecker` | Interference, clearance, hole alignment |
| `drw` | `ViewGenerator`, `IBomProvider`, `IViewTypeProvider` | Hidden-line views; pluggable BOM rows and view types (used by `ecad/out` and `link`) |
| `ecad/sch` | `NetlistCompiler`, `ErcChecker` | Connectivity model, ERC |
| `ecad/pcb` | `BoardDb`, `Router`, `PourEngine`, `RuleResolver`, `DrcEngine` | Integer-nm board model, routing, pours, rules, DRC |
| `ecad/out` | `OutputJob`, `IFabWriter` | Gerber X2, Excellon, IPC-2581, P&P, IPC-D-356, BOM |
| `link` | `LinkManager` | Create/evaluate/lock/break links; register providers into `drw` and `asm` |
| `sim` | `MotionEngine`, `SpiceEngine`, `StackUpAnalyzer` | Chrono dynamics, ngspice, tolerance analysis |
| `io` | `IImporter`, `IExporter` registries | Neutral formats and 3D-print export |
| `render` | `Device`, `Scene`, `Canvas2D`, `PathTracer` | Viewport, 2D canvas, picking, offline rendering |

## Regeneration contract

```cpp
class RegenNode
{
public:
    [[nodiscard]] virtual core::ObjectId id() const = 0;
    [[nodiscard]] virtual std::span<const core::ObjectId> inputs() const = 0;
    [[nodiscard]] virtual Result<void> regenerate( RegenContext& ctx ) = 0;   ///< Must be deterministic.
    [[nodiscard]] virtual RegenState state() const = 0;
};
```

Rules: deterministic output for identical inputs; a failing node sets `Error` and keeps its last good output for display (P-21); downstream nodes of an errored node are `OutOfDate`, not deleted.
