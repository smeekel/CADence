// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   link/Module.h
/// @brief  Identity of the link module.
/// @req    B-02

#pragma once

#include "core/Module.h"

/// @brief Parametric links between the mechanical and electronics models (spec section 12).
///
/// See the module page in src/link/README.md for its purpose and allowed dependencies.
namespace app::link
{

/// @brief  Describe the link module.
/// @return The module name "link" and the product version core::kVersion.
[[nodiscard]] core::ModuleInfo moduleInfo();

} // namespace app::link
