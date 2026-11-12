// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   render/Module.cpp
/// @brief  Identity of the render module.

#include "render/Module.h"

namespace app::render
{

core::ModuleInfo moduleInfo()
{
    return core::ModuleInfo{ "render", core::kVersion };
}

} // namespace app::render
