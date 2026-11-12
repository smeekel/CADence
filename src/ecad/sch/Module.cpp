// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   ecad/sch/Module.cpp
/// @brief  Identity of the ecad/sch module.

#include "ecad/sch/Module.h"

namespace app::ecad::sch
{

core::ModuleInfo moduleInfo()
{
    return core::ModuleInfo{ "ecad/sch", core::kVersion };
}

} // namespace app::ecad::sch
