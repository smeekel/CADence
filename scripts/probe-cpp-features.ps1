# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 The APP Authors
<#
.SYNOPSIS
    Compile the C++ feature probe on MSVC, GCC and Clang and print the feature matrix (spike SP-1b, L-01 ... L-05).

.DESCRIPTION
    Builds tests/core/spikes/cpp_feature_probe.cpp once per feature (-DAPP_PROBE_<NAME>), links it and runs it:
      MSVC   cl /std:c++latest /permissive- /Zc:__cplusplus /Zc:preprocessor /utf-8 /EHsc /W4 /WX (Visual Studio 2026)
      GCC    g++-14 (or g++ when it is version 14 or later) -std=c++2c -Wall -Wextra -Werror, in WSL
      Clang  clang++-18 (or clang++ when it is version 18 or later) -std=c++2c -stdlib=libc++, in WSL
    A compiler that is not installed is reported as "not available" for every row. Results: "yes" (builds and the
    program returns 0), "runtime-fail" (builds, wrong result), "no" (does not compile or link) with the first
    diagnostic. Writes a Markdown table to -OutFile when given. Exits 0 unless MSVC itself cannot be found.

.PARAMETER OutFile
    Optional Markdown file for the result table.

.EXAMPLE
    powershell -ExecutionPolicy Bypass -File scripts\probe-cpp-features.ps1 -OutFile out\logs\cpp-feature-probe.md
#>
[CmdletBinding()]
param(
    [string]$OutFile
)

. "$PSScriptRoot\common.ps1"

$root    = Get-AppRepoRoot
$probe   = Join-Path $root 'tests\core\spikes\cpp_feature_probe.cpp'
$workDir = Join-Path $root 'out\probe-cpp-features'

# Feature name, docs/cpp-features.md section, probe macro suffix.
$features = @(
    @( 'Concepts and constrained templates', 'Allowed', 'CONCEPTS' ),
    @( 'Ranges (C++20)', 'Allowed', 'RANGES' ),
    @( 'std::format', 'Allowed', 'FORMAT' ),
    @( 'std::expected', 'Allowed', 'EXPECTED' ),
    @( 'std::print / std::println', 'Allowed', 'PRINT' ),
    @( 'Explicit object parameter (deducing this)', 'Allowed', 'DEDUCING_THIS' ),
    @( 'std::ranges::to', 'Allowed', 'RANGES_TO' ),
    @( 'std::to_chars / from_chars for floating point', 'Allowed', 'TO_CHARS_FLOAT' ),
    @( 'std::span, std::byte, std::bit_cast', 'Allowed', 'SPAN_BYTE_BITCAST' ),
    @( 'std::jthread, std::stop_token', 'Allowed', 'JTHREAD' ),
    @( 'std::source_location', 'Allowed', 'SOURCE_LOCATION' ),
    @( '[[nodiscard]], [[maybe_unused]], [[likely]]', 'Allowed', 'ATTRIBUTES' ),
    @( 'std::views::zip, zip_transform, enumerate', 'Conditional', 'VIEWS_ZIP_ENUMERATE' ),
    @( 'std::mdspan', 'Conditional', 'MDSPAN' ),
    @( 'std::flat_map / std::flat_set', 'Conditional', 'FLAT_MAP' ),
    @( 'Named modules (import std;)', 'Not allowed', 'IMPORT_STD' ),
    @( 'std::generator', 'Not allowed', 'GENERATOR' ),
    @( 'std::stacktrace', 'Not allowed', 'STACKTRACE' ),
    @( 'std::move_only_function', 'Not allowed', 'MOVE_ONLY_FUNCTION' ),
    @( 'Pack indexing (Ts...[N])', 'Not allowed', 'PACK_INDEXING' ),
    @( '_ placeholder variables', 'Not allowed', 'PLACEHOLDER' ),
    @( '= delete("reason")', 'Not allowed', 'DELETE_REASON' ),
    @( 'Saturation arithmetic (std::add_sat)', 'Not allowed', 'SATURATION' ),
    @( 'std::inplace_vector', 'Not allowed', 'INPLACE_VECTOR' ),
    @( 'Static reflection', 'Not allowed', 'REFLECTION' ),
    @( 'Contracts (pre, post, contract_assert)', 'Not allowed', 'CONTRACTS' )
)

function Get-FirstDiagnostic( [string[]]$Lines )
{
    $line = $Lines | Where-Object { $_ -match '(error|fatal)' } | Select-Object -First 1

    if( -not $line )
    {
        $line = $Lines | Where-Object { $_ -match '\S' } | Select-Object -First 1
    }

    if( -not $line )
    {
        return ''
    }

    $line = ( $line -replace '^.*?(error|fatal error)', '$1' ).Trim()

    if( $line.Length -gt 110 )
    {
        $line = $line.Substring( 0, 110 ) + '...'
    }

    return $line
}

# MSVC environment from vcvars64.bat, imported once.
$vcvars = Join-Path ( Get-AppVsInstallPath ) 'VC\Auxiliary\Build\vcvars64.bat'

