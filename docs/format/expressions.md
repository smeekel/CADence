# Expression language {#format-expressions}

The shared, unit-aware expression language (spec A-08) is used wherever a number can be typed: sketch dimensions,
feature parameters, global variables, PCB rule values and link mappings. In the file format an expression is stored
as text, for example `{ "expr": "Board.Width + 2 mm" }` (section 8 of the file format, @ref format).

This page is normative for the syntax and the evaluation rules. They are implemented by `src/core/expr/`: `Lexer`,
`Parser` and `Ast` (S0.03.6), `Evaluator` and `Functions` (S0.03.7), parameter scopes and dependencies (S0.03.8).

## Grammar

EBNF, from loosest to tightest binding. Whitespace (space, tab, CR, LF) may appear between any two tokens.

    expression = comparison ;
    comparison = additive , [ ( "<" | "<=" | ">" | ">=" | "==" | "!=" ) , additive ] ;
    additive   = term , { ( "+" | "-" ) , term } ;
    term       = unary , { ( "*" | "/" ) , unary } ;
    unary      = ( "+" | "-" ) , unary | power ;
    power      = primary , [ "^" , unary ] ;
    primary    = number , [ unit ]
               | name , "(" , [ comparison , { "," , comparison } ] , ")"
               | name , { "." , name }
               | "(" , comparison , ")" ;

    number     = ( digits , [ "." , digits ] | "." , digits ) , [ ( "e" | "E" ) , [ "+" | "-" ] , digits ] ;
    digits     = digit , { digit } ;
    name       = name-start , { name-start | digit } ;
    name-start = "A" … "Z" | "a" … "z" | "_" | "µ" | "μ" ;
    unit       = "mm" | "cm" | "m" | "in" | "mil" | "thou" | "um" | "µm" | "μm" | "deg" | "rad" ;

## Rules

| Rule | Example | Meaning |
|---|---|---|
| `+ -` and `* /` associate to the left | `8 / 4 / 2` | `(8 / 4) / 2` = 1 |
| `^` associates to the right | `2 ^ 3 ^ 2` | `2 ^ (3 ^ 2)` = 512 |
| `^` binds tighter than a prefix sign | `-2 ^ 2` | `-(2 ^ 2)` = −4 |
| The exponent may carry a sign | `2 ^ -1` | 0.5 |
| A comparison cannot be chained | `a < b < c` | error |
| A unit follows a number literal only, with or without a space | `10 mm`, `0.5in` | the number in that unit |
| A name followed by `(` is a function call | `max(a, b)` | |
| Other names are parameter references, possibly dotted | `Board.Width` | |

- A `.` belongs to a number only when a digit follows it, and `e` starts an exponent only when a digit (after an
  optional sign) follows, so `2em` is the number 2 followed by the name `em` (an unknown unit).
- A name directly after a number must be a unit; `10 mn` is the error "Unknown unit 'mn'". To multiply, write `*`.
- The unit names come from the display units (`core/Units.h`, P-06): `µm` may be written with the micro sign
  (U+00B5), the Greek mu (U+03BC) or as `um`. Units are case-sensitive.
- Names are ASCII letters, digits and `_`, starting with a letter or `_` (`µ` and `μ` are accepted for `µm`).
- Function names are not dotted. The parser accepts any function name; the evaluator rejects unknown names and wrong
  argument counts (S0.03.7). The functions are `sin cos tan asin acos atan atan2 sqrt abs min max floor ceil round
  if`.
- The comparison operators exist for the first argument of `if` and for rule conditions. They are an addition to
  the operator list in spec A-08 (`+ - * / ^`), which `if` needs to be useful.
- Nesting deeper than 64 levels is rejected. The whole expression, each parenthesis or argument list, each prefix sign
  and each `^` adds a level.

## Evaluation and unit checking

Implemented by `src/core/expr/Evaluator` and `Functions` (S0.03.7). A value is a number in internal units (mm for
length, rad for angle) with a dimension: the exponents of length and angle (`core/Quantity.h`).

