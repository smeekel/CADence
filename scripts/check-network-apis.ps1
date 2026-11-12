# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 The APP Authors
<#
.SYNOPSIS
    Static check that APP itself uses and links no networking API (P-03, CR-06, T-11; ADR 0026).

.DESCRIPTION
    APP needs no network support. Fails when any of the following is found outside the allow-list (empty by
    default, P-03):
      1. src/: network headers (Winsock, WinHTTP, WinINet, URLMon, BSD sockets, Qt Network, curl, Asio, ...) or
         network API calls (WSAStartup, getaddrinfo, InternetOpen, WinHttpOpen, URLDownloadToFile, ...);
      2. .vcxproj/.props/.targets and #pragma comment(lib): network link libraries (ws2_32, winhttp, wininet,
         urlmon, Qt6Network, libcurl, ...);
      3. vcpkg.json: network ports (curl, cpr, asio, grpc, ...) or network features of qtbase (network, dnslookup,
         openssl, brotli, windeployqt), because APP would be asking for network support;
      4. with -BinaryDir: executables built by this repository that import a network DLL directly.
    A third-party DLL that imports a network DLL (for example Qt6Core importing ws2_32.dll), or that is a network
    library itself, is allowed and is not a failure. Every such import is listed in a summary (library, network
    DLL and imported APIs) that is printed on every run and, with -SummaryPath, written as Markdown.

.PARAMETER Root
    Repository root to check. Defaults to the repository containing this script.

.PARAMETER AllowList
    Repository-relative files that may use network APIs. Empty by default (P-03); adding an entry needs an ADR.

.PARAMETER BinaryDir
    Optional build output folders to scan, for example out\Debug.

.PARAMETER SummaryPath
    Optional Markdown file for the allowed third-party import summary, for example docs\network-imports.md.

.EXAMPLE
    powershell -ExecutionPolicy Bypass -File scripts\check-network-apis.ps1 -BinaryDir out\Debug
    powershell -ExecutionPolicy Bypass -File scripts\check-network-apis.ps1 -BinaryDir out\Debug,out\Headless -SummaryPath docs\network-imports.md
#>
[CmdletBinding()]
param(
    [string]$Root,

    [string[]]$AllowList = @(),

    [string[]]$BinaryDir = @(),

    [string]$SummaryPath
)

. "$PSScriptRoot\common.ps1"

if( -not $Root )
{
    $Root = Get-AppRepoRoot
}

$Root       = ( Resolve-Path $Root ).Path
$violations = New-Object System.Collections.Generic.List[string]
$allowed    = @{}

$networkHeaders = '^\s*#\s*include\s*[<"](winsock2?\.h|ws2tcpip\.h|ws2def\.h|ws2ipdef\.h|mswsock\.h|winhttp\.h|wininet\.h|urlmon\.h|iphlpapi\.h|windns\.h|netioapi\.h|wsdapi\.h|sys/socket\.h|netinet/[^>"]+|arpa/inet\.h|netdb\.h|curl/[^>"]+|cpr/[^>"]+|httplib\.h|asio\.hpp|asio/[^>"]+|boost/asio[^>"]*|boost/beast[^>"]*|QtNetwork[^>"]*|QNetwork\w*|QTcpSocket|QTcpServer|QUdpSocket|QSslSocket|QHostInfo|QHostAddress|QDnsLookup|QtWebSockets[^>"]*|QWebSocket\w*|grpc[^>"]*|zmq\.h|libwebsockets\.h)[>"]'
$networkCalls   = '\b(WSAStartup|WSASocket\w*|WSAConnect|getaddrinfo|GetAddrInfo\w*|gethostbyname|gethostbyaddr|InternetOpen\w*|InternetConnect\w*|InternetReadFile|HttpOpenRequest\w*|HttpSendRequest\w*|WinHttp[A-Z]\w*|URLDownloadTo\w+|URLOpen\w*Stream\w*|DnsQuery\w*|curl_easy_\w+|curl_multi_\w+)\s*\('
$networkLibs    = '\b(ws2_32|wsock32|mswsock|winhttp|wininet|urlmon|iphlpapi|dnsapi|Qt6Network\w*|Qt5Network\w*|libcurl\w*|cpr)\.lib\b'
$networkDll     = '^(ws2_32|wsock32|mswsock|winhttp|wininet|urlmon|iphlpapi|dnsapi|libcurl[\w-]*|qt[56]network\w*)\.dll$'
$networkPorts   = @( 'curl', 'cpr', 'cpp-httplib', 'asio', 'boost-asio', 'boost-beast', 'grpc', 'zeromq', 'cppzmq', 'libwebsockets', 'websocketpp', 'qtwebsockets', 'qtwebengine', 'qthttpserver', 'qtnetworkauth', 'restinio', 'drogon' )
$qtbaseNetwork  = @( 'network', 'dnslookup', 'openssl', 'brotli', 'windeployqt', 'securetransport' )

