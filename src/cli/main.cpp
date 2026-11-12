// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   cli/main.cpp
/// @brief  Entry point of the headless command-line runner.

#include "cli/CliApp.h"

#include <iostream>
#include <span>
#include <string_view>
#include <vector>

/// @brief  Start the command-line runner.
/// @param[in] argc  Number of entries in @p argv.
/// @param[in] argv  Program name followed by the arguments.
/// @return The exit code of app::cli::run().
int main( int argc, char* argv[] )
{
    const std::span<char*>        raw( argv, static_cast<std::size_t>( argc ) );
    std::vector<std::string_view> args;

    for( const char* arg : raw.subspan( 1 ) )
    {
        args.emplace_back( arg );
    }

    return static_cast<int>( app::cli::run( args, std::cout, std::cerr ) );
}
