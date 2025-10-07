#include "m33.h"
#include <cmath>

M33 M33::Identity() { return M33(V3(1, 0, 0), V3(0, 1, 0), V3(0, 0, 1)); }

M33 M33::RotX(float a) {
    float c = std::cos(a), s = std::sin(a);
    return M33(V3(1, 0, 0), V3(0, c, s), V3(0, -s, c));
}
M33 M33::RotY(float a) {
    float c = std::cos(a), s = std::sin(a);
    return M33(V3(c, 0, -s), V3(0, 1, 0), V3(s, 0, c));
}
M33 M33::RotZ(float a) {
    float c = std::cos(a), s = std::sin(a);
    return M33(V3(c, s, 0), V3(-s, c, 0), V3(0, 0, 1));
}

V3 M33::operator*(const V3& v) const {
    // columns form: M v = c0*v.x + c1*v.y + c2*v.z
    return cols[0] * v[0] + cols[1] * v[1] + cols[2] * v[2];
}

M33 M33::operator*(const M33& m) const {
    // this * m
    V3 c0 = (*this) * m[0];
    V3 c1 = (*this) * m[1];
    V3 c2 = (*this) * m[2];
    return M33(c0, c1, c2);
}

M33 M33::Transposed() const {
    // rows become columns
    V3 r0(cols[0][0], cols[1][0], cols[2][0]);
    V3 r1(cols[0][1], cols[1][1], cols[2][1]);
    V3 r2(cols[0][2], cols[1][2], cols[2][2]);
    return M33(r0, r1, r2);
}

float M33::Det() const {
    const V3& a = cols[0];
    const V3& b = cols[1];
    const V3& c = cols[2];
    return a[0] * (b[1] * c[2] - b[2] * c[1]) - a[1] * (b[0] * c[2] - b[2] * c[0]) + a[2] * (b[0] * c[1] - b[1] * c[0]);
}

M33 M33::Inverted() const {
    // adjugate / det
    const V3& a = cols[0], b = cols[1], c = cols[2];
    V3 r0(b[1] * c[2] - b[2] * c[1], a[2] * c[1] - a[1] * c[2], a[1] * b[2] - a[2] * b[1]);
    V3 r1(b[2] * c[0] - b[0] * c[2], a[0] * c[2] - a[2] * c[0], a[2] * b[0] - a[0] * b[2]);
    V3 r2(b[0] * c[1] - b[1] * c[0], a[1] * c[0] - a[0] * c[1], a[0] * b[1] - a[1] * b[0]);
    M33 adj(r0, r1, r2);
    float d = a[0] * r0[0] + a[1] * r1[0] + a[2] * r2[0];
    return (1.0f / d) * adj.Transposed();
}
