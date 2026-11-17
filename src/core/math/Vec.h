// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   core/math/Vec.h
/// @brief  2D and 3D vectors of doubles for the mechanical frame (millimetres), stored as Eigen vectors (ADR 0022).
/// @req    P-06

#pragma once

#include <Eigen/Geometry>

#include <cmath>
#include <optional>

namespace app::core::math
{

/// @brief 2D vector or point in the mechanical frame. @units mm
///
/// Callers use this API; eigen() is for the modules rule 030 allows to use Eigen directly.
class Vec2
{
public:
    /// @brief The zero vector.
    Vec2() noexcept = default;

    /// @brief Vector from its components.
    /// @param x  X component. @units mm
    /// @param y  Y component. @units mm
    Vec2( double x, double y ) noexcept
        : m_v( x, y )
    {
    }

    /// @brief Vector from an Eigen vector.
    /// @param v  Components. @units mm
    explicit Vec2( const Eigen::Vector2d& v ) noexcept
        : m_v( v )
    {
    }

    /// @brief  X component.
    /// @return The component. @units mm
    [[nodiscard]] double x() const noexcept
    {
        return m_v.x();
    }

    /// @brief  Y component.
    /// @return The component. @units mm
    [[nodiscard]] double y() const noexcept
    {
        return m_v.y();
    }

    /// @brief  The underlying Eigen vector.
    /// @return Components. @units mm
    [[nodiscard]] const Eigen::Vector2d& eigen() const noexcept
    {
        return m_v;
    }

    /// @brief Component-wise sum.
    [[nodiscard]] friend Vec2 operator+( const Vec2& a, const Vec2& b ) noexcept
    {
        return Vec2( a.m_v + b.m_v );
    }

    /// @brief Component-wise difference.
    [[nodiscard]] friend Vec2 operator-( const Vec2& a, const Vec2& b ) noexcept
    {
        return Vec2( a.m_v - b.m_v );
    }

    /// @brief Negation.
    [[nodiscard]] friend Vec2 operator-( const Vec2& v ) noexcept
    {
        return Vec2( Eigen::Vector2d( -v.m_v ) );
    }

    /// @brief Scaling by a factor.
    [[nodiscard]] friend Vec2 operator*( const Vec2& v, double s ) noexcept
    {
        return Vec2( v.m_v * s );
    }

    /// @brief Scaling by a factor.
    [[nodiscard]] friend Vec2 operator*( double s, const Vec2& v ) noexcept
    {
        return Vec2( v.m_v * s );
    }

    /// @brief Division by a factor.
    [[nodiscard]] friend Vec2 operator/( const Vec2& v, double s ) noexcept
    {
        return Vec2( v.m_v / s );
    }

    /// @brief Exact component-wise equality; use a tolerance for computed values.
    [[nodiscard]] friend bool operator==( const Vec2& a, const Vec2& b ) noexcept
    {
        return a.m_v == b.m_v;
    }

private:
    Eigen::Vector2d m_v = Eigen::Vector2d::Zero();
};

/// @brief 3D vector or point in the mechanical frame. @units mm
///
/// Callers use this API; eigen() is for the modules rule 030 allows to use Eigen directly.
class Vec3
{
public:
    /// @brief The zero vector.
    Vec3() noexcept = default;

    /// @brief Vector from its components.
    /// @param x  X component. @units mm
    /// @param y  Y component. @units mm
    /// @param z  Z component. @units mm
    Vec3( double x, double y, double z ) noexcept
        : m_v( x, y, z )
    {
    }

    /// @brief Vector from an Eigen vector.
    /// @param v  Components. @units mm
    explicit Vec3( const Eigen::Vector3d& v ) noexcept
        : m_v( v )
    {
    }

    /// @brief  X component.
    /// @return The component. @units mm
    [[nodiscard]] double x() const noexcept
    {
        return m_v.x();
    }

    /// @brief  Y component.
    /// @return The component. @units mm
    [[nodiscard]] double y() const noexcept
    {
        return m_v.y();
    }

    /// @brief  Z component.
    /// @return The component. @units mm
    [[nodiscard]] double z() const noexcept
    {
        return m_v.z();
    }

