// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   core/Compat.h
/// @brief  Feature-test checks for the C++23/26 features APP relies on, and wrappers with fallbacks for the
///         conditional ones (docs/cpp-features.md).
///
/// Every "Allowed" feature of docs/cpp-features.md is required here: a compiler that lacks one stops with an
/// `#error` naming the feature-test macro. The "Allowed with a feature-test check and fallback" features are used
/// only through the wrappers below; scripts/check-cpp-features.ps1 fails on direct use anywhere else. Each wrapper
/// picks the standard facility when its macro is defined and the fallback in `app::core::compat::fallback`
/// otherwise. The fallbacks are always compiled so that tests can compare them with the standard facility.
///
/// @req    L-03, L-04, L-05

#pragma once

#include <version>

#if !defined( __cpp_concepts ) || __cpp_concepts < 202002L
#    error "APP needs concepts (__cpp_concepts >= 202002L), see docs/cpp-features.md"
#endif

#if !defined( __cpp_lib_ranges ) || __cpp_lib_ranges < 201911L
#    error "APP needs ranges (__cpp_lib_ranges >= 201911L), see docs/cpp-features.md"
#endif

#if !defined( __cpp_lib_format ) || __cpp_lib_format < 201907L
#    error "APP needs std::format (__cpp_lib_format >= 201907L), see docs/cpp-features.md"
#endif

#if !defined( __cpp_lib_expected ) || __cpp_lib_expected < 202202L
#    error "APP needs std::expected (__cpp_lib_expected >= 202202L), see docs/cpp-features.md"
#endif

#if !defined( __cpp_lib_print ) || __cpp_lib_print < 202207L
#    error "APP needs std::print (__cpp_lib_print >= 202207L), see docs/cpp-features.md"
#endif

#if !defined( __cpp_explicit_this_parameter ) || __cpp_explicit_this_parameter < 202110L
#    error "APP needs explicit object parameters (__cpp_explicit_this_parameter >= 202110L), see docs/cpp-features.md"
#endif

#if !defined( __cpp_lib_ranges_to_container ) || __cpp_lib_ranges_to_container < 202202L
#    error "APP needs std::ranges::to (__cpp_lib_ranges_to_container >= 202202L), see docs/cpp-features.md"
#endif

#if !defined( __cpp_lib_to_chars ) || __cpp_lib_to_chars < 201611L
#    error "APP needs floating-point std::to_chars (__cpp_lib_to_chars >= 201611L), see docs/cpp-features.md"
#endif

#if !defined( __cpp_lib_span ) || __cpp_lib_span < 202002L
#    error "APP needs std::span (__cpp_lib_span >= 202002L), see docs/cpp-features.md"
#endif

#if !defined( __cpp_lib_bit_cast ) || __cpp_lib_bit_cast < 201806L
#    error "APP needs std::bit_cast (__cpp_lib_bit_cast >= 201806L), see docs/cpp-features.md"
#endif

#if !defined( __cpp_lib_jthread ) || __cpp_lib_jthread < 201911L
#    error "APP needs std::jthread (__cpp_lib_jthread >= 201911L), see docs/cpp-features.md"
#endif

#if !defined( __cpp_lib_source_location ) || __cpp_lib_source_location < 201907L
#    error "APP needs std::source_location (__cpp_lib_source_location >= 201907L), see docs/cpp-features.md"
#endif

/// @def   APP_HAS_RANGES_ZIP
/// @brief 1 when `std::views::zip` and `std::views::zip_transform` are available (`__cpp_lib_ranges_zip`), else 0.
///
/// @def   APP_HAS_RANGES_ENUMERATE
/// @brief 1 when `std::views::enumerate` is available (`__cpp_lib_ranges_enumerate`), else 0.
///
/// @def   APP_HAS_MDSPAN
/// @brief 1 when `std::mdspan` is available (`__cpp_lib_mdspan`), else 0.
///
/// @def   APP_HAS_FLAT_MAP
/// @brief 1 when `std::flat_map` is available (`__cpp_lib_flat_map`), else 0.
///
/// @def   APP_HAS_FLAT_SET
/// @brief 1 when `std::flat_set` is available (`__cpp_lib_flat_set`), else 0.

