// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   geom/Module.h
/// @brief  Identity of the geom module.
/// @req    B-02

#pragma once

#include "core/Module.h"

/// @brief Geometry-kernel adapter over Open CASCADE Technology, tessellation and persistent topological naming
/// (`TopoRef`).
///
/// See the module page in src/geom/README.md for its purpose and allowed dependencies.
namespace app::geom
{

/// @brief  Describe the geom module.
/// @return The module name "geom" and the product version core::kVersion.
[[nodiscard]] core::ModuleInfo moduleInfo();

} // namespace app::geom
