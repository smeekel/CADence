// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   cli/CliAppTests.cpp
/// @brief  Unit tests for argument handling of the command-line runner (T-01).

#include "cli/CliApp.h"
#include "core/Module.h"

#include <gtest/gtest.h>

#include <array>
#include <sstream>
#include <string>
#include <string_view>

namespace
{

std::string expectedVersionLine()
{
    return std::string( app::core::kProductName ) + " " + std::string( app::core::kVersion ) + "\n";
}

} // namespace

TEST( CliApp, PrintsVersion_WhenVersionRequested )
{
    const std::array<std::string_view, 1> args = { "--version" };
    std::ostringstream                    out;
    std::ostringstream                    err;

    const app::cli::ExitCode code = app::cli::run( args, out, err );

    EXPECT_EQ( code, app::cli::ExitCode::Success );
    EXPECT_EQ( out.str(), expectedVersionLine() );
    EXPECT_TRUE( err.str().empty() );
}

TEST( CliApp, PrintsVersion_WhenNoArguments )
{
    std::ostringstream out;
    std::ostringstream err;

    const app::cli::ExitCode code = app::cli::run( {}, out, err );

    EXPECT_EQ( code, app::cli::ExitCode::Success );
    EXPECT_EQ( out.str(), expectedVersionLine() );
}

TEST( CliApp, ReportsUsageError_WhenArgumentUnknown )
{
    const std::array<std::string_view, 1> args = { "--no-such-option" };
    std::ostringstream                    out;
    std::ostringstream                    err;

    const app::cli::ExitCode code = app::cli::run( args, out, err );

    EXPECT_EQ( code, app::cli::ExitCode::UsageError );
    EXPECT_TRUE( out.str().empty() );
    EXPECT_NE( err.str().find( "Usage:" ), std::string::npos );
}
