// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   core/ModuleTests.cpp
/// @brief  Unit tests for the identity of the core module and the product version (T-01).

#include "core/Module.h"

#include <gtest/gtest.h>

#include <format>
#include <string>

TEST( CoreModule, ReportsNameAndProductVersion_WhenQueried )
{
    const app::core::ModuleInfo info = app::core::moduleInfo();

    EXPECT_EQ( info.name, "core" );
    EXPECT_EQ( info.version, app::core::kVersion );
}

TEST( ProductVersion, StringMatchesNumericParts_WhenFormatted )
{
    const std::string expected =
        std::format( "{}.{}.{}", app::core::kVersionMajor, app::core::kVersionMinor, app::core::kVersionPatch );

    EXPECT_EQ( app::core::kVersion, expected );
}
