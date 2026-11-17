// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   core/ServiceLocator.cpp
/// @brief  Service map and registrations.

#include "core/ServiceLocator.h"

#include <mutex>
#include <utility>

namespace app::core
{

ServiceRegistration::ServiceRegistration( ServiceLocator& locator, std::type_index type, const void* service ) noexcept
    : m_locator( &locator )
    , m_type( type )
    , m_service( service )
{
}

ServiceRegistration::ServiceRegistration( ServiceRegistration&& other ) noexcept
    : m_locator( std::exchange( other.m_locator, nullptr ) )
    , m_type( other.m_type )
    , m_service( std::exchange( other.m_service, nullptr ) )
{
}

ServiceRegistration& ServiceRegistration::operator=( ServiceRegistration&& other ) noexcept
{
    if( this != &other )
    {
        reset();
        m_locator = std::exchange( other.m_locator, nullptr );
        m_type    = other.m_type;
        m_service = std::exchange( other.m_service, nullptr );
    }

    return *this;
}

ServiceRegistration::~ServiceRegistration()
{
    reset();
}

void ServiceRegistration::reset() noexcept
{
    if( m_locator != nullptr )
    {
        m_locator->remove( m_type, m_service );
        m_locator = nullptr;
        m_service = nullptr;
    }
}

bool ServiceRegistration::isActive() const noexcept
{
    return m_locator != nullptr;
}

ServiceLocator& ServiceLocator::global()
{
    static ServiceLocator instance;

    return instance;
}

std::size_t ServiceLocator::size() const
{
    const std::shared_lock lock( m_mutex );

    return m_services.size();
}

Result<> ServiceLocator::add( std::type_index type, void* service, std::string_view typeName )
{
    const std::unique_lock lock( m_mutex );

    if( m_services.contains( type ) )
    {
        return fail( ErrorCode::Conflict, "A service of this type is already provided.", std::string( typeName ) );
    }

    m_services.emplace( type, service );
    return ok();
}

void* ServiceLocator::lookup( std::type_index type ) const
{
    const std::shared_lock lock( m_mutex );
    const auto             found = m_services.find( type );

    return found == m_services.end() ? nullptr : found->second;
}

void ServiceLocator::remove( std::type_index type, const void* service ) noexcept
{
    const std::unique_lock lock( m_mutex );
    const auto             found = m_services.find( type );

    if( found != m_services.end() && found->second == service )
    {
        m_services.erase( found );
    }
}

Error ServiceLocator::notFound( std::string_view typeName )
{
    return fail( ErrorCode::NotFound, "A required service is not provided.", std::string( typeName ) ).error();
}

} // namespace app::core
