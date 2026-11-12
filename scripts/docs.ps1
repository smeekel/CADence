# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 The APP Authors
<#
.SYNOPSIS
    Generate the HTML manual (and optionally the PDF) with Doxygen, offline (rule 020).

.DESCRIPTION
    Creates out/docs (Doxygen does not create nested output folders), runs Doxygen on the repository Doxyfile from
    the repository root and fails when Doxygen fails or writes any warning to out/docs/doxygen-warnings.log.
    Output: out/docs/html/index.html. The HTML build needs no LaTeX. With -Pdf, Doxygen also writes out/docs/latex
    (GENERATE_LATEX = YES is passed on top of the Doxyfile) and make.bat there builds refman.pdf; this needs a LaTeX
    distribution (for example MiKTeX) with pdflatex and epstopdf on PATH.

.PARAMETER Pdf
    Also build out/docs/latex/refman.pdf.

.PARAMETER Doxygen
    Path to doxygen.exe. Defaults to doxygen on PATH, then %ProgramFiles%\doxygen\bin\doxygen.exe.

.EXAMPLE
    powershell -ExecutionPolicy Bypass -File scripts\docs.ps1
    powershell -ExecutionPolicy Bypass -File scripts\docs.ps1 -Pdf
#>
[CmdletBinding()]
param(
    [switch]$Pdf,

    [string]$Doxygen
)

. "$PSScriptRoot\common.ps1"

$root    = Get-AppRepoRoot
$outDir  = Join-Path $root 'out\docs'
$warnLog = Join-Path $outDir 'doxygen-warnings.log'

if( -not $Doxygen )
{
    $command = Get-Command doxygen -ErrorAction SilentlyContinue

    if( $command )
    {
        $Doxygen = $command.Path
    }
    elseif( Test-Path ( Join-Path $env:ProgramFiles 'doxygen\bin\doxygen.exe' ) )
    {
        $Doxygen = Join-Path $env:ProgramFiles 'doxygen\bin\doxygen.exe'
    }
}

if( -not $Doxygen -or -not ( Test-Path $Doxygen ) )
{
    Write-Host 'docs: doxygen.exe not found. Install Doxygen (https://www.doxygen.nl, or "winget install DimitriVanHeesch.Doxygen") or pass -Doxygen <path>.' -ForegroundColor Red
    exit 1
}

if( $Pdf )
{
    foreach( $tool in @( 'pdflatex', 'epstopdf' ) )
    {
        if( -not ( Get-Command $tool -ErrorAction SilentlyContinue ) )
        {
            Write-Host "docs: $tool not found. Install a LaTeX distribution such as MiKTeX to build the PDF." -ForegroundColor Red
            exit 1
        }
    }
}

New-Item -ItemType Directory -Force $outDir | Out-Null

if( Test-Path $warnLog )
{
    Remove-Item -Force $warnLog
}

Write-Host "docs: $Doxygen"
Push-Location $root

try
{
    if( $Pdf )
    {
        # "doxygen -" reads the configuration from standard input; a later assignment overrides the Doxyfile value.
        ( @( Get-Content 'Doxyfile' ) + 'GENERATE_LATEX = YES' ) | & $Doxygen '-'
    }
    else
    {
        & $Doxygen 'Doxyfile'
    }

    $exitCode = $LASTEXITCODE
}
finally
{
    Pop-Location
}

if( $exitCode -ne 0 )
{
    Write-Host "docs: FAILED (doxygen exit code $exitCode)" -ForegroundColor Red
    exit 1
}

$warnings = @()

if( Test-Path $warnLog )
{
    $warnings = @( Get-Content $warnLog | Where-Object { $_ -match '\S' } )
}

if( $warnings.Count -gt 0 )
{
    Write-Host "docs: FAILED with $( $warnings.Count ) Doxygen warning line(s):" -ForegroundColor Red
    $warnings | ForEach-Object { Write-Host "  $_" }
    exit 1
}

Write-Host "docs: HTML OK -> $( Join-Path $outDir 'html\index.html' )" -ForegroundColor Green

if( $Pdf )
{
    $latexDir = Join-Path $outDir 'latex'
    $makeBat  = Join-Path $latexDir 'make.bat'

    if( -not ( Test-Path $makeBat ) )
    {
        Write-Host "docs: '$makeBat' not found; Doxygen did not write the LaTeX output." -ForegroundColor Red
        exit 1
    }

    Push-Location $latexDir

    try
    {
        & cmd.exe /c make.bat
        $exitCode = $LASTEXITCODE
    }
    finally
    {
        Pop-Location
    }

    $pdfPath = Join-Path $latexDir 'refman.pdf'

    if( $exitCode -ne 0 -or -not ( Test-Path $pdfPath ) )
    {
        Write-Host "docs: PDF build FAILED (make.bat exit code $exitCode)" -ForegroundColor Red
        exit 1
    }

    Write-Host "docs: PDF OK -> $pdfPath" -ForegroundColor Green
}

exit 0
