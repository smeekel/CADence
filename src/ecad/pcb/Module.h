// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   ecad/pcb/Module.h
/// @brief  Identity of the ecad/pcb module.
/// @req    B-02

#pragma once

#include "core/Module.h"

/// @brief Board database in integer nanometres, layer stack, placement, routing, copper pours, design rules and DRC.
///
/// See the module page in src/ecad/pcb/README.md for its purpose and allowed dependencies.
namespace app::ecad::pcb
{

/// @brief  Describe the ecad/pcb module.
/// @return The module name "ecad/pcb" and the product version core::kVersion.
[[nodiscard]] core::ModuleInfo moduleInfo();

} // namespace app::ecad::pcb
