// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   drw/ModuleTests.cpp
/// @brief  Unit tests for the identity of the drw module (T-01).

#include "drw/Module.h"

#include <gtest/gtest.h>

TEST( DrwModule, ReportsNameAndProductVersion_WhenQueried )
{
    const app::core::ModuleInfo info = app::drw::moduleInfo();

    EXPECT_EQ( info.name, "drw" );
    EXPECT_EQ( info.version, app::core::kVersion );
}
