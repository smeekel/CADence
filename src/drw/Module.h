// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   drw/Module.h
/// @brief  Identity of the drw module.
/// @req    B-02

#pragma once

#include "core/Module.h"

/// @brief Drawings: sheets, views, annotations, tables and PDF/DXF/SVG output.
///
/// See the module page in src/drw/README.md for its purpose and allowed dependencies.
namespace app::drw
{

/// @brief  Describe the drw module.
/// @return The module name "drw" and the product version core::kVersion.
[[nodiscard]] core::ModuleInfo moduleInfo();

} // namespace app::drw
