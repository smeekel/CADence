# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 The APP Authors
<#
.SYNOPSIS
    Clean-room and data-source check (CR-02, CR-03, CR-05, CR-06, T-11).

.DESCRIPTION
    Reads the term tables of docs/clean-room.md and fails on:
      1. a vendor trademark term (CR-02) in a string literal of a C++ file under src/, in a Qt .ui, .ts or .qml file,
         or in a JSON file under libraries/;
      2. a native CAD file extension of another product (CR-03) in a string literal of a C++ file under src/ or in a
         Qt .ui file (file-dialog filters, importer and exporter registrations);
      3. a telemetry or update-check keyword (CR-06) in the code or string literals of a C++ file under src/, or in
         any other text file under src/ except Markdown documentation; C++ comments are skipped;
      4. a file under libraries/ (other than README.md files and libraries/seed-manifest.json) without a complete
         row in docs/data-sources.md whose "Table / item" column names it or a folder containing it (ending in "/").
    Terms match case-insensitively on whole words; a space in a term also matches no space, "_" or "-".
    C++ raw string literals are not recognised as strings.

.PARAMETER Root
    Repository root to check. Defaults to the repository containing this script.

.EXAMPLE
    powershell -ExecutionPolicy Bypass -File scripts\check-clean-room.ps1
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

# First-column code spans of the table that follows the heading containing $Heading.
function Get-TermTable( [string[]]$Lines, [string]$Heading )
{
    $terms   = New-Object System.Collections.Generic.List[string]
    $inside  = $false
    $inTable = $false

    foreach( $line in $Lines )
    {
        if( $line -match '^#+\s' )
        {
            if( $inTable )
            {
                break
            }

            $inside = $line.Contains( $Heading )
            continue
        }

        if( -not $inside )
        {
            continue
        }

        if( $line -match '^\s*\|' )
        {
            $inTable = $true
            $cell    = ( $line.Trim().Trim( '|' ) -split '\|' )[0].Trim()

            if( $cell -match '^`([^`]+)`$' )
            {
                $terms.Add( $Matches[1] )
            }
        }
        elseif( $inTable )
        {
            break
        }
    }

    return ,$terms
}

function ConvertTo-TermPattern( [string]$Term )
{
    $words = @( $Term.Trim() -split '\s+' | ForEach-Object { [regex]::Escape( $_ ) } )
    return '(?<![A-Za-z0-9])' + ( $words -join '[\s_-]*' ) + '(?![A-Za-z0-9])'
}

$cleanRoomPath = Join-Path $Root 'docs\clean-room.md'

if( -not ( Test-Path $cleanRoomPath ) )
{
    Write-Host 'check-clean-room: FAILED: docs/clean-room.md is missing' -ForegroundColor Red
    exit 1
}

$docLines   = [IO.File]::ReadAllLines( $cleanRoomPath )
$trademarks = Get-TermTable $docLines 'Vendor trademark terms (rule 1)'
$extensions = Get-TermTable $docLines 'Native CAD file extensions (rule 2)'
$keywords   = Get-TermTable $docLines 'Telemetry and update-check keywords (rule 3)'

foreach( $table in @( @{ Name = 'Vendor trademark terms (rule 1)'; Terms = $trademarks },
                      @{ Name = 'Native CAD file extensions (rule 2)'; Terms = $extensions },
                      @{ Name = 'Telemetry and update-check keywords (rule 3)'; Terms = $keywords } ) )
{
    if( $table.Terms.Count -eq 0 )
    {
        $violations.Add( "docs/clean-room.md: table '$( $table.Name )' is missing or empty" )
    }
}