# WinSock 1.1 exports have fixed ordinals; DLLs such as Qt6Core import them by ordinal only.
$winsockOrdinals = @{
    1 = 'accept'; 2 = 'bind'; 3 = 'closesocket'; 4 = 'connect'; 5 = 'getpeername'; 6 = 'getsockname'; 7 = 'getsockopt'
    8 = 'htonl'; 9 = 'htons'; 10 = 'ioctlsocket'; 11 = 'inet_addr'; 12 = 'inet_ntoa'; 13 = 'listen'; 14 = 'ntohl'
    15 = 'ntohs'; 16 = 'recv'; 17 = 'recvfrom'; 18 = 'select'; 19 = 'send'; 20 = 'sendto'; 21 = 'setsockopt'
    22 = 'shutdown'; 23 = 'socket'; 51 = 'gethostbyaddr'; 52 = 'gethostbyname'; 53 = 'getprotobyname'
    54 = 'getprotobynumber'; 55 = 'getservbyname'; 56 = 'getservbyport'; 57 = 'gethostname'; 101 = 'WSAAsyncSelect'
    102 = 'WSAAsyncGetHostByAddr'; 103 = 'WSAAsyncGetHostByName'; 104 = 'WSAAsyncGetProtoByNumber'
    105 = 'WSAAsyncGetProtoByName'; 106 = 'WSAAsyncGetServByPort'; 107 = 'WSAAsyncGetServByName'
    108 = 'WSACancelAsyncRequest'; 109 = 'WSASetBlockingHook'; 110 = 'WSAUnhookBlockingHook'; 111 = 'WSAGetLastError'
    112 = 'WSASetLastError'; 113 = 'WSACancelBlockingCall'; 114 = 'WSAIsBlocking'; 115 = 'WSAStartup'
    116 = 'WSACleanup'; 151 = '__WSAFDIsSet'
}

function Test-Allowed( [string]$RelativePath )
{
    return $AllowList -contains $RelativePath
}

# Imported functions per DLL from "dumpbin /imports" (normal and delay-load sections).
function Get-ImportedFunctions( [string]$Dumpbin, [string]$Path )
{
    $result  = @{}
    $current = $null

    foreach( $line in & $Dumpbin /nologo /imports $Path )
    {
        $text = $line.Trim()

        if( $text -match '^[\w.-]+\.dll$' )
        {
            $current = $text.ToLowerInvariant()

            if( -not $result.ContainsKey( $current ) )
            {
                $result[$current] = New-Object System.Collections.Generic.List[string]
            }
        }
        elseif( $text -eq 'Summary' )
        {
            $current = $null
        }
        elseif( $current -and $text -match '^Ordinal\s+(\d+)$' )
        {
            $ordinal = [int]$Matches[1]
            $name    = "ordinal $ordinal"

            if( $current -match '^(ws2_32|wsock32)\.dll$' -and $winsockOrdinals.ContainsKey( $ordinal ) )
            {
                $name = "$( $winsockOrdinals[$ordinal] ) (ordinal $ordinal)"
            }

            $result[$current].Add( $name )
        }
        elseif( $current -and $text -match '^[0-9A-Fa-f]+\s+(\w+)$' -and $Matches[1] -ne 'Characteristics' )
        {
            $result[$current].Add( $Matches[1] )
        }
    }

    return $result
}

