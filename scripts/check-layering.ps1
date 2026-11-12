# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 The APP Authors
<#
.SYNOPSIS
    Static check for module layering (A-01, T-11).

.DESCRIPTION
    Enforces the allowed-dependency table of .cursor/rules/030-architecture-layering.mdc on:
      1. ProjectReference items in every .vcxproj under src/ and tests/;
      2. ClCompile, ClInclude and QtMoc items that point into another module's folder;
      3. #include paths in every C++ file under src/ and tests/ ("geom/Kernel.h" belongs to module geom);
      4. confinement of third-party headers to their adapter module (OCCT in geom, Clipper2 in ecad/pcb, Chrono and
         ngspice in sim, Embree in render, lib3mf in io, Qt in ui (non-Widgets Qt also in render and platform,
         rule 010), GoogleTest only in tests) and of Win32 headers to platform (P-04).
    tests/<m> may use <m> and everything <m> may use, transitively.
    The table below and the rule file change together, and only with an ADR.

.PARAMETER Root
    Repository root to check. Defaults to the repository containing this script.

.EXAMPLE
    powershell -ExecutionPolicy Bypass -File scripts\check-layering.ps1
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

$Root = ( Resolve-Path $Root ).Path

# Allowed application dependencies (rule 030). 'cli' = core + domain modules doc ... io (spec A-01).
$Allowed = @{
    'core'     = @()
    'platform' = @( 'core' )
    'doc'      = @( 'core' )
    'geom'     = @( 'core', 'doc' )
    'sketch'   = @( 'core', 'doc', 'geom' )
    'part'     = @( 'core', 'doc', 'geom', 'sketch' )
    'asm'      = @( 'core', 'doc', 'geom', 'sketch', 'part' )
    'drw'      = @( 'core', 'doc', 'geom', 'part', 'asm' )
    'ecad/lib' = @( 'core', 'doc' )
    'ecad/sch' = @( 'core', 'doc', 'ecad/lib' )
    'ecad/pcb' = @( 'core', 'doc', 'ecad/lib', 'ecad/sch' )
    'ecad/out' = @( 'core', 'doc', 'ecad/lib', 'ecad/pcb', 'drw' )
    'link'     = @( 'core', 'doc', 'geom', 'sketch', 'part', 'asm', 'ecad/pcb' )
    'sim'      = @( 'core', 'doc', 'geom', 'asm', 'ecad/sch' )
    'io'       = @( 'core', 'doc', 'geom', 'part', 'asm', 'ecad/pcb' )
    'render'   = @( 'core', 'doc', 'geom', 'platform' )
    'cli'      = @( 'core', 'doc', 'geom', 'sketch', 'part', 'asm', 'drw', 'ecad/lib', 'ecad/sch', 'ecad/pcb',
                    'ecad/out', 'link', 'sim', 'io' )
    'ui'       = @( 'core', 'platform', 'doc', 'geom', 'sketch', 'part', 'asm', 'drw', 'ecad/lib', 'ecad/sch',
                    'ecad/pcb', 'ecad/out', 'link', 'sim', 'io', 'render' )
}

# Third-party headers confined to modules. Pattern is matched against the include path; "modules" lists the
# src modules allowed to include it (tests/<m> inherits the modules allowed for <m> and its dependencies).
$ThirdParty = @(
    @{ Name = 'OCCT';            Pattern = '\.hxx$';                                 Modules = @( 'geom' ) },
    @{ Name = 'Clipper2';        Pattern = '^clipper2/';                             Modules = @( 'ecad/pcb' ) },
    @{ Name = 'Project Chrono';  Pattern = '^chrono/';                               Modules = @( 'sim' ) },
    @{ Name = 'ngspice';         Pattern = '^ngspice/';                              Modules = @( 'sim' ) },
    @{ Name = 'Embree';          Pattern = '^embree\d*/';                            Modules = @( 'render' ) },
    @{ Name = 'lib3mf';          Pattern = '^lib3mf';                                Modules = @( 'io' ) },
    @{ Name = 'Qt Widgets';      Pattern = '^(QtWidgets/|Q(Application|Widget|MainWindow|Dialog|DockWidget|Menu|MenuBar|ToolBar|PushButton|Label|LineEdit|LayoutItem|BoxLayout|VBoxLayout|HBoxLayout|GridLayout|Layout|AbstractButton|StatusBar|OpenGLWidget|FileDialog|MessageBox|TreeView|TableView|ListView|Style|ScrollArea|TabWidget|Splitter)$)'; Modules = @( 'ui' ) },
    @{ Name = 'Qt';              Pattern = '^(Qt[A-Za-z]*/|Q[A-Z][A-Za-z]*$|Qt[A-Z][A-Za-z]*$)';   Modules = @( 'ui', 'render', 'platform' ) },
    @{ Name = 'GoogleTest';      Pattern = '^(gtest|gmock)/';                        Modules = @() },
    @{ Name = 'Win32 (P-04)';    Pattern = '^(windows|winuser|winbase|windef|winnt|shlobj|shlwapi|shellapi|objbase|combaseapi|comdef|atlbase|wrl/.*|d3d\d*|dxgi\d*|d2d1|dwrite)\.h$'; Modules = @( 'platform' ) }
)

