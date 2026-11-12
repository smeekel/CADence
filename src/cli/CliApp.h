// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   cli/CliApp.h
/// @brief  Argument handling of the headless command-line runner.
/// @req    A-02

#pragma once

#include <iosfwd>
#include <span>
#include <string_view>

namespace app::cli
{

/// @brief Process exit codes of the command-line runner.
enum class ExitCode : int
{
    Success    = 0, ///< The request completed.
    UsageError = 2, ///< The arguments were not understood; usage was printed to the error stream.
};

/// @brief  Run the command-line runner with already split arguments.
///
/// With no arguments or with `--version`, prints the product name and version, for example "APP 0.0.0", and
/// succeeds. `--help` also prints the usage line.
/// Opening projects and running command scripts arrive in Stage 0 (A-02).
///
/// @param[in]     args  Arguments without the program name.
/// @param[in,out] out   Stream for normal output.
/// @param[in,out] err   Stream for diagnostics and usage errors.
/// @return The process exit code.
[[nodiscard]] ExitCode run( std::span<const std::string_view> args, std::ostream& out, std::ostream& err );

} // namespace app::cli
