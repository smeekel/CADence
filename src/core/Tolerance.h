// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   core/Tolerance.h
/// @brief  Geometric precision constants and comparisons for mechanical values (P-07).
/// @req    P-07

#pragma once

namespace app::core
{

/// @brief Default mechanical confusion tolerance: two points closer than this are the same point. Matches the
///        geometry kernel's confusion tolerance. @units mm
inline constexpr double kConfusionTolerance = 1e-7;

/// @brief  Whether two lengths are equal within a tolerance.
/// @param  a          First length. @units mm
/// @param  b          Second length. @units mm
/// @param  tolerance  Largest difference treated as equal; defaults to kConfusionTolerance. @units mm
/// @return True when `|a - b| <= tolerance`; false if either value is NaN.
[[nodiscard]] constexpr bool isEqualWithin( double a, double b, double tolerance = kConfusionTolerance ) noexcept
{
    const double difference = a - b;

    return difference <= tolerance && -difference <= tolerance;
}

} // namespace app::core