$violations = New-Object System.Collections.Generic.List[string]

function Get-Closure( [string]$Module )
{
    $seen  = New-Object System.Collections.Generic.HashSet[string]
    $stack = New-Object System.Collections.Generic.Stack[string]

    $stack.Push( $Module )

    while( $stack.Count -gt 0 )
    {
        $current = $stack.Pop()

        if( $seen.Add( $current ) )
        {
            foreach( $dependency in $Allowed[$current] )
            {
                $stack.Push( $dependency )
            }
        }
    }

    return @( $seen )
}

# Module that a repository-relative path (src/... or tests/...) belongs to, or $null.
function Get-ModuleOfPath( [string]$RelativePath )
{
    $parts = $RelativePath -split '/'

    if( $parts.Count -lt 3 -or ( $parts[0] -ne 'src' -and $parts[0] -ne 'tests' ) )
    {
        return $null
    }

    $candidate = $parts[1]

    if( $candidate -eq 'ecad' -and $parts.Count -ge 4 )
    {
        $candidate = 'ecad/' + $parts[2]
    }

    if( $Allowed.ContainsKey( $candidate ) )
    {
        return $candidate
    }

    return "?$candidate"
}

# Module named by an include path such as "ecad/pcb/Board.h", or $null when the path is not module-rooted.
function Get-ModuleOfInclude( [string]$IncludePath )
{
    $parts = $IncludePath -split '/'

    if( $parts.Count -lt 2 )
    {
        return $null
    }

    if( $parts[0] -eq 'ecad' -and $parts.Count -ge 3 )
    {
        $candidate = 'ecad/' + $parts[1]

        if( $Allowed.ContainsKey( $candidate ) )
        {
            return $candidate
        }

        return "?$candidate"
    }

    if( $Allowed.ContainsKey( $parts[0] ) )
    {
        return $parts[0]
    }

    return $null
}

# Modules a file or project in $Owner may use: itself plus its table entry; tests get the transitive closure.
function Get-Permitted( [string]$Owner, [bool]$IsTest )
{
    if( $IsTest )
    {
        return Get-Closure $Owner
    }

    return @( $Owner ) + $Allowed[$Owner]
}

# 1 and 2: project files.
$projects = @( Get-AppSourceFiles -Root $Root -Extensions @( '.vcxproj' ) |
               Where-Object { ( Get-AppRelativePath $Root $_.FullName ) -match '^(src|tests)/' } )

