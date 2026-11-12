// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   core/Module.cpp
/// @brief  Identity of the core module.

#include "core/Module.h"

namespace app::core
{

ModuleInfo moduleInfo()
{
    return ModuleInfo{ "core", kVersion };
}

} // namespace app::core
