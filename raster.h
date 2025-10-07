#pragma once
class FrameBuffer; class PPC; class V3;

namespace Raster {
    // Solid-colored triangle with per-vertex colors (ABGR packed in V3 as rgb? or pass as V3 color).
    // Here: pass colors as V3 (r,g,b in [0,1]), we’ll pack to ABGR.
    void DrawTri(FrameBuffer* fb, const PPC* cam,
        const V3& P0, const V3& C0,
        const V3& P1, const V3& C1,
        const V3& P2, const V3& C2);
}

