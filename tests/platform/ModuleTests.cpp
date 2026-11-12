// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   platform/ModuleTests.cpp
/// @brief  Unit tests for the identity of the platform module (T-01).

#include "platform/Module.h"

#include <gtest/gtest.h>

TEST( PlatformModule, ReportsNameAndProductVersion_WhenQueried )
{
    const app::core::ModuleInfo info = app::platform::moduleInfo();

    EXPECT_EQ( info.name, "platform" );
    EXPECT_EQ( info.version, app::core::kVersion );
}
