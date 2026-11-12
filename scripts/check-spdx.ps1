# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 The APP Authors
<#
.SYNOPSIS
    Static check for SPDX headers and provenance of reused code (CR-07, P-30, T-11).

.DESCRIPTION
    Checks every source file (C++, PowerShell, Python, batch, MSBuild .vcxproj/.props/.targets) outside out/,
    vcpkg_installed/ and tool folders:
      1. an "SPDX-License-Identifier:" line within the first 10 lines;
      2. original files (licence GPL-3.0-or-later, not under third_party/, no "Adapted from" marker) also carry
         "SPDX-FileCopyrightText: <year> The APP Authors";
      3. third-party files (under third_party/, marked "Adapted from", or with another licence or copyright holder)
         are named in docs/provenance.md: by their repository-relative path, or by their third_party/<project>/
         folder;
      4. LICENSE (GPL-3.0 text) and THIRD_PARTY_NOTICES.md exist.
    JSON, Markdown and IDE-generated files (.slnx, .vcxproj.filters) carry no header and are not checked.

.PARAMETER Root
    Repository root to check. Defaults to the repository containing this script.

.EXAMPLE
    powershell -ExecutionPolicy Bypass -File scripts\check-spdx.ps1
#>
[CmdletBinding()]
param(
    [string]$Root
)

. "$PSScriptRoot\common.ps1"

if( -not $Root )
{
    $Root = Get-AppRepoRoot
}

$Root = ( Resolve-Path $Root ).Path

$extensions = @( '.h', '.hpp', '.cpp', '.cxx', '.cc', '.c', '.inl', '.ipp', '.ps1', '.psm1', '.psd1', '.py', '.bat',
                 '.cmd', '.vcxproj', '.props', '.targets' )
$headerLines = 10
$productLicence = 'GPL-3.0-or-later'
$violations = New-Object System.Collections.Generic.List[string]

$licensePath = Join-Path $Root 'LICENSE'

if( -not ( Test-Path $licensePath ) )
{
    $violations.Add( 'LICENSE: missing (P-30 requires the GPL-3.0 text)' )
}
else
{
    $head = ( Get-Content $licensePath -TotalCount 3 ) -join ' '

    if( $head -notmatch 'GNU GENERAL PUBLIC LICENSE' -or $head -notmatch 'Version 3' )
    {
        $violations.Add( 'LICENSE: does not start with the GNU GPL version 3 text' )
    }
}

if( -not ( Test-Path ( Join-Path $Root 'THIRD_PARTY_NOTICES.md' ) ) )
{
    $violations.Add( 'THIRD_PARTY_NOTICES.md: missing (P-30, CR-07)' )
}

$provenancePath = Join-Path $Root 'docs\provenance.md'
$provenance     = ''

if( Test-Path $provenancePath )
{
    $provenance = ( Get-Content -Raw $provenancePath ) -replace '\\', '/'
}

$files = @( Get-AppSourceFiles -Root $Root -Extensions $extensions )

foreach( $file in $files )
{
    $rel   = Get-AppRelativePath $Root $file.FullName
    $lines = @( Get-Content $file.FullName -TotalCount $headerLines -ErrorAction SilentlyContinue )
    $text  = $lines -join "`n"

    if( $text -notmatch 'SPDX-License-Identifier:\s*([^\s>]+(?:\s+(?:OR|AND|WITH)\s+[^\s>]+)*)' )
    {
        $violations.Add( "${rel}: missing 'SPDX-License-Identifier:' in the first $headerLines lines" )
        continue
    }

    $licence      = $Matches[1]
    $appCopyright = $text -match 'SPDX-FileCopyrightText:\s*\d{4}(-\d{4})?\s+The APP Authors'
    $anyCopyright = $text -match 'SPDX-FileCopyrightText:'
    $adapted      = ( Get-Content -Raw $file.FullName ) -match '(?m)^\s*(//|#|<!--|;|REM)\s*Adapted from\s'
    $inThirdParty = $rel.StartsWith( 'third_party/' )
    $isThirdParty = $inThirdParty -or $adapted -or ( $licence -ne $productLicence ) -or ( $anyCopyright -and -not $appCopyright )

    if( -not $isThirdParty )
    {
        if( -not $appCopyright )
        {
            $violations.Add( "${rel}: missing 'SPDX-FileCopyrightText: <year> The APP Authors' in the first $headerLines lines" )
        }

        continue
    }

    $key = $rel

    if( $inThirdParty )
    {
        $key = 'third_party/' + ( $rel -split '/' )[1] + '/'
    }

    if( -not $provenance )
    {
        $violations.Add( "${rel}: third-party file (licence '$licence') but docs/provenance.md does not exist (CR-07)" )
    }
    elseif( $provenance.IndexOf( $key, [StringComparison]::OrdinalIgnoreCase ) -lt 0 )
    {
        $violations.Add( "${rel}: third-party file (licence '$licence') has no entry for '$key' in docs/provenance.md (CR-07)" )
    }
}

if( $violations.Count -gt 0 )
{
    Write-Host "check-spdx: FAILED with $( $violations.Count ) violation(s):" -ForegroundColor Red
    $violations | ForEach-Object { Write-Host "  $_" }
    exit 1
}

Write-Host "check-spdx: OK ($( $files.Count ) files)" -ForegroundColor Green
exit 0
