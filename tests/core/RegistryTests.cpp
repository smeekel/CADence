// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   core/RegistryTests.cpp
/// @brief  Unit tests for the factory registry and the service locator (T-01, A-11).

#include "core/Registry.h"
#include "core/ServiceLocator.h"

#include <gtest/gtest.h>

#include <atomic>
#include <memory>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

using app::ErrorCode;
using app::fail;
using app::Result;
using app::core::isValidRegistryName;
using app::core::Registry;
using app::core::ServiceLocator;
using app::core::ServiceRegistration;

namespace
{

class Exporter
{
public:
    Exporter()                             = default;
    Exporter( const Exporter& )            = delete;
    Exporter& operator=( const Exporter& ) = delete;
    Exporter( Exporter&& )                 = delete;
    Exporter& operator=( Exporter&& )      = delete;
    virtual ~Exporter()                    = default;

    [[nodiscard]] virtual std::string describe() const = 0;
};

class StlExporter final : public Exporter
{
public:
    explicit StlExporter( bool binary )
        : m_binary( binary )
    {
    }

    [[nodiscard]] std::string describe() const override
    {
        return m_binary ? "stl binary" : "stl ascii";
    }

private:
    bool m_binary;
};

using ExporterRegistry = Registry<Exporter, bool>;

[[nodiscard]] ExporterRegistry::Factory stlFactory()
{
    return []( bool binary ) -> Result<std::unique_ptr<Exporter>>
    {
        return std::make_unique<StlExporter>( binary );
    };
}

class IClock
{
public:
    IClock()                           = default;
    IClock( const IClock& )            = delete;
    IClock& operator=( const IClock& ) = delete;
    IClock( IClock&& )                 = delete;
    IClock& operator=( IClock&& )      = delete;
    virtual ~IClock()                  = default;

    [[nodiscard]] virtual int now() const = 0;
};

class FixedClock final : public IClock
{
public:
    explicit FixedClock( int value )
        : m_value( value )
    {
    }

    [[nodiscard]] int now() const override
    {
        return m_value;
    }

private:
    int m_value;
};

} // namespace

TEST( Registry, CreatesAProduct_WhenLookedUpByName )
{
    ExporterRegistry formats( "export format" );
    ASSERT_TRUE( formats.add( "stl", stlFactory() ).has_value() );

    EXPECT_TRUE( formats.contains( "stl" ) );
    EXPECT_NE( formats.find( "stl" ), nullptr );

    const auto binary = formats.create( "stl", true );
    ASSERT_TRUE( binary.has_value() );
    EXPECT_EQ( ( *binary )->describe(), "stl binary" );

    const auto ascii = formats.create( "stl", false );
    ASSERT_TRUE( ascii.has_value() );
    EXPECT_EQ( ( *ascii )->describe(), "stl ascii" );
}

TEST( Registry, RejectsADuplicateName_AndKeepsTheFirstFactory )
{
    ExporterRegistry formats( "export format" );
    ASSERT_TRUE( formats.add( "stl", stlFactory() ).has_value() );

    const auto duplicate = formats.add( "stl",
                                        []( bool ) -> Result<std::unique_ptr<Exporter>>
                                        {
                                            return fail( ErrorCode::Unsupported, "Replacement." );
                                        } );

    ASSERT_FALSE( duplicate.has_value() );
    EXPECT_EQ( duplicate.error().code, ErrorCode::Conflict );
    EXPECT_EQ( duplicate.error().message, "The export format 'stl' is already registered." );
    EXPECT_EQ( formats.size(), 1U );
    EXPECT_TRUE( formats.create( "stl", true ).has_value() );
}

TEST( Registry, ReportsNotFound_WhenTheNameIsUnknown )
{
    const ExporterRegistry formats( "export format" );

    const auto product = formats.create( "dxf", true );

    ASSERT_FALSE( product.has_value() );
    EXPECT_EQ( product.error().code, ErrorCode::NotFound );
    EXPECT_EQ( product.error().message, "Unknown export format 'dxf'." );
    EXPECT_EQ( formats.find( "dxf" ), nullptr );
}

TEST( Registry, IsCaseSensitive )
{
    ExporterRegistry formats( "export format" );
    ASSERT_TRUE( formats.add( "stl", stlFactory() ).has_value() );

    EXPECT_FALSE( formats.contains( "STL" ) );
    EXPECT_TRUE( formats.add( "STL", stlFactory() ).has_value() );
}

TEST( Registry, RejectsInvalidNamesAndEmptyFactories )
{
    using namespace std::string_view_literals;

    ExporterRegistry formats( "export format" );

    for( const std::string_view name : { ""sv, "has space"sv, "tab\t"sv, "é"sv, "a/b"sv, "x\0y"sv } )
    {
        const auto added = formats.add( name, stlFactory() );
        ASSERT_FALSE( added.has_value() ) << name;
        EXPECT_EQ( added.error().code, ErrorCode::InvalidArgument );
    }

    EXPECT_FALSE( formats.add( std::string( 65, 'a' ), stlFactory() ).has_value() );
    EXPECT_TRUE( formats.add( std::string( 64, 'a' ), stlFactory() ).has_value() );

    const auto empty = formats.add( "step", ExporterRegistry::Factory() );
    ASSERT_FALSE( empty.has_value() );
    EXPECT_EQ( empty.error().code, ErrorCode::InvalidArgument );

    EXPECT_TRUE( isValidRegistryName( "part.extrude_2-b" ) );
}

