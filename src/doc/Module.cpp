// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   doc/Module.cpp
/// @brief  Identity of the doc module.

#include "doc/Module.h"

namespace app::doc
{

core::ModuleInfo moduleInfo()
{
    return core::ModuleInfo{ "doc", core::kVersion };
}

} // namespace app::doc