$selfMarker = '(is a network library)'

# Markdown code spans, comma-separated.
function ConvertTo-CodeList( [string[]]$Items )
{
    $tick = [string][char]96
    return ( @( $Items | ForEach-Object { $tick + $_ + $tick } ) -join ', ' )
}

function Add-AllowedImport( [string]$Library, [string]$NetworkDll, [string[]]$Apis, [string]$Folder )
{
    $key = "$Library|$NetworkDll".ToLowerInvariant()

    if( -not $allowed.ContainsKey( $key ) )
    {
        $allowed[$key] = [pscustomobject]@{
            Library    = $Library
            NetworkDll = $NetworkDll
            Apis       = New-Object System.Collections.Generic.SortedSet[string]
            Folders    = New-Object System.Collections.Generic.SortedSet[string]
        }
    }

    foreach( $api in $Apis )
    {
        [void]$allowed[$key].Apis.Add( $api )
    }

    [void]$allowed[$key].Folders.Add( $Folder )
}

# 1. Sources.
$cppFiles = @( Get-AppSourceFiles -Root $Root -Extensions @( '.h', '.hpp', '.cpp', '.cxx', '.cc', '.c', '.inl', '.ipp' ) |
               Where-Object { ( Get-AppRelativePath $Root $_.FullName ) -match '^src/' } )

foreach( $file in $cppFiles )
{
    $rel = Get-AppRelativePath $Root $file.FullName

    if( Test-Allowed $rel )
    {
        continue
    }

    $lineNo = 0

    foreach( $line in [IO.File]::ReadAllLines( $file.FullName ) )
    {
        $lineNo++

        if( $line -match $networkHeaders )
        {
            $violations.Add( "${rel}:${lineNo}: network header '$($Matches[1])'" )
        }

        if( $line -match $networkCalls )
        {
            $violations.Add( "${rel}:${lineNo}: network API call '$($Matches[1])'" )
        }

        if( $line -match '#\s*pragma\s+comment\s*\(\s*lib' -and $line -match $networkLibs )
        {
            $violations.Add( "${rel}:${lineNo}: links network library '$($Matches[0])' via #pragma comment(lib)" )
        }
    }
}

# 2. Link inputs in project and property files.
$buildFiles = @( Get-AppSourceFiles -Root $Root -Extensions @( '.vcxproj', '.props', '.targets' ) )

foreach( $file in $buildFiles )
{
    $rel    = Get-AppRelativePath $Root $file.FullName
    $lineNo = 0

    if( Test-Allowed $rel )
    {
        continue
    }

    foreach( $line in [IO.File]::ReadAllLines( $file.FullName ) )
    {
        $lineNo++

        if( $line -match $networkLibs )
        {
            $violations.Add( "${rel}:${lineNo}: link input '$($Matches[0])' is a network library" )
        }
    }
}

# 3. vcpkg manifest.
$manifestPath = Join-Path $Root 'vcpkg.json'

