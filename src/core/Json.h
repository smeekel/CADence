// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   core/Json.h
/// @brief  The JSON value type used for command parameters, results, journals and documents.
///
/// `core` and `doc` are the only modules that include `<nlohmann/json.hpp>` (ADR 0016, rule 030); every other
/// module builds JSON values through this alias.
///
/// @req    A-03, F-02

#pragma once

#include <nlohmann/json.hpp>

namespace app::core
{

/// @brief Insertion-ordered JSON value (`nlohmann::ordered_json`, ADR 0010). Key order is kept as written, which
///        deterministic output (F-03) and the leading `$magic` key (F-09) rely on.
using Json = nlohmann::ordered_json;

} // namespace app::core
