// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   io/ModuleTests.cpp
/// @brief  Unit tests for the identity of the io module (T-01).

#include "io/Module.h"

#include <gtest/gtest.h>

TEST( IoModule, ReportsNameAndProductVersion_WhenQueried )
{
    const app::core::ModuleInfo info = app::io::moduleInfo();

    EXPECT_EQ( info.name, "io" );
    EXPECT_EQ( info.version, app::core::kVersion );
}
