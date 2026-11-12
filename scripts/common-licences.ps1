# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 The APP Authors
#
# Licence helpers shared by check-licences.ps1 and update-notices.ps1 (P-30, CR-07, spec 4.3).
# Dot-source it after common.ps1:  . "$PSScriptRoot\common-licences.ps1"

# Spec 4.3. 'link' applies to libraries APP links or loads; 'copy' to code reused into APP files, where
# LGPL-2.1-only is not allowed.
$script:AppLicencesAllowed = @( 'MIT', 'BSD-2-Clause', 'BSD-3-Clause', 'BSL-1.0', 'Zlib', 'ISC', 'Apache-2.0', 'MPL-2.0',
                                'LGPL-2.1-or-later', 'LGPL-3.0-only', 'LGPL-3.0-or-later',
                                'GPL-2.0-or-later', 'GPL-3.0-only', 'GPL-3.0-or-later' )
$script:AppLicencesLinkOnly = @( 'LGPL-2.1-only' )
$script:AppLicencesDecision = @( 'AGPL-3.0-only', 'AGPL-3.0-or-later' )

# Exceptions only add permissions, so a known exception keeps the verdict of the licence it modifies.
$script:AppLicenceExceptions = @( 'PCRE2-exception', 'OCCT-exception-1.0', 'LLVM-exception', 'Classpath-exception-2.0',
                                  'GCC-exception-3.1', 'Qt-GPL-exception-1.0', 'Qt-LGPL-exception-1.1', 'Bison-exception-2.2',
                                  'Autoconf-exception-3.0' )

$script:AppLicenceRoles = @( 'linked', 'test', 'build', 'system' )

# Property value of a JSON object, or $null when absent (StrictMode forbids reading a missing property).
function Get-AppJsonField( [object]$Object, [string]$Name )
{
    if( $null -eq $Object )
    {
        return $null
    }

    $property = $Object.PSObject.Properties[$Name]

    if( $null -eq $property )
    {
        return $null
    }

    return $property.Value
}

function Get-AppLicenceIdVerdict( [string]$Id, [string]$Use, [string]$Role, [string]$Decision )
{
    if( $script:AppLicencesAllowed -contains $Id )
    {
        return ''
    }

    if( $script:AppLicencesLinkOnly -contains $Id )
    {
        if( $Use -eq 'link' )
        {
            return ''
        }

        return "$Id is not allowed for code copied into APP files (spec 4.3; linking the library is allowed)"
    }

    if( $script:AppLicencesDecision -contains $Id )
    {
        if( $Use -eq 'link' -and $Decision )
        {
            return ''
        }

        return "$Id is allowed only by a recorded decision (spec 4.3)"
    }

    if( $Id.StartsWith( 'LicenseRef-' ) -and $Role -eq 'system' )
    {
        return ''
    }

    if( $Id -match '^(GPL-1\.0|GPL-2\.0|LGPL-2\.0|LGPL-2\.1)(-only)?$' )
    {
        return "$Id is not allowed (incompatible with GPL-3.0, spec 4.3)"
    }

    if( $Id -match '^(NOASSERTION|NONE)$' )
    {
        return "no licence ($Id) is not allowed (spec 4.3)"
    }

    return "$Id is not an allowed licence (spec 4.3: proprietary, source-available, non-commercial and unknown licences are not allowed)"
}

function Read-AppLicenceTerm( [hashtable]$State )
{
    if( $State.Index -ge $State.Tokens.Count )
    {
        throw 'unexpected end'
    }

    $token = $State.Tokens[$State.Index]
    $State.Index++

    if( $token -eq '(' )
    {
        $verdict = Read-AppLicenceOr $State

        if( $State.Index -ge $State.Tokens.Count -or $State.Tokens[$State.Index] -ne ')' )
        {
            throw "missing ')'"
        }

        $State.Index++
        return $verdict
    }

    if( @( '(', ')', 'AND', 'OR', 'WITH' ) -contains $token )
    {
        throw "unexpected '$token'"
    }

    $verdict = Get-AppLicenceIdVerdict $token $State.Use $State.Role $State.Decision

    if( $State.Index -lt $State.Tokens.Count -and $State.Tokens[$State.Index] -eq 'WITH' )
    {
        $State.Index++

        if( $State.Index -ge $State.Tokens.Count )
        {
            throw "missing exception after 'WITH'"
        }

        $exception = $State.Tokens[$State.Index]
        $State.Index++

        if( $script:AppLicenceExceptions -notcontains $exception -and -not $verdict )
        {
            $verdict = "$token WITH $exception uses an unknown exception (add it to common-licences.ps1 after review)"
        }
    }

    return $verdict
}

