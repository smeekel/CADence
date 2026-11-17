// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   core/MathTests.cpp
/// @brief  Unit tests for the Eigen-backed math primitives: Vec, Mat3, Transform and Box3 (T-01, P-06).

#include "core/math/Box.h"
#include "core/math/Mat.h"
#include "core/math/Transform.h"
#include "core/math/Vec.h"
#include "core/Tolerance.h"

#include <gtest/gtest.h>

#include <numbers>
#include <optional>

using app::core::math::Box3;
using app::core::math::Mat3;
using app::core::math::Transform;
using app::core::math::Vec2;
using app::core::math::Vec3;

namespace
{

constexpr double kTolerance = 1e-12;

void expectNear( const Vec3& actual, const Vec3& expected, double tolerance = kTolerance )
{
    EXPECT_NEAR( actual.x(), expected.x(), tolerance );
    EXPECT_NEAR( actual.y(), expected.y(), tolerance );
    EXPECT_NEAR( actual.z(), expected.z(), tolerance );
}

} // namespace

TEST( Vec, ComputesDotCrossAndLength_WhenGivenComponents )
{
    const Vec3 a( 1.0, 2.0, 3.0 );
    const Vec3 b( 4.0, -5.0, 6.0 );

    EXPECT_EQ( a + b, Vec3( 5.0, -3.0, 9.0 ) );
    EXPECT_EQ( a - b, Vec3( -3.0, 7.0, -3.0 ) );
    EXPECT_EQ( -a, Vec3( -1.0, -2.0, -3.0 ) );
    EXPECT_EQ( 2.0 * a, Vec3( 2.0, 4.0, 6.0 ) );
    EXPECT_EQ( a / 2.0, Vec3( 0.5, 1.0, 1.5 ) );
    EXPECT_EQ( app::core::math::dot( a, b ), 12.0 );
    EXPECT_EQ( app::core::math::cross( Vec3( 1, 0, 0 ), Vec3( 0, 1, 0 ) ), Vec3( 0, 0, 1 ) );
    EXPECT_EQ( app::core::math::length( Vec3( 2.0, 3.0, 6.0 ) ), 7.0 );
    EXPECT_EQ( app::core::math::length( Vec2( 3.0, 4.0 ) ), 5.0 );
    EXPECT_EQ( app::core::math::cross( Vec2( 1, 0 ), Vec2( 0, 1 ) ), 1.0 );
    EXPECT_EQ( Vec3(), Vec3( 0.0, 0.0, 0.0 ) );
}

TEST( Vec, ReturnsNothing_WhenNormalisingZeroVector )
{
    EXPECT_FALSE( app::core::math::normalized( Vec3() ).has_value() );
    EXPECT_FALSE( app::core::math::normalized( Vec2() ).has_value() );
    EXPECT_EQ( app::core::math::normalized( Vec3( 0.0, 0.0, 5.0 ) ), std::optional<Vec3>( Vec3( 0.0, 0.0, 1.0 ) ) );
}

TEST( Mat3, InvertsAndMultiplies_WhenNonSingular )
{
    const Mat3 m( Vec3( 2, 0, 1 ), Vec3( 1, 3, 0 ), Vec3( 0, 1, 4 ) );

    EXPECT_NEAR( m.determinant(), 25.0, kTolerance );

    const std::optional<Mat3> inverse = m.inverse();

    ASSERT_TRUE( inverse.has_value() );

    const Mat3 product = m * inverse.value_or( Mat3() );

    for( int r = 0; r < 3; ++r )
    {
        for( int c = 0; c < 3; ++c )
        {
            EXPECT_NEAR( product( r, c ), r == c ? 1.0 : 0.0, kTolerance );
        }
    }

    EXPECT_EQ( m.transposed()( 0, 1 ), 1.0 );
    EXPECT_EQ( Mat3::identity() * Vec3( 1, 2, 3 ), Vec3( 1, 2, 3 ) );
    EXPECT_FALSE( Mat3( Vec3( 1, 2, 3 ), Vec3( 2, 4, 6 ), Vec3( 0, 0, 1 ) ).inverse().has_value() );
}

TEST( Transform, RotatesAboutAxisThroughPoint_WhenGivenOriginAndAngle )
{
    const std::optional<Transform> quarterTurn =
        Transform::rotation( Vec3( 1.0, 0.0, 0.0 ), Vec3( 0.0, 0.0, 2.0 ), std::numbers::pi / 2.0 );

    ASSERT_TRUE( quarterTurn.has_value() );

    const Transform t = quarterTurn.value_or( Transform() );

    expectNear( t.applyToPoint( Vec3( 2.0, 0.0, 5.0 ) ), Vec3( 1.0, 1.0, 5.0 ) );
    expectNear( t.applyToPoint( Vec3( 1.0, 0.0, 0.0 ) ), Vec3( 1.0, 0.0, 0.0 ) );
    expectNear( t.applyToVector( Vec3( 1.0, 0.0, 0.0 ) ), Vec3( 0.0, 1.0, 0.0 ) );
    EXPECT_TRUE( t.isRigid() );
}

