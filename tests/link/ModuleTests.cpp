// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   link/ModuleTests.cpp
/// @brief  Unit tests for the identity of the link module (T-01).

#include "link/Module.h"

#include <gtest/gtest.h>

TEST( LinkModule, ReportsNameAndProductVersion_WhenQueried )
{
    const app::core::ModuleInfo info = app::link::moduleInfo();

    EXPECT_EQ( info.name, "link" );
    EXPECT_EQ( info.version, app::core::kVersion );
}
