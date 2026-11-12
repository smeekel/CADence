# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 The APP Authors
<#
.SYNOPSIS
    Static check that the build stays migration-ready (B-01, B-02, B-03, B-07, T-11).

.DESCRIPTION
    Fails when:
      1. a C++ source uses "#pragma comment(lib" (no build logic in source code, B-07);
      2. a .vcxproj sets a shared compiler or linker setting that belongs in build/props (B-03): warning level,
         warnings-as-errors, disabled warnings, external-header warnings, language standard, conformance mode,
         standard preprocessor, multi-processor build, SDL checks, platform toolset, character set, output or
         intermediate folder, or /W, /std:, /permissive, /utf-8 or /Zc: in AdditionalOptions;
      3. a source needs a precompiled header to compile (B-07): it includes "pch.h" or "stdafx.h", or a .vcxproj
         sets PrecompiledHeader to Use or Create or uses ForcedIncludeFiles. Sources include what they use; a
         precompiled header, if ever added, is an optional forced include configured once in build/props;
      4. any CMake file exists (CMakeLists.txt, *.cmake, CMakePresets.json, CMakeUserPresets.json), because the
         solution is the only build system for now (P-02);
      5. a module folder (src/<module>/ or src/ecad/<module>/) does not contain exactly one .vcxproj, or a project
         sits below module level (B-02);
      6. APP.slnx is missing, or a .vcxproj under src/ or tests/ is not listed in it, or it lists a project that
         does not exist (B-01).

.PARAMETER Root
    Repository root to check. Defaults to the repository containing this script.

.EXAMPLE
    powershell -ExecutionPolicy Bypass -File scripts\check-build-hygiene.ps1
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

$sharedElements = @( 'WarningLevel', 'TreatWarningAsError', 'TreatLinkerWarningAsErrors', 'TreatLibWarningAsErrors',
                     'DisableSpecificWarnings', 'ExternalWarningLevel', 'TreatAngleIncludeAsExternal',
                     'LanguageStandard', 'LanguageStandard_C', 'ConformanceMode', 'UseStandardPreprocessor',
                     'MultiProcessorCompilation', 'SDLCheck', 'PlatformToolset', 'CharacterSet', 'OutDir', 'IntDir' )
$sharedOptions  = '(^|\s)[/-](W[0-4X]|Wall|std:|permissive|utf-8|Zc:)'

# 1 and 3: C++ sources.
$cppFiles = @( Get-AppSourceFiles -Root $Root -Extensions @( '.h', '.hpp', '.cpp', '.cxx', '.cc', '.c', '.inl', '.ipp' ) )

foreach( $file in $cppFiles )
{
    $rel    = Get-AppRelativePath $Root $file.FullName
    $lineNo = 0

    foreach( $line in [IO.File]::ReadAllLines( $file.FullName ) )
    {
        $lineNo++

        if( $line -match '^\s*#\s*pragma\s+comment\s*\(\s*lib\b' )
        {
            $violations.Add( "${rel}:${lineNo}: '#pragma comment(lib' puts build logic in source code (B-07)" )
        }

        if( $line -match '^\s*#\s*include\s*[<"](pch|stdafx)\.h[>"]' )
        {
            $violations.Add( "${rel}:${lineNo}: includes the precompiled header '$( $Matches[1] ).h'; sources must compile without a PCH (B-07)" )
        }
    }
}

# 2 and 3: project files.
$projects = @( Get-AppSourceFiles -Root $Root -Extensions @( '.vcxproj' ) )

foreach( $project in $projects )
{
    $rel      = Get-AppRelativePath $Root $project.FullName
    [xml]$xml = Get-Content -Raw $project.FullName
    $ns       = New-Object System.Xml.XmlNamespaceManager( $xml.NameTable )
    $ns.AddNamespace( 'm', 'http://schemas.microsoft.com/developer/msbuild/2003' )

    foreach( $name in $sharedElements )
    {
        foreach( $node in $xml.SelectNodes( "//m:$name", $ns ) )
        {
            $violations.Add( "${rel}: sets <$name> ('$( $node.InnerText )'); shared settings belong in build/props (B-03)" )
        }
    }

    foreach( $node in $xml.SelectNodes( '//m:AdditionalOptions', $ns ) )
    {
        if( $node.InnerText -match $sharedOptions )
        {
            $violations.Add( "${rel}: AdditionalOptions '$( $node.InnerText.Trim() )' sets a shared compiler switch; it belongs in build/props (B-03)" )
        }
    }

    foreach( $node in $xml.SelectNodes( '//m:PrecompiledHeader', $ns ) )
    {
        if( $node.InnerText -match '^(Use|Create)$' )
        {
            $violations.Add( "${rel}: PrecompiledHeader '$( $node.InnerText )'; a PCH must stay optional and is configured only in build/props (B-07)" )
        }
    }

    foreach( $node in $xml.SelectNodes( '//m:ForcedIncludeFiles', $ns ) )
    {
        $violations.Add( "${rel}: ForcedIncludeFiles '$( $node.InnerText )'; sources must include what they use (B-07)" )
    }
}