function Read-AppLicenceAnd( [hashtable]$State )
{
    $reasons = New-Object System.Collections.Generic.List[string]
    $verdict = Read-AppLicenceTerm $State

    if( $verdict ) { $reasons.Add( $verdict ) }

    while( $State.Index -lt $State.Tokens.Count -and $State.Tokens[$State.Index] -eq 'AND' )
    {
        $State.Index++
        $verdict = Read-AppLicenceTerm $State

        if( $verdict ) { $reasons.Add( $verdict ) }
    }

    return ( $reasons -join '; ' )
}

function Read-AppLicenceOr( [hashtable]$State )
{
    $reasons = New-Object System.Collections.Generic.List[string]
    $allowed = $false
    $verdict = Read-AppLicenceAnd $State

    if( $verdict ) { $reasons.Add( $verdict ) } else { $allowed = $true }

    while( $State.Index -lt $State.Tokens.Count -and $State.Tokens[$State.Index] -eq 'OR' )
    {
        $State.Index++
        $verdict = Read-AppLicenceAnd $State

        if( $verdict ) { $reasons.Add( $verdict ) } else { $allowed = $true }
    }

    if( $allowed )
    {
        return ''
    }

    return ( $reasons -join ' / ' )
}

# Empty string when the SPDX expression is allowed by spec 4.3, otherwise the reason.
# Use: 'link' for libraries, 'copy' for reused code. Role 'system' admits LicenseRef-* system libraries.
# Decision: the recorded decision that admits AGPL-3.0 for a library.
function Get-AppLicenceVerdict( [string]$Expression, [string]$Use = 'link', [string]$Role = '', [string]$Decision = '' )
{
    if( [string]::IsNullOrWhiteSpace( $Expression ) )
    {
        return 'no licence given (spec 4.3)'
    }

    $tokens = @( [regex]::Matches( $Expression, '\(|\)|[^\s()]+' ) | ForEach-Object { $_.Value } )
    $state  = @{ Tokens = $tokens; Index = 0; Use = $Use; Role = $Role; Decision = $Decision }

    try
    {
        $verdict = Read-AppLicenceOr $state

        if( $state.Index -lt $tokens.Count )
        {
            throw "unexpected '$( $tokens[$state.Index] )'"
        }
    }
    catch
    {
        return "'$Expression' is not a valid SPDX expression ($( $_.Exception.Message ))"
    }

    return $verdict
}

# vcpkg version string as vcpkg prints it: "1.2.3" or "1.2.3#2" when the port version is not zero.
function Format-AppPortVersion( [string]$Version, [object]$PortVersion )
{
    if( $null -ne $PortVersion -and [int]$PortVersion -ne 0 )
    {
        return "$Version#$PortVersion"
    }

    return $Version
}

function Read-AppLicenceManifest( [string]$Root )
{
    $path = Join-Path $Root 'docs\licence-manifest.json'

    if( -not ( Test-Path $path ) )
    {
        throw "docs/licence-manifest.json does not exist"
    }

    return @( ( [IO.File]::ReadAllText( $path ) | ConvertFrom-Json ).ports )
}

# Direct dependencies of vcpkg.json (including feature dependencies) with their requested features, and the
# pinned override versions.
function Read-AppVcpkgManifest( [string]$Root )
{
    $json         = [IO.File]::ReadAllText( ( Join-Path $Root 'vcpkg.json' ) ) | ConvertFrom-Json
    $dependencies = [ordered]@{}
    $lists        = New-Object System.Collections.Generic.List[object]
    $lists.Add( @( Get-AppJsonField $json 'dependencies' ) )
    $features     = Get-AppJsonField $json 'features'

    if( $features )
    {
        foreach( $feature in $features.PSObject.Properties )
        {
            $lists.Add( @( Get-AppJsonField $feature.Value 'dependencies' ) )
        }
    }

    foreach( $list in $lists )
    {
        foreach( $dependency in $list )
        {
            if( $null -eq $dependency )
            {
                continue
            }

            $name      = $dependency
            $requested = @()

            if( $dependency -isnot [string] )
            {
                $name      = Get-AppJsonField $dependency 'name'
                $requested = @( Get-AppJsonField $dependency 'features' | Where-Object { $_ } | ForEach-Object { "$_" } )
            }

            if( -not $dependencies.Contains( $name ) )
            {
                $dependencies[$name] = New-Object System.Collections.Generic.List[string]
            }

            foreach( $f in $requested )
            {
                if( -not $dependencies[$name].Contains( $f ) )
                {
                    $dependencies[$name].Add( $f )
                }
            }
        }
    }

    $overrides = @{}

    foreach( $override in @( Get-AppJsonField $json 'overrides' ) )
    {
        if( $null -ne $override )
        {
            $overrides[( Get-AppJsonField $override 'name' )] = Format-AppPortVersion ( Get-AppJsonField $override 'version' ) ( Get-AppJsonField $override 'port-version' )
        }
    }

    return [pscustomobject]@{ Dependencies = $dependencies; Overrides = $overrides }
}

