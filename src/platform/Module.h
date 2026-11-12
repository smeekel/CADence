// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   platform/Module.h
/// @brief  Identity of the platform module.
/// @req    B-02

#pragma once

#include "core/Module.h"

/// @brief The only place for operating-system specific code: known folders, DPI, crash handler, dynamic loading, system
/// information.
///
/// See the module page in src/platform/README.md for its purpose and allowed dependencies.
namespace app::platform
{

/// @brief  Describe the platform module.
/// @return The module name "platform" and the product version core::kVersion.
[[nodiscard]] core::ModuleInfo moduleInfo();

} // namespace app::platform
