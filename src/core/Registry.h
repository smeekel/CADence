// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   core/Registry.h
/// @brief  Name-keyed factory registry: the extensibility seam through which modules add feature types, export
///         formats, DRC checks and commands without a central switch statement.
/// @req    A-11

#pragma once

#include "core/Result.h"

#include <algorithm>
#include <cstddef>
#include <format>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace app::core
{

/// @brief  Whether a registry name is valid: 1 to 64 characters from ASCII letters, digits, '.', '_' and '-'.
///         Names are case-sensitive; dots separate namespaces, for example "part.extrude".
/// @param  name  Name to check.
/// @return True when valid.
[[nodiscard]] constexpr bool isValidRegistryName( std::string_view name ) noexcept
{
    constexpr std::size_t kMaxLength = 64;

    return !name.empty() && name.size() <= kMaxLength
           && std::ranges::all_of( name,
                                   []( char c )
                                   {
                                       return ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' )
                                              || ( c >= '0' && c <= '9' ) || c == '.' || c == '_' || c == '-';
                                   } );
}

/// @brief Factories of @p Product keyed by name. Each module registers its factories at start-up (for example in its
///        `registerFeatures( Registry<Feature>& )`), and callers create products by name.
///
/// Registration is not synchronised: register everything before the registry is shared with other threads. After
/// that the const members may be called from any thread.
///
/// @tparam Product  Base type of the products.
/// @tparam Args     Arguments passed to each factory.
template <typename Product, typename... Args>
class Registry
{
public:
    /// @brief A factory: builds a product from the arguments or explains why it cannot.
    using Factory = std::function<Result<std::unique_ptr<Product>>( Args... )>;

    /// @brief Empty registry.
    /// @param kind  What the names denote, in lower case for messages, for example "export format".
    explicit Registry( std::string kind )
        : m_kind( std::move( kind ) )
    {
    }

    /// @brief  Register a factory under a name.
    /// @param  name     Name; see isValidRegistryName().
    /// @param  factory  Factory; must not be empty.
    /// @return Success; ErrorCode::Conflict if the name is already registered; ErrorCode::InvalidArgument for an
    ///         invalid name or an empty factory.
    [[nodiscard]] Result<> add( std::string_view name, Factory factory )
    {
        if( !isValidRegistryName( name ) )
        {
            return fail( ErrorCode::InvalidArgument,
                         std::format( "'{}' is not a valid {} name.", name, m_kind ),
                         "Use 1 to 64 ASCII letters, digits, '.', '_' or '-'." );
        }

        if( !factory )
        {
            return fail( ErrorCode::InvalidArgument, std::format( "The {} '{}' has no factory.", m_kind, name ) );
        }

        if( m_factories.contains( name ) )
        {
            return fail( ErrorCode::Conflict, std::format( "The {} '{}' is already registered.", m_kind, name ) );
        }

        m_factories.emplace( std::string( name ), std::move( factory ) );
        return ok();
    }

    /// @brief  Whether a name is registered.
    /// @param  name  Name.
    /// @return True if registered.
    [[nodiscard]] bool contains( std::string_view name ) const
    {
        return m_factories.contains( name );
    }

    /// @brief  The factory registered under a name.
    /// @param  name  Name.
    /// @return The factory, or null if the name is not registered. Valid until the registry is destroyed.
    [[nodiscard]] const Factory* find( std::string_view name ) const
    {
        const auto found = m_factories.find( name );

        return found == m_factories.end() ? nullptr : &found->second;
    }

    /// @brief  Create a product by name.
    /// @param  name  Name.
    /// @param  args  Arguments for the factory.
    /// @return The product, the factory's error, or ErrorCode::NotFound for an unknown name. A factory that returns
    ///         null is reported as ErrorCode::Internal.
    [[nodiscard]] Result<std::unique_ptr<Product>> create( std::string_view name, Args... args ) const
    {
        const Factory* const factory = find( name );

        if( factory == nullptr )
        {
            return fail( ErrorCode::NotFound, std::format( "Unknown {} '{}'.", m_kind, name ) );
        }

        Result<std::unique_ptr<Product>> product = ( *factory )( std::forward<Args>( args )... );

        if( product && *product == nullptr )
        {
            return fail( ErrorCode::Internal, std::format( "The {} '{}' factory returned nothing.", m_kind, name ) );
        }

        return product;
    }

    /// @brief  The registered names in ascending byte order.
    /// @return Names.
    [[nodiscard]] std::vector<std::string> names() const
    {
        std::vector<std::string> result;
        result.reserve( m_factories.size() );

        for( const auto& entry : m_factories )
        {
            result.push_back( entry.first );
        }

        return result;
    }

    /// @brief  Number of registered names.
    /// @return The count.
    [[nodiscard]] std::size_t size() const noexcept
    {
        return m_factories.size();
    }

    /// @brief  What the names denote, as given to the constructor.
    /// @return The kind.
    [[nodiscard]] const std::string& kind() const noexcept
    {
        return m_kind;
    }

private:
    std::string                                 m_kind;
    std::map<std::string, Factory, std::less<>> m_factories;
};

} // namespace app::core
