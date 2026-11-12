// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   ui/ModuleTests.cpp
/// @brief  Unit tests for the identity of the ui module (T-01).

#include "ui/Module.h"

#include <gtest/gtest.h>

TEST( UiModule, ReportsNameAndProductVersion_WhenQueried )
{
    const app::core::ModuleInfo info = app::ui::moduleInfo();

    EXPECT_EQ( info.name, "ui" );
    EXPECT_EQ( info.version, app::core::kVersion );
}
