// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   ecad/sch/Module.h
/// @brief  Identity of the ecad/sch module.
/// @req    B-02

#pragma once

#include "core/Module.h"

/// @brief Schematic model, netlist compiler, electrical rule check and annotation.
///
/// See the module page in src/ecad/sch/README.md for its purpose and allowed dependencies.
namespace app::ecad::sch
{

/// @brief  Describe the ecad/sch module.
/// @return The module name "ecad/sch" and the product version core::kVersion.
[[nodiscard]] core::ModuleInfo moduleInfo();

} // namespace app::ecad::sch
