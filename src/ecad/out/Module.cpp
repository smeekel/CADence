// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   ecad/out/Module.cpp
/// @brief  Identity of the ecad/out module.

#include "ecad/out/Module.h"

namespace app::ecad::out
{

core::ModuleInfo moduleInfo()
{
    return core::ModuleInfo{ "ecad/out", core::kVersion };
}

} // namespace app::ecad::out