TEST( Transform, AppliesRightOperandFirst_WhenComposed )
{
    const Transform move = Transform::translation( Vec3( 10.0, 0.0, 0.0 ) );
    const Transform turn =
        Transform::rotation( Vec3(), Vec3( 0, 0, 1 ), std::numbers::pi / 2.0 ).value_or( Transform() );

    expectNear( ( move * turn ).applyToPoint( Vec3( 1, 0, 0 ) ), Vec3( 10, 1, 0 ) );
    expectNear( ( turn * move ).applyToPoint( Vec3( 1, 0, 0 ) ), Vec3( 0, 11, 0 ) );
    expectNear( ( move * turn ).applyToPoint( Vec3( 1, 0, 0 ) ),
                move.applyToPoint( turn.applyToPoint( Vec3( 1, 0, 0 ) ) ) );
}

TEST( Transform, UndoesItself_WhenInverted )
{
    const Transform t = Transform::translation( Vec3( 3, -2, 7 ) )
                        * Transform::rotation( Vec3( 1, 1, 1 ), Vec3( 1, 2, 3 ), 0.7 ).value_or( Transform() )
                        * Transform::scaling( 2.5 );
    const std::optional<Transform> inverse = t.inverse();

    ASSERT_TRUE( inverse.has_value() );

    const Vec3 p( 4.0, -1.5, 9.25 );

    expectNear( inverse.value_or( Transform() ).applyToPoint( t.applyToPoint( p ) ), p, 1e-11 );
    EXPECT_FALSE( t.isRigid() );
    EXPECT_FALSE( Transform::scaling( 0.0 ).inverse().has_value() );
}

TEST( Transform, RejectsRotation_WhenAxisIsZero )
{
    EXPECT_FALSE( Transform::rotation( Vec3(), Vec3(), 1.0 ).has_value() );
}

TEST( Transform, SeparatesLinearPartAndTranslation_WhenBuiltFromBoth )
{
    const Transform t( Mat3::identity() * 2.0, Vec3( 1, 2, 3 ) );

    EXPECT_EQ( t.translationPart(), Vec3( 1, 2, 3 ) );
    EXPECT_EQ( t.linear()( 1, 1 ), 2.0 );
    EXPECT_EQ( t.applyToPoint( Vec3( 1, 1, 1 ) ), Vec3( 3, 4, 5 ) );
    EXPECT_EQ( t.applyToVector( Vec3( 1, 1, 1 ) ), Vec3( 2, 2, 2 ) );
    EXPECT_TRUE( Transform::identity().isRigid() );
}

TEST( Box3, IsEmpty_UntilAPointIsAdded )
{
    Box3 box;

    EXPECT_TRUE( box.isEmpty() );
    EXPECT_FALSE( box.contains( Vec3() ) );

    box.extend( Vec3( 1, 2, 3 ) );

    EXPECT_FALSE( box.isEmpty() );
    EXPECT_EQ( box.min(), Vec3( 1, 2, 3 ) );
    EXPECT_EQ( box.max(), Vec3( 1, 2, 3 ) );
}

TEST( Box3, ContainsAndIntersectsWithinTolerance_WhenNearTheFaces )
{
    const Box3 box = Box3::fromCorners( Vec3( 10, 10, 10 ), Vec3( 0, 0, 0 ) );

    EXPECT_EQ( box.min(), Vec3( 0, 0, 0 ) );
    EXPECT_EQ( box.size(), Vec3( 10, 10, 10 ) );
    EXPECT_EQ( box.center(), Vec3( 5, 5, 5 ) );
    EXPECT_TRUE( box.contains( Vec3( 10.0 + 0.5 * app::core::kConfusionTolerance, 5, 5 ) ) );
    EXPECT_FALSE( box.contains( Vec3( 10.0 + 2.0 * app::core::kConfusionTolerance, 5, 5 ) ) );

    const Box3 touching = Box3::fromCorners( Vec3( 10, 0, 0 ), Vec3( 20, 10, 10 ) );
    const Box3 apart    = Box3::fromCorners( Vec3( 10.001, 0, 0 ), Vec3( 20, 10, 10 ) );

    EXPECT_TRUE( box.intersects( touching ) );
    EXPECT_FALSE( box.intersects( apart ) );
    EXPECT_FALSE( box.intersects( Box3() ) );
}

TEST( Box3, BoundsTransformedCorners_WhenRotated )
{
    const Box3      box = Box3::fromCorners( Vec3( 0, 0, 0 ), Vec3( 2, 1, 1 ) );
    const Transform turn =
        Transform::rotation( Vec3(), Vec3( 0, 0, 1 ), std::numbers::pi / 2.0 ).value_or( Transform() );
    const Box3 out = box.transformed( turn );

    expectNear( out.min(), Vec3( -1, 0, 0 ) );
    expectNear( out.max(), Vec3( 0, 2, 1 ) );
    EXPECT_TRUE( Box3().transformed( turn ).isEmpty() );

    Box3 merged = box;

    merged.extend( Box3::fromCorners( Vec3( -1, -1, -1 ), Vec3( 0, 0, 0 ) ) );
    merged.extend( Box3() );

    EXPECT_EQ( merged.min(), Vec3( -1, -1, -1 ) );
    EXPECT_EQ( merged.max(), Vec3( 2, 1, 1 ) );
}
