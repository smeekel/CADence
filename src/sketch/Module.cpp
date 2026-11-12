// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   sketch/Module.cpp
/// @brief  Identity of the sketch module.

#include "sketch/Module.h"

namespace app::sketch
{

core::ModuleInfo moduleInfo()
{
    return core::ModuleInfo{ "sketch", core::kVersion };
}

} // namespace app::sketch
