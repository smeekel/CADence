# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 The APP Authors
<#
.SYNOPSIS
    Build and run the whole automated test suite with one command (T-13).

.DESCRIPTION
    1. Builds the chosen configuration with scripts/build.ps1 (Headless by default: no Qt, no UI projects).
    2. Runs every *.Tests.exe under out/<Configuration>/ with --gtest_output=xml (JUnit-style XML).
    3. Runs the CLI smoke test (Cli.exe --version must print the version and exit 0).
    4. Runs the static checks: check-layering, check-spdx, check-network-apis (with the built binaries) and
       check-headless (with the built binaries when the configuration is Headless).
    JUnit XML for all of them is written to out/test-results/<Configuration>/. Exits non-zero on any failure.

.PARAMETER Configuration
    Configuration to build and test. Headless (default) for automated runs; Debug also runs Ui.Tests.

.PARAMETER Filter
    GoogleTest filter passed as --gtest_filter, for example "CliApp.*".

.PARAMETER NoBuild
    Skip the build and test the existing binaries.

.PARAMETER NoChecks
    Skip the static checks.

.PARAMETER Bench
    Run benchmark executables (*.Bench.exe, T-08) instead of the tests.

.PARAMETER Fuzz
    Run fuzz executables (*.Fuzz.exe, T-09) instead of the tests.

.PARAMETER UpdateGolden
    Let golden-data tests rewrite their expected outputs (T-02, T-04). Needs -Reason. The tests see the
    environment variables APP_UPDATE_GOLDEN=1 and APP_UPDATE_GOLDEN_REASON.

.PARAMETER Reason
    Why the golden outputs change; put the same text in the commit message.

.PARAMETER Coverage
    Collect code coverage with the Visual Studio code coverage console (Cobertura XML in the results folder).

.EXAMPLE
    powershell -ExecutionPolicy Bypass -File scripts\test.ps1
    powershell -ExecutionPolicy Bypass -File scripts\test.ps1 -Configuration Debug
#>
[CmdletBinding()]
param(
    [ValidateSet( 'Debug', 'Release', 'Headless' )]
    [string]$Configuration = 'Headless',

    [string]$Filter,

    [switch]$NoBuild,

    [switch]$NoChecks,

    [switch]$Bench,

    [switch]$Fuzz,

    [switch]$UpdateGolden,

    [string]$Reason,

    [switch]$Coverage
)

. "$PSScriptRoot\common.ps1"

$root       = Get-AppRepoRoot
$binDir     = Join-Path $root "out\$Configuration"
$resultsDir = Join-Path $root "out\test-results\$Configuration"
$failures   = New-Object System.Collections.Generic.List[string]

if( $UpdateGolden )
{
    if( [string]::IsNullOrWhiteSpace( $Reason ) )
    {
        Write-Host 'test: -UpdateGolden needs -Reason "<why the golden outputs change>".' -ForegroundColor Red
        exit 1
    }

    $env:APP_UPDATE_GOLDEN        = '1'
    $env:APP_UPDATE_GOLDEN_REASON = $Reason
    Write-Host "test: golden outputs may be rewritten. Reason: $Reason" -ForegroundColor Yellow
}
else
{
    Remove-Item Env:\APP_UPDATE_GOLDEN -ErrorAction SilentlyContinue
    Remove-Item Env:\APP_UPDATE_GOLDEN_REASON -ErrorAction SilentlyContinue
}

$coverageTool = $null

if( $Coverage )
{
    $coverageTool = Join-Path ( Get-AppVsInstallPath ) 'Common7\IDE\Extensions\Microsoft\CodeCoverage.Console\Microsoft.CodeCoverage.Console.exe'

    if( -not ( Test-Path $coverageTool ) )
    {
        Write-Host "test: -Coverage needs the Visual Studio code coverage console ($coverageTool)." -ForegroundColor Red
        exit 1
    }

    $env:DOTNET_COVERAGE_TELEMETRY_OPTOUT = '1'
    $env:DOTNET_COVERAGE_NOLOGO           = '1'
}

if( -not $NoBuild )
{
    & "$PSScriptRoot\build.ps1" -Configuration $Configuration

    if( $LASTEXITCODE -ne 0 )
    {
        Write-Host 'test: build failed; no tests were run.' -ForegroundColor Red
        exit 1
    }
}

if( -not ( Test-Path $binDir ) )
{
    Write-Host "test: '$binDir' does not exist; build first." -ForegroundColor Red
    exit 1
}

