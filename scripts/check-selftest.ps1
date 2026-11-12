# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 The APP Authors
<#
.SYNOPSIS
    Prove that the static checks detect what they are meant to detect (T-11).

.DESCRIPTION
    Copies the checked parts of the repository (src, tests, build, scripts, docs, root files; no build output) into
    a temporary folder, verifies that every check named by a case below passes on the clean copy, then applies one
    deliberate violation per case to a fresh copy and verifies that the matching check fails.
    A case with an expected message must also report it. Allowed cases (a third-party DLL that imports ws2_32.dll,
    ADR 0026) must pass and appear in the check's summary. Finally THIRD_PARTY_NOTICES.md is regenerated on a copy
    from the installed vcpkg trees (vcpkg_installed\, created by scripts\build.ps1): it must already be up to date,
    a new docs/provenance.md row must make check-licences fail until it is regenerated, and a second
    update-notices run must change nothing. The repository itself is never modified. Exits non-zero if any
    expectation is not met.

.PARAMETER KeepFixtures
    Keep the temporary copies for inspection.

.EXAMPLE
    powershell -ExecutionPolicy Bypass -File scripts\check-selftest.ps1
#>
[CmdletBinding()]
param(
    [switch]$KeepFixtures
)

. "$PSScriptRoot\common.ps1"

$root     = Get-AppRepoRoot
$tempBase = Join-Path ( [IO.Path]::GetTempPath() ) ( 'app-check-selftest-' + [Guid]::NewGuid().ToString( 'N' ).Substring( 0, 8 ) )
$copyList = @( 'src', 'tests', 'build', 'scripts', 'docs', 'libraries', 'Directory.Build.props', 'Directory.Build.targets',
               'vcpkg.json', 'vcpkg-configuration.json', 'LICENSE', 'THIRD_PARTY_NOTICES.md', 'APP.slnx', '.clang-format',
               '.clang-tidy' )
$utf8     = New-Object System.Text.UTF8Encoding( $false )
$failures = New-Object System.Collections.Generic.List[string]

function New-Fixture( [string]$Name )
{
    $dir = Join-Path $tempBase $Name
    New-Item -ItemType Directory -Force $dir | Out-Null

    foreach( $item in $copyList )
    {
        $source = Join-Path $root $item

        if( Test-Path $source )
        {
            Copy-Item -Recurse -Force $source ( Join-Path $dir $item )
        }
    }

    return $dir
}

function Add-Text( [string]$Path, [string]$Text )
{
    $existing = ''

    if( Test-Path $Path )
    {
        $existing = [IO.File]::ReadAllText( $Path )
    }

    [IO.File]::WriteAllText( $Path, $existing + $Text, $utf8 )
}

function Set-Text( [string]$Path, [string]$Text )
{
    [IO.File]::WriteAllText( $Path, $Text, $utf8 )
}

function Invoke-Script( [string]$Name, [string[]]$Arguments )
{
    $output = & powershell.exe -NoProfile -ExecutionPolicy Bypass -File ( Join-Path $PSScriptRoot "$Name.ps1" ) @Arguments 2>&1
    $code   = $LASTEXITCODE

    return [pscustomobject]@{ Code = $code; Output = @( $output | ForEach-Object { "$_" } ) }
}

function Invoke-Check( [string]$Check, [string]$FixtureRoot, [string]$BinaryDir )
{
    $arguments = @( '-Root', $FixtureRoot )

    if( $BinaryDir )
    {
        $arguments += @( '-BinaryDir', ( Join-Path $FixtureRoot $BinaryDir ) )
    }

    return Invoke-Script $Check $arguments
}

# A Windows DLL that imports WINHTTP.dll directly, to stand in for an APP-built DLL using WinHTTP.
function Get-WinHttpFixture
{
    $dumpbin = Get-AppDumpbinPath

    foreach( $name in @( 'bitsigd.dll', 'AxInstSv.dll', 'AuthBroker.dll', 'cloudidsvc.dll' ) )
    {
        $path = Join-Path $env:SystemRoot "System32\$name"

        if( ( Test-Path $path ) -and ( @( & $dumpbin /nologo /dependents $path ) -match '^\s*WINHTTP\.dll\s*$' ) )
        {
            return $path
        }
    }

    throw 'no Windows DLL importing WINHTTP.dll found for the network-own-dll-winhttp case; add one to Get-WinHttpFixture'
}

