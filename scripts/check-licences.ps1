# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 The APP Authors
<#
.SYNOPSIS
    Licence manifest check (P-30, CR-07, spec 4.3, T-11).

.DESCRIPTION
    Fails when:
      1. a docs/licence-manifest.json entry is incomplete, duplicated, has an unknown role, or its licence is not
         allowed by spec 4.3 for a linked library (GPL-2.0-only, LGPL-2.0, AGPL without a recorded decision,
         proprietary, no licence or an unknown identifier; LicenseRef-* only for role "system" with a note);
      2. a vcpkg.json dependency (including feature dependencies) or override has no manifest entry, a direct
         dependency is not pinned with "overrides", or the pinned version differs from the version whose licence
         was verified;
      3. with an installed vcpkg tree: an installed port has no manifest entry or a different version, a manifest
         port is neither a dependency nor installed, the licence vcpkg concluded differs from the manifest without
         a note, or THIRD_PARTY_NOTICES.md is not what scripts/update-notices.ps1 generates;
      4. THIRD_PARTY_NOTICES.md lacks the generated row of a manifest port or of a complete docs/provenance.md row;
      5. a docs/provenance.md row names a licence that spec 4.3 does not allow for copied code (LGPL-2.1-only and
         GPL-2.0-only are not allowed there).
    Without an installed tree (a fresh clone before the first build) rule 3 is skipped and the output says so.

.PARAMETER Root
    Repository root to check. Defaults to the repository containing this script.

.PARAMETER InstalledRoot
    Folder holding the installed vcpkg trees. Defaults to <Root>\vcpkg_installed.

.EXAMPLE
    powershell -ExecutionPolicy Bypass -File scripts\check-licences.ps1
