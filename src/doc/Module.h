// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   doc/Module.h
/// @brief  Identity of the doc module.
/// @req    B-02

#pragma once

#include "core/Module.h"

/// @brief Document model and project container, JSON and BSON encodings, magic-UUID sniffing, migrations and
/// library-scope resolution.
///
/// See the module page in src/doc/README.md for its purpose and allowed dependencies.
namespace app::doc
{

/// @brief  Describe the doc module.
/// @return The module name "doc" and the product version core::kVersion.
[[nodiscard]] core::ModuleInfo moduleInfo();

} // namespace app::doc
