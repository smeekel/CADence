// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   io/Module.h
/// @brief  Identity of the io module.
/// @req    B-02

#pragma once

#include "core/Module.h"

/// @brief Neutral-format import and export: STEP, IGES, STL, 3MF, OBJ, glTF, VRML, DXF and IDF.
///
/// See the module page in src/io/README.md for its purpose and allowed dependencies.
namespace app::io
{

/// @brief  Describe the io module.
/// @return The module name "io" and the product version core::kVersion.
[[nodiscard]] core::ModuleInfo moduleInfo();

} // namespace app::io
