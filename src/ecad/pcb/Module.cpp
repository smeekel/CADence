// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   ecad/pcb/Module.cpp
/// @brief  Identity of the ecad/pcb module.

#include "ecad/pcb/Module.h"

namespace app::ecad::pcb
{

core::ModuleInfo moduleInfo()
{
    return core::ModuleInfo{ "ecad/pcb", core::kVersion };
}

} // namespace app::ecad::pcb
