// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   core/math/IntGeom.h
/// @brief  Integer-nanometre 2D geometry for the PCB board frame, with overflow-checked arithmetic.
///
/// Board geometry is stored as signed 64-bit nanometres (A-09). Everything here is exact integer arithmetic: there is
/// no floating point and no conversion to millimetres, which happens only in `link` and `render`. Operations that can
/// overflow return a Result instead of wrapping.
///
/// @req    A-09, P-06, P-07

#pragma once

#include "core/Result.h"

#include <algorithm>
#include <cstdint>

namespace app::core::math
{

/// @brief A signed length or coordinate in nanometres. @units nm
using Nm = std::int64_t;

/// @brief Displacement in the board frame. @units nm
struct NmVector
{
    Nm x = 0; ///< X component. @units nm
    Nm y = 0; ///< Y component. @units nm

    /// @brief Vectors are equal when both components are equal.
    [[nodiscard]] friend constexpr bool operator==( const NmVector&, const NmVector& ) noexcept = default;
};

/// @brief Point in the board frame. @units nm
struct NmPoint
{
    Nm x = 0; ///< X coordinate. @units nm
    Nm y = 0; ///< Y coordinate. @units nm

    /// @brief Points are equal when both coordinates are equal.
    [[nodiscard]] friend constexpr bool operator==( const NmPoint&, const NmPoint& ) noexcept = default;
};

/// @brief  `a + b`, or an error when the result does not fit in 64 bits.
/// @param  a  First term. @units nm
/// @param  b  Second term. @units nm
/// @return The sum, or ErrorCode::InvalidArgument "Integer overflow".
[[nodiscard]] Result<Nm> checkedAdd( Nm a, Nm b );

/// @brief  `a - b`, or an error when the result does not fit in 64 bits.
/// @param  a  Minuend. @units nm
/// @param  b  Subtrahend. @units nm
/// @return The difference, or ErrorCode::InvalidArgument "Integer overflow".
[[nodiscard]] Result<Nm> checkedSubtract( Nm a, Nm b );

/// @brief  `a * b`, or an error when the result does not fit in 64 bits.
/// @param  a  First factor.
/// @param  b  Second factor.
/// @return The product, or ErrorCode::InvalidArgument "Integer overflow".
[[nodiscard]] Result<Nm> checkedMultiply( Nm a, Nm b );

/// @brief  Point moved by a vector.
/// @param  p  Point.
/// @param  v  Displacement.
/// @return `p + v`, or an overflow error.
[[nodiscard]] Result<NmPoint> translate( const NmPoint& p, const NmVector& v );

/// @brief  Vector from one point to another.
/// @param  from  Start point.
/// @param  to    End point.
/// @return `to - from`, or an overflow error.
[[nodiscard]] Result<NmVector> difference( const NmPoint& from, const NmPoint& to );

/// @brief  Sum of two vectors.
/// @param  a  First vector.
/// @param  b  Second vector.
/// @return `a + b`, or an overflow error.
[[nodiscard]] Result<NmVector> add( const NmVector& a, const NmVector& b );

/// @brief  Vector scaled by an integer factor.
/// @param  v       Vector.
/// @param  factor  Integer factor.
/// @return `v * factor`, or an overflow error.
[[nodiscard]] Result<NmVector> scale( const NmVector& v, Nm factor );

/// @brief  Dot product. @units nmÂ²
/// @param  a  First vector.
/// @param  b  Second vector.
/// @return `a Â· b`, or an overflow error (the result exceeds 64 bits for lengths above about 3 m).
[[nodiscard]] Result<Nm> dot( const NmVector& a, const NmVector& b );

/// @brief  Z component of the cross product; positive when @p b is counter-clockwise of @p a. @units nmÂ²
/// @param  a  First vector.
/// @param  b  Second vector.
/// @return `a.x * b.y - a.y * b.x`, or an overflow error.
[[nodiscard]] Result<Nm> cross( const NmVector& a, const NmVector& b );

/// @brief  Squared length, for exact distance comparisons without square roots. @units nmÂ²
/// @param  v  Vector.
/// @return `v Â· v`, or an overflow error.
[[nodiscard]] Result<Nm> lengthSquared( const NmVector& v );

/// @brief Axis-aligned rectangle in the board frame with inclusive edges (`min <= max` on both axes). @units nm
class NmRect
{
public:
    /// @brief The degenerate rectangle at the origin.
    constexpr NmRect() noexcept = default;

    /// @brief  Smallest rectangle containing two corner points, in any order.
    /// @param  a  First corner.
    /// @param  b  Second corner.
    /// @return The rectangle.
    [[nodiscard]] static constexpr NmRect fromCorners( const NmPoint& a, const NmPoint& b ) noexcept
    {
        NmRect rect;

        rect.m_min = { std::min( a.x, b.x ), std::min( a.y, b.y ) };
        rect.m_max = { std::max( a.x, b.x ), std::max( a.y, b.y ) };
        return rect;
    }

    /// @brief  Minimum corner.
    /// @return The corner.
    [[nodiscard]] constexpr NmPoint min() const noexcept
    {
        return m_min;
    }

    /// @brief  Maximum corner.
    /// @return The corner.
    [[nodiscard]] constexpr NmPoint max() const noexcept
    {
        return m_max;
    }

    /// @brief  Width along x.
    /// @return `max.x - min.x`, or an overflow error. @units nm
    [[nodiscard]] Result<Nm> width() const
    {
        return checkedSubtract( m_max.x, m_min.x );
    }

    /// @brief  Height along y.
    /// @return `max.y - min.y`, or an overflow error. @units nm
    [[nodiscard]] Result<Nm> height() const
    {
        return checkedSubtract( m_max.y, m_min.y );
    }

    /// @brief  Whether a point lies inside or on the edge.
    /// @param  p  Point.
    /// @return True when `min <= p <= max` on both axes.
    [[nodiscard]] constexpr bool contains( const NmPoint& p ) const noexcept
    {
        return p.x >= m_min.x && p.x <= m_max.x && p.y >= m_min.y && p.y <= m_max.y;
    }

    /// @brief  Whether two rectangles share at least one point (touching edges count).
    /// @param  other  Other rectangle.
    /// @return True when they overlap or touch.
    [[nodiscard]] constexpr bool intersects( const NmRect& other ) const noexcept
    {
        return m_min.x <= other.m_max.x && other.m_min.x <= m_max.x && m_min.y <= other.m_max.y
               && other.m_min.y <= m_max.y;
    }

    /// @brief  Smallest rectangle containing both rectangles.
    /// @param  other  Other rectangle.
    /// @return The union bounds.
    [[nodiscard]] constexpr NmRect united( const NmRect& other ) const noexcept
    {
        return fromCorners( { std::min( m_min.x, other.m_min.x ), std::min( m_min.y, other.m_min.y ) },
                            { std::max( m_max.x, other.m_max.x ), std::max( m_max.y, other.m_max.y ) } );
    }

    /// @brief  Rectangle grown by @p distance on every side (shrunk when negative).
    /// @param  distance  Distance to add on each side. @units nm
    /// @return The rectangle, an overflow error, or ErrorCode::InvalidArgument when shrinking would invert it.
    [[nodiscard]] Result<NmRect> inflated( Nm distance ) const;

    /// @brief Rectangles are equal when both corners are equal.
    [[nodiscard]] friend constexpr bool operator==( const NmRect&, const NmRect& ) noexcept = default;

private:
    NmPoint m_min;
    NmPoint m_max;
};

} // namespace app::core::math
