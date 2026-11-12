// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors
/**
 * @file cpp_feature_probe.cpp
 * @brief Spike SP-1b feature probe: one block per row of docs/cpp-features.md (L-01 ... L-05).
 *
 * Not part of any project. scripts/probe-cpp-features.ps1 compiles and links this file once per feature with
 * -DAPP_PROBE_<NAME> for MSVC (/std:c++latest) and, when available, GCC and Clang (-std=c++2c), and records whether
 * it builds and runs. Blocks for "Not allowed" features are expected to fail on at least one compiler; this file is
 * exempt from scripts/check-cpp-features.ps1 for that reason.
 */

#if defined( APP_PROBE_CONCEPTS )
#    include <concepts>

template <std::integral T>
constexpr T twice( T value )
{
    return value * 2;
}

int main()
{
    return twice( 2 ) == 4 ? 0 : 1;
}

#elif defined( APP_PROBE_RANGES )
#    include <ranges>
#    include <vector>

int main()
{
    std::vector<int> values{ 1, 2, 3, 4 };
    auto             even = values
                            | std::views::filter(
                                []( int v )
                                {
                        return v % 2 == 0;
                                } );
    return std::ranges::distance( even ) == 2 ? 0 : 1;
}

#elif defined( APP_PROBE_FORMAT )
#    include <format>
#    include <string>

int main()
{
    return std::format( "{:.2f}", 1.5 ) == "1.50" ? 0 : 1;
}

#elif defined( APP_PROBE_EXPECTED )
#    include <expected>
#    include <string>

static std::expected<int, std::string> parse( bool ok )
{
    if( ok )
    {
        return 7;
    }

    return std::unexpected( std::string( "bad" ) );
}

int main()
{
    return ( parse( true ).value() == 7 && !parse( false ).has_value() ) ? 0 : 1;
}

#elif defined( APP_PROBE_PRINT )
#    include <print>

int main()
{
    std::println( "{}", 42 );
    return 0;
}

#elif defined( APP_PROBE_DEDUCING_THIS )
struct Counter
{
    int value = 0;

    auto& add( this auto& self, int amount )
    {
        self.value += amount;
        return self;
    }
};

int main()
{
    Counter c;
    c.add( 1 ).add( 2 );
    return c.value == 3 ? 0 : 1;
}

#elif defined( APP_PROBE_RANGES_TO )
#    include <ranges>
#    include <vector>

int main()
{
    auto values = std::views::iota( 0, 4 ) | std::ranges::to<std::vector<int>>();
    return values.size() == 4 ? 0 : 1;
}

#elif defined( APP_PROBE_TO_CHARS_FLOAT )
#    include <charconv>
#    include <string_view>

int main()
{
    char   buffer[64];
    auto   result = std::to_chars( buffer, buffer + sizeof( buffer ), 0.1 );
    double back   = 0.0;
    std::from_chars( buffer, result.ptr, back );
    return ( std::string_view( buffer, result.ptr ) == "0.1" && back == 0.1 ) ? 0 : 1;
}

#elif defined( APP_PROBE_SPAN_BYTE_BITCAST )
#    include <bit>
#    include <cstddef>
#    include <cstdint>
#    include <span>

int main()
{
    std::byte            bytes[4]{};
    std::span<std::byte> view( bytes );
    auto                 bits = std::bit_cast<std::uint64_t>( 1.0 );
    return ( view.size() == 4 && bits == 0x3FF0000000000000ULL ) ? 0 : 1;
}

#elif defined( APP_PROBE_JTHREAD )
#    include <stop_token>
#    include <thread>

int main()
{
    int ran = 0;
    {
        std::jthread worker(
            [&ran]( std::stop_token token )
            {
                ran = 1;

                while( !token.stop_requested() )
                {
                    std::this_thread::yield();
                }
            } );
    }
    return ran == 1 ? 0 : 1;
}

#elif defined( APP_PROBE_SOURCE_LOCATION )
#    include <source_location>

int main()
{
    return std::source_location::current().line() > 0 ? 0 : 1;
}

#elif defined( APP_PROBE_ATTRIBUTES )
[[nodiscard]] static int value( [[maybe_unused]] int unused )
{
    return 1;
}

