# ADR 0013: OpenGL 4.5 behind `render::Device` (D-12) {#adr-0013}

- **Status:** Accepted (spec decision D-12)
- **Date:** 2026-11-11

## Context

Spec D-12 chooses OpenGL 4.5 in Stage 0 behind a `render::Device` interface, as the fastest route to a working
viewport with no lock-in; a Vulkan or Direct3D 12 backend may be added later. Phase 1 (C-09, ADR 0023) keeps `render`
free of Qt so it builds in the Headless configuration.

## Decision

- `src/render/` defines `render::Device` and an OpenGL 4.5 implementation. It never includes Qt headers.
- `ui` creates the window and hands `render` a current OpenGL context. Headless rendering (snapshot tests, CLI image
  export) uses an off-screen context created by `platform` (step S0.11.6).
- Shaders, picking, edge rendering and section caps live in `render`; no OpenGL calls outside it.

## Licence verification (S0.01.6, 2026-11-12)

Checked against the vcpkg port metadata (`ports/<port>/vcpkg.json`) and portfile at the pinned baseline
`177e8175` (read from vcpkg's registry cache), the upstream licence file (GitHub licence API) and, for installed
ports, the installed `share/<port>/copyright` file. Spec section 4.3 is the yardstick.

OpenGL itself is the Windows system library `opengl32` (vcpkg `opengl` port, Windows SDK licence) plus the
Khronos headers (`opengl-registry`, MIT per file; `egl-registry`, Apache-2.0), all allowed. Open item for
S0.12.1: the GL function loader is chosen in its own ADR; candidates at the baseline are `glad` 0.1.36 (MIT),
`libepoxy` 1.5.10#2 (MIT) and `glbinding` 3.5.0 (MIT).

## Consequences

- Snapshot tests (T-06) run without a visible window.
- P-11 (5 million triangles at 60 fps) is measured from S3 (SP-8a) and met in S10 with instancing and LOD.

## Alternatives considered

- Vulkan or Direct3D 12 first: much more code before the first pixel.
- Qt RHI / Qt 3D: would bind `render` to Qt (ADR 0023).

## Related requirements

D-12, P-11, U-08, U-09, T-06; ADR 0023; steps S0.11.6, S0.12.1 … S0.12.5, S10.02.4, S10.02.6.
