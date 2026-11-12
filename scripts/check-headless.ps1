# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 The APP Authors
<#
.SYNOPSIS
    Static check that the Headless configuration has no Qt and no Windows UI dependency (P-04, B-04, A-01, T-11).

.DESCRIPTION
    The Headless configuration builds every domain module, the CLI and the tests without Qt or any UI project
    (spec B-04; P-04 Accept). This check fails when:
      1. a project built in Headless (every project in APP.slnx without <Build Solution="Headless|*"
         Project="false" />) imports Qt.props, names a Qt library (Qt6Widgets, Qt6Gui, ...) as a link input, or
         references, directly or transitively, a project that is excluded from Headless or imports Qt.props;
      2. vcpkg.json lists Qt outside the "ui" feature (Headless installs only the default dependencies);
      3. a C++ file compiled by a Headless project includes a Qt header;
      4. a C++ file outside src/platform/ includes a Windows UI header (windows.h, winuser.h, commctrl.h, Direct2D,
         DirectWrite, Direct3D, DXGI, GDI+, ...) or calls a Windows UI API (CreateWindowEx, MessageBox,
         GetMessage, ...), or a project outside src/platform/ names a Windows UI library as a link input;
      5. with -BinaryDir (the Headless output, for example out\Headless): a binary is a Qt DLL or imports one.

.PARAMETER Root
    Repository root to check. Defaults to the repository containing this script.

.PARAMETER BinaryDir
    Optional Headless build output folders to scan, for example out\Headless.

.EXAMPLE
    powershell -ExecutionPolicy Bypass -File scripts\check-headless.ps1 -BinaryDir out\Headless
#>
[CmdletBinding()]
param(
    [string]$Root,

    [string[]]$BinaryDir = @()
)

. "$PSScriptRoot\common.ps1"

if( -not $Root )
{
    $Root = Get-AppRepoRoot
}

$Root       = ( Resolve-Path $Root ).Path
$violations = New-Object System.Collections.Generic.List[string]

$qtLibs       = '\bQt6\w*\.lib\b'
$qtHeader     = '^\s*#\s*include\s*[<"]((Qt[A-Za-z]*/[^>"]*)|(Q[A-Z][A-Za-z]*)|(Qt[A-Z][A-Za-z]*))[>"]'
$qtDll        = '^qt6\w*\.dll$'
$winUiHeader  = '^\s*#\s*include\s*[<"](windows\.h|winuser\.h|windowsx\.h|wingdi\.h|commctrl\.h|commdlg\.h|shellapi\.h|shlobj\.h|shlobj_core\.h|uxtheme\.h|dwmapi\.h|gdiplus\w*\.h|d2d1\w*\.h|dwrite\w*\.h|d3d\d*\w*\.h|dxgi\d*\w*\.h|wincodec\.h|richedit\.h)[>"]'
$winUiCall    = '\b(CreateWindowEx[AW]?|CreateWindow[AW]?|MessageBox[AW]?|MessageBoxEx[AW]?|GetMessage[AW]?|PeekMessage[AW]?|DispatchMessage[AW]?|TranslateMessage|RegisterClass(Ex)?[AW]?|DefWindowProc[AW]?|ShowWindow|UpdateWindow|BeginPaint|EndPaint|GetDC|ReleaseDC|D2D1CreateFactory|DWriteCreateFactory|D3D11CreateDevice\w*|D3D12CreateDevice|CreateDXGIFactory\w*)\s*\('
$winUiLibs    = '\b(user32|gdi32|comctl32|comdlg32|uxtheme|dwmapi|gdiplus|d2d1|dwrite|d3d9|d3d11|d3d12|dxgi|windowscodecs)\.lib\b'

# 1. Project graph from the solution.
$slnPath = Join-Path $Root 'APP.slnx'

if( -not ( Test-Path $slnPath ) )
{
    Write-Host 'check-headless: FAILED: APP.slnx not found' -ForegroundColor Red
    exit 1
}

[xml]$sln = Get-Content -Raw $slnPath
$solutionProjects = @{}

foreach( $node in $sln.SelectNodes( '//Project' ) )
{
    $path     = [IO.Path]::GetFullPath( ( Join-Path $Root $node.Path ) )
    $excluded = $false

    foreach( $build in $node.SelectNodes( 'Build' ) )
    {
        if( $build.Solution -like 'Headless*' -and $build.Project -eq 'false' )
        {
            $excluded = $true
        }
    }

    $solutionProjects[$path.ToLowerInvariant()] = [pscustomobject]@{ Path = $path; Headless = -not $excluded }
}