#if defined( __cpp_lib_ranges_zip ) && __cpp_lib_ranges_zip >= 202110L
#    define APP_HAS_RANGES_ZIP 1
#else
#    define APP_HAS_RANGES_ZIP 0
#endif

#if defined( __cpp_lib_ranges_enumerate ) && __cpp_lib_ranges_enumerate >= 202302L
#    define APP_HAS_RANGES_ENUMERATE 1
#else
#    define APP_HAS_RANGES_ENUMERATE 0
#endif

#if defined( __cpp_lib_mdspan ) && __cpp_lib_mdspan >= 202207L
#    define APP_HAS_MDSPAN 1
#else
#    define APP_HAS_MDSPAN 0
#endif

#if defined( __cpp_lib_flat_map ) && __cpp_lib_flat_map >= 202207L
#    define APP_HAS_FLAT_MAP 1
#else
#    define APP_HAS_FLAT_MAP 0
#endif

#if defined( __cpp_lib_flat_set ) && __cpp_lib_flat_set >= 202207L
#    define APP_HAS_FLAT_SET 1
#else
#    define APP_HAS_FLAT_SET 0
#endif

#include <algorithm>
#include <functional>
#include <map>
#include <ranges>
#include <set>
#include <tuple>
#include <type_traits>
#include <utility>

#if APP_HAS_FLAT_MAP
#    include <flat_map>
#endif

#if APP_HAS_FLAT_SET
#    include <flat_set>
#endif

