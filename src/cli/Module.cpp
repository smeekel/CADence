// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   cli/Module.cpp
/// @brief  Identity of the cli module.

#include "cli/Module.h"

namespace app::cli
{

core::ModuleInfo moduleInfo()
{
    return core::ModuleInfo{ "cli", core::kVersion };
}

} // namespace app::cli
