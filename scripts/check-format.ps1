# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 The APP Authors
<#
.SYNOPSIS
    Formatting check: every C++ file in src/ and tests/ matches .clang-format (rule 010, T-11).

.DESCRIPTION
    Runs the clang-format that ships with Visual Studio (VC\Tools\Llvm\x64\bin) with --dry-run --Werror and the
    repository's .clang-format (passed explicitly, so a copy of the tree never falls back to another style).
    Fails and lists every place that would change. Fix with:
        clang-format -i --style=file <files>

.PARAMETER Root
    Repository root to check. Defaults to the repository containing this script.

.EXAMPLE
    powershell -ExecutionPolicy Bypass -File scripts\check-format.ps1
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

$Root        = ( Resolve-Path $Root ).Path
$styleFile   = Join-Path $Root '.clang-format'
$clangFormat = Join-Path ( Get-AppVsInstallPath ) 'VC\Tools\Llvm\x64\bin\clang-format.exe'

if( -not ( Test-Path $styleFile ) )
{
    Write-Host 'check-format: FAILED: .clang-format is missing' -ForegroundColor Red
    exit 1
}

if( -not ( Test-Path $clangFormat ) )
{
    Write-Host "check-format: FAILED: '$clangFormat' not found; install the Visual Studio component 'C++ Clang tools for Windows'" -ForegroundColor Red
    exit 1
}

$files = @( Get-AppSourceFiles -Root $Root -Extensions @( '.h', '.hpp', '.cpp', '.cxx', '.cc', '.c', '.inl', '.ipp' ) |
            Where-Object { ( Get-AppRelativePath $Root $_.FullName ) -match '^(src|tests)/' } )

if( $files.Count -eq 0 )
{
    Write-Host 'check-format: OK (no C++ files)' -ForegroundColor Green
    exit 0
}

$violations = New-Object System.Collections.Generic.SortedSet[string]
$failed     = $false

# Batches keep the command line below the Windows limit.
for( $i = 0; $i -lt $files.Count; $i += 50 )
{
    $batch = @( $files[$i..( [Math]::Min( $i + 49, $files.Count - 1 ) )] | ForEach-Object { $_.FullName } )

    $previous              = $ErrorActionPreference
    $ErrorActionPreference = 'Continue'
    $output                = @( & $clangFormat --dry-run --Werror "--style=file:$styleFile" $batch 2>&1 | ForEach-Object { "$_" } )
    $code                  = $LASTEXITCODE
    $ErrorActionPreference = $previous

    if( $code -ne 0 )
    {
        $failed = $true
    }

    foreach( $line in $output )
    {
        if( $line -match '^(?<file>[A-Za-z]:[^:]+):(?<line>\d+):(?<col>\d+): error: (?<text>.+)$' )
        {
            [void]$violations.Add( "$( Get-AppRelativePath $Root $Matches['file'] ):$( $Matches['line'] ): $( $Matches['text'] )" )
        }
        elseif( $line -match 'error' -and $line -notmatch '^\s' )
        {
            [void]$violations.Add( $line )
        }
    }
}

if( $failed -or $violations.Count -gt 0 )
{
    Write-Host "check-format: FAILED with $( $violations.Count ) difference(s) from .clang-format:" -ForegroundColor Red
    $violations | ForEach-Object { Write-Host "  $_" }
    exit 1
}

Write-Host "check-format: OK ($( $files.Count ) files, clang-format $( ( & $clangFormat --version ) -replace '^.*version ([\d.]+).*$', '$1' ))" -ForegroundColor Green
exit 0
