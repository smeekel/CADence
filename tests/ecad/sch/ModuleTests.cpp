// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   ecad/sch/ModuleTests.cpp
/// @brief  Unit tests for the identity of the ecad/sch module (T-01).

#include "ecad/sch/Module.h"

#include <gtest/gtest.h>

TEST( EcadSchModule, ReportsNameAndProductVersion_WhenQueried )
{
    const app::core::ModuleInfo info = app::ecad::sch::moduleInfo();

    EXPECT_EQ( info.name, "ecad/sch" );
    EXPECT_EQ( info.version, app::core::kVersion );
}