$trademarkRegex = @( $trademarks | ForEach-Object { [pscustomobject]@{ Term = $_; Regex = New-Object regex( ( ConvertTo-TermPattern $_ ), 'IgnoreCase' ) } } )
$keywordRegex   = @( $keywords | ForEach-Object { [pscustomobject]@{ Term = $_; Regex = New-Object regex( ( ConvertTo-TermPattern $_ ), 'IgnoreCase' ) } } )
$extensionRegex = @( $extensions | ForEach-Object {
                         $ext = $_.TrimStart( '.' )
                         [pscustomobject]@{ Term = ".$ext"; Regex = New-Object regex( ( '\.' + [regex]::Escape( $ext ) + '(?![A-Za-z0-9])' ), 'IgnoreCase' ) } } )

# String literals are kept, comments become blanks with the same line breaks, so line numbers stay valid.
$cppTokens = New-Object regex( '("(?:[^"\\\n]|\\.)*"|''(?:[^''\\\n]|\\.)*'')|//[^\n]*|/\*[\s\S]*?\*/' )
$stringLit = New-Object regex( '"(?:[^"\\\n]|\\.)*"' )

function Remove-CppComments( [string]$Text )
{
    return $cppTokens.Replace( $Text, {
        param( $m )

        if( $m.Groups[1].Success )
        {
            return $m.Value
        }

        return ( $m.Value -replace '[^\n]', ' ' )
    } )
}

function Test-Terms( [string]$Rel, [int]$LineNo, [string]$Text, [object[]]$Patterns, [string]$What )
{
    foreach( $pattern in $Patterns )
    {
        if( $pattern.Regex.IsMatch( $Text ) )
        {
            $violations.Add( "${Rel}:${LineNo}: $What '$( $pattern.Term )'" )
        }
    }
}

$srcRoot    = Join-Path $Root 'src'
$cppFiles   = @( Get-AppSourceFiles -Root $Root -Extensions $cppExt | Where-Object { ( Get-AppRelativePath $Root $_.FullName ) -match '^src/' } )
$uiFiles    = @( Get-AppSourceFiles -Root $Root -Extensions @( '.ui', '.ts', '.qml' ) )
$otherSrc   = @()
$dataFiles  = @()

if( Test-Path $srcRoot )
{
    $binaryExt = @( '.md', '.png', '.jpg', '.jpeg', '.gif', '.ico', '.bmp', '.svgz', '.qm', '.bin', '.apb', '.bson' )
    $otherSrc  = @( Get-ChildItem -Path $srcRoot -Recurse -File |
                    Where-Object { $cppExt -notcontains $_.Extension.ToLowerInvariant() -and $binaryExt -notcontains $_.Extension.ToLowerInvariant() } )
}

# Rules 1, 2 and 3 in C++ sources.
foreach( $file in $cppFiles )
{
    $rel    = Get-AppRelativePath $Root $file.FullName
    $code   = ( Remove-CppComments ( [IO.File]::ReadAllText( $file.FullName ) -replace "`r`n", "`n" ) ) -split "`n"
    $lineNo = 0

    foreach( $line in $code )
    {
        $lineNo++
        Test-Terms $rel $lineNo $line $keywordRegex 'telemetry or update-check keyword (CR-06)'

        if( $line -match '^\s*#\s*include\b' )
        {
            continue
        }

        foreach( $literal in $stringLit.Matches( $line ) )
        {
            Test-Terms $rel $lineNo $literal.Value $trademarkRegex 'vendor trademark term in a UI string (CR-02)'
            Test-Terms $rel $lineNo $literal.Value $extensionRegex 'native CAD file extension of another product (CR-03)'
        }
    }
}

# Rules 1 and 2 in Qt UI and translation files, rule 3 in other text files under src/.
foreach( $file in $uiFiles )
{
    $rel    = Get-AppRelativePath $Root $file.FullName
    $lineNo = 0

    foreach( $line in [IO.File]::ReadAllLines( $file.FullName ) )
    {
        $lineNo++
        Test-Terms $rel $lineNo $line $trademarkRegex 'vendor trademark term in a UI string (CR-02)'

        if( $file.Extension -eq '.ui' )
        {
            Test-Terms $rel $lineNo $line $extensionRegex 'native CAD file extension of another product (CR-03)'
        }
    }
}