int main()
{
    if( value( 0 ) == 1 ) [[likely]]
    {
        return 0;
    }

    return 1;
}

#elif defined( APP_PROBE_VIEWS_ZIP_ENUMERATE )
#    include <ranges>
#    include <vector>

int main()
{
    std::vector<int> a{ 1, 2 };
    std::vector<int> b{ 3, 4 };
    int              sum = 0;

    for( auto [x, y] : std::views::zip( a, b ) )
    {
        sum += x * y;
    }

    for( auto [i, x] : std::views::enumerate( a ) )
    {
        sum += static_cast<int>( i ) * x;
    }

    auto products = std::views::zip_transform(
        []( int x, int y )
        {
            return x + y;
        },
        a,
        b );
    return ( sum == 13 && *products.begin() == 4 ) ? 0 : 1;
}

#elif defined( APP_PROBE_MDSPAN )
#    include <mdspan>

int main()
{
    int                                       data[6]{ 0, 1, 2, 3, 4, 5 };
    std::mdspan<int, std::extents<int, 2, 3>> grid( data );
    return grid[1, 2] == 5 ? 0 : 1;
}

#elif defined( APP_PROBE_FLAT_MAP )
#    include <flat_map>
#    include <flat_set>

int main()
{
    std::flat_map<int, int> map;
    map[2] = 3;
    std::flat_set<int> set{ 3, 1, 2 };
    return ( map.at( 2 ) == 3 && *set.begin() == 1 ) ? 0 : 1;
}

#elif defined( APP_PROBE_IMPORT_STD )
import std;

int main()
{
    std::vector<int> values{ 1 };
    return values.size() == 1 ? 0 : 1;
}

#elif defined( APP_PROBE_GENERATOR )
#    include <generator>

static std::generator<int> count( int n )
{
    for( int i = 0; i < n; ++i )
    {
        co_yield i;
    }
}

int main()
{
    int sum = 0;

    for( int v : count( 3 ) )
    {
        sum += v;
    }

    return sum == 3 ? 0 : 1;
}

#elif defined( APP_PROBE_STACKTRACE )
#    include <stacktrace>

int main()
{
    return std::stacktrace::current().size() > 0 ? 0 : 1;
}

#elif defined( APP_PROBE_MOVE_ONLY_FUNCTION )
#    include <functional>
#    include <memory>

int main()
{
    std::move_only_function<int()> f = [p = std::make_unique<int>( 5 )]()
    {
        return *p;
    };
    return f() == 5 ? 0 : 1;
}

#elif defined( APP_PROBE_PACK_INDEXING )
template <typename... Ts>
constexpr auto second( Ts... values )
{
    return values...[1];
}

int main()
{
    return second( 1, 2, 3 ) == 2 ? 0 : 1;
}

#elif defined( APP_PROBE_PLACEHOLDER )
static int pair()
{
    return 1;
}

int main()
{
    auto _ = pair();
    auto _ = pair();
    return 0;
}

#elif defined( APP_PROBE_DELETE_REASON )
void legacy() = delete( "use modern() instead" );

int main()
{
    return 0;
}

#elif defined( APP_PROBE_SATURATION )
#    include <cstdint>
#    include <numeric>

int main()
{
    return std::add_sat<std::uint8_t>( 250, 10 ) == 255 ? 0 : 1;
}

#elif defined( APP_PROBE_INPLACE_VECTOR )
#    include <inplace_vector>

int main()
{
    std::inplace_vector<int, 4> values;
    values.push_back( 1 );
    return values.size() == 1 ? 0 : 1;
}

#elif defined( APP_PROBE_REFLECTION )
#    include <meta>

int main()
{
    constexpr auto info = ^^int;
    return std::meta::is_type( info ) ? 0 : 1;
}

#elif defined( APP_PROBE_CONTRACTS )
static int checked( int x ) pre( x > 0 )
{
    contract_assert( x < 100 );
    return x;
}

int main()
{
    return checked( 1 ) == 1 ? 0 : 1;
}

#else
#    error "Define one APP_PROBE_<NAME> macro; see scripts/probe-cpp-features.ps1"
#endif
