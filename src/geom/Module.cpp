// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   geom/Module.cpp
/// @brief  Identity of the geom module.

#include "geom/Module.h"

namespace app::geom
{

core::ModuleInfo moduleInfo()
{
    return core::ModuleInfo{ "geom", core::kVersion };
}

} // namespace app::geom
