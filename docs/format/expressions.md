# Expression language {#format-expressions}

The shared, unit-aware expression language (spec A-08) is used wherever a number can be typed: sketch dimensions,
feature parameters, global variables, PCB rule values and link mappings. In the file format an expression is stored
as text, for example `{ "expr": "Board.Width + 2 mm" }` (section 8 of the file format, @ref format).

This page is normative for the syntax. It is implemented by `src/core/expr/` (`Lexer`, `Parser`, `Ast`; S0.03.6).
Evaluation, the functions and unit checking (S0.03.7) and parameter scopes (S0.03.8) are described here as they are
implemented.

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
