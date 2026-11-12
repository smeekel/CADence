// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   part/ModuleTests.cpp
/// @brief  Unit tests for the identity of the part module (T-01).

#include "part/Module.h"

#include <gtest/gtest.h>

TEST( PartModule, ReportsNameAndProductVersion_WhenQueried )
{
    const app::core::ModuleInfo info = app::part::moduleInfo();

    EXPECT_EQ( info.name, "part" );
    EXPECT_EQ( info.version, app::core::kVersion );
}