$projectInfo = @{}

function Get-ProjectInfo( [string]$Path )
{
    $key = $Path.ToLowerInvariant()

    if( $projectInfo.ContainsKey( $key ) )
    {
        return $projectInfo[$key]
    }

    $info = [pscustomobject]@{ Path = $Path; Exists = ( Test-Path $Path ); UsesQt = $false; References = @(); Items = @(); Text = '' }

    if( $info.Exists )
    {
        $info.Text = Get-Content -Raw $Path
        [xml]$xml  = $info.Text
        $ns        = New-Object System.Xml.XmlNamespaceManager( $xml.NameTable )
        $ns.AddNamespace( 'm', 'http://schemas.microsoft.com/developer/msbuild/2003' )
        $dir       = Split-Path -Parent $Path

        $info.UsesQt = @( $xml.SelectNodes( '//m:Import', $ns ) | Where-Object { $_.Project -match 'Qt\.props' } ).Count -gt 0
        $info.References = @( $xml.SelectNodes( '//m:ProjectReference', $ns ) |
                              ForEach-Object { [IO.Path]::GetFullPath( ( Join-Path $dir $_.Include ) ) } )
        $info.Items = @( $xml.SelectNodes( '//m:ClCompile[@Include] | //m:ClInclude[@Include]', $ns ) |
                         ForEach-Object { [IO.Path]::GetFullPath( ( Join-Path $dir $_.Include ) ) } )
    }

    $projectInfo[$key] = $info
    return $info
}

$headlessProjects = @( $solutionProjects.Values | Where-Object { $_.Headless } )
$headlessFiles    = New-Object System.Collections.Generic.HashSet[string]( [StringComparer]::OrdinalIgnoreCase )

foreach( $project in $headlessProjects )
{
    $rel  = Get-AppRelativePath $Root $project.Path
    $info = Get-ProjectInfo $project.Path

    if( -not $info.Exists )
    {
        $violations.Add( "${rel}: listed in APP.slnx but missing" )
        continue
    }

    if( $info.UsesQt )
    {
        $violations.Add( "${rel}: built in Headless but imports Qt.props (B-04)" )
    }

    if( $info.Text -match $qtLibs )
    {
        $violations.Add( "${rel}: built in Headless but links Qt library '$( $Matches[0] )' (P-04)" )
    }

    foreach( $item in $info.Items )
    {
        [void]$headlessFiles.Add( $item )
    }

    # Transitive project references.
    $seen  = New-Object System.Collections.Generic.HashSet[string]( [StringComparer]::OrdinalIgnoreCase )
    $stack = New-Object System.Collections.Generic.Stack[string]
    $info.References | ForEach-Object { $stack.Push( $_ ) }

    while( $stack.Count -gt 0 )
    {
        $current = $stack.Pop()

        if( -not $seen.Add( $current ) )
        {
            continue
        }

        $cRel  = Get-AppRelativePath $Root $current
        $cInfo = Get-ProjectInfo $current
        $entry = $solutionProjects[$current.ToLowerInvariant()]

        if( $null -ne $entry -and -not $entry.Headless )
        {
            $violations.Add( "${rel}: built in Headless but depends on '$cRel', which is excluded from Headless (UI project)" )
        }
        elseif( $cInfo.UsesQt )
        {
            $violations.Add( "${rel}: built in Headless but depends on '$cRel', which uses Qt" )
        }

        $cInfo.References | ForEach-Object { $stack.Push( $_ ) }
    }
}

# Windows UI link inputs outside src/platform/ (props apply to every project).
$buildFiles = @( Get-AppSourceFiles -Root $Root -Extensions @( '.vcxproj', '.props', '.targets' ) )

foreach( $file in $buildFiles )
{
    $rel = Get-AppRelativePath $Root $file.FullName

    if( $rel.StartsWith( 'src/platform/' ) -or $rel.StartsWith( 'tests/platform/' ) )
    {
        continue
    }

    $lineNo = 0

    foreach( $line in [IO.File]::ReadAllLines( $file.FullName ) )
    {
        $lineNo++

        if( $line -match $winUiLibs )
        {
            $violations.Add( "${rel}:${lineNo}: Windows UI library '$( $Matches[0] )' is allowed only in src/platform/ (P-04)" )
        }
    }
}

