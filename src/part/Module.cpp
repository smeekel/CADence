// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   part/Module.cpp
/// @brief  Identity of the part module.

#include "part/Module.h"

namespace app::part
{

core::ModuleInfo moduleInfo()
{
    return core::ModuleInfo{ "part", core::kVersion };
}

} // namespace app::part
