#pragma once
#include "v3.h"

// Simple Phong: inputs are normalized N, L, V; colors as V3 (0..1)
inline V3 Phong(const V3& N, const V3& L, const V3& V,
    const V3& ka, const V3& kd, const V3& ks, float shininess) {
    V3 n = N.Normalized();
    V3 l = L.Normalized();
    V3 v = V.Normalized();
    V3 r = (n * (2.0f * n.dot(l)) - l).Normalized();

    float ndotl = std::max(0.0f, n.dot(l));
    float rdotv = std::max(0.0f, r.dot(v));
    V3 color = ka + kd * ndotl + ks * std::pow(rdotv, shininess);
    return color;
}