foreach( $line in & cmd.exe /c "`"$vcvars`" >nul 2>&1 && set" )
{
    if( $line -match '^([^=]+)=(.*)$' )
    {
        [Environment]::SetEnvironmentVariable( $Matches[1], $Matches[2], 'Process' )
    }
}

$cl = Get-Command cl.exe -ErrorAction SilentlyContinue

if( -not $cl )
{
    Write-Host 'probe-cpp-features: cl.exe not found after vcvars64.bat' -ForegroundColor Red
    exit 1
}

$msvcVersion = ( & cmd.exe /c 'cl.exe 2>&1' | Select-Object -First 1 ).Trim()

# WSL compilers.
function Invoke-Wsl( [string]$Command )
{
    # Native stderr (for example WSL's drive-translation notices) must not stop the script.
    $ErrorActionPreference = 'Continue'
    $output = & wsl.exe -e bash -c $Command 2>&1 | ForEach-Object { "$_" } |
              Where-Object { $_ -notmatch '^wsl: Failed to translate' }
    return [pscustomobject]@{ Code = $LASTEXITCODE; Output = @( $output ) }
}

function Find-WslCompiler( [string[]]$Candidates, [int]$MinMajor )
{
    foreach( $candidate in $Candidates )
    {
        $result = Invoke-Wsl "command -v $candidate >/dev/null 2>&1 && $candidate -dumpversion"

        if( $result.Code -eq 0 -and $result.Output.Count -gt 0 -and $result.Output[-1] -match '^(\d+)' -and [int]$Matches[1] -ge $MinMajor )
        {
            return [pscustomobject]@{ Command = $candidate; Version = $result.Output[-1].Trim() }
        }
    }

    return $null
}

$wslAvailable = $null -ne ( Get-Command wsl.exe -ErrorAction SilentlyContinue )
$gcc   = $null
$clang = $null

if( $wslAvailable )
{
    $gcc   = Find-WslCompiler @( 'g++-14', 'g++-15', 'g++' ) 14
    $clang = Find-WslCompiler @( 'clang++-18', 'clang++-19', 'clang++-20', 'clang++' ) 18
}

New-Item -ItemType Directory -Force $workDir | Out-Null
$wslProbe = ''
$wslWork  = ''

if( $gcc -or $clang )
{
    $wslProbe = ( Invoke-Wsl "wslpath -a '$( $probe -replace '\\', '/' )'" ).Output[-1].Trim()
    $wslWork  = '/tmp/app-probe-cpp-features'
    [void]( Invoke-Wsl "mkdir -p $wslWork" )
}

function Test-Msvc( [string]$Macro )
{
    $ErrorActionPreference = 'Continue'
    $exe = Join-Path $workDir "msvc_$Macro.exe"
    $obj = Join-Path $workDir "msvc_$Macro.obj"
    $out = & cl.exe /nologo /std:c++latest /permissive- /Zc:__cplusplus /Zc:preprocessor /utf-8 /EHsc /W4 /WX `
                    "/DAPP_PROBE_$Macro" $probe "/Fe:$exe" "/Fo:$obj" 2>&1 | ForEach-Object { "$_" }

    if( $LASTEXITCODE -ne 0 )
    {
        return "no: $( Get-FirstDiagnostic $out )"
    }

    & $exe | Out-Null

    if( $LASTEXITCODE -ne 0 )
    {
        return 'runtime-fail'
    }

    return 'yes'
}

function Test-Wsl( $Compiler, [string]$Flags, [string]$Macro, [string]$Tag )
{
    if( -not $Compiler )
    {
        return 'not available'
    }

    $exe    = "$wslWork/${Tag}_$Macro"
    $result = Invoke-Wsl "$( $Compiler.Command ) -std=c++2c $Flags -Wall -Wextra -Werror -DAPP_PROBE_$Macro '$wslProbe' -o '$exe'"

    if( $result.Code -ne 0 )
    {
        return "no: $( Get-FirstDiagnostic $result.Output )"
    }

    $run = Invoke-Wsl "'$exe' >/dev/null"

    if( $run.Code -ne 0 )
    {
        return 'runtime-fail'
    }

    return 'yes'
}

$gccLabel   = 'GCC: not available'
$clangLabel = 'Clang: not available'

if( $gcc )
{
    $gccLabel = "GCC $( $gcc.Version ) ($( $gcc.Command ))"
}

if( $clang )
{
    $clangLabel = "Clang $( $clang.Version ) ($( $clang.Command ), libc++)"
}

$rows = New-Object System.Collections.Generic.List[string]
$rows.Add( "| Feature | Section | $msvcVersion | $gccLabel | $clangLabel |" )
$rows.Add( '|---|---|---|---|---|' )

foreach( $feature in $features )
{
    $macro = $feature[2]
    $msvc  = Test-Msvc $macro
    $g     = Test-Wsl $gcc '' $macro 'gcc'
    $c     = Test-Wsl $clang '-stdlib=libc++' $macro 'clang'
    $row   = "| $( $feature[0] ) | $( $feature[1] ) | $msvc | $g | $c |"
    $rows.Add( $row )
    Write-Host $row
}

if( $OutFile )
{
    if( -not [IO.Path]::IsPathRooted( $OutFile ) )
    {
        $OutFile = Join-Path $root $OutFile
    }

    [IO.File]::WriteAllText( $OutFile, ( ( $rows -join "`n" ) + "`n" ), ( New-Object System.Text.UTF8Encoding( $false ) ) )
    Write-Host "probe-cpp-features: table written to $( Get-AppRelativePath $root $OutFile )"
}

exit 0
