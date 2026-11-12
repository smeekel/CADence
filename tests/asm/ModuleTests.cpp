// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   asm/ModuleTests.cpp
/// @brief  Unit tests for the identity of the asm module (T-01).

#include "asm/Module.h"

#include <gtest/gtest.h>

TEST( AsmModule, ReportsNameAndProductVersion_WhenQueried )
{
    const app::core::ModuleInfo info = app::assembly::moduleInfo();

    EXPECT_EQ( info.name, "asm" );
    EXPECT_EQ( info.version, app::core::kVersion );
}