# A vcpkg dependency pinned with an override, optionally reviewed in docs/licence-manifest.json and listed in
# THIRD_PARTY_NOTICES.md.
function Add-FixturePort( [string]$FixtureRoot, [string]$Port, [string]$Licence, [bool]$Reviewed, [bool]$Noticed )
{
    $p = Join-Path $FixtureRoot 'vcpkg.json'
    $x = [IO.File]::ReadAllText( $p )
    $x = ( [regex]'"dependencies": \[' ).Replace( $x, "`"dependencies`": [ `"$Port`",", 1 )
    $x = $x.Replace( '"overrides": [', "`"overrides`": [ { `"name`": `"$Port`", `"version`": `"1.0.0`" }," )
    Set-Text $p $x

    if( $Reviewed )
    {
        $p = Join-Path $FixtureRoot 'docs\licence-manifest.json'
        $x = [IO.File]::ReadAllText( $p )
        $x = $x.Replace( '"ports": [', "`"ports`": [ { `"port`": `"$Port`", `"library`": `"Fixture`", `"version`": `"1.0.0`", `"licence`": `"$Licence`", `"role`": `"linked`", `"usedBy`": `"core`", `"shipped`": `"Yes (DLL)`", `"verified`": `"S0.02.2`" }," )
        Set-Text $p $x
    }

    if( $Noticed )
    {
        Add-Text ( Join-Path $FixtureRoot 'THIRD_PARTY_NOTICES.md' ) "`n| Fixture | ``$Port`` 1.0.0 | $Licence | linked | core | Yes (DLL) |`n"
    }
}

$header = "// SPDX-License-Identifier: GPL-3.0-or-later`n// SPDX-FileCopyrightText: 2026 The APP Authors`n"
$adapted = "// SPDX-License-Identifier: MIT`n// SPDX-FileCopyrightText: 2020 Example Project`n// Adapted from example https://example.org/example @ 0123abc`n"
$provenanceRow = "| 2026-11-12 | Example | https://example.org/example | 0123abc | src/example.cpp | MIT | ``src/core/Adapted.cpp`` | none | S0.02.1 |`n"

$cases = @(
    @{ Name = 'layering-include'; Check = 'check-layering'
       What = 'core includes a ui header'
       Apply = { param( $r ) Add-Text ( Join-Path $r 'src\core\Module.cpp' ) "`n#include `"ui/MainWindow.h`"`n" } },
    @{ Name = 'layering-project-reference'; Check = 'check-layering'
       What = 'geom references the ui project'
       Apply = { param( $r )
                 $p = Join-Path $r 'src\geom\Geom.vcxproj'
                 $x = [IO.File]::ReadAllText( $p )
                 $x = $x.Replace( '<ProjectReference Include="..\core\Core.vcxproj">', '<ProjectReference Include="..\ui\Ui.vcxproj"><Project>{FD1FCC4C-7A67-43E1-9A7C-221342B59D06}</Project></ProjectReference><ProjectReference Include="..\core\Core.vcxproj">' )
                 Set-Text $p $x } },
    @{ Name = 'layering-third-party'; Check = 'check-layering'
       What = 'part includes an OCCT header and doc includes a Qt Widgets header'
       Apply = { param( $r )
                 Add-Text ( Join-Path $r 'src\part\Module.cpp' ) "`n#include <gp_Pnt.hxx>`n"
                 Add-Text ( Join-Path $r 'src\doc\Module.cpp' ) "`n#include <QWidget>`n" } },
    # Accept: CR-07a
    @{ Name = 'spdx-missing-header'; Check = 'check-spdx'
       What = 'a new C++ file without an SPDX header'
       Apply = { param( $r ) Set-Text ( Join-Path $r 'src\core\NoHeader.cpp' ) "int answer() { return 42; }`n" } },
    # Accept: CR-07b
    @{ Name = 'spdx-third-party-without-provenance'; Check = 'check-spdx'
       What = 'an MIT-licensed adapted file with no docs/provenance.md entry'
       Apply = { param( $r ) Set-Text ( Join-Path $r 'src\core\Adapted.cpp' ) $adapted } },
    # Accept: CR-07b
    @{ Name = 'spdx-third-party-prose-only'; Check = 'check-spdx'
       What = 'an adapted file named only in the prose of docs/provenance.md'
       Apply = { param( $r )
                 Set-Text ( Join-Path $r 'src\core\Adapted.cpp' ) $adapted
                 Add-Text ( Join-Path $r 'docs\provenance.md' ) "`nNote: src/core/Adapted.cpp was adapted from the example project.`n" } },
    # Accept: CR-07b
    @{ Name = 'spdx-third-party-incomplete-row'; Check = 'check-spdx'
       What = 'an adapted file whose provenance row has no URL'
       Apply = { param( $r )
                 Set-Text ( Join-Path $r 'src\core\Adapted.cpp' ) $adapted
                 Add-Text ( Join-Path $r 'docs\provenance.md' ) ( $provenanceRow.Replace( '| https://example.org/example |', '| |' ) ) } },
    # Accept: P-03b
    @{ Name = 'network-source'; Check = 'check-network-apis'
       What = 'io includes winsock2.h and calls WSAStartup'
       Expect = "src/io/Net\.cpp:4: network API call 'WSAStartup'"
       Apply = { param( $r )
                 Set-Text ( Join-Path $r 'src\io\Net.cpp' ) ( $header + "#include <winsock2.h>`nint start() { WSADATA d; return WSAStartup( 0x0202, &d ); }`n" ) } },
    @{ Name = 'network-link-input'; Check = 'check-network-apis'
       What = 'Cli.vcxproj links ws2_32.lib'
       Expect = "src/cli/Cli\.vcxproj:\d+: link input 'ws2_32\.lib' is a network library"
       Apply = { param( $r )
                 $p = Join-Path $r 'src\cli\Cli.vcxproj'
                 $x = [IO.File]::ReadAllText( $p )
                 $x = $x.Replace( '<SubSystem>Console</SubSystem>', '<SubSystem>Console</SubSystem><AdditionalDependencies>ws2_32.lib;%(AdditionalDependencies)</AdditionalDependencies>' )
                 Set-Text $p $x } },
    @{ Name = 'network-vcpkg-port'; Check = 'check-network-apis'
       What = 'vcpkg.json adds curl'
       Expect = "vcpkg\.json: dependency 'curl' is a networking library"
       Apply = { param( $r )
                 $p = Join-Path $r 'vcpkg.json'
                 Set-Text $p ( [IO.File]::ReadAllText( $p ).Replace( '"name": "gtest",', '"name": "curl" }, { "name": "gtest",' ) ) } },
    @{ Name = 'network-vcpkg-qt-feature'; Check = 'check-network-apis'
       What = 'vcpkg.json adds the qtbase network feature'
       Expect = "vcpkg\.json: qtbase feature 'network' enables networking"
       Apply = { param( $r )
                 $p = Join-Path $r 'vcpkg.json'
                 Set-Text $p ( [IO.File]::ReadAllText( $p ).Replace( '"widgets"', '"widgets", "network"' ) ) } },
    # wsock32.dll ships with Windows and imports ws2_32.dll, so it stands in for a binary with a socket import.
    @{ Name = 'network-own-binary'; Check = 'check-network-apis'; BinaryDir = 'bin'
       What = 'an APP-built executable (Cli.exe) imports ws2_32.dll'
       Expect = "bin/Cli/Cli\.exe: APP-built executable imports network DLL 'WS2_32\.dll'"
       Apply = { param( $r )
                 New-Item -ItemType Directory -Force ( Join-Path $r 'bin\Cli' ) | Out-Null
                 Copy-Item ( Join-Path $env:SystemRoot 'System32\wsock32.dll' ) ( Join-Path $r 'bin\Cli\Cli.exe' ) } },
    @{ Name = 'network-own-dll-winhttp'; Check = 'check-network-apis'; BinaryDir = 'bin'
       What = 'an APP-built DLL (Core.dll) imports WINHTTP.dll'
       Expect = "bin/Core/Core\.dll: APP-built DLL imports network DLL 'WINHTTP\.dll'"
       Apply = { param( $r )
                 New-Item -ItemType Directory -Force ( Join-Path $r 'bin\Core' ) | Out-Null
                 Copy-Item ( Get-WinHttpFixture ) ( Join-Path $r 'bin\Core\Core.dll' ) } },
    # Accept: P-04
    @{ Name = 'headless-links-qt-widgets'; Check = 'check-headless'
       What = 'Cli.vcxproj (built in Headless) links Qt6Widgets.lib'
       Apply = { param( $r )
                 $p = Join-Path $r 'src\cli\Cli.vcxproj'
                 $x = [IO.File]::ReadAllText( $p )
                 $x = $x.Replace( '<SubSystem>Console</SubSystem>', '<SubSystem>Console</SubSystem><AdditionalDependencies>Qt6Widgets.lib;%(AdditionalDependencies)</AdditionalDependencies>' )
                 Set-Text $p $x } },
    @{ Name = 'headless-references-ui'; Check = 'check-headless'
       What = 'Cli.vcxproj (built in Headless) references the Ui project'
       Apply = { param( $r )
                 $p = Join-Path $r 'src\cli\Cli.vcxproj'
                 $x = [IO.File]::ReadAllText( $p )
                 $x = $x.Replace( '<ProjectReference Include="..\core\Core.vcxproj">', '<ProjectReference Include="..\ui\Ui.vcxproj"><Project>{FD1FCC4C-7A67-43E1-9A7C-221342B59D06}</Project></ProjectReference><ProjectReference Include="..\core\Core.vcxproj">' )
                 Set-Text $p $x } },
    @{ Name = 'headless-windows-ui'; Check = 'check-headless'
       What = 'doc includes windows.h and calls MessageBoxW outside src/platform/'
       Apply = { param( $r )
                 Add-Text ( Join-Path $r 'src\doc\Module.cpp' ) "`n#include <windows.h>`nstatic int warn() { return MessageBoxW( nullptr, L`"x`", L`"x`", 0 ); }`n" } },
    @{ Name = 'hygiene-pragma-lib'; Check = 'check-build-hygiene'
       What = 'a source links a library with #pragma comment(lib'
       Apply = { param( $r ) Add-Text ( Join-Path $r 'src\core\Module.cpp' ) "`n#pragma comment(lib, `"shlwapi.lib`")`n" } },
    @{ Name = 'hygiene-project-warning-level'; Check = 'check-build-hygiene'
       What = 'Core.vcxproj sets its own warning level and language standard'
       Apply = { param( $r )
                 $p = Join-Path $r 'src\core\Core.vcxproj'
                 $x = [IO.File]::ReadAllText( $p )
                 $x = $x.Replace( '<ConfigurationType>', '<WarningLevel>Level3</WarningLevel><LanguageStandard>stdcpp20</LanguageStandard><ConfigurationType>' )
                 Set-Text $p $x } },
    @{ Name = 'hygiene-needs-pch'; Check = 'check-build-hygiene'
       What = 'a source includes pch.h and so needs the precompiled header'
       Apply = { param( $r ) Set-Text ( Join-Path $r 'src\core\NeedsPch.cpp' ) ( $header + "#include `"pch.h`"`n" ) } },
    @{ Name = 'hygiene-cmake'; Check = 'check-build-hygiene'
       What = 'a CMakeLists.txt is added'
       Apply = { param( $r ) Set-Text ( Join-Path $r 'src\core\CMakeLists.txt' ) "add_library(core Module.cpp)`n" } },
    @{ Name = 'hygiene-two-projects'; Check = 'check-build-hygiene'
       What = 'a module folder holds two projects'
       Apply = { param( $r ) Copy-Item ( Join-Path $r 'src\core\Core.vcxproj' ) ( Join-Path $r 'src\core\CoreExtra.vcxproj' ) } },
    @{ Name = 'format-misformatted'; Check = 'check-format'
       What = 'a test source with cramped braces and double spaces'
       Expect = 'tests/core/Misformatted\.cpp:\d+: code should be clang-formatted'
       Apply = { param( $r ) Set-Text ( Join-Path $r 'tests\core\Misformatted.cpp' ) ( $header + "int  misformatted(int a){return a;}`n" ) } },
    @{ Name = 'cleanroom-trademark'; Check = 'check-clean-room'
       What = 'a UI string names a vendor feature (SmartMate)'
       Expect = "src/ui/Labels\.cpp:3: vendor trademark term in a UI string \(CR-02\) 'SmartMate'"
       Apply = { param( $r ) Set-Text ( Join-Path $r 'src\ui\Labels.cpp' ) ( $header + "const char* autoMateLabel = `"SmartMate...`";`n" ) } },
    @{ Name = 'cleanroom-translation'; Check = 'check-clean-room'
       What = 'a Qt translation names a vendor product'
       Expect = "src/ui/translations/app_de\.ts:3: vendor trademark term in a UI string \(CR-02\) 'SOLIDWORKS'"
       Apply = { param( $r )
                 New-Item -ItemType Directory -Force ( Join-Path $r 'src\ui\translations' ) | Out-Null
                 Set-Text ( Join-Path $r 'src\ui\translations\app_de.ts' ) "<TS version=`"2.1`" language=`"de`">`n<message><source>Import</source>`n<translation>Aus SolidWorks importieren</translation></message>`n</TS>`n" } },
    @{ Name = 'cleanroom-native-extension'; Check = 'check-clean-room'
       What = 'io registers a native CAD file extension'
       Expect = "src/io/Formats\.cpp:3: native CAD file extension of another product \(CR-03\) '\.sldprt'"
       Apply = { param( $r ) Set-Text ( Join-Path $r 'src\io\Formats.cpp' ) ( $header + "const char* partFilter = `"Parts (*.SLDPRT)`";`n" ) } },
    @{ Name = 'cleanroom-telemetry'; Check = 'check-clean-room'
       What = 'core adds an update check'
       Expect = "src/core/Updates\.cpp:3: telemetry or update-check keyword \(CR-06\) 'check for updates'"
       Apply = { param( $r ) Set-Text ( Join-Path $r 'src\core\Updates.cpp' ) ( $header + "void checkForUpdates() {}`n" ) } },
    @{ Name = 'cleanroom-undocumented-data'; Check = 'check-clean-room'
       What = 'a library data file without a docs/data-sources.md row'
       Expect = "libraries/materials/steel\.json: data file has no complete row in docs/data-sources\.md"
       Apply = { param( $r ) Set-Text ( Join-Path $r 'libraries\materials\steel.json' ) "{ `"name`": `"Steel`" }`n" } },
    # Accept: P-30
    @{ Name = 'licences-gpl2-dependency'; Check = 'check-licences'
       What = 'vcpkg.json adds a reviewed and noticed GPL-2.0-only dependency'
       Expect = "'fixture-gpl' \(GPL-2\.0-only\): GPL-2\.0-only is not allowed"
       Apply = { param( $r ) Add-FixturePort $r 'fixture-gpl' 'GPL-2.0-only' $true $true } },
    @{ Name = 'licences-unreviewed-dependency'; Check = 'check-licences'
       What = 'vcpkg.json adds a dependency without a docs/licence-manifest.json entry'
       Expect = "dependency 'fixture-new' has no entry in docs/licence-manifest\.json"
       Apply = { param( $r ) Add-FixturePort $r 'fixture-new' 'MIT' $false $false } },
    @{ Name = 'licences-missing-notice'; Check = 'check-licences'
       What = 'a reviewed MIT dependency missing from THIRD_PARTY_NOTICES.md'
       Expect = "THIRD_PARTY_NOTICES\.md: no row for port 'fixture-mit'"
       Apply = { param( $r ) Add-FixturePort $r 'fixture-mit' 'MIT' $true $false } },
    @{ Name = 'licences-version-not-reverified'; Check = 'check-licences'
       What = 'vcpkg.json pins gtest to a version whose licence was not verified'
       Expect = "'gtest' is pinned to 1\.17\.0 but its licence was verified for 1\.18\.0"
       Apply = { param( $r )
                 $p = Join-Path $r 'vcpkg.json'
                 Set-Text $p ( [IO.File]::ReadAllText( $p ).Replace( '"version": "1.18.0"', '"version": "1.17.0"' ) ) } },
    # Accept: P-30
    @{ Name = 'licences-provenance-gpl2-only'; Check = 'check-licences'
       What = 'docs/provenance.md records code reused from a GPL-2.0-only project'
       Expect = "docs/provenance\.md: 'Example' \(GPL-2\.0-only\): GPL-2\.0-only is not allowed"
       Apply = { param( $r ) Add-Text ( Join-Path $r 'docs\provenance.md' ) ( $provenanceRow.Replace( '| MIT |', '| GPL-2.0-only |' ) ) } }
)

# Cases that must pass, with output that must match.
$allowedCases = @(
    @{ Name = 'spdx-third-party-complete-row'; Check = 'check-spdx'
       What = 'an adapted file listed in a complete docs/provenance.md row passes'
       Expect = 'check-spdx: OK'
       Apply = { param( $r )
                 Set-Text ( Join-Path $r 'src\core\Adapted.cpp' ) $adapted
                 Add-Text ( Join-Path $r 'docs\provenance.md' ) $provenanceRow } },
    @{ Name = 'network-third-party-import'; Check = 'check-network-apis'; BinaryDir = 'bin'
       What = 'a third-party DLL imports ws2_32.dll: allowed and listed in the summary'
       Expect = 'allowed: vendornet\.dll -> WS2_32\.dll'
       Apply = { param( $r )
                 New-Item -ItemType Directory -Force ( Join-Path $r 'bin\Vendor' ) | Out-Null
                 Copy-Item ( Join-Path $env:SystemRoot 'System32\wsock32.dll' ) ( Join-Path $r 'bin\Vendor\vendornet.dll' ) } },
    @{ Name = 'cleanroom-allowed'; Check = 'check-clean-room'
       What = 'a comment about telemetry, a local crash-dump string, a term the owner ruled generic and a documented data file pass'
       Expect = 'check-clean-room: OK .* 1 library data files'
       Apply = { param( $r )
                 Set-Text ( Join-Path $r 'src\core\Notes.cpp' ) ( $header + "// No telemetry and no update check (CR-06).`nconst char* dumpFolder = `"crash dumps stay local`";`nconst char* holeLabel  = `"Hole Wizard`";`n" )
                 Set-Text ( Join-Path $r 'libraries\materials\steel.json' ) "{ `"name`": `"Steel`" }`n"
                 Add-Text ( Join-Path $r 'docs\data-sources.md' ) "| material | ``libraries/materials/steel.json`` | Example handbook, Example Press, 2020 | public factual data | 2026-11-12 | original values |`n" } },
    # Accept: P-03b
    @{ Name = 'network-qt6core-import'; Check = 'check-network-apis'; BinaryDir = 'bin'
       What = 'the installed Qt6Core.dll imports ws2_32.dll: listed in the summary, not a failure'
       Expect = 'allowed: Qt6Core\.dll -> WS2_32\.dll: .*WSAStartup'
       Apply = { param( $r )
                 $qt = Join-Path $root 'vcpkg_installed\full\x64-windows\bin\Qt6Core.dll'

                 if( -not ( Test-Path $qt ) )
                 {
                     throw "'$qt' is missing; run scripts\build.ps1 -Configuration Debug first"
                 }

                 New-Item -ItemType Directory -Force ( Join-Path $r 'bin\Ui' ) | Out-Null
                 Copy-Item $qt ( Join-Path $r 'bin\Ui\Qt6Core.dll' ) } }
)

$checkNames = @( @( $cases ) + @( $allowedCases ) | ForEach-Object { $_.Check } | Select-Object -Unique )

try
{
    $clean = New-Fixture 'clean'

    foreach( $check in $checkNames )
    {
        $result = Invoke-Check $check $clean
        Write-Host "[clean] $check -> exit $( $result.Code )"
        $result.Output | ForEach-Object { Write-Host "    $_" }

        if( $result.Code -ne 0 )
        {
            $failures.Add( "$check failed on the clean copy" )
        }
    }

    foreach( $case in $cases )
    {
        $fixture = New-Fixture $case.Name
        & $case.Apply $fixture

        $result = Invoke-Check $case.Check $fixture $case['BinaryDir']
        Write-Host "[$( $case.Name )] $( $case.What ): $( $case.Check ) -> exit $( $result.Code )"
        $result.Output | ForEach-Object { Write-Host "    $_" }

        if( $result.Code -eq 0 )
        {
            $failures.Add( "$( $case.Check ) did not detect: $( $case.What )" )
        }
        elseif( $case['Expect'] -and -not ( $result.Output | Where-Object { $_ -match $case.Expect } ) )
        {
            $failures.Add( "$( $case.Check ) failed without reporting '$( $case.Expect )': $( $case.What )" )
        }
    }

    foreach( $case in $allowedCases )
    {
        $fixture = New-Fixture $case.Name
        & $case.Apply $fixture

        $result = Invoke-Check $case.Check $fixture $case['BinaryDir']
        Write-Host "[$( $case.Name )] $( $case.What ): $( $case.Check ) -> exit $( $result.Code )"
        $result.Output | ForEach-Object { Write-Host "    $_" }

        if( $result.Code -ne 0 )
        {
            $failures.Add( "$( $case.Check ) failed on an allowed case: $( $case.What )" )
        }
        elseif( -not ( $result.Output | Where-Object { $_ -match $case.Expect } ) )
        {
            $failures.Add( "$( $case.Check ) did not report '$( $case.Expect )': $( $case.What )" )
        }
    }

    # Notices regeneration against the real installed vcpkg trees: a new provenance row makes the notices stale,
    # update-notices fixes them, and a second run changes nothing.
    $installedRoot = Join-Path $root 'vcpkg_installed'

    if( -not ( Test-Path $installedRoot ) )
    {
        $failures.Add( "notices regeneration needs the installed vcpkg trees in '$installedRoot'; run scripts\build.ps1 first" )
    }
    else
    {
        $fixture  = New-Fixture 'notices-regeneration'
        $notices  = Join-Path $fixture 'THIRD_PARTY_NOTICES.md'
        $withTree = @( '-Root', $fixture, '-InstalledRoot', $installedRoot )
        $steps    = New-Object System.Collections.Generic.List[string]

        $unchanged = Invoke-Script 'update-notices' ( $withTree + '-Check' )
        $steps.Add( "update-notices -Check on the copy -> exit $( $unchanged.Code )" )

        if( $unchanged.Code -ne 0 )
        {
            $failures.Add( 'THIRD_PARTY_NOTICES.md is not what update-notices.ps1 generates; run scripts\update-notices.ps1' )
        }

        Set-Text ( Join-Path $fixture 'src\core\Adapted.cpp' ) $adapted
        Add-Text ( Join-Path $fixture 'docs\provenance.md' ) $provenanceRow

        $stale = Invoke-Script 'check-licences' $withTree
        $steps.Add( "check-licences after adding a provenance row -> exit $( $stale.Code )" )

        if( $stale.Code -eq 0 -or -not ( $stale.Output | Where-Object { $_ -match "no row for reused project 'Example'" } ) )
        {
            $failures.Add( 'check-licences did not report stale notices after a new docs/provenance.md row' )
        }

        $first     = Invoke-Script 'update-notices' $withTree
        $firstText = [IO.File]::ReadAllText( $notices )
        $second    = Invoke-Script 'update-notices' $withTree
        $passed    = Invoke-Script 'check-licences' $withTree
        $steps.Add( "update-notices -> exit $( $first.Code ), again -> exit $( $second.Code ) ($( $second.Output -join ' ' )), check-licences -> exit $( $passed.Code )" )

        if( $first.Code -ne 0 -or $second.Code -ne 0 -or ( [IO.File]::ReadAllText( $notices ) -cne $firstText ) -or
            -not ( $second.Output | Where-Object { $_ -match 'is up to date' } ) )
        {
            $failures.Add( 'update-notices.ps1 is not idempotent: the second run changed THIRD_PARTY_NOTICES.md or failed' )
        }

        if( $passed.Code -ne 0 -or -not ( Select-String -Path $notices -SimpleMatch '| Example | https://example.org/example |' -Quiet ) )
        {
            $failures.Add( 'check-licences failed after regeneration, or the reused project is not listed in the notices' )
        }

        Write-Host '[notices-regeneration] notices regenerated from the installed trees and a new provenance row:'
        $steps | ForEach-Object { Write-Host "    $_" }
    }
}
finally
{
    if( $KeepFixtures )
    {
        Write-Host "check-selftest: fixtures kept in $tempBase"
    }
    elseif( Test-Path $tempBase )
    {
        Remove-Item -Recurse -Force $tempBase
    }
}

if( $failures.Count -gt 0 )
{
    Write-Host "check-selftest: FAILED ($( $failures.Count )):" -ForegroundColor Red
    $failures | ForEach-Object { Write-Host "  $_" }
    exit 1
}

Write-Host "check-selftest: OK ($( $checkNames.Count ) checks pass on the clean copy; $( $cases.Count ) deliberate violations detected; $( $allowedCases.Count ) allowed case(s) pass and are reported; notices regeneration is idempotent)" -ForegroundColor Green
exit 0
