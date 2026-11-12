# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 The APP Authors
<#
.SYNOPSIS
    Run MSVC Code Analysis and clang-tidy on the MSBuild projects (T-11).

.DESCRIPTION
    Builds APP.slnx (or one project) with /p:AppAnalysis=true, which build/props/CodeAnalysis.props turns into
    RunCodeAnalysis with build/props/CodeAnalysis.ruleset and the Visual Studio clang-tidy integration (VS-bundled
    LLVM, checks from .clang-tidy). The analysis build is a clean build into out/analyze/ and out/obj/analyze/.
    Every MSVC analysis warning (C6xxx, C26xxx) or other build warning (out/logs/analyze-<Configuration>.warnings.log)
    and every clang-tidy finding in APP code (the projects' *.ClangTidy.log files) is a finding; the script fails on
    any finding or build error and lists them, deduplicated. clang-tidy findings in code generated under out/ (Qt
    moc output) are listed but do not fail.

    -SelfTest copies the core module and the build settings to out/analyze-selftest/, seeds a function that
    dereferences a null pointer, and passes only if the analysis fails and both MSVC (C6011) and clang-tidy
    (clang-analyzer-core.NullDereference) report it.

.PARAMETER Configuration
    Debug (default; all projects, including the UI), Release or Headless.

.PARAMETER Root
    Repository root. Defaults to the repository containing this script.

.PARAMETER Project
    Repository-relative .vcxproj to analyse instead of the whole solution.

.PARAMETER SelfTest
    Prove that a seeded null dereference is reported by both analysers.

.EXAMPLE
    powershell -ExecutionPolicy Bypass -File scripts\analyze.ps1
    powershell -ExecutionPolicy Bypass -File scripts\analyze.ps1 -SelfTest
#>
[CmdletBinding()]
param(
    [ValidateSet( 'Debug', 'Release', 'Headless' )]
    [string]$Configuration = 'Debug',

    [string]$Root,

    [string]$Project,

    [switch]$SelfTest
)

. "$PSScriptRoot\common.ps1"

if( -not $Root )
{
    $Root = Get-AppRepoRoot
}

$Root = ( Resolve-Path $Root ).Path

if( $SelfTest )
{
    # Not under %TEMP%: MSBuild warns (MSB8029) when output folders are there.
    $fixture = Join-Path $Root 'out\analyze-selftest'

    if( Test-Path $fixture )
    {
        Remove-Item -Recurse -Force $fixture
    }

    New-Item -ItemType Directory -Force ( Join-Path $fixture 'src' ) | Out-Null

    foreach( $item in @( 'Directory.Build.props', 'Directory.Build.targets', 'vcpkg.json', 'vcpkg-configuration.json', '.clang-tidy', 'build' ) )
    {
        Copy-Item -Recurse -Force ( Join-Path $Root $item ) ( Join-Path $fixture $item )
    }

    Copy-Item -Recurse -Force ( Join-Path $Root 'src\core' ) ( Join-Path $fixture 'src\core' )

    $source = @(
        '// SPDX-License-Identifier: GPL-3.0-or-later'
        '// SPDX-FileCopyrightText: 2026 The APP Authors'
        'namespace app::core'
        '{'
        'int seededNullDereference( bool pick )'
        '{'
        '    int value = 1;'
        '    int* pointer = nullptr;'
        ''
        '    if( pick )'
        '    {'
        '        pointer = &value;'
        '    }'
        ''
        '    return *pointer;'
        '}'
        '} // namespace app::core'
    )
    [IO.File]::WriteAllText( ( Join-Path $fixture 'src\core\SeededNullDereference.cpp' ), ( ( $source -join "`r`n" ) + "`r`n" ), ( New-Object System.Text.UTF8Encoding( $false ) ) )

    $projectPath = Join-Path $fixture 'src\core\Core.vcxproj'
    $xml         = [IO.File]::ReadAllText( $projectPath )
    $marker      = '<ClCompile Include='
    $at          = $xml.IndexOf( $marker )

    if( $at -lt 0 )
    {
        Write-Host 'analyze: -SelfTest cannot find a ClCompile item in src/core/Core.vcxproj' -ForegroundColor Red
        exit 1
    }

    $xml = $xml.Insert( $at, '<ClCompile Include="SeededNullDereference.cpp" />' + "`r`n    " )
    [IO.File]::WriteAllText( $projectPath, $xml, ( New-Object System.Text.UTF8Encoding( $false ) ) )

    $output = & powershell.exe -NoProfile -ExecutionPolicy Bypass -File $PSCommandPath -Root $fixture -Project 'src\core\Core.vcxproj' -Configuration Headless 2>&1 |
              ForEach-Object { "$_" }
    $code   = $LASTEXITCODE
    $output | ForEach-Object { Write-Host "    $_" }

    $expected = @( 'C6011', 'clang-analyzer-core\.NullDereference' )
    $missing  = @( $expected | Where-Object { $pattern = $_; -not ( $output | Where-Object { $_ -match "SeededNullDereference\.cpp.*$pattern" } ) } )

    if( $code -eq 0 -or $missing.Count -gt 0 )
    {
        Write-Host "analyze: self-test FAILED (exit $code; not reported: $( $missing -join ', ' ))" -ForegroundColor Red
        exit 1
    }

    Remove-Item -Recurse -Force $fixture
    Write-Host 'analyze: self-test OK (the seeded null dereference is reported by MSVC C6011 and clang-tidy)' -ForegroundColor Green
    exit 0
}

$msbuild = Get-AppMSBuildPath
$logDir  = Join-Path $Root 'out\logs'
$warnLog = Join-Path $logDir "analyze-$Configuration.warnings.log"
$fullLog = Join-Path $logDir "analyze-$Configuration.log"
$target  = Join-Path $Root 'APP.slnx'

if( $Project )
{
    $target = Join-Path $Root $Project
}

New-Item -ItemType Directory -Force $logDir | Out-Null

# A clean analysis build: an incremental one would skip unchanged files and so hide their findings.
$analysisDirs = @( ( Join-Path $Root "out\analyze\$Configuration" ), ( Join-Path $Root "out\obj\analyze\$Configuration" ) )
$analysisDirs | Where-Object { Test-Path $_ } | ForEach-Object { Remove-Item -Recurse -Force $_ }

$arguments = @(
    $target,
    '/m',
    '/nologo',
    '/t:Build',
    "/p:Configuration=$Configuration",
    '/p:Platform=x64',
    '/p:AppAnalysis=true',
    '/v:minimal',
    "/flp1:logfile=$warnLog;warningsonly",
    "/flp2:logfile=$fullLog;verbosity=normal"
)

Write-Host "analyze: $( Get-AppRelativePath $Root $target ) Configuration=$Configuration (MSVC Code Analysis + clang-tidy)"

& $msbuild @arguments | Out-Null
$exitCode = $LASTEXITCODE
$findings = @()

if( Test-Path $warnLog )
{
    $findings = @( Get-Content $warnLog | Where-Object { $_ -match '\S' } |
                   ForEach-Object { ( $_ -replace '\s+\[[^\]]+\.vcxproj\]\s*$', '' -replace '^\s*\d+>', '' ).Trim() } |
                   Sort-Object -Unique )
}

$errors = @()

if( $exitCode -ne 0 )
{
    $errors = @( Get-Content $fullLog | Where-Object { $_ -match ':\s+(fatal )?error\s' } | ForEach-Object { ( $_ -replace '\s+\[[^\]]+\.vcxproj\]\s*$', '' ).Trim() } |
                 Sort-Object -Unique )
}

# clang-tidy writes its findings to <IntDir><Target>.ClangTidy.log, not to the MSBuild log. A header is reported by
# every project that includes it, so findings are deduplicated by normalised path. Code generated into out/ (Qt moc)
# is not APP code: its findings are counted, not failed.
$tidy      = New-Object System.Collections.Generic.SortedSet[string]
$generated = New-Object System.Collections.Generic.SortedSet[string]
$tidyLogs  = @()
$outPrefix = ( Join-Path $Root 'out' ) + '\'

if( Test-Path $analysisDirs[1] )
{
    $tidyLogs = @( Get-ChildItem -Path $analysisDirs[1] -Recurse -File -Filter '*.ClangTidy.log' )
}

foreach( $log in $tidyLogs )
{
    foreach( $line in [IO.File]::ReadAllLines( $log.FullName ) )
    {
        if( $line -notmatch '^(?<file>[A-Za-z]:[^:]+):(?<line>\d+):(?<col>\d+): (?<kind>warning|error): (?<text>.+)$' )
        {
            continue
        }

        $file    = [IO.Path]::GetFullPath( $Matches['file'] )
        $finding = "$( Get-AppRelativePath $Root $file ):$( $Matches['line'] ):$( $Matches['col'] ): clang-tidy $( $Matches['kind'] ): $( $Matches['text'] )"

        if( $file.StartsWith( $outPrefix, [StringComparison]::OrdinalIgnoreCase ) )
        {
            [void]$generated.Add( $finding )
        }
        else
        {
            [void]$tidy.Add( $finding )
        }
    }
}

$total = $findings.Count + $tidy.Count

if( $generated.Count -gt 0 )
{
    Write-Host "analyze: $( $generated.Count ) clang-tidy finding(s) in generated code under out/ (not APP code, not failed):"
    $generated | ForEach-Object { Write-Host "  $_" }
}

if( $exitCode -ne 0 -or $total -gt 0 )
{
    Write-Host "analyze: FAILED (MSBuild exit code $exitCode, $total finding(s), $( $errors.Count ) error(s)); full log: $fullLog" -ForegroundColor Red
    $findings | ForEach-Object { Write-Host "  $_" }
    $tidy     | ForEach-Object { Write-Host "  $_" }
    $errors   | ForEach-Object { Write-Host "  $_" }
    exit 1
}

Write-Host "analyze: OK ($Configuration, 0 findings; MSVC Code Analysis and clang-tidy on $( $tidyLogs.Count ) projects)" -ForegroundColor Green
exit 0
