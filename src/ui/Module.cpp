// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   ui/Module.cpp
/// @brief  Identity of the ui module.

#include "ui/Module.h"

namespace app::ui
{

core::ModuleInfo moduleInfo()
{
    return core::ModuleInfo{ "ui", core::kVersion };
}

} // namespace app::ui
