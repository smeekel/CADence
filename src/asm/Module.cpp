// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   asm/Module.cpp
/// @brief  Identity of the asm module.

#include "asm/Module.h"

namespace app::assembly
{

core::ModuleInfo moduleInfo()
{
    return core::ModuleInfo{ "asm", core::kVersion };
}

} // namespace app::assembly
