// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   core/Quantity.h
/// @brief  Physical dimension and a value with a dimension, in internal units (millimetres and radians).
/// @req    P-06, A-08

#pragma once

#include "core/Result.h"

#include <string>

namespace app::core
{

/// @brief Exponents of the base dimensions of a quantity: length (internal unit mm) and plane angle (internal unit
///        rad). `{ 2, 0 }` is an area, `{ 0, 0 }` is dimensionless.
struct Dimension
{
    int length = 0; ///< Exponent of length.
    int angle  = 0; ///< Exponent of plane angle.

    /// @brief  Whether every exponent is zero.
    /// @return True for a pure number.
    [[nodiscard]] constexpr bool isDimensionless() const noexcept
    {
        return length == 0 && angle == 0;
    }

    /// @brief Dimensions are equal when all exponents are equal.
    [[nodiscard]] friend constexpr bool operator==( const Dimension&, const Dimension& ) noexcept = default;
};

/// @brief A pure number.
inline constexpr Dimension kDimensionless{ 0, 0 };

/// @brief Length (internal unit mm).
inline constexpr Dimension kLength{ 1, 0 };

/// @brief Plane angle (internal unit rad).
inline constexpr Dimension kAngle{ 0, 1 };

/// @brief  Readable name of a dimension, for error messages: "dimensionless", "length", "angle", "length^2",
///         "length/angle", ...
/// @param  dimension  Dimension to describe.
/// @return The name.
[[nodiscard]] std::string describe( const Dimension& dimension );

/// @brief A value with a dimension, stored in internal units (mm for length, rad for angle).
///
/// Multiplication and division combine dimensions and always succeed. Addition and subtraction need equal dimensions
/// and return an error otherwise, which is how the expression engine reports `1 mm + 1 deg` (A-08).
class Quantity
{
public:
    /// @brief The dimensionless zero.
    constexpr Quantity() noexcept = default;

    /// @brief Quantity from a value in internal units and its dimension.
    /// @param value      Value in internal units. @units mm and rad per @p dimension
    /// @param dimension  Dimension of the value.
    constexpr Quantity( double value, Dimension dimension ) noexcept
        : m_value( value )
        , m_dimension( dimension )
    {
    }

    /// @brief  A dimensionless number.
    /// @param  value  The number.
    /// @return The quantity.
    [[nodiscard]] static constexpr Quantity number( double value ) noexcept
    {
        return { value, kDimensionless };
    }

    /// @brief  A length.
    /// @param  millimetres  Length. @units mm
    /// @return The quantity.
    [[nodiscard]] static constexpr Quantity length( double millimetres ) noexcept
    {
        return { millimetres, kLength };
    }

    /// @brief  A plane angle.
    /// @param  radians  Angle. @units rad
    /// @return The quantity.
    [[nodiscard]] static constexpr Quantity angle( double radians ) noexcept
    {
        return { radians, kAngle };
    }

    /// @brief  Value in internal units.
    /// @return The value. @units mm and rad per dimension()
    [[nodiscard]] constexpr double value() const noexcept
    {
        return m_value;
    }

    /// @brief  Dimension of the value.
    /// @return The dimension.
    [[nodiscard]] constexpr Dimension dimension() const noexcept
    {
        return m_dimension;
    }

    /// @brief  Product; dimensions add.
    /// @param  lhs  Left factor.
    /// @param  rhs  Right factor.
    /// @return `lhs * rhs`.
    [[nodiscard]] friend constexpr Quantity operator*( const Quantity& lhs, const Quantity& rhs ) noexcept
    {
        return { lhs.m_value * rhs.m_value,
                 { lhs.m_dimension.length + rhs.m_dimension.length, lhs.m_dimension.angle + rhs.m_dimension.angle } };
    }

    /// @brief  Quotient; dimensions subtract. Division by zero follows IEEE 754 (infinity or NaN).
    /// @param  lhs  Dividend.
    /// @param  rhs  Divisor.
    /// @return `lhs / rhs`.
    [[nodiscard]] friend constexpr Quantity operator/( const Quantity& lhs, const Quantity& rhs ) noexcept
    {
        return { lhs.m_value / rhs.m_value,
                 { lhs.m_dimension.length - rhs.m_dimension.length, lhs.m_dimension.angle - rhs.m_dimension.angle } };
    }

    /// @brief  Negation; the dimension is unchanged.
    /// @param  operand  Quantity to negate.
    /// @return `-operand`.
    [[nodiscard]] friend constexpr Quantity operator-( const Quantity& operand ) noexcept
    {
        return { -operand.m_value, operand.m_dimension };
    }

    /// @brief Quantities are equal when value and dimension are equal. Use a tolerance (Tolerance.h) for computed
    ///        values.
    [[nodiscard]] friend constexpr bool operator==( const Quantity&, const Quantity& ) noexcept = default;

private:
    double    m_value = 0.0;
    Dimension m_dimension;
};

/// @brief  Sum of two quantities of the same dimension.
/// @param  lhs  First term.
/// @param  rhs  Second term.
/// @return `lhs + rhs`, or ErrorCode::InvalidArgument naming both dimensions when they differ.
[[nodiscard]] Result<Quantity> add( const Quantity& lhs, const Quantity& rhs );

/// @brief  Difference of two quantities of the same dimension.
/// @param  lhs  Minuend.
/// @param  rhs  Subtrahend.
/// @return `lhs - rhs`, or ErrorCode::InvalidArgument naming both dimensions when they differ.
[[nodiscard]] Result<Quantity> subtract( const Quantity& lhs, const Quantity& rhs );

} // namespace app::core
