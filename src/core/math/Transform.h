// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   core/math/Transform.h
/// @brief  Affine 3D transform (linear part plus translation), stored as an Eigen affine transform (ADR 0022).
/// @req    P-06

#pragma once

#include "core/math/Mat.h"
#include "core/math/Vec.h"

#include <Eigen/Geometry>

#include <cmath>
#include <optional>

namespace app::core::math
{

/// @brief Affine transform of 3D space: `p' = L p + t`. Lengths are in millimetres.
///
/// Composition reads right to left: `( a * b ).applyToPoint( p )` equals `a.applyToPoint( b.applyToPoint( p ) )`.
/// Callers use this API; eigen() is for the modules rule 030 allows to use Eigen directly.
class Transform
{
public:
    /// @brief The identity transform.
    Transform() noexcept = default;

    /// @brief Transform from an Eigen affine transform.
    /// @param t  The transform.
    explicit Transform( const Eigen::Affine3d& t ) noexcept
        : m_t( t )
    {
    }

    /// @brief Transform from a linear part and a translation.
    /// @param linear       Linear part L.
    /// @param translation  Translation t. @units mm
    Transform( const Mat3& linear, const Vec3& translation ) noexcept
    {
        m_t.linear()      = linear.eigen();
        m_t.translation() = translation.eigen();
    }

    /// @brief  The identity transform.
    /// @return I.
    [[nodiscard]] static Transform identity() noexcept
    {
        return {};
    }

    /// @brief  Translation by a vector.
    /// @param  offset  Translation. @units mm
    /// @return The transform.
    [[nodiscard]] static Transform translation( const Vec3& offset ) noexcept
    {
        return Transform( Mat3::identity(), offset );
    }

    /// @brief  Rotation about an axis through a point (right-hand rule).
    /// @param  origin   A point on the axis. @units mm
    /// @param  axis     Axis direction; any non-zero length.
    /// @param  radians  Angle. @units rad
    /// @return The transform, or `std::nullopt` when @p axis has zero or non-finite length.
    [[nodiscard]] static std::optional<Transform>
    rotation( const Vec3& origin, const Vec3& axis, double radians ) noexcept
    {
        const std::optional<Vec3> unitAxis = normalized( axis );

        if( !unitAxis.has_value() )
        {
            return std::nullopt;
        }

        const Mat3 r = Mat3::rotation( *unitAxis, radians );

        return Transform( r, origin - r * origin );
    }

    /// @brief  Uniform scaling about the origin.
    /// @param  factor  Scale factor.
    /// @return The transform.
    [[nodiscard]] static Transform scaling( double factor ) noexcept
    {
        return Transform( Mat3::identity() * factor, Vec3() );
    }

    /// @brief  Linear part L.
    /// @return The 3x3 matrix.
    [[nodiscard]] Mat3 linear() const noexcept
    {
        return Mat3( Eigen::Matrix3d( m_t.linear() ) );
    }

    /// @brief  Translation part t.
    /// @return The translation. @units mm
    [[nodiscard]] Vec3 translationPart() const noexcept
    {
        return Vec3( Eigen::Vector3d( m_t.translation() ) );
    }

    /// @brief  The underlying Eigen transform.
    /// @return The transform.
    [[nodiscard]] const Eigen::Affine3d& eigen() const noexcept
    {
        return m_t;
    }

    /// @brief  Apply to a point (linear part and translation).
    /// @param  p  Point. @units mm
    /// @return `L p + t`. @units mm
    [[nodiscard]] Vec3 applyToPoint( const Vec3& p ) const noexcept
    {
        return Vec3( Eigen::Vector3d( m_t * p.eigen() ) );
    }

    /// @brief  Apply to a direction or displacement (linear part only).
    /// @param  v  Vector. @units mm
    /// @return `L v`. @units mm
    [[nodiscard]] Vec3 applyToVector( const Vec3& v ) const noexcept
    {
        return Vec3( Eigen::Vector3d( m_t.linear() * v.eigen() ) );
    }

    /// @brief  Inverse transform.
    /// @return The inverse, or `std::nullopt` when the linear part is singular.
    [[nodiscard]] std::optional<Transform> inverse() const noexcept
    {
        if( !linear().inverse().has_value() )
        {
            return std::nullopt;
        }

        return Transform( Eigen::Affine3d( m_t.inverse( Eigen::Affine ) ) );
    }

    /// @brief  Whether the transform preserves lengths and orientation: `LᵀL = I` and `det L = +1`, element-wise
    ///         within @p tolerance.
    /// @param  tolerance  Allowed deviation of each element.
    /// @return True for rotations and translations and their combinations.
    [[nodiscard]] bool isRigid( double tolerance = 1e-12 ) const noexcept
    {
        const Eigen::Matrix3d l = m_t.linear();

        return ( l.transpose() * l - Eigen::Matrix3d::Identity() ).cwiseAbs().maxCoeff() <= tolerance
               && std::abs( l.determinant() - 1.0 ) <= tolerance;
    }

    /// @brief Composition: `a * b` applies @p b first, then @p a.
    [[nodiscard]] friend Transform operator*( const Transform& a, const Transform& b ) noexcept
    {
        return Transform( Eigen::Affine3d( a.m_t * b.m_t ) );
    }

private:
    Eigen::Affine3d m_t = Eigen::Affine3d::Identity();
};

} // namespace app::core::math
