// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   ui/MainWindow.cpp
/// @brief  Top-level application window.

#include "ui/MainWindow.h"

#include "core/Module.h"

#include <QString>
#include <QWidget>

namespace app::ui
{

MainWindow::MainWindow( QWidget* parent )
    : QMainWindow( parent )
{
    const std::string_view title = core::kProductName;

    setWindowTitle( QString::fromUtf8( title.data(), static_cast<qsizetype>( title.size() ) ) );
    setCentralWidget( new QWidget( this ) );
    resize( 1280, 800 );
}

} // namespace app::ui
