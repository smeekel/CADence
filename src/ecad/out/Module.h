// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   ecad/out/Module.h
/// @brief  Identity of the ecad/out module.
/// @req    B-02

#pragma once

#include "core/Module.h"

/// @brief Manufacturing outputs: Gerber X2, Excellon, IPC-2581, pick-and-place, IPC-D-356 and BOM.
///
/// See the module page in src/ecad/out/README.md for its purpose and allowed dependencies.
namespace app::ecad::out
{

/// @brief  Describe the ecad/out module.
/// @return The module name "ecad/out" and the product version core::kVersion.
[[nodiscard]] core::ModuleInfo moduleInfo();

} // namespace app::ecad::out
