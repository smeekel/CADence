// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   doc/ModuleTests.cpp
/// @brief  Unit tests for the identity of the doc module (T-01).

#include "doc/Module.h"

#include <gtest/gtest.h>

TEST( DocModule, ReportsNameAndProductVersion_WhenQueried )
{
    const app::core::ModuleInfo info = app::doc::moduleInfo();

    EXPECT_EQ( info.name, "doc" );
    EXPECT_EQ( info.version, app::core::kVersion );
}
