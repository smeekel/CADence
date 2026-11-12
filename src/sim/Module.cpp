// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   sim/Module.cpp
/// @brief  Identity of the sim module.

#include "sim/Module.h"

namespace app::sim
{

core::ModuleInfo moduleInfo()
{
    return core::ModuleInfo{ "sim", core::kVersion };
}

} // namespace app::sim