TEST( Registry, ListsNamesInOrder )
{
    ExporterRegistry formats( "export format" );

    for( const char* name : { "step", "obj", "stl", "3mf" } )
    {
        ASSERT_TRUE( formats.add( name, stlFactory() ).has_value() );
    }

    EXPECT_EQ( formats.names(), ( std::vector<std::string>{ "3mf", "obj", "step", "stl" } ) );
    EXPECT_EQ( formats.kind(), "export format" );
}

TEST( Registry, PassesOnFactoryErrors_AndRejectsNullProducts )
{
    ExporterRegistry formats( "export format" );
    ASSERT_TRUE( formats
                     .add( "step",
                           []( bool ) -> Result<std::unique_ptr<Exporter>>
                           {
                               return fail( ErrorCode::Unsupported, "STEP export arrives in S1." );
                           } )
                     .has_value() );
    ASSERT_TRUE( formats
                     .add( "broken",
                           []( bool ) -> Result<std::unique_ptr<Exporter>>
                           {
                               return std::unique_ptr<Exporter>();
                           } )
                     .has_value() );

    const auto step = formats.create( "step", true );
    ASSERT_FALSE( step.has_value() );
    EXPECT_EQ( step.error().code, ErrorCode::Unsupported );

    const auto broken = formats.create( "broken", true );
    ASSERT_FALSE( broken.has_value() );
    EXPECT_EQ( broken.error().code, ErrorCode::Internal );
}

TEST( ServiceLocator, FindsTheProvidedService_UntilItsRegistrationEnds )
{
    ServiceLocator locator;
    FixedClock     clock( 7 );
    const IClock*  missing = locator.find<IClock>();
    EXPECT_EQ( missing, nullptr );
    {
        auto registration = locator.provide<IClock>( clock );
        ASSERT_TRUE( registration.has_value() );
        EXPECT_TRUE( registration->isActive() );

        ASSERT_NE( locator.find<IClock>(), nullptr );
        EXPECT_EQ( locator.find<IClock>()->now(), 7 );

        const auto found = locator.get<IClock>();
        ASSERT_TRUE( found.has_value() );
        EXPECT_EQ( found->get().now(), 7 );
        EXPECT_EQ( locator.size(), 1U );
    }

    EXPECT_EQ( locator.find<IClock>(), nullptr );
    EXPECT_EQ( locator.size(), 0U );
}

TEST( ServiceLocator, RejectsASecondProvider_OfTheSameType )
{
    ServiceLocator locator;
    FixedClock     first( 1 );
    FixedClock     second( 2 );
    const auto     kept = locator.provide<IClock>( first );
    ASSERT_TRUE( kept.has_value() );

    const auto duplicate = locator.provide<IClock>( second );

    ASSERT_FALSE( duplicate.has_value() );
    EXPECT_EQ( duplicate.error().code, ErrorCode::Conflict );
    EXPECT_EQ( locator.find<IClock>()->now(), 1 );
}

TEST( ServiceLocator, KeysServicesByTheRequestedType )
{
    ServiceLocator locator;
    FixedClock     clock( 3 );
    const auto     registration = locator.provide<FixedClock>( clock );
    ASSERT_TRUE( registration.has_value() );

    EXPECT_EQ( locator.find<IClock>(), nullptr );
    EXPECT_EQ( locator.find<FixedClock>(), &clock );
}

TEST( ServiceLocator, ReportsNotFound_WhenNoServiceIsProvided )
{
    const ServiceLocator locator;

    const auto found = locator.get<IClock>();

    ASSERT_FALSE( found.has_value() );
    EXPECT_EQ( found.error().code, ErrorCode::NotFound );
    EXPECT_NE( found.error().detail.find( "IClock" ), std::string::npos );
}

TEST( ServiceLocator, MovesAndResetsRegistrations )
{
    ServiceLocator locator;
    FixedClock     clock( 5 );

    auto provided = locator.provide<IClock>( clock );
    ASSERT_TRUE( provided.has_value() );
    ServiceRegistration moved = std::move( *provided );
    EXPECT_NE( locator.find<IClock>(), nullptr );

    ServiceRegistration target;
    target = std::move( moved );
    EXPECT_NE( locator.find<IClock>(), nullptr );

    target.reset();
    EXPECT_FALSE( target.isActive() );
    EXPECT_EQ( locator.find<IClock>(), nullptr );

    target.reset();
    EXPECT_TRUE( locator.provide<IClock>( clock ).has_value() );
}

TEST( ServiceLocator, HasOneGlobalInstance )
{
    EXPECT_EQ( &ServiceLocator::global(), &ServiceLocator::global() );

    FixedClock clock( 9 );
    {
        const auto registration = ServiceLocator::global().provide<IClock>( clock );
        ASSERT_TRUE( registration.has_value() );
        EXPECT_EQ( ServiceLocator::global().find<IClock>()->now(), 9 );
    }
    EXPECT_EQ( ServiceLocator::global().find<IClock>(), nullptr );
}

TEST( ServiceLocator, AnswersLookupsFromManyThreads )
{
    ServiceLocator locator;
    FixedClock     clock( 11 );
    const auto     registration = locator.provide<IClock>( clock );
    ASSERT_TRUE( registration.has_value() );

    std::atomic<int> found = 0;
    {
        std::vector<std::jthread> threads;
        threads.reserve( 4 );
        for( int thread = 0; thread < 4; ++thread )
        {
            threads.emplace_back(
                [&locator, &found]
                {
                    for( int index = 0; index < 1'000; ++index )
                    {
                        if( const IClock* service = locator.find<IClock>(); service != nullptr && service->now() == 11 )
                        {
                            ++found;
                        }
                    }
                } );
        }
    }

    EXPECT_EQ( found, 4'000 );
}
