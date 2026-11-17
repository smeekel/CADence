// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   core/math/Mat.h
/// @brief  3x3 matrix of doubles for the linear part of transforms, stored as an Eigen matrix (ADR 0022).
/// @req    P-06

#pragma once

#include "core/math/Vec.h"

#include <Eigen/Geometry>

#include <cmath>
#include <optional>

namespace app::core::math
{

/// @brief 3x3 matrix. Multiplying a column vector: `m * v`.
///
/// Callers use this API; eigen() is for the modules rule 030 allows to use Eigen directly.
class Mat3
{
public:
    /// @brief The zero matrix.
    Mat3() noexcept = default;

    /// @brief Matrix from its rows.
    /// @param r0  First row.
    /// @param r1  Second row.
    /// @param r2  Third row.
    Mat3( const Vec3& r0, const Vec3& r1, const Vec3& r2 ) noexcept
    {
        m_m.row( 0 ) = r0.eigen().transpose();
        m_m.row( 1 ) = r1.eigen().transpose();
        m_m.row( 2 ) = r2.eigen().transpose();
    }

    /// @brief Matrix from an Eigen matrix.
    /// @param m  Elements.
    explicit Mat3( const Eigen::Matrix3d& m ) noexcept
        : m_m( m )
    {
    }

    /// @brief  The identity matrix.
    /// @return I.
    [[nodiscard]] static Mat3 identity() noexcept
    {
        return Mat3( Eigen::Matrix3d::Identity() );
    }

    /// @brief  Rotation about an axis through the origin (right-hand rule).
    /// @param  unitAxis  Rotation axis; must have length 1.
    /// @param  radians   Angle. @units rad
    /// @return The rotation matrix.
    [[nodiscard]] static Mat3 rotation( const Vec3& unitAxis, double radians ) noexcept
    {
        return Mat3( Eigen::AngleAxisd( radians, unitAxis.eigen() ).toRotationMatrix() );
    }

    /// @brief  Element at a row and column.
    /// @param  row     Row index 0 to 2.
    /// @param  column  Column index 0 to 2.
    /// @return The element.
    [[nodiscard]] double operator()( int row, int column ) const noexcept
    {
        return m_m( row, column );
    }

    /// @brief  The underlying Eigen matrix.
    /// @return Elements.
    [[nodiscard]] const Eigen::Matrix3d& eigen() const noexcept
    {
        return m_m;
    }

    /// @brief  Transpose.
    /// @return `mᵀ`.
    [[nodiscard]] Mat3 transposed() const noexcept
    {
        return Mat3( Eigen::Matrix3d( m_m.transpose() ) );
    }

    /// @brief  Determinant.
    /// @return `det m`.
    [[nodiscard]] double determinant() const noexcept
    {
        return m_m.determinant();
    }

    /// @brief  Inverse.
    /// @return `m⁻¹`, or `std::nullopt` when the determinant is zero or not finite.
    [[nodiscard]] std::optional<Mat3> inverse() const noexcept
    {
        const double det = determinant();

        if( det == 0.0 || !std::isfinite( det ) )
        {
            return std::nullopt;
        }

        return Mat3( Eigen::Matrix3d( m_m.inverse() ) );
    }

    /// @brief Matrix product.
    [[nodiscard]] friend Mat3 operator*( const Mat3& a, const Mat3& b ) noexcept
    {
        return Mat3( Eigen::Matrix3d( a.m_m * b.m_m ) );
    }

    /// @brief Matrix times column vector.
    [[nodiscard]] friend Vec3 operator*( const Mat3& m, const Vec3& v ) noexcept
    {
        return Vec3( Eigen::Vector3d( m.m_m * v.eigen() ) );
    }

    /// @brief Matrix times scalar.
    [[nodiscard]] friend Mat3 operator*( const Mat3& m, double s ) noexcept
    {
        return Mat3( Eigen::Matrix3d( m.m_m * s ) );
    }

    /// @brief Exact element-wise equality; use a tolerance for computed values.
    [[nodiscard]] friend bool operator==( const Mat3& a, const Mat3& b ) noexcept
    {
        return a.m_m == b.m_m;
    }

private:
    Eigen::Matrix3d m_m = Eigen::Matrix3d::Zero();
};

} // namespace app::core::math
