// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   render/Module.h
/// @brief  Identity of the render module.
/// @req    B-02

#pragma once

#include "core/Module.h"

/// @brief OpenGL 4.5 viewport behind `render::Device`, 2D canvas, picking and the offline path tracer.
///
/// See the module page in src/render/README.md for its purpose and allowed dependencies.
namespace app::render
{

/// @brief  Describe the render module.
/// @return The module name "render" and the product version core::kVersion.
[[nodiscard]] core::ModuleInfo moduleInfo();

} // namespace app::render
