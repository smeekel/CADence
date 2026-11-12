// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   ui/MainWindow.h
/// @brief  Top-level application window.
/// @req    U-01

#pragma once

#include <QMainWindow>

namespace app::ui
{

/// @brief Top-level window of the desktop application.
///
/// The scaffold shows an empty window titled with the product name. Docking panels, the ribbon, the viewport and
/// the theme arrive in Stage 0 (U-01 ... U-06).
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /// @brief Create the window with the product title and an empty central area.
    /// @param[in] parent  Parent widget; nullptr for the top-level window.
    explicit MainWindow( QWidget* parent = nullptr );
};

} // namespace app::ui
