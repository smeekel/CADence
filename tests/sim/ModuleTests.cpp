// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   sim/ModuleTests.cpp
/// @brief  Unit tests for the identity of the sim module (T-01).

#include "sim/Module.h"

#include <gtest/gtest.h>

TEST( SimModule, ReportsNameAndProductVersion_WhenQueried )
{
    const app::core::ModuleInfo info = app::sim::moduleInfo();

    EXPECT_EQ( info.name, "sim" );
    EXPECT_EQ( info.version, app::core::kVersion );
}
