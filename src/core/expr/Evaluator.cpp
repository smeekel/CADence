// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   core/expr/Evaluator.cpp
/// @brief  Evaluation of expression trees with unit checking.

#include "core/expr/Evaluator.h"

#include "core/expr/Functions.h"
#include "core/expr/Lexer.h"
#include "core/expr/Parser.h"
#include "core/Tolerance.h"
#include "core/Units.h"

#include <cmath>
#include <format>
#include <type_traits>
#include <utility>
#include <vector>

namespace app::core::expr
{

namespace
{

// Largest distance of a scaled dimension exponent from a whole number that still counts as whole, so that
// `(x ^ (1 / 3)) ^ 3` keeps its dimension despite rounding of 1/3.
constexpr double kExponentTolerance = 1e-9;

[[nodiscard]] std::string_view withoutFinalPeriod( std::string_view message ) noexcept
{
    if( message.ends_with( '.' ) )
    {
        message.remove_suffix( 1 );
    }

    return message;
}

[[nodiscard]] bool isComparison( BinaryOp op ) noexcept
{
    return op == BinaryOp::Less || op == BinaryOp::LessEqual || op == BinaryOp::Greater || op == BinaryOp::GreaterEqual
           || op == BinaryOp::Equal || op == BinaryOp::NotEqual;
}

[[nodiscard]] bool compare( BinaryOp op, double a, double b ) noexcept
{
    switch( op )
    {
        case BinaryOp::Less:
            return a < b - kConfusionTolerance;
        case BinaryOp::LessEqual:
            return a <= b + kConfusionTolerance;
        case BinaryOp::Greater:
            return a > b + kConfusionTolerance;
        case BinaryOp::GreaterEqual:
            return a >= b - kConfusionTolerance;
        case BinaryOp::Equal:
            return isEqualWithin( a, b );
        case BinaryOp::NotEqual:
            return !isEqualWithin( a, b );
        default:
            return false;
    }
}

[[nodiscard]] bool isWhole( double value ) noexcept
{
    return std::abs( value - std::round( value ) ) <= kExponentTolerance;
}

[[nodiscard]] Evaluated power( const Quantity& base, const Quantity& exponent )
{
    if( !exponent.dimension().isDimensionless() )
    {
        return std::unexpected(
            std::format( "The exponent must be a number, not {}", describe( exponent.dimension() ) ) );
    }

    const double n = exponent.value();

    if( base.value() < 0.0 && !isWhole( n ) )
    {
        return std::unexpected( std::string( "A negative number cannot be raised to a fractional power" ) );
    }

    const double length = base.dimension().length * n;
    const double angle  = base.dimension().angle * n;

    if( !isWhole( length ) || !isWhole( angle ) )
    {
        return std::unexpected( std::format( "Cannot raise {} to the power {}",
                                             describe( base.dimension() ),
                                             formatNumber( n, std::nullopt ) ) );
    }

    const Quantity result( std::pow( base.value(), n ),
                           { static_cast<int>( std::round( length ) ), static_cast<int>( std::round( angle ) ) } );

    if( !std::isfinite( result.value() ) )
    {
        return std::unexpected( std::string( "The power is not a finite number" ) );
    }

    return result;
}

class Evaluator
{
public:
    Evaluator( std::string_view source, const ReferenceResolver& resolve )
        : m_source( source )
        , m_resolve( resolve )
    {
    }

    [[nodiscard]] Result<Quantity> evaluate( const Node& node ) const
    {
        return std::visit(
            [&]( const auto& value ) -> Result<Quantity>
            {
                using T = std::decay_t<decltype( value )>;

                if constexpr( std::is_same_v<T, NumberLiteral> )
                {
                    return number( value );
                }
                else if constexpr( std::is_same_v<T, Reference> )
                {
                    return reference( value, node.range.offset );
                }
                else if constexpr( std::is_same_v<T, Unary> )
                {
                    Result<Quantity> operand = evaluate( *value.operand );

                    if( operand.has_value() && value.op == UnaryOp::Minus )
                    {
                        return -*operand;
                    }

                    return operand;
                }
                else if constexpr( std::is_same_v<T, Binary> )
                {
                    return binary( value );
                }
                else
                {
                    return call( value, node.range.offset );
                }
            },
            node.value );
    }

private:
    [[nodiscard]] std::unexpected<Error> errorAt( std::size_t offset, std::string_view message ) const
    {
        return syntaxError( m_source, offset, withoutFinalPeriod( message ) );
    }

