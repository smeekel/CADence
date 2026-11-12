// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   cli/Module.h
/// @brief  Identity of the cli module.
/// @req    B-02

#pragma once

#include "core/Module.h"

/// @brief Headless command-line runner: version, command scripts, journal replay and batch export (A-02).
///
/// See the module page in src/cli/README.md for its purpose and allowed dependencies.
namespace app::cli
{

/// @brief  Describe the cli module.
/// @return The module name "cli" and the product version core::kVersion.
[[nodiscard]] core::ModuleInfo moduleInfo();

} // namespace app::cli
