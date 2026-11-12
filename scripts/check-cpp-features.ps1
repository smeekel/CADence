# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 The APP Authors
<#
.SYNOPSIS
    C++ feature allow-list check (L-03, L-04, L-05, T-11).

.DESCRIPTION
    Reads the "Detected by" column of two tables in docs/cpp-features.md and fails on:
      1. a feature from "Not allowed" (named modules, <generator>, std::stacktrace outside src/platform/, ...)
         anywhere except the path prefixes in its "Allowed in" column;
      2. a feature from "Allowed with a feature-test check and fallback" used directly outside src/core/Compat.h,
         which wraps it with a feature-test check and a fallback (L-04).
    Each code span in the "Detected by" column is a .NET regular expression ("\|" stands for "|"), matched against
    every line of the C++ files under src/ and tests/ after comments and the contents of string and character
    literals are blanked, so line numbers stay valid. C++ raw string literals are not recognised as strings.
    tests/core/spikes/cpp_feature_probe.cpp uses banned features on purpose and is skipped (ADR 0034).

.PARAMETER Root
    Repository root to check. Defaults to the repository containing this script.

.EXAMPLE
    powershell -ExecutionPolicy Bypass -File scripts\check-cpp-features.ps1
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

$Root       = ( Resolve-Path $Root ).Path
$violations = New-Object System.Collections.Generic.List[string]
$cppExt     = @( '.h', '.hpp', '.cpp', '.cxx', '.cc', '.c', '.inl', '.ipp' )
$exempt     = @( 'tests/core/spikes/cpp_feature_probe.cpp' )
$compatPath = 'src/core/Compat.h'
$docPath    = Join-Path $Root 'docs\cpp-features.md'

if( -not ( Test-Path $docPath ) )
{
    Write-Host 'check-cpp-features: docs/cpp-features.md not found' -ForegroundColor Red
    exit 1
}

# Rows of the table under the heading that starts with $Heading: Cells maps column name to text.
function Get-FeatureRows( [string[]]$Lines, [string]$Heading )
{
    $rows    = New-Object System.Collections.Generic.List[object]
    $inside  = $false
    $columns = $null

    foreach( $line in $Lines )
    {
        if( $line -match '^#+\s' )
        {
            if( $inside -and $null -ne $columns )
            {
                break
            }

            $inside = $line -match ( '^#+\s+' + [regex]::Escape( $Heading ) )
            continue
        }

        if( -not $inside )
        {
            continue
        }

        if( $line -notmatch '^\s*\|' )
        {
            if( $null -ne $columns )
            {
                break
            }

            continue
        }

        $cells = @( ( $line.Trim() -replace '^\|', '' -replace '(?<!\\)\|$', '' ) -split '(?<!\\)\|' | ForEach-Object { $_.Trim() } )

        if( $null -eq $columns )
        {
            $columns = $cells
            continue
        }

        if( $line -match '^\s*\|[\s:|-]+\|\s*$' )
        {
            continue
        }

        $row = @{}

        for( $i = 0; $i -lt $columns.Count; $i++ )
        {
            $row[$columns[$i]] = ''

            if( $i -lt $cells.Count )
            {
                $row[$columns[$i]] = $cells[$i]
            }
        }

        $rows.Add( $row )
    }

    return ,$rows
}

function Get-CodeSpans( [string]$Cell )
{
    return @( [regex]::Matches( $Cell, '`([^`]+)`' ) | ForEach-Object { $_.Groups[1].Value -replace '\\\|', '|' } )
}

$docLines = [IO.File]::ReadAllLines( $docPath )
$rules    = New-Object System.Collections.Generic.List[object]
$tables   = @(
    @{ Heading = 'Not allowed'; Kind = 'not allowed (L-03, L-05)'; Allowed = 'Allowed in' },
    @{ Heading = 'Allowed with a feature-test check and fallback'; Kind = 'conditional, use it through src/core/Compat.h (L-04)'; Allowed = $null }
)

