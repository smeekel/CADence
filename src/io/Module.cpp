// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   io/Module.cpp
/// @brief  Identity of the io module.

#include "io/Module.h"

namespace app::io
{

core::ModuleInfo moduleInfo()
{
    return core::ModuleInfo{ "io", core::kVersion };
}

} // namespace app::io