/// @brief Portable access to conditional C++23/26 library features (L-04).
namespace app::core::compat
{

/// @brief True when `std::views::zip` and `std::views::zip_transform` are available.
inline constexpr bool kHasRangesZip = APP_HAS_RANGES_ZIP != 0;

/// @brief True when `std::views::enumerate` is available.
inline constexpr bool kHasRangesEnumerate = APP_HAS_RANGES_ENUMERATE != 0;

/// @brief True when `std::mdspan` is available. There is no wrapper yet; the first user adds one with a fallback.
inline constexpr bool kHasMdspan = APP_HAS_MDSPAN != 0;

/// @brief True when `std::flat_map` is available.
inline constexpr bool kHasFlatMap = APP_HAS_FLAT_MAP != 0;

/// @brief True when `std::flat_set` is available.
inline constexpr bool kHasFlatSet = APP_HAS_FLAT_SET != 0;

/// @brief A range the fallbacks can index: random access with a known size.
template <typename R>
concept IndexableRange = std::ranges::random_access_range<R> && std::ranges::sized_range<R>;

/// @brief Fallbacks used when a standard facility is missing. Always compiled, so tests can check them.
namespace fallback
{

/// @brief  Pair each element of @p range with its index, like `std::views::enumerate`.
/// @param  range  Range to view; must outlive the returned view.
/// @return A view of `std::tuple<index, reference>`, the index of type `std::ranges::range_difference_t<R>`.
template <IndexableRange R>
[[nodiscard]] auto enumerate( R& range )
{
    using Index = std::ranges::range_difference_t<R>;
    using Ref   = std::ranges::range_reference_t<R>;

    return std::views::iota( Index{ 0 }, static_cast<Index>( std::ranges::size( range ) ) )
           | std::views::transform(
               [&range]( Index index )
               {
                   return std::tuple<Index, Ref>( index, std::ranges::begin( range )[index] );
               } );
}

/// @brief  View two ranges element by element, like `std::views::zip`; stops at the shorter range.
/// @param  first   First range; must outlive the returned view.
/// @param  second  Second range; must outlive the returned view.
/// @return A view of `std::tuple<reference of first, reference of second>`.
template <IndexableRange A, IndexableRange B>
[[nodiscard]] auto zip( A& first, B& second )
{
    using Index = std::common_type_t<std::ranges::range_difference_t<A>, std::ranges::range_difference_t<B>>;
    using RefA  = std::ranges::range_reference_t<A>;
    using RefB  = std::ranges::range_reference_t<B>;

    const Index count =
        std::min( static_cast<Index>( std::ranges::size( first ) ), static_cast<Index>( std::ranges::size( second ) ) );

    return std::views::iota( Index{ 0 }, count )
           | std::views::transform(
               [&first, &second]( Index index )
               {
                   return std::tuple<RefA, RefB>( std::ranges::begin( first )[index],
                                                  std::ranges::begin( second )[index] );
               } );
}

/// @brief  Apply @p function to the elements of two ranges pairwise, like `std::views::zip_transform`.
/// @param  function  Callable taking one element of each range; copied into the view.
/// @param  first     First range; must outlive the returned view.
/// @param  second    Second range; must outlive the returned view.
/// @return A view of the results; stops at the shorter range.
template <typename F, IndexableRange A, IndexableRange B>
[[nodiscard]] auto zipTransform( F function, A& first, B& second )
{
    using Index = std::common_type_t<std::ranges::range_difference_t<A>, std::ranges::range_difference_t<B>>;

    const Index count =
        std::min( static_cast<Index>( std::ranges::size( first ) ), static_cast<Index>( std::ranges::size( second ) ) );

    return std::views::iota( Index{ 0 }, count )
           | std::views::transform(
               [function = std::move( function ), &first, &second]( Index index )
               {
                   return std::invoke( function,
                                       std::ranges::begin( first )[index],
                                       std::ranges::begin( second )[index] );
               } );
}

/// @brief Sorted associative container standing in for `std::flat_map`.
template <typename Key, typename Value, typename Compare = std::less<Key>>
using FlatMap = std::map<Key, Value, Compare>;

/// @brief Sorted set standing in for `std::flat_set`.
template <typename Key, typename Compare = std::less<Key>>
using FlatSet = std::set<Key, Compare>;

} // namespace fallback

/// @brief  Pair each element of @p range with its index (`std::views::enumerate` or the fallback).
/// @param  range  Lvalue range to view; must outlive the returned view.
/// @return A view of `(index, reference)` tuples usable with structured bindings: `for( auto [i, x] : ... )`.
template <IndexableRange R>
[[nodiscard]] auto enumerate( R& range )
{
#if APP_HAS_RANGES_ENUMERATE
    return std::views::enumerate( range );
#else
    return fallback::enumerate( range );
#endif
}

/// @brief  View two ranges element by element (`std::views::zip` or the fallback); stops at the shorter range.
/// @param  first   First lvalue range; must outlive the returned view.
/// @param  second  Second lvalue range; must outlive the returned view.
/// @return A view of reference tuples usable with structured bindings.
template <IndexableRange A, IndexableRange B>
[[nodiscard]] auto zip( A& first, B& second )
{
#if APP_HAS_RANGES_ZIP
    return std::views::zip( first, second );
#else
    return fallback::zip( first, second );
#endif
}

/// @brief  Apply @p function pairwise to two ranges (`std::views::zip_transform` or the fallback).
/// @param  function  Callable taking one element of each range.
/// @param  first     First lvalue range; must outlive the returned view.
/// @param  second    Second lvalue range; must outlive the returned view.
/// @return A view of the results; stops at the shorter range.
template <typename F, IndexableRange A, IndexableRange B>
[[nodiscard]] auto zipTransform( F function, A& first, B& second )
{
#if APP_HAS_RANGES_ZIP
    return std::views::zip_transform( std::move( function ), first, second );
#else
    return fallback::zipTransform( std::move( function ), first, second );
#endif
}

/// @brief Sorted associative container: `std::flat_map`, or `std::map` as the fallback. Use only the interface the
///        two share (insert, emplace, find, contains, at, operator[], erase, size, ordered iteration) and never keep
///        iterators or references across an insertion or erasure.
#if APP_HAS_FLAT_MAP
template <typename Key, typename Value, typename Compare = std::less<Key>>
using FlatMap = std::flat_map<Key, Value, Compare>;
#else
template <typename Key, typename Value, typename Compare = std::less<Key>>
using FlatMap = fallback::FlatMap<Key, Value, Compare>;
#endif

/// @brief Sorted set: `std::flat_set`, or `std::set` as the fallback. Use only the interface the two share and never
///        keep iterators across an insertion or erasure.
#if APP_HAS_FLAT_SET
template <typename Key, typename Compare = std::less<Key>>
using FlatSet = std::flat_set<Key, Compare>;
#else
template <typename Key, typename Compare = std::less<Key>>
using FlatSet = fallback::FlatSet<Key, Compare>;
#endif

} // namespace app::core::compat
