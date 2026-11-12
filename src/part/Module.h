// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   part/Module.h
/// @brief  Identity of the part module.
/// @req    B-02

#pragma once

#include "core/Module.h"

/// @brief Part features, feature tree, regeneration, materials and mass properties; later sheet metal and weldments.
///
/// See the module page in src/part/README.md for its purpose and allowed dependencies.
namespace app::part
{

/// @brief  Describe the part module.
/// @return The module name "part" and the product version core::kVersion.
[[nodiscard]] core::ModuleInfo moduleInfo();

} // namespace app::part
