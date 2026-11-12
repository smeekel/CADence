// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   sim/Module.h
/// @brief  Identity of the sim module.
/// @req    B-02

#pragma once

#include "core/Module.h"

/// @brief Simulation: motion (Project Chrono), circuit simulation (ngspice) and tolerance stack-up.
///
/// See the module page in src/sim/README.md for its purpose and allowed dependencies.
namespace app::sim
{

/// @brief  Describe the sim module.
/// @return The module name "sim" and the product version core::kVersion.
[[nodiscard]] core::ModuleInfo moduleInfo();

} // namespace app::sim
