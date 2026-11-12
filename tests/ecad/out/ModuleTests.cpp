// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   ecad/out/ModuleTests.cpp
/// @brief  Unit tests for the identity of the ecad/out module (T-01).

#include "ecad/out/Module.h"

#include <gtest/gtest.h>

TEST( EcadOutModule, ReportsNameAndProductVersion_WhenQueried )
{
    const app::core::ModuleInfo info = app::ecad::out::moduleInfo();

    EXPECT_EQ( info.name, "ecad/out" );
    EXPECT_EQ( info.version, app::core::kVersion );
}