foreach( $file in $otherSrc )
{
    $rel    = Get-AppRelativePath $Root $file.FullName
    $lineNo = 0

    foreach( $line in [IO.File]::ReadAllLines( $file.FullName ) )
    {
        $lineNo++
        Test-Terms $rel $lineNo $line $keywordRegex 'telemetry or update-check keyword (CR-06)'
    }
}

# Rules 1 and 4 under libraries/.
$librariesRoot = Join-Path $Root 'libraries'

if( Test-Path $librariesRoot )
{
    $dataFiles = @( Get-ChildItem -Path $librariesRoot -Recurse -File |
                    Where-Object { $_.Name -ne 'README.md' -and $_.Name -ne '.gitkeep' -and ( Get-AppRelativePath $Root $_.FullName ) -ne 'libraries/seed-manifest.json' } )
}

foreach( $file in ( $dataFiles | Where-Object { $_.Extension -eq '.json' } ) )
{
    $rel    = Get-AppRelativePath $Root $file.FullName
    $lineNo = 0

    foreach( $line in [IO.File]::ReadAllLines( $file.FullName ) )
    {
        $lineNo++
        Test-Terms $rel $lineNo $line $trademarkRegex 'vendor trademark term in library data (CR-02)'
    }
}

$sourcesPath = Join-Path $Root 'docs\data-sources.md'
$documented  = New-Object System.Collections.Generic.List[string]
$required    = @( 'Library kind', 'Table / item', 'Source (title, publisher, URL or citation)', 'Terms', 'Retrieved', 'Transformation' )

if( Test-Path $sourcesPath )
{
    $columns = $null

    foreach( $line in [IO.File]::ReadAllLines( $sourcesPath ) )
    {
        if( $line -notmatch '^\s*\|' )
        {
            $columns = $null
            continue
        }

        $cells = @( $line.Trim().Trim( '|' ) -split '\|' | ForEach-Object { $_.Trim() } )

        if( $null -eq $columns )
        {
            if( $cells -contains 'Table / item' )
            {
                $columns = $cells
            }

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

        if( @( $required | Where-Object { -not $row.ContainsKey( $_ ) -or $row[$_] -match '^(|-|\?|TBD|n/a)$' } ).Count -gt 0 )
        {
            continue
        }

        foreach( $entry in ( $row['Table / item'] -split '(?:<br\s*/?>|,|;)' ) )
        {
            $entry = ( $entry -replace '[`*]', '' -replace '\\', '/' ).Trim()

            if( $entry )
            {
                $documented.Add( $entry )
            }
        }
    }
}
elseif( $dataFiles.Count -gt 0 )
{
    $violations.Add( 'docs/data-sources.md: missing, but libraries/ contains data files (CR-05)' )
}

foreach( $file in $dataFiles )
{
    $rel   = Get-AppRelativePath $Root $file.FullName
    $found = @( $documented | Where-Object {
                    $_.Equals( $rel, [StringComparison]::OrdinalIgnoreCase ) -or
                    ( $_.EndsWith( '/' ) -and $rel.StartsWith( $_, [StringComparison]::OrdinalIgnoreCase ) ) } )

    if( $found.Count -eq 0 )
    {
        $violations.Add( "${rel}: data file has no complete row in docs/data-sources.md naming it in 'Table / item' (CR-05)" )
    }
}

if( $violations.Count -gt 0 )
{
    Write-Host "check-clean-room: FAILED with $( $violations.Count ) violation(s):" -ForegroundColor Red
    $violations | ForEach-Object { Write-Host "  $_" }
    exit 1
}

$format = 'check-clean-room: OK ({0} C++ files, {1} UI and translation files, {2} other src files, {3} library data files; ' +
          '{4} trademark terms, {5} extensions, {6} telemetry keywords)'
Write-Host ( $format -f $cppFiles.Count, $uiFiles.Count, $otherSrc.Count, $dataFiles.Count, $trademarks.Count, $extensions.Count,
             $keywords.Count ) -ForegroundColor Green
exit 0