if( Test-Path $manifestPath )
{
    $manifest     = Get-Content -Raw $manifestPath | ConvertFrom-Json
    $dependencies = New-Object System.Collections.Generic.List[object]

    if( $manifest.PSObject.Properties.Name -contains 'dependencies' )
    {
        $manifest.dependencies | ForEach-Object { $dependencies.Add( $_ ) }
    }

    if( $manifest.PSObject.Properties.Name -contains 'features' )
    {
        foreach( $feature in $manifest.features.PSObject.Properties )
        {
            if( $feature.Value.PSObject.Properties.Name -contains 'dependencies' )
            {
                $feature.Value.dependencies | ForEach-Object { $dependencies.Add( $_ ) }
            }
        }
    }

    foreach( $dependency in $dependencies )
    {
        $name     = $dependency
        $features = @()
        $defaults = $true

        if( $dependency -isnot [string] )
        {
            $name = $dependency.name

            if( $dependency.PSObject.Properties.Name -contains 'features' )
            {
                $features = @( $dependency.features | ForEach-Object { if( $_ -is [string] ) { $_ } else { $_.name } } )
            }

            if( $dependency.PSObject.Properties.Name -contains 'default-features' )
            {
                $defaults = [bool]$dependency.'default-features'
            }
        }

        if( $networkPorts -contains $name )
        {
            $violations.Add( "vcpkg.json: dependency '$name' is a networking library" )
        }

        if( $name -eq 'qtbase' )
        {
            if( $defaults )
            {
                $violations.Add( "vcpkg.json: qtbase must use ""default-features"": false (its defaults include the Network module)" )
            }

            foreach( $feature in $features )
            {
                if( $qtbaseNetwork -contains $feature )
                {
                    $violations.Add( "vcpkg.json: qtbase feature '$feature' enables networking" )
                }
            }
        }
    }
}

# 4. Built binaries.
# "powershell -File" passes a comma list as one string.
$BinaryDir    = @( $BinaryDir | ForEach-Object { $_ -split ',' } | Where-Object { $_ } )
$binaryCount  = 0
$libraryCount = 0
$scannedDirs  = New-Object System.Collections.Generic.List[string]

if( $BinaryDir.Count -gt 0 )
{
    $dumpbin  = Get-AppDumpbinPath
    $projects = @( Get-AppSourceFiles -Root $Root -Extensions @( '.vcxproj' ) | ForEach-Object { $_.BaseName } )
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

    $scannedDirs.Add( ( Get-AppRelativePath $Root $dir ) )

    foreach( $binary in Get-ChildItem -Path $dir -Recurse -File -Include *.exe, *.dll )
    {
        $isOwn   = ( $binary.Extension -eq '.exe' ) -and ( $projects -contains $binary.BaseName )
        $folder  = Get-AppRelativePath $Root $binary.DirectoryName
        $imports = @( & $dumpbin /nologo /dependents $binary.FullName |
                      ForEach-Object { $_.Trim() } |
                      Where-Object { $_ -match $networkDll } | Sort-Object -Unique )

        if( $isOwn )
        {
            $binaryCount++

            foreach( $import in $imports )
            {
                $violations.Add( "$( Get-AppRelativePath $Root $binary.FullName ): APP-built executable imports network DLL '$import'" )
            }

            continue
        }

        $libraryCount++

        if( $binary.Name -match $networkDll )
        {
            Add-AllowedImport $binary.Name $selfMarker @() $folder
        }

        if( $imports.Count -gt 0 )
        {
            $functions = Get-ImportedFunctions $dumpbin $binary.FullName

            foreach( $import in $imports )
            {
                $apis = @()

                if( $functions.ContainsKey( $import.ToLowerInvariant() ) )
                {
                    $apis = @( $functions[$import.ToLowerInvariant()] )
                }

                Add-AllowedImport $binary.Name $import $apis $folder
            }
        }
    }
}

# Summary of allowed third-party network imports (ADR 0026): printed on every run, optionally written as Markdown.
$entries = @( $allowed.Values | Sort-Object Library, NetworkDll )

