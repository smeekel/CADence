# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 The APP Authors
<#
.SYNOPSIS
    Regenerate the generated sections of THIRD_PARTY_NOTICES.md (P-30, CR-07).

.DESCRIPTION
    Rewrites the text between the "BEGIN generated" and "END generated" markers of THIRD_PARTY_NOTICES.md:
      - libraries: one row per port of docs/licence-manifest.json with its reviewed licence, the features
        requested in vcpkg.json, the ports of each installed vcpkg tree and the SHA-256 of every installed
        licence text (vcpkg_installed/<tree>/x64-windows/share/<port>/copyright);
      - reused-code: the complete rows of docs/provenance.md.
    Text outside the markers is kept as written. The output depends only on its inputs, so a second run
    changes nothing. Fails when an installed port has no manifest entry, a manifest port is not installed, or
    the installed version differs from the manifest; scripts/check-licences.ps1 reports the licence policy.

.PARAMETER Root
    Repository root. Defaults to the repository containing this script.

.PARAMETER InstalledRoot
    Folder holding the installed vcpkg trees. Defaults to <Root>\vcpkg_installed (created by scripts\build.ps1).

.PARAMETER Check
    Do not write; exit 1 when THIRD_PARTY_NOTICES.md is not up to date.

.EXAMPLE
    powershell -ExecutionPolicy Bypass -File scripts\update-notices.ps1
#>
[CmdletBinding()]
param(
    [string]$Root,

    [string]$InstalledRoot,

    [switch]$Check
)

. "$PSScriptRoot\common.ps1"
. "$PSScriptRoot\common-licences.ps1"

if( -not $Root )
{
    $Root = Get-AppRepoRoot
}

$Root = ( Resolve-Path $Root ).Path

if( -not $InstalledRoot )
{
    $InstalledRoot = Join-Path $Root 'vcpkg_installed'
}

$noticesPath = Join-Path $Root 'THIRD_PARTY_NOTICES.md'
$errors      = New-Object System.Collections.Generic.List[string]
$manifest    = Read-AppLicenceManifest $Root
$vcpkg       = Read-AppVcpkgManifest $Root
$baseline    = Read-AppVcpkgBaseline $Root
$installed   = Read-AppInstalledPorts $InstalledRoot

if( $installed.Count -eq 0 )
{
    Write-Host "update-notices: no installed vcpkg tree under '$InstalledRoot'; run scripts\build.ps1 first." -ForegroundColor Red
    exit 1
}

$manifestPorts = @( $manifest | ForEach-Object { $_.port } )

foreach( $item in $installed )
{
    $entry = @( $manifest | Where-Object { $_.port -eq $item.Port } ) | Select-Object -First 1

    if( -not $entry )
    {
        $errors.Add( "port '$( $item.Port )' is installed in '$( $item.Tree )' but has no entry in docs/licence-manifest.json" )
    }
    elseif( $item.Version -ne $entry.version )
    {
        $errors.Add( "port '$( $item.Port )' is installed as $( $item.Version ) in '$( $item.Tree )'; docs/licence-manifest.json verified $( $entry.version )" )
    }

    if( -not $item.Copyright )
    {
        $errors.Add( "port '$( $item.Port )' in '$( $item.Tree )' installed no copyright file" )
    }
}

foreach( $port in $manifestPorts )
{
    if( -not ( $installed | Where-Object { $_.Port -eq $port } ) )
    {
        $errors.Add( "port '$port' is in docs/licence-manifest.json but not installed in any tree" )
    }
}

$hashes = [ordered]@{}

foreach( $item in ( $installed | Where-Object { $_.Copyright } ) )
{
    $hash = ( Get-FileHash -Algorithm SHA256 -Path $item.Copyright ).Hash.ToLowerInvariant()

    if( $hashes.Contains( $item.Port ) -and $hashes[$item.Port] -ne $hash )
    {
        $errors.Add( "port '$( $item.Port )' has different licence texts in the installed trees" )
    }

    $hashes[$item.Port] = $hash
}

if( $errors.Count -gt 0 )
{
    Write-Host "update-notices: FAILED with $( $errors.Count ) error(s):" -ForegroundColor Red
    $errors | ForEach-Object { Write-Host "  $_" }
    exit 1
}

$libraries = New-Object System.Collections.Generic.List[string]
$libraries.Add( '## Libraries' )
$libraries.Add( '' )
$libraries.Add( "vcpkg baseline ``$( $baseline.Baseline )`` ($( $baseline.Repository )), triplet ``x64-windows``. Each port is" )
$libraries.Add( 'listed with the version whose licence was verified (`docs/licence-manifest.json`); `scripts/check-licences.ps1` fails' )
$libraries.Add( 'when the installed version, the pinned version in `vcpkg.json` or this file differs. Roles: `linked` is linked or loaded' )
$libraries.Add( 'by APP and shipped, `test` only by test executables, `build` is used at build time only, `system` is part of Windows.' )
$libraries.Add( '' )
$libraries.Add( '| Library | vcpkg port and version | Licence (SPDX) | Role | Used by | Shipped |' )
$libraries.Add( '|---|---|---|---|---|---|' )