| Operation | Rule |
|---|---|
| Number with a unit | Converted to internal units: `0.5 in` is 12.7 mm, `90 deg` is π/2 rad |
| Number without a unit | Dimensionless |
| `a + b`, `a - b` | Equal dimensions; `1 mm + 1 deg` is the error "Cannot add length and angle" |
| `a * b`, `a / b` | Dimensions add or subtract: `2 mm * 3 mm` is an area; dividing by zero is an error |
| `a ^ b` | `b` is dimensionless; each dimension exponent of `a` times `b` must be whole: `(2 mm) ^ 2` is an area, `(4 mm * 1 mm) ^ 0.5` is 2 mm, `(2 mm) ^ 0.5` is an error; a negative `a` needs a whole `b` |
| Comparisons | Equal dimensions; the result is 1 or 0. They use the confusion tolerance (1e-7 in internal units, `core/Tolerance.h`): `a == b` when they differ by at most 1e-7, `a < b` only when `a` is smaller by more than that |
| References | Resolved by the caller's parameter lookup (parameter scopes, S0.03.8) |
| Any result | Must be finite; otherwise "The result is not a finite number" |

| Function | Arguments | Result |
|---|---|---|
| `sin cos tan` | An angle, or a number read as radians | Number |
| `asin acos` | A number from −1 to 1 | Angle |
| `atan` | A number | Angle |
| `atan2(y, x)` | Two values of the same dimension | Angle |
| `sqrt` | A non-negative value whose dimension exponents are even | Half the exponents: `sqrt(4 mm * 1 mm)` is 2 mm |
| `abs` | Any value | Same dimension |
| `min max` | One or more values of the same dimension | Same dimension |
| `floor ceil round` | A number (`round(x / 1 mm) * 1 mm` rounds a length to whole millimetres); `round` takes halves away from zero | Number |
| `if(c, a, b)` | A number `c`; `a` if `c` is not 0, else `b` | Only the selected branch is evaluated, so `if(x > 0, sqrt(x), 0)` never fails on the other branch |

`floor`, `ceil` and `round` refuse lengths and angles because rounding the internal millimetres of a value typed in
inches, or the radians of one typed in degrees, would not round what the user sees.

## Parameter scopes and dependencies

Implemented by `src/core/expr/IParameterScope.h`, `ParameterScope` and `Dependencies` (S0.03.8).

- Scopes form a chain from the innermost (for example a part) to the outermost (the project). A reference resolves
  in the nearest scope that defines the name; an inner parameter shadows an outer one of the same name.
- A dotted name such as `Board.Width` is one parameter name, looked up whole in each scope. Parameter names follow
  the `name { "." name }` grammar above, without spaces.
- A parameter's expression is evaluated in the scope that defines it, so its references resolve from there outwards,
  not from where it is used. An inner `t = t * 2` therefore refers to itself.
- Values are computed on each lookup from the current expressions.
- A cycle is an error naming every parameter in it, in the order they refer to each other:
  "Parameter cycle in part: a -> b -> c -> a.". An error inside a referenced parameter is reported with its name:
  "Parameter 'a': Cannot add length and angle at column 6.". The column is in that parameter's expression.
- `referencedNames()` lists the parameters an expression refers to, each once, in order of first appearance, for
  dependency tracking (A-07) and "where used" lists. Both branches of `if` count.

## Errors

A syntax error reports the 1-based column, counted in Unicode characters, and shows the text with a caret:

    Expected ')' at column 7.
    (1 + 2
          ^

Messages: "Unexpected character 'c'", "Unexpected byte 0xNN" (invalid UTF-8), "Unexpected control character
U+NNNN", "Malformed number 'n'", "Number 'n' is out of range", "Unexpected 't'", "Unexpected end of expression",
"Expected ')'", "Expected ',' or ')'", "Expected a name after '.'", "Unknown unit 'u'", "Function name 'a.b' cannot
be dotted", "Comparisons cannot be chained", "Expression is nested more than 64 levels deep" and "Expression is
empty.".

Evaluation errors use the same form and point at the operator, function name, reference or `if` condition at fault:
"Cannot add length and angle at column 6.", "Unknown function 'foo'", "atan2 takes 2 arguments, not 1", "sqrt of a
negative number", "Division by zero", "Unknown parameter 'x'". An error returned by the parameter lookup keeps its
error code and is reported at the reference.
