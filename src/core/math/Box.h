// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   core/math/Box.h
/// @brief  Axis-aligned 3D bounding box, stored as an Eigen aligned box (ADR 0022).
/// @req    P-06, P-07

#pragma once

#include "core/math/Transform.h"
#include "core/math/Vec.h"
#include "core/Tolerance.h"

#include <Eigen/Geometry>

namespace app::core::math
{

/// @brief Axis-aligned box in the mechanical frame; empty until a point is added. @units mm
///
/// Callers use this API; eigen() is for the modules rule 030 allows to use Eigen directly.
class Box3
{
public:
    /// @brief An empty box.
    Box3() noexcept = default;

    /// @brief  Smallest box containing two corner points, in any order.
    /// @param  a  First corner. @units mm
    /// @param  b  Second corner. @units mm
    /// @return The box.
    [[nodiscard]] static Box3 fromCorners( const Vec3& a, const Vec3& b ) noexcept
    {
        Box3 box;

        box.extend( a );
        box.extend( b );
        return box;
    }

    /// @brief  Whether the box contains no point.
    /// @return True for a default-constructed box.
    [[nodiscard]] bool isEmpty() const noexcept
    {
        return m_box.isEmpty();
    }

    /// @brief  Minimum corner. @pre `!isEmpty()`.
    /// @return The corner. @units mm
    [[nodiscard]] Vec3 min() const noexcept
    {
        return Vec3( m_box.min() );
    }

    /// @brief  Maximum corner. @pre `!isEmpty()`.
    /// @return The corner. @units mm
    [[nodiscard]] Vec3 max() const noexcept
    {
        return Vec3( m_box.max() );
    }

    /// @brief  Centre. @pre `!isEmpty()`.
    /// @return The centre point. @units mm
    [[nodiscard]] Vec3 center() const noexcept
    {
        return Vec3( Eigen::Vector3d( m_box.center() ) );
    }

    /// @brief  Edge lengths along x, y and z. @pre `!isEmpty()`.
    /// @return The size. @units mm
    [[nodiscard]] Vec3 size() const noexcept
    {
        return Vec3( Eigen::Vector3d( m_box.sizes() ) );
    }

    /// @brief  The underlying Eigen box.
    /// @return The box.
    [[nodiscard]] const Eigen::AlignedBox3d& eigen() const noexcept
    {
        return m_box;
    }

    /// @brief Grow the box to contain a point.
    /// @param p  Point. @units mm
    void extend( const Vec3& p ) noexcept
    {
        m_box.extend( p.eigen() );
    }

    /// @brief Grow the box to contain another box; an empty @p other changes nothing.
    /// @param other  Box to include.
    void extend( const Box3& other ) noexcept
    {
        if( !other.isEmpty() )
        {
            m_box.extend( other.m_box );
        }
    }

    /// @brief  Whether a point lies inside or within @p tolerance of the box.
    /// @param  p          Point. @units mm
    /// @param  tolerance  Distance outside the faces still counted as inside. @units mm
    /// @return False for an empty box.
    [[nodiscard]] bool contains( const Vec3& p, double tolerance = kConfusionTolerance ) const noexcept
    {
        if( isEmpty() )
        {
            return false;
        }

        const Eigen::Vector3d margin = Eigen::Vector3d::Constant( tolerance );

        return ( p.eigen().array() >= ( m_box.min() - margin ).array() ).all()
               && ( p.eigen().array() <= ( m_box.max() + margin ).array() ).all();
    }

    /// @brief  Whether two boxes overlap or are within @p tolerance of touching.
    /// @param  other      Other box.
    /// @param  tolerance  Gap still counted as overlapping. @units mm
    /// @return False if either box is empty.
    [[nodiscard]] bool intersects( const Box3& other, double tolerance = kConfusionTolerance ) const noexcept
    {
        if( isEmpty() || other.isEmpty() )
        {
            return false;
        }

        const Eigen::Vector3d margin = Eigen::Vector3d::Constant( tolerance );

        return ( m_box.min().array() <= ( other.m_box.max() + margin ).array() ).all()
               && ( other.m_box.min().array() <= ( m_box.max() + margin ).array() ).all();
    }

    /// @brief  Box containing the image of this box under a transform (its eight transformed corners).
    /// @param  transform  Transform to apply.
    /// @return The transformed box; empty if this box is empty.
    [[nodiscard]] Box3 transformed( const Transform& transform ) const noexcept
    {
        Box3 result;

        if( isEmpty() )
        {
            return result;
        }

        for( int i = 0; i < 8; ++i )
        {
            const Eigen::Vector3d corner = m_box.corner( static_cast<Eigen::AlignedBox3d::CornerType>( i ) );

            result.extend( transform.applyToPoint( Vec3( corner ) ) );
        }

        return result;
    }

private:
    Eigen::AlignedBox3d m_box;
};

} // namespace app::core::math
