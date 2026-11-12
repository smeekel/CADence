# ADR 0014: Embree and an in-house path tracer (D-13) {#adr-0014}

- **Status:** Accepted (spec decision D-13)
- **Date:** 2026-11-11

## Context

Spec D-13 chooses Intel Embree (Apache-2.0) and an in-house progressive path tracer for photorealistic rendering
(M-VIS-01, stage S10). It is CPU-only, so it runs on any machine offline.

## Decision

- Embree is added through vcpkg in step S10.04.3 and used only in `src/render/pathtracer/`. The rule 030 table and
  the `$ThirdParty` table in `scripts/check-layering.ps1` allow it only in `render`.
- The path tracer uses the same metallic-roughness appearances as the viewport (S10.04.1). A denoiser is optional and
  sits behind a seam; any denoiser library needs its own ADR and licence check.
- Rendering runs as a cancellable job (A-10); output PNG and EXR up to 8K.

## Licence verification (S0.01.6, 2026-11-12)

Checked against the vcpkg port metadata (`ports/<port>/vcpkg.json`) and portfile at the pinned baseline
`177e8175` (read from vcpkg's registry cache), the upstream licence file (GitHub licence API) and, for installed
ports, the installed `share/<port>/copyright` file. Spec section 4.3 is the yardstick.

`embree` 4.4.1 at the baseline: the port metadata has no licence field; the portfile installs upstream
`LICENSE.txt`, and GitHub `RenderKit/embree` reports Apache-2.0. Allowed (4.3: Apache-2.0 is compatible with
GPL-3.0). Its threading dependency (oneTBB, Apache-2.0) is verified and both are pinned when Embree is added in S10.

## Consequences

- No GPU requirement for photoreal output; preview time target (M-VIS-01b, under 60 s at 1920×1080) is measured by
  `Bench.RenderPreview1080p`.

## Alternatives considered

- GPU path tracing (OptiX, Vulkan RT): vendor or driver dependent.
- Third-party renderers (Cycles, LuxCore): heavy integration, licence review needed.

## Related requirements

D-13, M-VIS-01, A-10; steps S10.04.1 … S10.04.6.
