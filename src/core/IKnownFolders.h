// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 The APP Authors

/// @file   core/IKnownFolders.h
/// @brief  The OS folders APP uses, behind an interface that `platform` implements with Win32 known-folder APIs and
///         that tests implement with temporary folders (ADR 0024).
/// @req    F-10, P-04, A-03

#pragma once

#include "core/Result.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string_view>

namespace app::core
{

/// @brief A folder APP reads or writes outside projects.
enum class KnownFolder : std::uint8_t
{
    GlobalLibraries, ///< Global library scope, shared by all users (F-10); default `%ProgramData%\APP\libraries`.
    UserLibraries,   ///< User library scope (F-10); default `%APPDATA%\APP\libraries`.
    UserSettings,    ///< User settings and shortcut files (F-10, U-10); default `%APPDATA%\APP`.
    Journal,         ///< Session journals (A-03); default `%LOCALAPPDATA%\APP\journal`.
    Recovery,        ///< Autosave and crash recovery; default `%LOCALAPPDATA%\APP\recovery`.
    Logs,            ///< Log files (P-23, ADR 0031); default `%LOCALAPPDATA%\APP\logs`.
    Temporary,       ///< Scratch files deleted after use; default `%TEMP%\APP`.
};

/// @brief Every KnownFolder value, in declaration order.
inline constexpr std::array kAllKnownFolders{ KnownFolder::GlobalLibraries,
                                              KnownFolder::UserLibraries,
                                              KnownFolder::UserSettings,
                                              KnownFolder::Journal,
                                              KnownFolder::Recovery,
                                              KnownFolder::Logs,
                                              KnownFolder::Temporary };

/// @brief  Stable camelCase name of a folder, for example "userLibraries"; used in settings keys and messages.
/// @param  folder  Folder.
/// @return The name.
[[nodiscard]] std::string_view toString( KnownFolder folder ) noexcept;

/// @brief Where the known folders are. Implementations do not create the folders.
class IKnownFolders
{
public:
    IKnownFolders()                                  = default;
    IKnownFolders( const IKnownFolders& )            = delete;
    IKnownFolders& operator=( const IKnownFolders& ) = delete;
    IKnownFolders( IKnownFolders&& )                 = delete;
    IKnownFolders& operator=( IKnownFolders&& )      = delete;
    virtual ~IKnownFolders()                         = default;

    /// @brief  The absolute path of a folder.
    /// @param  folder  Folder.
    /// @return The path, or ErrorCode::IoError if the OS cannot tell.
    [[nodiscard]] virtual Result<std::filesystem::path> path( KnownFolder folder ) const = 0;
};

/// @brief Known folders as fixed paths, each a sub-folder of one root; for tests, portable installs and command-line
///        overrides. No OS calls.
class FixedKnownFolders final : public IKnownFolders
{
public:
    /// @brief Each folder is `root / toString( folder )`, for example `root/userLibraries`.
    /// @param root  Root folder.
    explicit FixedKnownFolders( const std::filesystem::path& root );

    /// @brief Point one folder somewhere else.
    /// @param folder  Folder.
    /// @param path    Its new path.
    void set( KnownFolder folder, std::filesystem::path path );

    [[nodiscard]] Result<std::filesystem::path> path( KnownFolder folder ) const override;

private:
    std::array<std::filesystem::path, kAllKnownFolders.size()> m_paths;
};

} // namespace app::core
