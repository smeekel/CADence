// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   ecad/lib/Module.h
/// @brief  Identity of the ecad/lib module.
/// @req    B-02

#pragma once

#include "core/Module.h"

/// @brief Electronics design modules: library, schematic, board layout and manufacturing outputs.
namespace app::ecad
{
}

/// @brief Electronics library model: symbols, footprints, components and the footprint generator.
///
/// See the module page in src/ecad/lib/README.md for its purpose and allowed dependencies.
namespace app::ecad::lib
{

/// @brief  Describe the ecad/lib module.
/// @return The module name "ecad/lib" and the product version core::kVersion.
[[nodiscard]] core::ModuleInfo moduleInfo();

} // namespace app::ecad::lib
