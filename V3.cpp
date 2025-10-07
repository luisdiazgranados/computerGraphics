#include "v3.h"

V3 V3::RotateAroundAxis(const V3& v, const V3& u_unit, float a) {
    const float c = std::cos(a), s = std::sin(a);
    const V3 u = u_unit;
    return v * c + u.cross(v) * s + u * (u.dot(v)) * (1.0f - c);
}

