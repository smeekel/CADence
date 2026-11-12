// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   asm/Module.h
/// @brief  Identity of the asm module.
/// @req    B-02

#pragma once

#include "core/Module.h"

/// @brief Assemblies, mates and the mate solver, interference detection, exploded views and the animation timeline. The
/// namespace is `app::assembly` because `asm` is a C++ keyword.
///
/// See the module page in src/asm/README.md for its purpose and allowed dependencies.
namespace app::assembly
{

/// @brief  Describe the asm module.
/// @return The module name "asm" and the product version core::kVersion.
[[nodiscard]] core::ModuleInfo moduleInfo();

} // namespace app::assembly
