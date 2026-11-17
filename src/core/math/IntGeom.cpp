// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   core/math/IntGeom.cpp
/// @brief  Overflow-checked integer-nanometre arithmetic.

#include "core/math/IntGeom.h"

#include <limits>

namespace app::core::math
{

namespace
{

constexpr Nm kMax = std::numeric_limits<Nm>::max();
constexpr Nm kMin = std::numeric_limits<Nm>::min();

std::unexpected<Error> overflow( const char* operation )
{
    return fail( ErrorCode::InvalidArgument, "Integer overflow in board coordinates.", operation );
}

} // namespace

Result<Nm> checkedAdd( Nm a, Nm b )
{
    if( ( b > 0 && a > kMax - b ) || ( b < 0 && a < kMin - b ) )
    {
        return overflow( "add" );
    }

    return a + b;
}

Result<Nm> checkedSubtract( Nm a, Nm b )
{
    if( ( b < 0 && a > kMax + b ) || ( b > 0 && a < kMin + b ) )
    {
        return overflow( "subtract" );
    }

    return a - b;
}

Result<Nm> checkedMultiply( Nm a, Nm b )
{
    if( a == 0 || b == 0 )
    {
        return Nm{ 0 };
    }

    // Each test divides the bound by the factor with the right sign, so the check itself cannot overflow.
    const bool overflows = a > 0 ? ( b > 0 ? a > kMax / b : b < kMin / a ) : ( b > 0 ? a < kMin / b : b < kMax / a );

    if( overflows )
    {
        return overflow( "multiply" );
    }

    return a * b;
}

Result<NmPoint> translate( const NmPoint& p, const NmVector& v )
{
    return checkedAdd( p.x, v.x )
        .and_then(
            [&]( Nm x )
            {
                return checkedAdd( p.y, v.y )
                    .transform(
                        [x]( Nm y )
                        {
                            return NmPoint{ x, y };
                        } );
            } );
}

Result<NmVector> difference( const NmPoint& from, const NmPoint& to )
{
    return checkedSubtract( to.x, from.x )
        .and_then(
            [&]( Nm x )
            {
                return checkedSubtract( to.y, from.y )
                    .transform(
                        [x]( Nm y )
                        {
                            return NmVector{ x, y };
                        } );
            } );
}

Result<NmVector> add( const NmVector& a, const NmVector& b )
{
    return checkedAdd( a.x, b.x )
        .and_then(
            [&]( Nm x )
            {
                return checkedAdd( a.y, b.y )
                    .transform(
                        [x]( Nm y )
                        {
                            return NmVector{ x, y };
                        } );
            } );
}

Result<NmVector> scale( const NmVector& v, Nm factor )
{
    return checkedMultiply( v.x, factor )
        .and_then(
            [&]( Nm x )
            {
                return checkedMultiply( v.y, factor )
                    .transform(
                        [x]( Nm y )
                        {
                            return NmVector{ x, y };
                        } );
            } );
}

Result<Nm> dot( const NmVector& a, const NmVector& b )
{
    const Result<Nm> xx = checkedMultiply( a.x, b.x );
    const Result<Nm> yy = checkedMultiply( a.y, b.y );

    if( !xx.has_value() )
    {
        return xx;
    }

    if( !yy.has_value() )
    {
        return yy;
    }

    return checkedAdd( *xx, *yy );
}

Result<Nm> cross( const NmVector& a, const NmVector& b )
{
    const Result<Nm> xy = checkedMultiply( a.x, b.y );
    const Result<Nm> yx = checkedMultiply( a.y, b.x );

    if( !xy.has_value() )
    {
        return xy;
    }

    if( !yx.has_value() )
    {
        return yx;
    }

    return checkedSubtract( *xy, *yx );
}

Result<Nm> lengthSquared( const NmVector& v )
{
    return dot( v, v );
}

Result<NmRect> NmRect::inflated( Nm distance ) const
{
    const Result<Nm> minX = checkedSubtract( m_min.x, distance );
    const Result<Nm> minY = checkedSubtract( m_min.y, distance );
    const Result<Nm> maxX = checkedAdd( m_max.x, distance );
    const Result<Nm> maxY = checkedAdd( m_max.y, distance );

    if( !minX || !minY || !maxX || !maxY )
    {
        return overflow( "inflate" );
    }

    if( *minX > *maxX || *minY > *maxY )
    {
        return fail( ErrorCode::InvalidArgument, "Shrinking the rectangle by this distance would invert it." );
    }

    return fromCorners( { *minX, *minY }, { *maxX, *maxY } );
}

} // namespace app::core::math
