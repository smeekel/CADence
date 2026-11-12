// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   render/ModuleTests.cpp
/// @brief  Unit tests for the identity of the render module (T-01).

#include "render/Module.h"

#include <gtest/gtest.h>

TEST( RenderModule, ReportsNameAndProductVersion_WhenQueried )
{
    const app::core::ModuleInfo info = app::render::moduleInfo();

    EXPECT_EQ( info.name, "render" );
    EXPECT_EQ( info.version, app::core::kVersion );
}
