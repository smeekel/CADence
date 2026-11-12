// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   ecad/lib/ModuleTests.cpp
/// @brief  Unit tests for the identity of the ecad/lib module (T-01).

#include "ecad/lib/Module.h"

#include <gtest/gtest.h>

TEST( EcadLibModule, ReportsNameAndProductVersion_WhenQueried )
{
    const app::core::ModuleInfo info = app::ecad::lib::moduleInfo();

    EXPECT_EQ( info.name, "ecad/lib" );
    EXPECT_EQ( info.version, app::core::kVersion );
}
