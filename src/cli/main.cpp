// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   cli/main.cpp
/// @brief  Entry point of the headless command-line runner.

#include "cli/CliApp.h"

#include <cstdio>
#include <exception>
#include <iostream>
#include <span>
#include <string_view>
#include <vector>

/// @brief  Start the command-line runner.
/// @param[in] argc  Number of entries in @p argv.
/// @param[in] argv  Program name followed by the arguments.
/// @return The exit code of app::cli::run(), or app::cli::ExitCode::InternalError when an exception escapes it.
int main( int argc, char* argv[] )
{
    try
    {
        const std::span<char*>        raw( argv, static_cast<std::size_t>( argc ) );
        std::vector<std::string_view> args;

        for( const char* arg : raw.subspan( 1 ) )
        {
            args.emplace_back( arg );
        }

        return static_cast<int>( app::cli::run( args, std::cout, std::cerr ) );
    }
    catch( const std::exception& error )
    {
        // C stdio never throws, so nothing can escape main from the handlers.
        std::fputs( "APP: internal error: ", stderr );
        std::fputs( error.what(), stderr );
        std::fputs( "\n", stderr );
    }
    catch( ... )
    {
        std::fputs( "APP: internal error\n", stderr );
    }

    return static_cast<int>( app::cli::ExitCode::InternalError );
}
