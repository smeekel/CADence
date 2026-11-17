// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   core/ServiceLocator.h
/// @brief  The service locator: the one sanctioned process-wide object (rule 010), through which code that cannot
///         have a service injected finds the application services that `ui` or `cli` constructed at start-up.
/// @req    A-11

#pragma once

#include "core/Result.h"

#include <cstddef>
#include <functional>
#include <memory>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <utility>

namespace app::core
{

class ServiceLocator;

/// @brief Keeps a service provided; withdraws it when destroyed, so the locator never holds a dangling pointer.
///        Move-only. Must not outlive its locator.
class ServiceRegistration
{
public:
    /// @brief A registration of nothing.
    ServiceRegistration() noexcept = default;

    ServiceRegistration( const ServiceRegistration& )            = delete;
    ServiceRegistration& operator=( const ServiceRegistration& ) = delete;

    /// @brief Take over another registration.
    /// @param other  Registration left empty.
    ServiceRegistration( ServiceRegistration&& other ) noexcept;

    /// @brief  Withdraw the current service and take over another registration.
    /// @param  other  Registration left empty.
    /// @return This registration.
    ServiceRegistration& operator=( ServiceRegistration&& other ) noexcept;

    /// @brief Withdraw the service.
    ~ServiceRegistration();

    /// @brief Withdraw the service now; later calls do nothing.
    void reset() noexcept;

    /// @brief  Whether a service is held.
    /// @return False after reset() or a move.
    [[nodiscard]] bool isActive() const noexcept;

private:
    friend class ServiceLocator;

    ServiceRegistration( ServiceLocator& locator, std::type_index type, const void* service ) noexcept;

    ServiceLocator* m_locator = nullptr;
    std::type_index m_type    = typeid( void );
    const void*     m_service = nullptr;
};

/// @brief Non-owning map from service type to the one object providing it. Thread-safe.
///
/// Prefer passing services explicitly; use the locator only where that is impractical (for example deep inside a
/// factory). The application provides services to global() right after constructing them; tests use their own
/// locator.
class ServiceLocator
{
public:
    /// @brief Empty locator.
    ServiceLocator() = default;

    ServiceLocator( const ServiceLocator& )            = delete;
    ServiceLocator& operator=( const ServiceLocator& ) = delete;
    ServiceLocator( ServiceLocator&& )                 = delete;
    ServiceLocator& operator=( ServiceLocator&& )      = delete;
    ~ServiceLocator()                                  = default;

    /// @brief  The process-wide locator.
    /// @return The locator.
    [[nodiscard]] static ServiceLocator& global();

    /// @brief  Provide the object for service type @p Service until the returned registration is destroyed.
    /// @tparam Service  Service type, usually an interface; not const.
    /// @param  service  The object; must outlive the registration.
    /// @return The registration, or ErrorCode::Conflict if the type is already provided.
    template <typename Service>
    [[nodiscard]] Result<ServiceRegistration> provide( Service& service )
    {
        static_assert( !std::is_const_v<Service>, "Provide a mutable service; callers choose const access." );

        const std::type_index type( typeid( Service ) );

        if( Result<> added = add( type, std::addressof( service ), typeid( Service ).name() ); !added )
        {
            return std::unexpected( std::move( added.error() ) );
        }

        return ServiceRegistration( *this, type, std::addressof( service ) );
    }

    /// @brief  The object providing @p Service.
    /// @tparam Service  Service type.
    /// @return The object, or null if none is provided.
    template <typename Service>
    [[nodiscard]] Service* find() const
    {
        return static_cast<Service*>( lookup( typeid( Service ) ) );
    }

    /// @brief  The object providing @p Service, or an error saying it is missing.
    /// @tparam Service  Service type.
    /// @return The object, or ErrorCode::NotFound naming the type.
    template <typename Service>
    [[nodiscard]] Result<std::reference_wrapper<Service>> get() const
    {
        if( Service* const service = find<Service>(); service != nullptr )
        {
            return std::ref( *service );
        }

        return std::unexpected( notFound( typeid( Service ).name() ) );
    }

    /// @brief  Number of provided services.
    /// @return The count.
    [[nodiscard]] std::size_t size() const;

private:
    friend class ServiceRegistration;

    [[nodiscard]] Result<>     add( std::type_index type, void* service, std::string_view typeName );
    [[nodiscard]] void*        lookup( std::type_index type ) const;
    void                       remove( std::type_index type, const void* service ) noexcept;
    [[nodiscard]] static Error notFound( std::string_view typeName );

    mutable std::shared_mutex                  m_mutex;
    std::unordered_map<std::type_index, void*> m_services;
};

} // namespace app::core
