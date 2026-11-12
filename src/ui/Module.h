// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   ui/Module.h
/// @brief  Identity of the ui module.
/// @req    B-02

#pragma once

#include "core/Module.h"

/// @brief Qt 6 Widgets desktop application: main window, panels, tools and themes. The only module that uses Qt
/// Widgets.
///
/// See the module page in src/ui/README.md for its purpose and allowed dependencies.
namespace app::ui
{

/// @brief  Describe the ui module.
/// @return The module name "ui" and the product version core::kVersion.
[[nodiscard]] core::ModuleInfo moduleInfo();

} // namespace app::ui
