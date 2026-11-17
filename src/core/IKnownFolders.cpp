// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   core/IKnownFolders.cpp
/// @brief  Known-folder names and the fixed-path implementation.

#include "core/IKnownFolders.h"

#include <utility>

namespace app::core
{

std::string_view toString( KnownFolder folder ) noexcept
{
    switch( folder )
    {
        case KnownFolder::GlobalLibraries:
            return "globalLibraries";
        case KnownFolder::UserLibraries:
            return "userLibraries";
        case KnownFolder::UserSettings:
            return "userSettings";
        case KnownFolder::Journal:
            return "journal";
        case KnownFolder::Recovery:
            return "recovery";
        case KnownFolder::Logs:
            return "logs";
        case KnownFolder::Temporary:
            return "temporary";
    }

    return "unknown";
}

FixedKnownFolders::FixedKnownFolders( const std::filesystem::path& root )
{
    for( const KnownFolder folder : kAllKnownFolders )
    {
        m_paths[static_cast<std::size_t>( folder )] = root / toString( folder );
    }
}

void FixedKnownFolders::set( KnownFolder folder, std::filesystem::path path )
{
    if( const auto index = static_cast<std::size_t>( folder ); index < m_paths.size() )
    {
        m_paths[index] = std::move( path );
    }
}

Result<std::filesystem::path> FixedKnownFolders::path( KnownFolder folder ) const
{
    const auto index = static_cast<std::size_t>( folder );

    if( index >= m_paths.size() )
    {
        return fail( ErrorCode::InvalidArgument, "Unknown known folder." );
    }

    return m_paths[index];
}

} // namespace app::core
