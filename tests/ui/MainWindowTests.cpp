// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   ui/MainWindowTests.cpp
/// @brief  Display-free tests for the main window (T-01).

#include "ui/MainWindow.h"

#include <gtest/gtest.h>
#include <QApplication>
#include <QString>
#include <QtGlobal>

#include <memory>

namespace
{

// QApplication keeps references to argc and argv for its whole lifetime.
int   s_argc      = 1;
char  s_appName[] = "Ui.Tests";
char* s_argv[]    = { s_appName, nullptr };

class MainWindowTest : public ::testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        if( QApplication::instance() == nullptr )
        {
            // The vcpkg qtbase build ships the "minimal" platform plugin, which needs no display.
            qputenv( "QT_QPA_PLATFORM", "minimal" );
            s_application = std::make_unique<QApplication>( s_argc, s_argv );
        }
    }

    static void TearDownTestSuite()
    {
        s_application.reset();
    }

    static inline std::unique_ptr<QApplication> s_application;
};

} // namespace

TEST_F( MainWindowTest, HasProductTitle_WhenCreated )
{
    const app::ui::MainWindow window;

    EXPECT_EQ( window.windowTitle(), QString( "APP" ) );
}

TEST_F( MainWindowTest, HasEmptyCentralArea_WhenCreated )
{
    const app::ui::MainWindow window;

    ASSERT_NE( window.centralWidget(), nullptr );
    EXPECT_TRUE( window.centralWidget()->children().isEmpty() );
}
