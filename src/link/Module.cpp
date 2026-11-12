// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   link/Module.cpp
/// @brief  Identity of the link module.

#include "link/Module.h"

namespace app::link
{

core::ModuleInfo moduleInfo()
{
    return core::ModuleInfo{ "link", core::kVersion };
}

} // namespace app::link
