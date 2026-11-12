# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 The APP Authors
<#
.SYNOPSIS
    Build APP.slnx with MSBuild from Visual Studio 2026 (B-06).

.DESCRIPTION
    Runs "msbuild APP.slnx /m /p:Configuration=<Configuration> /p:Platform=x64". vcpkg restores dependencies on the
    first build (the first Debug or Release build compiles Qt from source and takes a long time).
    Fails when the build fails or when MSBuild reports any warning; warnings are listed from
    out/logs/build-<Configuration>.warnings.log.

.PARAMETER Configuration
    Debug, Release or Headless (no Qt, no UI projects).

.PARAMETER Rebuild
    Clean and build instead of an incremental build.

.PARAMETER Verbosity
    MSBuild console verbosity (quiet, minimal, normal, detailed, diagnostic).

.EXAMPLE
    powershell -ExecutionPolicy Bypass -File scripts\build.ps1 -Configuration Headless
#>
[CmdletBinding()]
param(
    [ValidateSet( 'Debug', 'Release', 'Headless' )]
    [string]$Configuration = 'Debug',

    [switch]$Rebuild,

    [ValidateSet( 'quiet', 'minimal', 'normal', 'detailed', 'diagnostic' )]
    [string]$Verbosity = 'minimal'
)

. "$PSScriptRoot\common.ps1"

$root     = Get-AppRepoRoot
$msbuild  = Get-AppMSBuildPath
$logDir   = Join-Path $root 'out\logs'
$warnLog  = Join-Path $logDir "build-$Configuration.warnings.log"
$fullLog  = Join-Path $logDir "build-$Configuration.log"
$target   = 'Build'

if( $Rebuild )
{
    $target = 'Rebuild'
}

New-Item -ItemType Directory -Force $logDir | Out-Null

$arguments = @(
    ( Join-Path $root 'APP.slnx' ),
    '/m',
    '/nologo',
    "/t:$target",
    "/p:Configuration=$Configuration",
    '/p:Platform=x64',
    "/v:$Verbosity",
    "/flp1:logfile=$warnLog;warningsonly",
    "/flp2:logfile=$fullLog;verbosity=normal"
)

Write-Host "build: $msbuild"
Write-Host "build: Configuration=$Configuration Platform=x64 Target=$target"

& $msbuild @arguments
$exitCode = $LASTEXITCODE

$warnings = @()

if( Test-Path $warnLog )
{
    $warnings = @( Get-Content $warnLog | Where-Object { $_ -match '\S' } )
}

if( $exitCode -ne 0 )
{
    Write-Host "build: FAILED (MSBuild exit code $exitCode). Full log: $fullLog" -ForegroundColor Red
    exit 1
}

if( $warnings.Count -gt 0 )
{
    Write-Host "build: FAILED with $( $warnings.Count ) warning line(s); warnings are not allowed:" -ForegroundColor Red
    $warnings | ForEach-Object { Write-Host "  $_" }
    exit 1
}

Write-Host "build: OK ($Configuration, 0 warnings)" -ForegroundColor Green
exit 0
