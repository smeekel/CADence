# Code style and documentation

Owner's style requirements, enforced by `.clang-format`, `.editorconfig`, rule `010-cpp-code-style.mdc` and rule `020-documentation-doxygen.mdc`.

## The five owner rules

| # | Rule | Enforced by |
|---|---|---|
| 1 | `{` and `}` on their own lines | `.clang-format` (`BreakBeforeBraces: Allman`, `InsertBraces: true`) |
| 2 | Spaces inside statement parentheses: `if( a == b )`, `while( true )`; applied consistently to calls and declarations: `solve( sketch, options )` | `SpaceBeforeParens: Never`, `SpacesInParens: Custom` (`InConditionalStatements`, `Other`) |
| 3 | Line up `=` on consecutive assignment lines | `AlignConsecutiveAssignments` |
| 4 | Variable names start in the same column on consecutive declaration lines | `AlignConsecutiveDeclarations` |
| 5 | White space between logical and function blocks | `SeparateDefinitionBlocks: Always` + author discipline (blank line between declaration, set-up, loop/branch and return groups) |
| 6 | Documentation usable for printed output | Doxygen → HTML + LaTeX/PDF (`Doxyfile`, `scripts/docs.ps1`) |

Alignment only spans lines with no blank line between them. Use blank lines to start a new alignment group where a different logical block begins.

## Complete example

### Header

```cpp
// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   MassProperties.h
/// @brief  Mass properties of solid bodies.
/// @req    M-PRT-09

#pragma once

#include "core/Result.h"
#include "geom/Shape.h"

#include <array>

namespace app::part
{

/// @brief Mass properties of one body or a set of bodies.
struct MassProperties
{
    double                mass     = 0.0; ///< Mass. @units kg
    double                volume   = 0.0; ///< Volume. @units mm³
    double                area     = 0.0; ///< Surface area. @units mm²
    std::array<double, 3> centroid = {};  ///< Centre of mass in the chosen frame. @units mm
    std::array<double, 9> inertia  = {};  ///< Inertia tensor, row-major, about the centroid. @units kg·mm²
};

/// @brief  Compute mass properties of @p shape.
/// @param[in] shape     Solid to measure. Must be a closed, valid solid.
/// @param[in] density   Material density. @units kg/mm³
/// @return The properties, or GeometryFailure if the solid is invalid.
/// @req    M-PRT-09
[[nodiscard]] Result<MassProperties> computeMassProperties( const geom::Shape& shape, double density );

} // namespace app::part
```

### Source

```cpp
// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

#include "part/MassProperties.h"

#include "geom/Kernel.h"

namespace app::part
{

Result<MassProperties> computeMassProperties( const geom::Shape& shape, double density )
{
    MassProperties props;
    geom::Kernel&  kernel = core::services().get<geom::Kernel>();

    if( !shape.isSolid() )
    {
        return std::unexpected( Error{ ErrorCode::GeometryFailure, "Shape is not a closed solid." } );
    }

    auto volumeResult = kernel.volumeProperties( shape );

    if( !volumeResult )
    {
        return std::unexpected( volumeResult.error() );
    }

    props.volume   = volumeResult->volume;
    props.mass     = props.volume * density;
    props.centroid = volumeResult->centroid;
    props.area     = kernel.surfaceArea( shape );

    return props;
}

} // namespace app::part
```

## Naming (proposed; change in rule 010 if you prefer another scheme)

Types `PascalCase`; functions and variables `camelCase`; members `m_name`; statics `s_name`; constants `kName`; namespaces `lower_case`; macros `APP_UPPER`; file names match the main type.

## Documentation outputs

- `scripts/docs.ps1` runs Doxygen and writes `out/docs/html/index.html`.
- `scripts/docs.ps1 -Pdf` also runs `make.bat` in `out/docs/latex` to produce `refman.pdf` (needs a LaTeX distribution such as MiKTeX; optional).
- Doxygen custom commands: `@req <ID>` (builds a Requirements Traceability page), `@cmd <name>` (command-bus name), `@units <text>`.
- Markdown under `docs/` is part of the generated manual: main page, folder structure, file format, commands, ADRs, module READMEs.
- Zero Doxygen warnings for touched code is part of every step's "Done when".
