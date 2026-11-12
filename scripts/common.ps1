# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 The APP Authors
#
# Shared helpers for the scripts in this folder. Dot-source it:  . "$PSScriptRoot\common.ps1"
# Compatible with Windows PowerShell 5.1 and PowerShell 7 (P-02: a clean machine has only Visual Studio and Git).

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

# Visual Studio 2026 is major version 18. The range keeps VS 2022 (17.x) from being picked.
$script:AppVsVersionRange = '[18.0,19.0)'

function Get-AppRepoRoot
{
    return ( Resolve-Path ( Join-Path $PSScriptRoot '..' ) ).Path
}

function Get-AppVsInstallPath
{
    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'

    if( -not ( Test-Path $vswhere ) )
    {
        throw "vswhere.exe not found at '$vswhere'. Install Visual Studio 2026 with the 'Desktop development with C++' workload."
    }

    $path = & $vswhere -version $script:AppVsVersionRange -latest -products '*' `
                       -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath

    if( -not $path )
    {
        throw "Visual Studio 2026 (version $script:AppVsVersionRange) with the C++ desktop workload was not found."
    }

    return ( $path | Select-Object -First 1 )
}

function Get-AppMSBuildPath
{
    $msbuild = Join-Path ( Get-AppVsInstallPath ) 'MSBuild\Current\Bin\amd64\MSBuild.exe'

    if( -not ( Test-Path $msbuild ) )
    {
        throw "MSBuild.exe not found at '$msbuild'."
    }

    return $msbuild
}

function Get-AppDumpbinPath
{
    $toolsRoot = Join-Path ( Get-AppVsInstallPath ) 'VC\Tools\MSVC'
    $dumpbin   = Get-ChildItem -Path $toolsRoot -Directory | Sort-Object Name -Descending |
                 ForEach-Object { Join-Path $_.FullName 'bin\Hostx64\x64\dumpbin.exe' } |
                 Where-Object { Test-Path $_ } | Select-Object -First 1

    if( -not $dumpbin )
    {
        throw "dumpbin.exe not found under '$toolsRoot'."
    }

    return $dumpbin
}

# Repository-relative path with forward slashes, used in all script output.
function Get-AppRelativePath( [string]$Root, [string]$Path )
{
    $rootFull = [IO.Path]::GetFullPath( $Root ).TrimEnd( '\' ) + '\'
    $full     = [IO.Path]::GetFullPath( $Path )

    if( $full.StartsWith( $rootFull, [StringComparison]::OrdinalIgnoreCase ) )
    {
        $full = $full.Substring( $rootFull.Length )
    }

    return ( $full -replace '\\', '/' )
}

# Files below $Root, skipping build output, package trees and tool state.
function Get-AppSourceFiles( [string]$Root, [string[]]$Extensions )
{
    # out\ and vcpkg_installed\ are skipped only at the root: src\ecad\out\ is a module.
    $skipTop = @( 'out', 'vcpkg_installed', '.vs', '.git', 'node_modules' )
    $skipAny = '\\(\.vs|\.git|node_modules)\\'
    $entries = Get-ChildItem -Path $Root -Force | Where-Object { $skipTop -notcontains $_.Name }
    $files   = foreach( $entry in $entries )
    {
        if( $entry.PSIsContainer )
        {
            Get-ChildItem -Path $entry.FullName -Recurse -File -Force
        }
        else
        {
            $entry
        }
    }

    return $files | Where-Object { $Extensions -contains $_.Extension.ToLowerInvariant() } |
                    Where-Object { $_.FullName -notmatch $skipAny }
}

# Minimal JUnit XML writer for results that do not come from GoogleTest (static checks, smoke runs).
function Write-AppJUnit( [string]$Path, [string]$SuiteName, [object[]]$Cases )
{
    $failures = @( $Cases | Where-Object { -not $_.Passed } ).Count
    $lines    = New-Object System.Collections.Generic.List[string]

    $lines.Add( '<?xml version="1.0" encoding="UTF-8"?>' )
    $lines.Add( ( '<testsuites tests="{0}" failures="{1}">' -f $Cases.Count, $failures ) )
    $lines.Add( ( '  <testsuite name="{0}" tests="{1}" failures="{2}">' -f $SuiteName, $Cases.Count, $failures ) )

    foreach( $case in $Cases )
    {
        $name = [Security.SecurityElement]::Escape( $case.Name )

        if( $case.Passed )
        {
            $lines.Add( ( '    <testcase classname="{0}" name="{1}" />' -f $SuiteName, $name ) )
        }
        else
        {
            $message = [Security.SecurityElement]::Escape( $case.Message )
            $lines.Add( ( '    <testcase classname="{0}" name="{1}">' -f $SuiteName, $name ) )
            $lines.Add( ( '      <failure message="{0}" />' -f $message ) )
            $lines.Add( '    </testcase>' )
        }
    }

    $lines.Add( '  </testsuite>' )
    $lines.Add( '</testsuites>' )

    $dir = Split-Path -Parent $Path

    if( -not ( Test-Path $dir ) )
    {
        New-Item -ItemType Directory -Force $dir | Out-Null
    }

    [IO.File]::WriteAllLines( $Path, $lines, ( New-Object System.Text.UTF8Encoding( $false ) ) )
}
