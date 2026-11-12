// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   sketch/ModuleTests.cpp
/// @brief  Unit tests for the identity of the sketch module (T-01).

#include "sketch/Module.h"

#include <gtest/gtest.h>

TEST( SketchModule, ReportsNameAndProductVersion_WhenQueried )
{
    const app::core::ModuleInfo info = app::sketch::moduleInfo();

    EXPECT_EQ( info.name, "sketch" );
    EXPECT_EQ( info.version, app::core::kVersion );
}
