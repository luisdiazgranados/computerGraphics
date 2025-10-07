#include "raster.h"
#include "framebuffer.h"
#include "ppc.h"
#include "v3.h"
#include <algorithm>
#include <cstdint>

static inline uint32_t packABGR(const V3& c) {
    auto sat = [](float x) { x = std::max(0.f, std::min(1.f, x)); return (unsigned)(x * 255.0f + 0.5f); };
    unsigned r = sat(c[0]), g = sat(c[1]), b = sat(c[2]);
    return (0xFFu << 24) | (b << 16) | (g << 8) | (r);
}

namespace Raster {

    void DrawTri(FrameBuffer* fb, const PPC* cam,
        const V3& P0, const V3& C0,
        const V3& P1, const V3& C1,
        const V3& P2, const V3& C2) {

        float u0, v0, z0, u1, v1, z1, u2, v2, z2;
        if (!cam->Project(P0, u0, v0, z0)) return;
        if (!cam->Project(P1, u1, v1, z1)) return;
        if (!cam->Project(P2, u2, v2, z2)) return;

        // Bounding box
        int minU = (int)std::floor(std::min({ u0,u1,u2 }));
        int maxU = (int)std::ceil(std::max({ u0,u1,u2 }));
        int minV = (int)std::floor(std::min({ v0,v1,v2 }));
        int maxV = (int)std::ceil(std::max({ v0,v1,v2 }));

        // Edge function setup
        auto edge = [](float ax, float ay, float bx, float by, float px, float py) {
            return (py - ay) * (bx - ax) - (px - ax) * (by - ay);
            };

        float A = edge(u0, v0, u1, v1, u2, v2); // triangle area * 2
        if (A == 0) return;
        const float invA = 1.0f / A;

        for (int v = minV; v <= maxV; ++v) {
            for (int u = minU; u <= maxU; ++u) {
                if (!fb->InBounds(u, v)) continue;
                float w0 = edge(u1, v1, u2, v2, (float)u + 0.5f, (float)v + 0.5f) * invA;
                float w1 = edge(u2, v2, u0, v0, (float)u + 0.5f, (float)v + 0.5f) * invA;
                float w2 = 1.0f - w0 - w1;
                if (w0 < 0 || w1 < 0 || w2 < 0) continue;

                // perspective-correct-ish z (simple linear here; replace if needed)
                float z = w0 * z0 + w1 * z1 + w2 * z2;
                if (fb->IsFarther(u, v, z)) continue;
                fb->SetZ(u, v, z);

                V3 C = C0 * w0 + C1 * w1 + C2 * w2;
                fb->Set(u, v, packABGR(C));
            }
        }
    }

} // namespace Raster
