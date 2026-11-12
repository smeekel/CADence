// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   platform/Module.cpp
/// @brief  Identity of the platform module.

#include "platform/Module.h"

namespace app::platform
{

core::ModuleInfo moduleInfo()
{
    return core::ModuleInfo{ "platform", core::kVersion };
}

} // namespace app::platform