    [[nodiscard]] static Result<Quantity> number( const NumberLiteral& literal )
    {
        if( !literal.unit.has_value() )
        {
            return Quantity::number( literal.value );
        }

        return Quantity( toInternal( literal.value, *literal.unit ), dimensionOf( *literal.unit ) );
    }

    [[nodiscard]] Result<Quantity> reference( const Reference& reference, std::size_t offset ) const
    {
        const std::string name = reference.dottedName();

        if( !m_resolve )
        {
            return errorAt( offset, std::format( "Unknown parameter '{}'", name ) );
        }

        Result<Quantity> value = m_resolve( name );

        // An unknown name is an error at this reference. Any other error comes from inside the referenced parameter
        // and already says where it is, so it passes through unchanged.
        if( !value.has_value() && value.error().code == ErrorCode::NotFound )
        {
            std::unexpected<Error> located = errorAt( offset, value.error().message );

            located.error().code    = ErrorCode::NotFound;
            located.error().objects = std::move( value.error().objects );
            return located;
        }

        return value;
    }

    [[nodiscard]] Result<Quantity> binary( const Binary& node ) const
    {
        const Result<Quantity> left = evaluate( *node.left );

        if( !left.has_value() )
        {
            return left;
        }

        const Result<Quantity> right = evaluate( *node.right );

        if( !right.has_value() )
        {
            return right;
        }

        const Quantity& a = *left;
        const Quantity& b = *right;

        if( isComparison( node.op ) )
        {
            if( a.dimension() != b.dimension() )
            {
                return errorAt(
                    node.operatorOffset,
                    std::format( "Cannot compare {} and {}", describe( a.dimension() ), describe( b.dimension() ) ) );
            }

            return Quantity::number( compare( node.op, a.value(), b.value() ) ? 1.0 : 0.0 );
        }

        Evaluated result;

        switch( node.op )
        {
            case BinaryOp::Add:
            case BinaryOp::Subtract:
            {
                const Result<Quantity> sum = node.op == BinaryOp::Add ? add( a, b ) : subtract( a, b );

                if( !sum.has_value() )
                {
                    return errorAt( node.operatorOffset, sum.error().message );
                }

                result = *sum;
                break;
            }
            case BinaryOp::Multiply:
                result = a * b;
                break;
            case BinaryOp::Divide:
                if( b.value() == 0.0 )
                {
                    return errorAt( node.operatorOffset, "Division by zero" );
                }

                result = a / b;
                break;
            default:
                result = power( a, b );
                break;
        }

        if( !result.has_value() )
        {
            return errorAt( node.operatorOffset, result.error() );
        }

        if( !std::isfinite( result->value() ) )
        {
            return errorAt( node.operatorOffset, "The result is not a finite number" );
        }

        return *result;
    }

    [[nodiscard]] Result<Quantity> call( const Call& node, std::size_t offset ) const
    {
        if( !isFunction( node.name ) )
        {
            return errorAt( offset, std::format( "Unknown function '{}'", node.name ) );
        }

        if( auto wrongCount = checkArgumentCount( node.name, node.arguments.size() ) )
        {
            return errorAt( offset, *wrongCount );
        }

        if( node.name == "if" )
        {
            const Result<Quantity> condition = evaluate( *node.arguments[0] );

            if( !condition.has_value() )
            {
                return condition;
            }

            if( !condition->dimension().isDimensionless() )
            {
                return errorAt(
                    node.arguments[0]->range.offset,
                    std::format( "if needs a number as its condition, not {}", describe( condition->dimension() ) ) );
            }

            return evaluate( *node.arguments[condition->value() != 0.0 ? 1 : 2] );
        }

        std::vector<Quantity> arguments;

        arguments.reserve( node.arguments.size() );

        for( const NodePtr& argument : node.arguments )
        {
            Result<Quantity> value = evaluate( *argument );

            if( !value.has_value() )
            {
                return value;
            }

            arguments.push_back( *value );
        }

        const Evaluated result = applyFunction( node.name, arguments );

        if( !result.has_value() )
        {
            return errorAt( offset, result.error() );
        }

        return *result;
    }

    std::string_view         m_source;
    const ReferenceResolver& m_resolve;
};

} // namespace

Result<Quantity> evaluate( const Node& root, std::string_view source, const ReferenceResolver& resolve )
{
    return Evaluator( source, resolve ).evaluate( root );
}

Result<Quantity> evaluateExpression( std::string_view source, const ReferenceResolver& resolve )
{
    const Result<NodePtr> root = parseExpression( source );

    if( !root.has_value() )
    {
        return std::unexpected( root.error() );
    }

    return evaluate( **root, source, resolve );
}

} // namespace app::core::expr
