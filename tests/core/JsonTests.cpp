// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   core/JsonTests.cpp
/// @brief  Unit tests for the core JSON alias (T-01, ADR 0010, ADR 0016).

#include "core/Json.h"

#include <gtest/gtest.h>

#include <string>
#include <vector>

TEST( Json, KeepsInsertionOrder_WhenSerialised )
{
    app::core::Json value;

    value["$magic"]        = "d9a3877a-3f27-4df8-9528-f8dfde59773d";
    value["formatVersion"] = "1.0.0";
    value["type"]          = "project";
    value["a"]             = 1;

    EXPECT_EQ( value.dump(),
               R"({"$magic":"d9a3877a-3f27-4df8-9528-f8dfde59773d","formatVersion":"1.0.0","type":"project","a":1})" );
}

TEST( Json, KeepsKeyOrder_WhenParsed )
{
    const app::core::Json    value = app::core::Json::parse( R"({ "z": 1, "b": 2, "m": 3 })" );
    std::vector<std::string> keys;

    for( const auto& item : value.items() )
    {
        keys.push_back( item.key() );
    }

    EXPECT_EQ( keys, ( std::vector<std::string>{ "z", "b", "m" } ) );
}