foreach( $table in $tables )
{
    $rows = Get-FeatureRows $docLines $table.Heading

    if( $rows.Count -eq 0 )
    {
        $violations.Add( "docs/cpp-features.md: no table found under the heading '$( $table.Heading )'" )
        continue
    }

    foreach( $row in $rows )
    {
        $feature  = ( $row['Feature'] -replace '`', '' ).Trim()
        $patterns = @()

        if( $row.ContainsKey( 'Detected by (regular expressions)' ) )
        {
            $patterns = @( Get-CodeSpans $row['Detected by (regular expressions)'] )
        }

        if( $patterns.Count -eq 0 )
        {
            $violations.Add( "docs/cpp-features.md: row '$feature' under '$( $table.Heading )' has no detection pattern" )
            continue
        }

        $allowedIn = @( $compatPath )

        if( $table.Allowed )
        {
            $allowedIn = @( Get-CodeSpans $row[$table.Allowed] )
        }

        foreach( $pattern in $patterns )
        {
            try
            {
                $regex = New-Object regex( $pattern, [Text.RegularExpressions.RegexOptions]::CultureInvariant )
            }
            catch
            {
                $violations.Add( "docs/cpp-features.md: row '$feature' has an invalid pattern '$pattern'" )
                continue
            }

            $rules.Add( [pscustomobject]@{ Feature = $feature; Kind = $table.Kind; Regex = $regex; AllowedIn = $allowedIn } )
        }
    }
}

# Comments and literal contents become blanks with the same line breaks, so line numbers stay valid.
$cppTokens = New-Object regex( '("(?:[^"\\\n]|\\.)*"|''(?:[^''\\\n]|\\.)*'')|//[^\n]*|/\*[\s\S]*?\*/' )

function Clear-CppCommentsAndLiterals( [string]$Text )
{
    return $cppTokens.Replace( $Text, {
        param( $m )

        if( $m.Groups[1].Success )
        {
            $quote = $m.Value.Substring( 0, 1 )
            return $quote + ( ' ' * ( $m.Value.Length - 2 ) ) + $quote
        }

        return ( $m.Value -replace '[^\n]', ' ' )
    } )
}

$files = @( Get-AppSourceFiles -Root $Root -Extensions $cppExt |
            Where-Object { ( Get-AppRelativePath $Root $_.FullName ) -match '^(src|tests)/' } |
            Where-Object { $exempt -notcontains ( Get-AppRelativePath $Root $_.FullName ) } )

foreach( $file in $files )
{
    $rel    = Get-AppRelativePath $Root $file.FullName
    $active = @( $rules | Where-Object {
                     $rule = $_
                     @( $rule.AllowedIn | Where-Object { $rel.StartsWith( $_, [StringComparison]::OrdinalIgnoreCase ) } ).Count -eq 0 } )

    if( $active.Count -eq 0 )
    {
        continue
    }

    $code   = ( Clear-CppCommentsAndLiterals ( [IO.File]::ReadAllText( $file.FullName ) -replace "`r`n", "`n" ) ) -split "`n"
    $lineNo = 0

    foreach( $line in $code )
    {
        $lineNo++

        foreach( $rule in $active )
        {
            if( $rule.Regex.IsMatch( $line ) )
            {
                $violations.Add( "${rel}:${lineNo}: C++ feature '$( $rule.Feature )' is $( $rule.Kind )" )
            }
        }
    }
}

if( $violations.Count -gt 0 )
{
    Write-Host "check-cpp-features: FAILED with $( $violations.Count ) violation(s):" -ForegroundColor Red
    $violations | ForEach-Object { Write-Host "  $_" }
    Write-Host '  See docs/cpp-features.md; a feature moves between sections only through an ADR.'
    exit 1
}

Write-Host "check-cpp-features: OK ($( $files.Count ) C++ files, $( $rules.Count ) patterns from docs/cpp-features.md)" -ForegroundColor Green
exit 0
