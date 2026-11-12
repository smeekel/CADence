// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   cli/CliApp.cpp
/// @brief  Argument handling of the headless command-line runner.

#include "cli/CliApp.h"

#include "core/Module.h"

#include <ostream>

namespace app::cli
{

namespace
{

constexpr std::string_view kUsage = "Usage: Cli [--version | --help]\n";

void printVersion( std::ostream& out )
{
    out << core::kProductName << ' ' << core::kVersion << '\n';
}

} // namespace

ExitCode run( std::span<const std::string_view> args, std::ostream& out, std::ostream& err )
{
    if( args.empty() || ( args.size() == 1 && args.front() == "--version" ) )
    {
        printVersion( out );
        return ExitCode::Success;
    }

    if( args.size() == 1 && args.front() == "--help" )
    {
        printVersion( out );
        out << kUsage;
        return ExitCode::Success;
    }

    err << "Unrecognised arguments.\n" << kUsage;

    return ExitCode::UsageError;
}

} // namespace app::cli
