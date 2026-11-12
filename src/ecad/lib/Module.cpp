// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   ecad/lib/Module.cpp
/// @brief  Identity of the ecad/lib module.

#include "ecad/lib/Module.h"

namespace app::ecad::lib
{

core::ModuleInfo moduleInfo()
{
    return core::ModuleInfo{ "ecad/lib", core::kVersion };
}

} // namespace app::ecad::lib