function Read-AppVcpkgBaseline( [string]$Root )
{
    $json     = [IO.File]::ReadAllText( ( Join-Path $Root 'vcpkg-configuration.json' ) ) | ConvertFrom-Json
    $registry = Get-AppJsonField $json 'default-registry'

    return [pscustomobject]@{ Repository = ( Get-AppJsonField $registry 'repository' ); Baseline = ( Get-AppJsonField $registry 'baseline' ) }
}

# Installed vcpkg trees under $InstalledRoot (vcpkg_installed\<tree>\x64-windows): one entry per tree and port with
# the installed version, the licence vcpkg concluded and the copyright file.
function Read-AppInstalledPorts( [string]$InstalledRoot, [string]$Triplet = 'x64-windows' )
{
    $ports = New-Object System.Collections.Generic.List[object]

    if( -not $InstalledRoot -or -not ( Test-Path $InstalledRoot ) )
    {
        return ,$ports
    }

    foreach( $tree in @( Get-ChildItem -Path $InstalledRoot -Directory | Sort-Object Name ) )
    {
        $info = Join-Path $tree.FullName 'vcpkg\info'

        if( -not ( Test-Path $info ) )
        {
            continue
        }

        foreach( $list in @( Get-ChildItem -Path $info -Filter "*_$Triplet.list" | Sort-Object Name ) )
        {
            $name      = $list.Name.Split( '_' )[0]
            $share     = Join-Path $tree.FullName "$Triplet\share\$name"
            $spdxPath  = Join-Path $share 'vcpkg.spdx.json'
            $version   = $null
            $concluded = $null

            if( Test-Path $spdxPath )
            {
                $spdx = [IO.File]::ReadAllText( $spdxPath ) | ConvertFrom-Json
                $port = @( $spdx.packages | Where-Object { $_.SPDXID -eq 'SPDXRef-port' } ) | Select-Object -First 1

                if( $port )
                {
                    $version   = Get-AppJsonField $port 'versionInfo'
                    $concluded = Get-AppJsonField $port 'licenseConcluded'
                }
            }

            $copyright = Join-Path $share 'copyright'

            if( -not ( Test-Path $copyright ) )
            {
                $copyright = $null
            }

            $ports.Add( [pscustomobject]@{ Tree = $tree.Name; Port = $name; Version = $version; Concluded = $concluded; Copyright = $copyright } )
        }
    }

    return ,$ports
}

function Format-AppNoticesPortRow( [object]$Entry, [string[]]$Features )
{
    $portCell = "``$( $Entry.port )`` $( $Entry.version )"

    if( $Features -and $Features.Count -gt 0 )
    {
        $portCell += ', features ' + ( ( $Features | ForEach-Object { "``$_``" } ) -join ', ' )
    }

    $licenceCell = $Entry.licence
    $usedUnder   = Get-AppJsonField $Entry 'usedUnder'

    if( $usedUnder )
    {
        $licenceCell += " (used under $usedUnder)"
    }

    return "| $( $Entry.library ) | $portCell | $licenceCell | $( $Entry.role ) | $( $Entry.usedBy ) | $( $Entry.shipped ) |"
}

function Format-AppNoticesProvenanceRow( [object]$Row )
{
    $cells = $Row.Cells
    return "| $( $cells['Project'] ) | $( $cells['URL'] ) | $( $cells['Version / commit'] ) | $( $cells['Licence'] ) | $( $cells['Destination file(s)'] ) | $( $cells['Step ID'] ) |"
}
