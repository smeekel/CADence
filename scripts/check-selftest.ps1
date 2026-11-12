# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 The APP Authors
<#
.SYNOPSIS
    Prove that the static checks detect what they are meant to detect (T-11).

.DESCRIPTION
    Copies the checked parts of the repository (src, tests, build, scripts, docs, root files; no build output) into
    a temporary folder, verifies that check-layering, check-spdx and check-network-apis pass on the clean copy, then
    applies one deliberate violation per case to a fresh copy and verifies that the matching check fails.
    Allowed cases (a third-party DLL that imports ws2_32.dll, ADR 0026) must pass and appear in the check's summary.
    The repository itself is never modified. Exits non-zero if any expectation is not met.

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
               'vcpkg.json', 'vcpkg-configuration.json', 'LICENSE', 'THIRD_PARTY_NOTICES.md', 'APP.slnx' )
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

function Invoke-Check( [string]$Check, [string]$FixtureRoot, [string]$BinaryDir )
{
    $script    = Join-Path $PSScriptRoot "$Check.ps1"
    $arguments = @( '-Root', $FixtureRoot )

    if( $BinaryDir )
    {
        $arguments += @( '-BinaryDir', ( Join-Path $FixtureRoot $BinaryDir ) )
    }

    $output = & powershell.exe -NoProfile -ExecutionPolicy Bypass -File $script @arguments 2>&1
    $code   = $LASTEXITCODE

    return [pscustomobject]@{ Code = $code; Output = ( $output | ForEach-Object { "$_" } ) }
}

$header = "// SPDX-License-Identifier: GPL-3.0-or-later`n// SPDX-FileCopyrightText: 2026 The APP Authors`n"

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
    @{ Name = 'spdx-missing-header'; Check = 'check-spdx'
       What = 'a new C++ file without an SPDX header'
       Apply = { param( $r ) Set-Text ( Join-Path $r 'src\core\NoHeader.cpp' ) "int answer() { return 42; }`n" } },
    @{ Name = 'spdx-third-party-without-provenance'; Check = 'check-spdx'
       What = 'an MIT-licensed adapted file with no docs/provenance.md entry'
       Apply = { param( $r )
                 Set-Text ( Join-Path $r 'src\core\Adapted.cpp' ) "// SPDX-License-Identifier: MIT`n// SPDX-FileCopyrightText: 2020 Example Project`n// Adapted from example https://example.org/example @ 0123abc`n" } },
    @{ Name = 'network-source'; Check = 'check-network-apis'
       What = 'io includes winsock2.h and calls WSAStartup'
       Apply = { param( $r )
                 Set-Text ( Join-Path $r 'src\io\Net.cpp' ) ( $header + "#include <winsock2.h>`nint start() { WSADATA d; return WSAStartup( 0x0202, &d ); }`n" ) } },
    @{ Name = 'network-link-input'; Check = 'check-network-apis'
       What = 'Cli.vcxproj links ws2_32.lib'
       Apply = { param( $r )
                 $p = Join-Path $r 'src\cli\Cli.vcxproj'
                 $x = [IO.File]::ReadAllText( $p )
                 $x = $x.Replace( '<SubSystem>Console</SubSystem>', '<SubSystem>Console</SubSystem><AdditionalDependencies>ws2_32.lib;%(AdditionalDependencies)</AdditionalDependencies>' )
                 Set-Text $p $x } },
    @{ Name = 'network-vcpkg'; Check = 'check-network-apis'
       What = 'vcpkg.json adds curl and the qtbase network feature'
       Apply = { param( $r )
                 $p = Join-Path $r 'vcpkg.json'
                 $x = [IO.File]::ReadAllText( $p )
                 $x = $x.Replace( '"name": "gtest",', '"name": "curl" }, { "name": "gtest",' ).Replace( '"widgets"', '"widgets", "network"' )
                 Set-Text $p $x } },
    # wsock32.dll ships with Windows and imports ws2_32.dll, so it stands in for a binary with a socket import.
    @{ Name = 'network-own-binary'; Check = 'check-network-apis'; BinaryDir = 'bin'
       What = 'an APP-built executable (Cli.exe) imports ws2_32.dll'
       Apply = { param( $r )
                 New-Item -ItemType Directory -Force ( Join-Path $r 'bin\Cli' ) | Out-Null
                 Copy-Item ( Join-Path $env:SystemRoot 'System32\wsock32.dll' ) ( Join-Path $r 'bin\Cli\Cli.exe' ) } }
)

# Cases that must pass, with output that must match.
$allowedCases = @(
    @{ Name = 'network-third-party-import'; Check = 'check-network-apis'; BinaryDir = 'bin'
       What = 'a third-party DLL imports ws2_32.dll: allowed and listed in the summary'
       Expect = 'allowed: vendornet\.dll -> WS2_32\.dll'
       Apply = { param( $r )
                 New-Item -ItemType Directory -Force ( Join-Path $r 'bin\Vendor' ) | Out-Null
                 Copy-Item ( Join-Path $env:SystemRoot 'System32\wsock32.dll' ) ( Join-Path $r 'bin\Vendor\vendornet.dll' ) } }
)

try
{
    $clean = New-Fixture 'clean'

    foreach( $check in 'check-layering', 'check-spdx', 'check-network-apis' )
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

Write-Host "check-selftest: OK (3 checks pass on the clean copy; $( $cases.Count ) deliberate violations detected; $( $allowedCases.Count ) allowed case passes and is reported)" -ForegroundColor Green
exit 0
