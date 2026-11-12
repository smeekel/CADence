// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   drw/Module.cpp
/// @brief  Identity of the drw module.

#include "drw/Module.h"

namespace app::drw
{

core::ModuleInfo moduleInfo()
{
    return core::ModuleInfo{ "drw", core::kVersion };
}

} // namespace app::drw
