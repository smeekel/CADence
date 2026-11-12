// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   ui/main.cpp
/// @brief  Entry point of the desktop application.

#include "core/Module.h"
#include "ui/MainWindow.h"

#include <QApplication>
#include <QString>

/// @brief  Start the desktop application and show the main window.
/// @param[in] argc  Number of entries in @p argv.
/// @param[in] argv  Program name followed by the arguments; Qt consumes its own options.
/// @return The exit code of the Qt event loop.
int main( int argc, char* argv[] )
{
    QApplication application( argc, argv );

    QApplication::setApplicationName(
        QString::fromUtf8( app::core::kProductName.data(), static_cast<qsizetype>( app::core::kProductName.size() ) ) );
    QApplication::setApplicationVersion(
        QString::fromUtf8( app::core::kVersion.data(), static_cast<qsizetype>( app::core::kVersion.size() ) ) );

    app::ui::MainWindow window;

    window.show();

    return QApplication::exec();
}