foreach( $entry in $manifest )
{
    $features = @()

    if( $vcpkg.Dependencies.Contains( $entry.port ) )
    {
        $features = @( $vcpkg.Dependencies[$entry.port] )
    }

    $libraries.Add( ( Format-AppNoticesPortRow $entry $features ) )
}

$libraries.Add( '' )
$libraries.Add( 'Installed trees (`vcpkg_installed/<tree>`; `full` is used by Debug and Release, `headless` by Headless):' )
$libraries.Add( '' )

foreach( $tree in @( $installed | ForEach-Object { $_.Tree } | Select-Object -Unique ) )
{
    $ports = @( $installed | Where-Object { $_.Tree -eq $tree } | ForEach-Object { "``$( $_.Port )``" } )
    $libraries.Add( "- ``$tree``: $( $ports -join ', ' )" )
}

$libraries.Add( '' )
$libraries.Add( '### Licence texts' )
$libraries.Add( '' )
$libraries.Add( 'vcpkg installs each port''s copyright and licence text as `share/<port>/copyright` in the tree; these files ship with' )
$libraries.Add( 'the application. The SHA-256 identifies the exact text the notices were generated from.' )
$libraries.Add( '' )
$libraries.Add( '| Port | Licence text | SHA-256 |' )
$libraries.Add( '|---|---|---|' )

foreach( $entry in $manifest )
{
    $libraries.Add( "| ``$( $entry.port )`` | ``share/$( $entry.port )/copyright`` | ``$( $hashes[$entry.port] )`` |" )
}

$reused   = New-Object System.Collections.Generic.List[string]
$rows     = Get-AppProvenanceRows $Root
$complete = @( $rows | Where-Object { $_.Missing.Count -eq 0 } )
$reused.Add( '## Reused source code' )
$reused.Add( '' )

if( $complete.Count -eq 0 )
{
    $reused.Add( 'None. Code reused under CR-04 is recorded in `docs/provenance.md` (CR-07) and listed here.' )
}
else
{
    $reused.Add( 'Code reused under CR-04, from the complete rows of `docs/provenance.md` (CR-07):' )
    $reused.Add( '' )
    $reused.Add( '| Project | URL | Version / commit | Licence | Destination file(s) | Step ID |' )
    $reused.Add( '|---|---|---|---|---|---|' )
    $complete | ForEach-Object { $reused.Add( ( Format-AppNoticesProvenanceRow $_ ) ) }
}

$sections = [ordered]@{
    'libraries'   = @{ Lines = $libraries; Source = 'docs/licence-manifest.json and the installed vcpkg trees' }
    'reused-code' = @{ Lines = $reused;    Source = 'docs/provenance.md' }
}

$text    = [IO.File]::ReadAllText( $noticesPath )
$newline = "`n"

if( $text.Contains( "`r`n" ) )
{
    $newline = "`r`n"
}

$updated = $text

foreach( $name in $sections.Keys )
{
    $begin   = "<!-- BEGIN generated by scripts/update-notices.ps1: $name -->"
    $end     = "<!-- END generated: $name -->"
    $first   = $updated.IndexOf( $begin )
    $last    = $updated.IndexOf( $end )

    if( $first -lt 0 -or $last -lt $first )
    {
        Write-Host "update-notices: THIRD_PARTY_NOTICES.md lacks the markers '$begin' ... '$end'." -ForegroundColor Red
        exit 1
    }

    $section = $sections[$name]
    $body    = @( "<!-- Do not edit: regenerate from $( $section.Source ). -->" ) + @( $section.Lines )
    $block   = $begin + $newline + ( $body -join $newline ) + $newline + $end
    $updated = $updated.Substring( 0, $first ) + $block + $updated.Substring( $last + $end.Length )
}

if( $updated -ceq $text )
{
    Write-Host 'update-notices: THIRD_PARTY_NOTICES.md is up to date' -ForegroundColor Green
    exit 0
}

if( $Check )
{
    Write-Host 'update-notices: THIRD_PARTY_NOTICES.md is out of date; run scripts\update-notices.ps1' -ForegroundColor Red
    exit 1
}

[IO.File]::WriteAllText( $noticesPath, $updated, ( New-Object System.Text.UTF8Encoding( $false ) ) )
Write-Host 'update-notices: THIRD_PARTY_NOTICES.md regenerated' -ForegroundColor Green
exit 0
