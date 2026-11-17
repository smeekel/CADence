// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   core/Units.h
/// @brief  Display units (mm, cm, m, in, mil, thou, µm, deg, rad), conversion to internal units, and locale-independent
///         formatting and parsing of values with units.
/// @req    P-06

#pragma once

#include "core/Quantity.h"
#include "core/Result.h"

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace app::core
{

/// @brief A unit a user can type or display. Internal units are Unit::Millimetre and Unit::Radian (P-06).
enum class Unit : std::uint8_t
{
    Millimetre, ///< mm, the internal length unit.
    Centimetre, ///< cm = 10 mm.
    Metre,      ///< m = 1000 mm.
    Inch,       ///< in = 25.4 mm exactly.
    Mil,        ///< mil = 0.001 in = 0.0254 mm.
    Thou,       ///< thou: the same length as mil, under its other name.
    Micrometre, ///< µm = 0.001 mm.
    Radian,     ///< rad, the internal angle unit.
    Degree      ///< deg = π/180 rad.
};

/// @brief  Dimension measured by @p unit: kLength or kAngle.
/// @param  unit  The unit.
/// @return Its dimension.
[[nodiscard]] Dimension dimensionOf( Unit unit ) noexcept;

/// @brief  Symbol used for display and in files: "mm", "cm", "m", "in", "mil", "thou", "µm" (U+00B5), "rad", "deg".
/// @param  unit  The unit.
/// @return The symbol, UTF-8.
[[nodiscard]] std::string_view symbol( Unit unit ) noexcept;

/// @brief  Unit for a symbol. Accepts every symbol() result, plus "um" and "μm" (Greek mu, U+03BC) for micrometres.
///         Case-sensitive.
/// @param  text  Symbol without surrounding whitespace.
/// @return The unit, or `std::nullopt` for an unknown symbol.
[[nodiscard]] std::optional<Unit> parseUnit( std::string_view text ) noexcept;

/// @brief  Convert a value in @p unit to internal units (mm or rad). The factor is applied as a ratio of exact
///         integers, so whole numbers of in, mil, thou, cm, m and µm convert to the correctly rounded millimetre value
///         (1 in gives exactly the double nearest 25.4).
/// @param  value  Value in @p unit.
/// @param  unit   Unit of @p value.
/// @return The value in internal units. @units mm or rad
[[nodiscard]] double toInternal( double value, Unit unit ) noexcept;

/// @brief  Convert a value in internal units (mm or rad) to @p unit.
/// @param  internal  Value in internal units. @units mm or rad
/// @param  unit      Target unit; must measure the same dimension as the value.
/// @return The value in @p unit.
[[nodiscard]] double fromInternal( double internal, Unit unit ) noexcept;

/// @brief Display units and precision of a document (the `units` object of a document file), with the defaults used
///        when a document does not say otherwise.
struct DisplayUnits
{
    Unit length         = Unit::Millimetre; ///< Unit for lengths.
    Unit angle          = Unit::Degree;     ///< Unit for angles.
    int  lengthDecimals = 3;                ///< Decimals shown for lengths.
    int  angleDecimals  = 2;                ///< Decimals shown for angles.
};

/// @brief  Locale-independent decimal text of a number. "-0" is written as "0".
/// @param  value     The number; must be finite.
/// @param  decimals  Fixed number of decimals, or `std::nullopt` for the shortest text that parses back to the same
///                   double.
/// @return The text, for example "12.700" or "12.7".
[[nodiscard]] std::string formatNumber( double value, std::optional<int> decimals );

/// @brief  Text of a value in a display unit: "<number> <symbol>", for example "0.500 in".
/// @param  internal  Value in internal units. @units mm or rad
/// @param  unit      Display unit, of the value's dimension.
/// @param  decimals  Fixed decimals, or `std::nullopt` for the shortest round-trip text.
/// @return The text.
[[nodiscard]] std::string formatValue( double internal, Unit unit, std::optional<int> decimals );

/// @brief  Text of a quantity in a document's display units: lengths and angles in DisplayUnits with their decimals,
///         pure numbers without a unit, and other dimensions in internal units with a composed symbol ("mm^2").
/// @param  quantity  Quantity to format.
/// @param  display   Display units and decimals.
/// @return The text.
[[nodiscard]] std::string formatQuantity( const Quantity& quantity, const DisplayUnits& display );

/// @brief  Parse "<number> [<unit>]", for example "12.7 mm", "0.5in", "-3 µm" or "45 deg". A number without a unit
///         takes @p defaultUnit. Numbers are read with `std::from_chars` and do not depend on the locale.
/// @param  text         Text to parse; surrounding whitespace is ignored.
/// @param  defaultUnit  Unit of a bare number.
/// @return The quantity in internal units, or ErrorCode::InvalidArgument naming the problem.
[[nodiscard]] Result<Quantity> parseQuantity( std::string_view text, Unit defaultUnit );

} // namespace app::core
