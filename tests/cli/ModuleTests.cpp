// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   cli/ModuleTests.cpp
/// @brief  Unit tests for the identity of the cli module (T-01).

#include "cli/Module.h"

#include <gtest/gtest.h>

TEST( CliModule, ReportsNameAndProductVersion_WhenQueried )
{
    const app::core::ModuleInfo info = app::cli::moduleInfo();

    EXPECT_EQ( info.name, "cli" );
    EXPECT_EQ( info.version, app::core::kVersion );
}
