# ADR 0026: Network-API check scope and third-party network imports (C-12) {#adr-0026}

- **Status:** Accepted (owner decision, 2026-11-11; Phase 1 conflict C-12)
- **Date:** 2026-11-11

## Context

P-03 says the static check (T-11) fails the build if networking APIs are *linked* outside an allow-list that is empty
by default. The scaffold confirmed what Phase 1 suspected: **`Qt6Core.dll` (and `Qt6Cored.dll`) imports
`ws2_32.dll` even though Qt Network is not built** (`qtbase` uses `default-features: false` with only `gui` and
`widgets`). It imports `WSAStartup`, `WSACleanup` and `WSAAsyncSelect` by ordinal. OCCT's kernel library may do the
same. A literal reading of P-03 would fail every UI build for reasons APP cannot change.

## Decision

The owner reduced the severity of the static part of P-03. APP needs no network support. A socket or network library
that a **third-party** library imports is allowed. It is not a failure, but the check must report it.

- `scripts/check-network-apis.ps1` fails on network use by APP itself:
  1. network headers and API calls in APP's own code in `src/` (`winsock2.h`, `WSAStartup`, WinHTTP, WinINet, sockets);
  2. network libraries in APP's project and props link inputs, and `#pragma comment(lib)` (`ws2_32`, `winhttp`,
     `wininet`, `Qt6Network`, …);
  3. network ports or `qtbase` network features in `vcpkg.json` (Qt Network, curl, …), because APP would be asking for
     network support;
  4. network imports in executables that APP builds (`-BinaryDir`).
- A third-party DLL that imports a network DLL, or that is a network library itself, is **allowed**. With
  `-BinaryDir`, the check prints a summary of every such import on every run: the library, the network DLL and the
  imported APIs (WinSock ordinals are translated to names), and the folders it was found in. With `-SummaryPath` it
  writes the same summary as Markdown. The committed summary is `docs/network-imports.md`. The first entry is
  `Qt6Cored.dll` → `WS2_32.dll` (`WSAAsyncSelect`, `WSACleanup`, `WSAStartup`).
- There is no reviewed list of third-party DLLs, and a new third-party importer does not fail the check. It appears
  in the summary, and reviewers see it when `docs/network-imports.md` is regenerated.
- The APP allow-list (P-03's "allow-list, empty by default") stays empty.
- The runtime part of P-03 is unchanged: the offline smoke run (`scripts/smoke-offline.ps1`, S0.08.7) blocks
  outbound traffic and fails on errors or on any outbound connection attempt. Every stage gate runs it.
- `scripts/check-selftest.ps1` keeps one deliberate violation per failure mode, including an APP-built executable
  that imports `ws2_32.dll`. It also has one allowed case: a third-party DLL that imports `ws2_32.dll` must pass
  and appear in the summary.

## Consequences

- The check stays strict for APP code, APP's build settings and APP's dependency requests.
- The spec's "linked" wording is read as "linked by APP". Third-party imports are visible but never block the build.
- A new dependency that brings in socket imports shows up in the summary and in the diff of
  `docs/network-imports.md`. It is reviewed with the dependency's licence check (rule 070), not by the network check.

## Alternatives considered

- Fail on Qt6Core's import: makes every UI build fail; Qt cannot be built without Windows sockets.
- A named list of reviewed third-party DLLs, with unlisted importers failing (the earlier proposal): stricter than the
  owner wants for a product that needs no network support; it adds no runtime protection.
- Ignore DLL imports entirely: would hide what third-party code can reach and would miss APP-built executables that
  import network DLLs.

## Related requirements

P-03, CR-06, T-11, D-03; ADR 0004; risk R-14; steps S0.01.1, S0.02.3, S0.02.7, S0.08.7, every `Sx.99.1`.