if( $BinaryDir.Count -eq 0 )
{
    Write-Host 'check-network-apis: third-party network imports: not scanned (no -BinaryDir)'
}
else
{
    Write-Host "check-network-apis: allowed third-party network imports in $( $scannedDirs -join ', ' ): $( $entries.Count )"

    foreach( $entry in $entries )
    {
        $apis = 'no named imports'

        if( $entry.Apis.Count -gt 0 )
        {
            $apis = @( $entry.Apis ) -join ', '
        }

        if( $entry.NetworkDll -eq $selfMarker )
        {
            $apis = 'deployed network library'
        }

        Write-Host "  allowed: $( $entry.Library ) -> $( $entry.NetworkDll ): $apis [$( @( $entry.Folders ) -join ', ' )]"
    }
}

if( $SummaryPath )
{
    if( -not [IO.Path]::IsPathRooted( $SummaryPath ) )
    {
        $SummaryPath = Join-Path $Root $SummaryPath
    }

    $tick    = [string][char]96
    $dirList = ConvertTo-CodeList @( $scannedDirs )
    $regen   = $tick + 'scripts\check-network-apis.ps1 -BinaryDir ' + ( $scannedDirs -join ',' ) + ' -SummaryPath docs\network-imports.md' + $tick

    $md = New-Object System.Collections.Generic.List[string]
    $md.Add( '# Network imports of third-party libraries {#network-imports}' )
    $md.Add( '' )
    $md.Add( 'Generated by `scripts/check-network-apis.ps1` on ' + ( Get-Date -Format 'yyyy-MM-dd' ) + ' from the build output in ' + $dirList + '.' )
    $md.Add( 'Regenerate it with ' + $regen + '.' )
    $md.Add( '' )
    $md.Add( 'P-03 as decided by the owner (ADR 0026): APP needs no network support. Network headers and API calls in' )
    $md.Add( "APP's own code in ``src/``, network link inputs in APP's projects, network ports or ``qtbase`` network features in" )
    $md.Add( '`vcpkg.json` and network imports in APP-built executables fail the check. A socket or network library that a' )
    $md.Add( 'third-party library imports is allowed: it is not a failure, and every such import is listed below.' )
    $md.Add( '' )

    if( $entries.Count -eq 0 )
    {
        $md.Add( 'No third-party library imports a network library.' )
    }
    else
    {
        $md.Add( '| Library | Network DLL | Imported APIs | Found in |' )
        $md.Add( '|---|---|---|---|' )

        foreach( $entry in $entries )
        {
            $apis = 'no named imports'
            $dll  = ConvertTo-CodeList @( $entry.NetworkDll )

            if( $entry.Apis.Count -gt 0 )
            {
                $apis = ConvertTo-CodeList @( $entry.Apis )
            }

            if( $entry.NetworkDll -eq $selfMarker )
            {
                $apis = 'deployed network library'
                $dll  = '(itself)'
            }

            $md.Add( '| ' + ( ConvertTo-CodeList @( $entry.Library ) ) + ' | ' + $dll + ' | ' + $apis + ' | ' + ( ConvertTo-CodeList @( $entry.Folders ) ) + ' |' )
        }
    }

    [IO.File]::WriteAllText( $SummaryPath, ( ( $md -join "`n" ) + "`n" ), ( New-Object System.Text.UTF8Encoding( $false ) ) )
    Write-Host "check-network-apis: summary written to $( Get-AppRelativePath $Root $SummaryPath )"
}

if( $violations.Count -gt 0 )
{
    Write-Host "check-network-apis: FAILED with $( $violations.Count ) violation(s):" -ForegroundColor Red
    $violations | ForEach-Object { Write-Host "  $_" }
    exit 1
}

$summary = "$( $cppFiles.Count ) source files, $( $buildFiles.Count ) build files, vcpkg.json"

if( $BinaryDir.Count -gt 0 )
{
    $summary += ", $binaryCount executables, $libraryCount third-party binaries, allowed third-party network imports: $( $entries.Count )"
}

Write-Host "check-network-apis: OK ($summary; allow-list: $( $AllowList.Count ) entries)" -ForegroundColor Green
exit 0