    /// @brief  The underlying Eigen vector.
    /// @return Components. @units mm
    [[nodiscard]] const Eigen::Vector3d& eigen() const noexcept
    {
        return m_v;
    }

    /// @brief Component-wise sum.
    [[nodiscard]] friend Vec3 operator+( const Vec3& a, const Vec3& b ) noexcept
    {
        return Vec3( a.m_v + b.m_v );
    }

    /// @brief Component-wise difference.
    [[nodiscard]] friend Vec3 operator-( const Vec3& a, const Vec3& b ) noexcept
    {
        return Vec3( a.m_v - b.m_v );
    }

    /// @brief Negation.
    [[nodiscard]] friend Vec3 operator-( const Vec3& v ) noexcept
    {
        return Vec3( Eigen::Vector3d( -v.m_v ) );
    }

    /// @brief Scaling by a factor.
    [[nodiscard]] friend Vec3 operator*( const Vec3& v, double s ) noexcept
    {
        return Vec3( v.m_v * s );
    }

    /// @brief Scaling by a factor.
    [[nodiscard]] friend Vec3 operator*( double s, const Vec3& v ) noexcept
    {
        return Vec3( v.m_v * s );
    }

    /// @brief Division by a factor.
    [[nodiscard]] friend Vec3 operator/( const Vec3& v, double s ) noexcept
    {
        return Vec3( v.m_v / s );
    }

    /// @brief Exact component-wise equality; use a tolerance for computed values.
    [[nodiscard]] friend bool operator==( const Vec3& a, const Vec3& b ) noexcept
    {
        return a.m_v == b.m_v;
    }

private:
    Eigen::Vector3d m_v = Eigen::Vector3d::Zero();
};

/// @brief  Dot product.
/// @param  a  First vector.
/// @param  b  Second vector.
/// @return `a · b`.
[[nodiscard]] inline double dot( const Vec2& a, const Vec2& b ) noexcept
{
    return a.eigen().dot( b.eigen() );
}

/// @brief  Dot product.
/// @param  a  First vector.
/// @param  b  Second vector.
/// @return `a · b`.
[[nodiscard]] inline double dot( const Vec3& a, const Vec3& b ) noexcept
{
    return a.eigen().dot( b.eigen() );
}

/// @brief  Z component of the 3D cross product of two 2D vectors; positive when @p b is counter-clockwise of @p a.
/// @param  a  First vector.
/// @param  b  Second vector.
/// @return `a.x * b.y - a.y * b.x`.
[[nodiscard]] inline double cross( const Vec2& a, const Vec2& b ) noexcept
{
    return a.x() * b.y() - a.y() * b.x();
}

/// @brief  Cross product (right-handed).
/// @param  a  First vector.
/// @param  b  Second vector.
/// @return `a × b`.
[[nodiscard]] inline Vec3 cross( const Vec3& a, const Vec3& b ) noexcept
{
    return Vec3( a.eigen().cross( b.eigen() ) );
}

/// @brief  Euclidean length.
/// @param  v  Vector.
/// @return `|v|`.
[[nodiscard]] inline double length( const Vec2& v ) noexcept
{
    return v.eigen().norm();
}

/// @brief  Euclidean length.
/// @param  v  Vector.
/// @return `|v|`.
[[nodiscard]] inline double length( const Vec3& v ) noexcept
{
    return v.eigen().norm();
}

/// @brief  Unit vector in the direction of @p v.
/// @param  v  Vector.
/// @return `v / |v|`, or `std::nullopt` when @p v has zero or non-finite length.
[[nodiscard]] inline std::optional<Vec2> normalized( const Vec2& v ) noexcept
{
    const double l = length( v );

    if( !( l > 0.0 ) || !std::isfinite( l ) )
    {
        return std::nullopt;
    }

    return v / l;
}

/// @brief  Unit vector in the direction of @p v.
/// @param  v  Vector.
/// @return `v / |v|`, or `std::nullopt` when @p v has zero or non-finite length.
[[nodiscard]] inline std::optional<Vec3> normalized( const Vec3& v ) noexcept
{
    const double l = length( v );

    if( !( l > 0.0 ) || !std::isfinite( l ) )
    {
        return std::nullopt;
    }

    return v / l;
}

} // namespace app::core::math
