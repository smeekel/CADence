// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   core/Result.h
/// @brief  `app::Result<T>`: a value or an app::Error (ADR 0042).
///
/// Every function that can fail across a module boundary returns `Result<T>` and is marked `[[nodiscard]]`
/// (rule 010). Use `app::fail()` to return an error and the `std::expected` monadic operations (`and_then`,
/// `transform`, `or_else`) to chain calls.
///
/// @req    A-02, A-03

#pragma once

#include "core/Error.h"

#include <expected>

namespace app
{

/// @brief A value of type @p T, or the app::Error that prevented it. `Result<>` (that is, `Result<void>`) carries
///        only success or an error.
template <typename T = void>
using Result = std::expected<T, Error>;

/// @brief  The successful `Result<void>`, for `return app::ok();`.
/// @return A `Result<void>` holding success.
[[nodiscard]] inline Result<> ok() noexcept
{
    return {};
}

} // namespace app
