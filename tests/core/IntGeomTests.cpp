// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   core/IntGeomTests.cpp
/// @brief  Unit tests for integer-nanometre geometry (T-01, A-09).

#include "core/math/IntGeom.h"

#include <gtest/gtest.h>

#include <limits>

using app::core::math::Nm;
using app::core::math::NmPoint;
using app::core::math::NmRect;
using app::core::math::NmVector;

namespace
{

constexpr Nm kMax = std::numeric_limits<Nm>::max();
constexpr Nm kMin = std::numeric_limits<Nm>::min();

// 25.4 mm, one inch, in nanometres.
constexpr Nm kInch = 25'400'000;

} // namespace

// Accept: A-09
TEST( IntGeom, ComputesExactly_WhenValuesFitIn64Bits )
{
    EXPECT_EQ( app::core::math::checkedAdd( kInch, kInch ), app::Result<Nm>( 2 * kInch ) );
    EXPECT_EQ( app::core::math::checkedSubtract( 0, kInch ), app::Result<Nm>( -kInch ) );
    EXPECT_EQ( app::core::math::checkedMultiply( kInch, -3 ), app::Result<Nm>( -3 * kInch ) );
    EXPECT_EQ( app::core::math::checkedAdd( kMax - 1, 1 ), app::Result<Nm>( kMax ) );
    EXPECT_EQ( app::core::math::checkedSubtract( kMin + 1, 1 ), app::Result<Nm>( kMin ) );
    EXPECT_EQ( app::core::math::checkedMultiply( kMin / 2, 2 ), app::Result<Nm>( kMin ) );
    EXPECT_EQ( app::core::math::checkedMultiply( 0, kMin ), app::Result<Nm>( 0 ) );

    const NmPoint  a{ 1'000, -2'000 };
    const NmPoint  b{ 4'000, 2'000 };
    const NmVector ab{ 3'000, 4'000 };

    EXPECT_EQ( app::core::math::difference( a, b ), app::Result<NmVector>( ab ) );
    EXPECT_EQ( app::core::math::translate( a, ab ), app::Result<NmPoint>( b ) );
    EXPECT_EQ( app::core::math::add( ab, ab ), app::Result<NmVector>( NmVector{ 6'000, 8'000 } ) );
    EXPECT_EQ( app::core::math::scale( ab, -2 ), app::Result<NmVector>( NmVector{ -6'000, -8'000 } ) );
    EXPECT_EQ( app::core::math::lengthSquared( ab ), app::Result<Nm>( 25'000'000 ) );
    EXPECT_EQ( app::core::math::dot( ab, NmVector{ 4'000, -3'000 } ), app::Result<Nm>( 0 ) );
    EXPECT_EQ( app::core::math::cross( NmVector{ 1, 0 }, NmVector{ 0, 1 } ), app::Result<Nm>( 1 ) );
}

// Accept: A-09
TEST( IntGeom, ReportsOverflow_WhenResultExceeds64Bits )
{
    const auto overflows = []( const app::Result<Nm>& result )
    {
        return !result.has_value() && result.error().code == app::ErrorCode::InvalidArgument;
    };

    EXPECT_TRUE( overflows( app::core::math::checkedAdd( kMax, 1 ) ) );
    EXPECT_TRUE( overflows( app::core::math::checkedAdd( kMin, -1 ) ) );
    EXPECT_TRUE( overflows( app::core::math::checkedSubtract( kMin, 1 ) ) );
    EXPECT_TRUE( overflows( app::core::math::checkedSubtract( 0, kMin ) ) );
    EXPECT_TRUE( overflows( app::core::math::checkedMultiply( kMax / 2 + 1, 2 ) ) );
    EXPECT_TRUE( overflows( app::core::math::checkedMultiply( kMin, -1 ) ) );
    EXPECT_TRUE( overflows( app::core::math::checkedMultiply( -1, kMin ) ) );
    EXPECT_TRUE( overflows( app::core::math::checkedMultiply( kMin / 2 - 1, 2 ) ) );

    // A 4 m vector: its squared length (1.6e19 nm^2) does not fit in int64.
    EXPECT_TRUE( overflows( app::core::math::lengthSquared( NmVector{ 4'000'000'000, 0 } ) ) );
    EXPECT_TRUE( overflows(
        app::core::math::dot( NmVector{ 3'000'000'000, 3'000'000'000 }, NmVector{ 3'000'000'000, 3'000'000'000 } ) ) );
    EXPECT_TRUE( overflows( app::core::math::cross( NmVector{ kMax, 0 }, NmVector{ 0, 2 } ) ) );

    EXPECT_FALSE( app::core::math::translate( NmPoint{ kMax, 0 }, NmVector{ 1, 0 } ).has_value() );
    EXPECT_FALSE( app::core::math::difference( NmPoint{ kMin, 0 }, NmPoint{ kMax, 0 } ).has_value() );
    EXPECT_FALSE( app::core::math::scale( NmVector{ 0, kMax }, 2 ).has_value() );
    EXPECT_FALSE( app::core::math::add( NmVector{ 0, kMin }, NmVector{ 0, -1 } ).has_value() );
}

TEST( IntGeom, TestsRectanglesInclusively_WhenPointsAreOnEdges )
{
    const NmRect board = NmRect::fromCorners( NmPoint{ 100'000'000, 0 }, NmPoint{ 0, 50'000'000 } );

    EXPECT_EQ( board.min(), ( NmPoint{ 0, 0 } ) );
    EXPECT_EQ( board.width(), app::Result<Nm>( 100'000'000 ) );
    EXPECT_EQ( board.height(), app::Result<Nm>( 50'000'000 ) );
    EXPECT_TRUE( board.contains( NmPoint{ 100'000'000, 50'000'000 } ) );
    EXPECT_FALSE( board.contains( NmPoint{ 100'000'001, 0 } ) );

    const NmRect touching = NmRect::fromCorners( NmPoint{ 100'000'000, 0 }, NmPoint{ 200'000'000, 10 } );
    const NmRect apart    = NmRect::fromCorners( NmPoint{ 100'000'001, 0 }, NmPoint{ 200'000'000, 10 } );

    EXPECT_TRUE( board.intersects( touching ) );
    EXPECT_FALSE( board.intersects( apart ) );
    EXPECT_EQ( board.united( apart ).max(), ( NmPoint{ 200'000'000, 50'000'000 } ) );
}

TEST( IntGeom, InflatesOrRejects_WhenGrowingOrShrinkingRectangle )
{
    const NmRect rect = NmRect::fromCorners( NmPoint{ 0, 0 }, NmPoint{ 10, 4 } );

    EXPECT_EQ( rect.inflated( 3 ), app::Result<NmRect>( NmRect::fromCorners( NmPoint{ -3, -3 }, NmPoint{ 13, 7 } ) ) );
    EXPECT_EQ( rect.inflated( -2 ), app::Result<NmRect>( NmRect::fromCorners( NmPoint{ 2, 2 }, NmPoint{ 8, 2 } ) ) );
    EXPECT_FALSE( rect.inflated( -3 ).has_value() );
    EXPECT_FALSE( NmRect::fromCorners( NmPoint{ kMin, 0 }, NmPoint{ kMax, 0 } ).inflated( 1 ).has_value() );
    EXPECT_FALSE( NmRect::fromCorners( NmPoint{ kMin, 0 }, NmPoint{ kMax, 0 } ).width().has_value() );
}