$pattern = '*.Tests.exe'

if( $Bench )
{
    $pattern = '*.Bench.exe'
}
elseif( $Fuzz )
{
    $pattern = '*.Fuzz.exe'
}

$executables = @( Get-ChildItem -Path $binDir -Recurse -File -Filter $pattern | Sort-Object Name )

if( $executables.Count -eq 0 )
{
    Write-Host "test: no '$pattern' executables found under '$binDir'." -ForegroundColor Red
    exit 1
}

if( Test-Path $resultsDir )
{
    Remove-Item -Recurse -Force $resultsDir
}

New-Item -ItemType Directory -Force $resultsDir | Out-Null

foreach( $exe in $executables )
{
    $xml       = Join-Path $resultsDir ( $exe.BaseName + '.xml' )
    $arguments = @( "--gtest_output=xml:$xml" )

    if( $Filter )
    {
        $arguments += "--gtest_filter=$Filter"
    }

    Write-Host "test: running $( $exe.Name )"

    if( $coverageTool )
    {
        $report = Join-Path $resultsDir ( $exe.BaseName + '.coverage.cobertura.xml' )
        & $coverageTool collect --nologo --output $report --output-format cobertura $exe.FullName @arguments
    }
    else
    {
        & $exe.FullName @arguments
    }

    if( $LASTEXITCODE -ne 0 )
    {
        $failures.Add( "$( $exe.Name ) exited with code $LASTEXITCODE" )
    }
}

if( -not $Bench -and -not $Fuzz )
{
    # CLI smoke test: the headless runner starts, prints its version and exits 0.
    $cli   = Join-Path $binDir 'Cli\Cli.exe'
    $cases = @()

    if( Test-Path $cli )
    {
        $output = @( & $cli --version )
        $code   = $LASTEXITCODE
        $ok     = ( $code -eq 0 ) -and ( $output.Count -eq 1 ) -and ( $output[0] -match '^APP \d+\.\d+\.\d+$' )
        $cases += [pscustomobject]@{ Name = 'Cli.PrintsVersionAndExitsZero'; Passed = $ok; Message = "exit $code, output '$( $output -join ' | ' )'" }
        Write-Host "test: Cli.exe --version -> '$( $output -join ' | ' )' (exit $code)"
    }
    else
    {
        $cases += [pscustomobject]@{ Name = 'Cli.PrintsVersionAndExitsZero'; Passed = $false; Message = "$cli not found" }
    }

    Write-AppJUnit -Path ( Join-Path $resultsDir 'cli-smoke.xml' ) -SuiteName 'CliSmoke' -Cases $cases
    $cases | Where-Object { -not $_.Passed } | ForEach-Object { $failures.Add( "$( $_.Name ): $( $_.Message )" ) }
}

if( -not $NoChecks -and -not $Bench -and -not $Fuzz )
{
    $cases        = @()
    $headlessArgs = @{}

    if( $Configuration -eq 'Headless' )
    {
        $headlessArgs = @{ BinaryDir = $binDir }
    }

    $checks = @(
        @{ Name = 'check-layering';     Args = @{} },
        @{ Name = 'check-spdx';         Args = @{} },
        @{ Name = 'check-network-apis'; Args = @{ BinaryDir = $binDir } },
        @{ Name = 'check-headless';     Args = $headlessArgs }
    )

    foreach( $check in $checks )
    {
        $checkArgs = $check.Args
        & "$PSScriptRoot\$( $check.Name ).ps1" @checkArgs
        $code   = $LASTEXITCODE
        $cases += [pscustomobject]@{ Name = $check.Name; Passed = ( $code -eq 0 ); Message = "exit code $code" }
    }

    Write-AppJUnit -Path ( Join-Path $resultsDir 'static-checks.xml' ) -SuiteName 'StaticChecks' -Cases $cases
    $cases | Where-Object { -not $_.Passed } | ForEach-Object { $failures.Add( "$( $_.Name ) failed" ) }
}

Write-Host ''
Write-Host "test: $( $executables.Count ) test executable(s); results in $resultsDir"

if( $failures.Count -gt 0 )
{
    Write-Host "test: FAILED ($( $failures.Count )):" -ForegroundColor Red
    $failures | ForEach-Object { Write-Host "  $_" }
    exit 1
}

Write-Host 'test: OK' -ForegroundColor Green
exit 0