# 4: CMake files anywhere outside build output and package trees.
$cmake = @( Get-AppSourceFiles -Root $Root -Extensions @( '.txt', '.cmake', '.json' ) |
            Where-Object { $_.Name -eq 'CMakeLists.txt' -or $_.Extension -eq '.cmake' -or $_.Name -match '^CMake(User)?Presets\.json$' } )

foreach( $file in $cmake )
{
    $violations.Add( "$( Get-AppRelativePath $Root $file.FullName ): CMake file; the Visual Studio solution is the only build system for now (P-02, rule 040)" )
}

# 5: one project per module folder.
$srcDir = Join-Path $Root 'src'
$moduleDirs = New-Object System.Collections.Generic.List[string]

if( Test-Path $srcDir )
{
    foreach( $dir in Get-ChildItem -Path $srcDir -Directory )
    {
        if( $dir.Name -eq 'ecad' )
        {
            Get-ChildItem -Path $dir.FullName -Directory | ForEach-Object { $moduleDirs.Add( $_.FullName ) }

            foreach( $stray in Get-ChildItem -Path $dir.FullName -File -Filter *.vcxproj )
            {
                $violations.Add( "$( Get-AppRelativePath $Root $stray.FullName ): project in the src/ecad/ grouping folder, not in a module folder (B-02)" )
            }
        }
        else
        {
            $moduleDirs.Add( $dir.FullName )
        }
    }
}

foreach( $dir in $moduleDirs )
{
    $rel   = Get-AppRelativePath $Root $dir
    $count = @( Get-ChildItem -Path $dir -File -Filter *.vcxproj ).Count

    if( $count -ne 1 )
    {
        $violations.Add( "${rel}/: module folder has $count projects; it must have exactly one (B-02)" )
    }

    foreach( $nested in Get-ChildItem -Path $dir -Directory -Recurse | ForEach-Object { Get-ChildItem -Path $_.FullName -File -Filter *.vcxproj } )
    {
        $violations.Add( "$( Get-AppRelativePath $Root $nested.FullName ): project below module level; one project per module folder (B-02)" )
    }
}

# 6: the solution lists exactly the projects under src/ and tests/.
$slnPath = Join-Path $Root 'APP.slnx'

if( -not ( Test-Path $slnPath ) )
{
    $violations.Add( 'APP.slnx: missing (B-01)' )
}
else
{
    [xml]$sln = Get-Content -Raw $slnPath
    $listed   = New-Object System.Collections.Generic.HashSet[string]( [StringComparer]::OrdinalIgnoreCase )

    foreach( $node in $sln.SelectNodes( '//Project' ) )
    {
        $path = $node.Path -replace '\\', '/'
        [void]$listed.Add( $path )

        if( -not ( Test-Path ( Join-Path $Root $path ) ) )
        {
            $violations.Add( "APP.slnx: lists '$path', which does not exist (B-01)" )
        }
    }

    foreach( $project in $projects )
    {
        $rel = Get-AppRelativePath $Root $project.FullName

        if( $rel -match '^(src|tests)/' -and -not $listed.Contains( $rel ) )
        {
            $violations.Add( "${rel}: not listed in APP.slnx (B-01)" )
        }
    }
}

if( $violations.Count -gt 0 )
{
    Write-Host "check-build-hygiene: FAILED with $( $violations.Count ) violation(s):" -ForegroundColor Red
    $violations | ForEach-Object { Write-Host "  $_" }
    exit 1
}

Write-Host "check-build-hygiene: OK ($( $projects.Count ) projects, $( $moduleDirs.Count ) module folders, $( $cppFiles.Count ) C++ files)" -ForegroundColor Green
exit 0