# 2. vcpkg.json: Qt only inside the "ui" feature.
$manifestPath = Join-Path $Root 'vcpkg.json'

if( Test-Path $manifestPath )
{
    $manifest = Get-Content -Raw $manifestPath | ConvertFrom-Json

    if( $manifest.PSObject.Properties.Name -contains 'dependencies' )
    {
        foreach( $dependency in $manifest.dependencies )
        {
            $name = $dependency

            if( $dependency -isnot [string] )
            {
                $name = $dependency.name
            }

            if( $name -match '^qt' )
            {
                $violations.Add( "vcpkg.json: '$name' is a default dependency; Qt belongs in the 'ui' feature so that Headless installs no Qt (B-04)" )
            }
        }
    }
}

# 3 and 4. C++ sources.
$cppFiles = @( Get-AppSourceFiles -Root $Root -Extensions @( '.h', '.hpp', '.cpp', '.inl', '.ipp', '.cxx', '.cc' ) |
               Where-Object { ( Get-AppRelativePath $Root $_.FullName ) -match '^(src|tests)/' } )

foreach( $file in $cppFiles )
{
    $rel        = Get-AppRelativePath $Root $file.FullName
    $inHeadless = $headlessFiles.Contains( $file.FullName )
    $inPlatform = $rel.StartsWith( 'src/platform/' ) -or $rel.StartsWith( 'tests/platform/' )
    $lineNo     = 0

    foreach( $line in [IO.File]::ReadAllLines( $file.FullName ) )
    {
        $lineNo++

        if( $inHeadless -and $line -match $qtHeader )
        {
            $violations.Add( "${rel}:${lineNo}: Qt header '$( $Matches[1] )' in a file built by the Headless configuration (P-04)" )
        }

        if( -not $inPlatform )
        {
            if( $line -match $winUiHeader )
            {
                $violations.Add( "${rel}:${lineNo}: Windows UI header '$( $Matches[1] )' is allowed only in src/platform/ (P-04)" )
            }

            if( $line -match $winUiCall )
            {
                $violations.Add( "${rel}:${lineNo}: Windows UI API '$( $Matches[1] )' is allowed only in src/platform/ (P-04)" )
            }
        }
    }
}

# 5. Built Headless binaries.
$BinaryDir   = @( $BinaryDir | ForEach-Object { $_ -split ',' } | Where-Object { $_ } )
$binaryCount = 0

if( $BinaryDir.Count -gt 0 )
{
    $dumpbin = Get-AppDumpbinPath
}

foreach( $dir in $BinaryDir )
{
    if( -not [IO.Path]::IsPathRooted( $dir ) )
    {
        $dir = Join-Path $Root $dir
    }

    if( -not ( Test-Path $dir ) )
    {
        $violations.Add( "BinaryDir '$dir' does not exist; build first" )
        continue
    }

    foreach( $binary in Get-ChildItem -Path $dir -Recurse -File -Include *.exe, *.dll )
    {
        $binaryCount++
        $bRel = Get-AppRelativePath $Root $binary.FullName

        if( $binary.Name -match $qtDll )
        {
            $violations.Add( "${bRel}: Qt DLL in the Headless output" )
            continue
        }

        $imports = @( & $dumpbin /nologo /dependents $binary.FullName | ForEach-Object { $_.Trim() } |
                      Where-Object { $_ -match $qtDll } | Sort-Object -Unique )

        foreach( $import in $imports )
        {
            $violations.Add( "${bRel}: imports Qt DLL '$import' in the Headless output" )
        }
    }
}

if( $violations.Count -gt 0 )
{
    Write-Host "check-headless: FAILED with $( $violations.Count ) violation(s):" -ForegroundColor Red
    $violations | ForEach-Object { Write-Host "  $_" }
    exit 1
}

$summary = "$( $headlessProjects.Count ) Headless projects, $( $cppFiles.Count ) C++ files"

if( $BinaryDir.Count -gt 0 )
{
    $summary += ", $binaryCount binaries"
}

Write-Host "check-headless: OK ($summary)" -ForegroundColor Green
exit 0
