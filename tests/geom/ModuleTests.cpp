// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   geom/ModuleTests.cpp
/// @brief  Unit tests for the identity of the geom module (T-01).

#include "geom/Module.h"

#include <gtest/gtest.h>

TEST( GeomModule, ReportsNameAndProductVersion_WhenQueried )
{
    const app::core::ModuleInfo info = app::geom::moduleInfo();

    EXPECT_EQ( info.name, "geom" );
    EXPECT_EQ( info.version, app::core::kVersion );
}