foreach( $project in $projects )
{
    $rel    = Get-AppRelativePath $Root $project.FullName
    $owner  = Get-ModuleOfPath $rel
    $isTest = $rel.StartsWith( 'tests/' )

    if( $null -eq $owner )
    {
        continue
    }

    if( $owner.StartsWith( '?' ) )
    {
        $violations.Add( "${rel}: project is in folder '$($owner.Substring( 1 ))', which is not a module in the layering table" )
        continue
    }

    $permitted = Get-Permitted $owner $isTest
    [xml]$xml  = Get-Content -Raw $project.FullName
    $ns        = New-Object System.Xml.XmlNamespaceManager( $xml.NameTable )
    $ns.AddNamespace( 'm', 'http://schemas.microsoft.com/developer/msbuild/2003' )

    foreach( $node in $xml.SelectNodes( '//m:ProjectReference', $ns ) )
    {
        $target = [IO.Path]::GetFullPath( ( Join-Path $project.DirectoryName $node.Include ) )
        $tRel   = $target

        if( $target.StartsWith( $Root, [StringComparison]::OrdinalIgnoreCase ) )
        {
            $tRel = ( $target.Substring( $Root.Length ).TrimStart( '\' ) ) -replace '\\', '/'
        }

        $tModule = Get-ModuleOfPath $tRel

        if( $null -eq $tModule -or $tModule.StartsWith( '?' ) -or -not $tRel.StartsWith( 'src/' ) )
        {
            $violations.Add( "${rel}: ProjectReference '$($node.Include)' is not a module project under src/" )
        }
        else
        {
            $references = $Allowed[$owner]

            if( $isTest )
            {
                $references = $permitted
            }

            if( $references -notcontains $tModule )
            {
                $violations.Add( "${rel}: '$owner' must not reference module '$tModule' (allowed: $( $references -join ', ' ))" )
            }
        }
    }

    foreach( $node in $xml.SelectNodes( '//m:ClCompile[@Include] | //m:ClInclude[@Include] | //m:QtMoc[@Include]', $ns ) )
    {
        $item = [IO.Path]::GetFullPath( ( Join-Path $project.DirectoryName $node.Include ) )

        if( -not $item.StartsWith( $Root, [StringComparison]::OrdinalIgnoreCase ) )
        {
            continue
        }

        $iRel    = ( $item.Substring( $Root.Length ).TrimStart( '\' ) ) -replace '\\', '/'
        $iModule = Get-ModuleOfPath $iRel

        if( $null -eq $iModule -or -not $iRel.StartsWith( 'src/' ) )
        {
            continue
        }

        $ok = $permitted -contains $iModule

        if( -not $isTest )
        {
            $ok = ( $iModule -eq $owner )
        }

        if( -not $ok )
        {
            $violations.Add( "${rel}: compiles '$iRel' from module '$iModule', which '$owner' may not use" )
        }
    }
}

# 3 and 4: include paths.
$cppFiles = @( Get-AppSourceFiles -Root $Root -Extensions @( '.h', '.hpp', '.cpp', '.inl', '.ipp', '.cxx', '.cc' ) |
               Where-Object { ( Get-AppRelativePath $Root $_.FullName ) -match '^(src|tests)/' } )

$includeRegex = '^\s*#\s*include\s*([<"])([^>"]+)[>"]'

foreach( $file in $cppFiles )
{
    $rel    = Get-AppRelativePath $Root $file.FullName
    $owner  = Get-ModuleOfPath $rel
    $isTest = $rel.StartsWith( 'tests/' )

    if( $null -eq $owner )
    {
        continue
    }

    if( $owner.StartsWith( '?' ) )
    {
        $violations.Add( "${rel}: file is in folder '$($owner.Substring( 1 ))', which is not a module in the layering table" )
        continue
    }

    $permitted = Get-Permitted $owner $isTest
    $lineNo    = 0

    foreach( $line in [IO.File]::ReadAllLines( $file.FullName ) )
    {
        $lineNo++

        if( $line -notmatch $includeRegex )
        {
            continue
        }

        $path    = $Matches[2].Trim() -replace '\\', '/'
        $iModule = Get-ModuleOfInclude $path

        if( $null -ne $iModule )
        {
            if( $iModule.StartsWith( '?' ) )
            {
                $violations.Add( "${rel}:${lineNo}: include '$path' names '$($iModule.Substring( 1 ))', which is not a module" )
            }
            elseif( $permitted -notcontains $iModule )
            {
                $violations.Add( "${rel}:${lineNo}: module '$owner' must not include '$path' from module '$iModule'" )
            }

            continue
        }

        foreach( $lib in $ThirdParty )
        {
            if( $path -notmatch $lib.Pattern )
            {
                continue
            }

            # Third-party confinement follows the module a test belongs to, not the modules it may call.
            $ok = $lib.Modules -contains $owner

            if( $isTest -and $lib.Name -eq 'GoogleTest' )
            {
                $ok = $true
            }

            if( -not $ok )
            {
                $where = 'test projects only'

                if( $lib.Modules.Count -gt 0 )
                {
                    $where = $lib.Modules -join ', '
                }

                $violations.Add( "${rel}:${lineNo}: $($lib.Name) header '$path' is allowed only in: $where" )
            }

            break
        }
    }
}

if( $violations.Count -gt 0 )
{
    Write-Host "check-layering: FAILED with $( $violations.Count ) violation(s):" -ForegroundColor Red
    $violations | ForEach-Object { Write-Host "  $_" }
    exit 1
}

Write-Host "check-layering: OK ($( $projects.Count ) projects, $( $cppFiles.Count ) C++ files)" -ForegroundColor Green
exit 0