#>
[CmdletBinding()]
param(
    [string]$Root,

    [string]$InstalledRoot
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

$violations = New-Object System.Collections.Generic.List[string]
$notes      = New-Object System.Collections.Generic.List[string]
$required   = @( 'port', 'library', 'version', 'licence', 'role', 'usedBy', 'shipped', 'verified' )

try
{
    $manifest = Read-AppLicenceManifest $Root
    $vcpkg    = Read-AppVcpkgManifest $Root
}
catch
{
    Write-Host "check-licences: FAILED: $( $_.Exception.Message )" -ForegroundColor Red
    exit 1
}

$entries = @{}

foreach( $entry in $manifest )
{
    $port    = Get-AppJsonField $entry 'port'
    $missing = @( $required | Where-Object { [string]::IsNullOrWhiteSpace( ( Get-AppJsonField $entry $_ ) ) } )

    if( $missing.Count -gt 0 )
    {
        $violations.Add( "docs/licence-manifest.json: entry '$port' lacks $( $missing -join ', ' )" )
        continue
    }

    if( $entries.ContainsKey( $port ) )
    {
        $violations.Add( "docs/licence-manifest.json: port '$port' is listed twice" )
        continue
    }

    $entries[$port] = $entry

    if( $script:AppLicenceRoles -notcontains $entry.role )
    {
        $violations.Add( "docs/licence-manifest.json: port '$port' has role '$( $entry.role )' (expected $( $script:AppLicenceRoles -join ', ' ))" )
        continue
    }

    if( $entry.role -eq 'system' -and -not ( Get-AppJsonField $entry 'note' ) )
    {
        $violations.Add( "docs/licence-manifest.json: system library '$port' needs a note explaining why its licence is acceptable" )
    }

    $verdict = Get-AppLicenceVerdict $entry.licence 'link' $entry.role ( Get-AppJsonField $entry 'decision' )

    if( $verdict )
    {
        $violations.Add( "vcpkg port '$port' ($( $entry.licence )): $verdict" )
    }
}

foreach( $port in $vcpkg.Dependencies.Keys )
{
    if( -not $entries.ContainsKey( $port ) )
    {
        $violations.Add( "vcpkg.json: dependency '$port' has no entry in docs/licence-manifest.json (verify its licence first, spec 4.3)" )
        continue
    }

    if( -not $vcpkg.Overrides.ContainsKey( $port ) )
    {
        $violations.Add( "vcpkg.json: dependency '$port' is not pinned with an 'overrides' entry (spec 4.3)" )
    }
}

foreach( $port in $vcpkg.Overrides.Keys )
{
    if( -not $entries.ContainsKey( $port ) )
    {
        $violations.Add( "vcpkg.json: override '$port' has no entry in docs/licence-manifest.json" )
    }
    elseif( $vcpkg.Overrides[$port] -ne $entries[$port].version )
    {
        $violations.Add( "vcpkg.json: '$port' is pinned to $( $vcpkg.Overrides[$port] ) but its licence was verified for $( $entries[$port].version ); re-verify it and update docs/licence-manifest.json (spec 4.3)" )
    }
}

$installed = Read-AppInstalledPorts $InstalledRoot
$trees     = @( $installed | ForEach-Object { $_.Tree } | Select-Object -Unique )

if( $installed.Count -eq 0 )
{
    $notes.Add( "no installed vcpkg tree under '$( Get-AppRelativePath $Root $InstalledRoot )': installed ports, licence texts and generated notices not checked" )
}
else
{
    foreach( $item in $installed )
    {
        if( -not $entries.ContainsKey( $item.Port ) )
        {
            $violations.Add( "vcpkg_installed/$( $item.Tree ): port '$( $item.Port )' has no entry in docs/licence-manifest.json (verify its licence first, spec 4.3)" )
            continue
        }

        $entry = $entries[$item.Port]

        if( $item.Version -ne $entry.version )
        {
            $violations.Add( "vcpkg_installed/$( $item.Tree ): '$( $item.Port )' is installed as $( $item.Version ) but its licence was verified for $( $entry.version ); re-verify it and update docs/licence-manifest.json" )
        }

        $concluded = "$( $item.Concluded )"

        if( $concluded -and $concluded -notmatch '^(NOASSERTION|NONE|LicenseRef-vcpkg-null)$' -and $concluded -ne $entry.licence -and -not ( Get-AppJsonField $entry 'note' ) )
        {
            $violations.Add( "vcpkg_installed/$( $item.Tree ): vcpkg reports '$concluded' for '$( $item.Port )', the manifest says '$( $entry.licence )'; resolve it or add a note" )
        }
    }

    foreach( $port in $entries.Keys )
    {
        if( -not $vcpkg.Dependencies.Contains( $port ) -and -not ( $installed | Where-Object { $_.Port -eq $port } ) )
        {
            $violations.Add( "docs/licence-manifest.json: port '$port' is neither a vcpkg.json dependency nor installed; remove the stale entry" )
        }
    }

    $output = & powershell.exe -NoProfile -ExecutionPolicy Bypass -File ( Join-Path $PSScriptRoot 'update-notices.ps1' ) -Root $Root -InstalledRoot $InstalledRoot -Check 2>&1

    if( $LASTEXITCODE -ne 0 )
    {
        $violations.Add( "THIRD_PARTY_NOTICES.md: $( ( $output | ForEach-Object { "$_".Trim() } | Where-Object { $_ } ) -join ' ' )" )
    }
}

$noticesPath = Join-Path $Root 'THIRD_PARTY_NOTICES.md'

if( -not ( Test-Path $noticesPath ) )
{
    $violations.Add( 'THIRD_PARTY_NOTICES.md: missing (P-30, CR-07)' )
}
else
{
    $notices = [IO.File]::ReadAllLines( $noticesPath )

    foreach( $port in $entries.Keys )
    {
        $features = @()

        if( $vcpkg.Dependencies.Contains( $port ) )
        {
            $features = @( $vcpkg.Dependencies[$port] )
        }

        $expected = Format-AppNoticesPortRow $entries[$port] $features

        if( $notices -notcontains $expected )
        {
            $violations.Add( "THIRD_PARTY_NOTICES.md: no row for port '$port' $( $entries[$port].version ) ($( $entries[$port].licence )); run scripts\update-notices.ps1" )
        }
    }
}

$provenance = Get-AppProvenanceRows $Root

foreach( $row in $provenance )
{
    $project = $row.Cells['Project']
    $licence = "$( $row.Cells['Licence'] )"

    if( $licence -and $licence -notmatch '^(-|\?|TBD|n/a)$' )
    {
        $verdict = Get-AppLicenceVerdict ( $licence -replace '`', '' ) 'copy'

        if( $verdict )
        {
            $violations.Add( "docs/provenance.md: '$project' ($licence): $verdict" )
        }
    }

    if( $row.Missing.Count -eq 0 -and ( Test-Path $noticesPath ) -and ( $notices -notcontains ( Format-AppNoticesProvenanceRow $row ) ) )
    {
        $violations.Add( "THIRD_PARTY_NOTICES.md: no row for reused project '$project'; run scripts\update-notices.ps1" )
    }
}

$notes | ForEach-Object { Write-Host "check-licences: note: $_" -ForegroundColor Yellow }

if( $violations.Count -gt 0 )
{
    Write-Host "check-licences: FAILED with $( $violations.Count ) violation(s):" -ForegroundColor Red
    $violations | ForEach-Object { Write-Host "  $_" }
    exit 1
}

$treeText = 'none'

if( $trees.Count -gt 0 )
{
    $treeText = $trees -join ', '
}

Write-Host "check-licences: OK ($( $entries.Count ) ports, $( $provenance.Count ) provenance rows; installed trees: $treeText)" -ForegroundColor Green
exit 0
