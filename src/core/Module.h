// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   core/Module.h
/// @brief  Product name and version, and the description every module gives of itself.
/// @req    B-02

#pragma once

#include <string_view>

#if !defined( APP_VERSION_MAJOR ) || !defined( APP_VERSION_MINOR ) || !defined( APP_VERSION_PATCH ) \
    || !defined( APP_VERSION_STRING )
#    error "APP_VERSION_* must be defined by build/props/Cpp.props"
#endif

/// @brief Root namespace of APP. Each module has one sub-namespace (rule 000).
namespace app
{
}

/// @brief Foundation services shared by every module: math, units, expressions, identities, commands and jobs.
///
/// See the module page in src/core/README.md for its purpose and allowed dependencies.
namespace app::core
{

/// @brief Working name of the product, shown in window titles and version output.
inline constexpr std::string_view kProductName = "APP";

/// @brief Major part of the product version (semantic versioning).
inline constexpr int kVersionMajor = APP_VERSION_MAJOR;

/// @brief Minor part of the product version.
inline constexpr int kVersionMinor = APP_VERSION_MINOR;

/// @brief Patch part of the product version.
inline constexpr int kVersionPatch = APP_VERSION_PATCH;

/// @brief Product version as "major.minor.patch", set once in build/props/Cpp.props.
inline constexpr std::string_view kVersion = APP_VERSION_STRING;

/// @brief Name and version of one module, as returned by its `moduleInfo()` function.
struct ModuleInfo
{
    std::string_view name;    ///< Module folder under src/, for example "ecad/pcb".
    std::string_view version; ///< Module version; equal to kVersion while all modules ship together.
};

/// @brief  Describe the core module.
/// @return The module name "core" and the product version kVersion.
[[nodiscard]] ModuleInfo moduleInfo();

} // namespace app::core
