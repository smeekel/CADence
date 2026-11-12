// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   sketch/Module.h
/// @brief  Identity of the sketch module.
/// @req    B-02

#pragma once

#include "core/Module.h"

/// @brief 2D and 3D sketch model and the in-house geometric constraint solver.
///
/// See the module page in src/sketch/README.md for its purpose and allowed dependencies.
namespace app::sketch
{

/// @brief  Describe the sketch module.
/// @return The module name "sketch" and the product version core::kVersion.
[[nodiscard]] core::ModuleInfo moduleInfo();

} // namespace app::sketch
