#pragma once
#include <cassert>
#include "v3.h"

// 3x3 matrix with column-major storage: M = [ c0 | c1 | c2 ]
class M33 {
public:
    V3 cols[3];

    M33() : cols{ V3(), V3(), V3() } {}
    M33(const V3& c0, const V3& c1, const V3& c2) : cols{ c0, c1, c2 } {}

    // column access
    inline       V3& operator[](int i) { assert(0 <= i && i < 3); return cols[i]; }
    inline const V3& operator[](int i) const { assert(0 <= i && i < 3); return cols[i]; }

    // factories
    static M33 Identity();
    static M33 RotX(float a);
    static M33 RotY(float a);
    static M33 RotZ(float a);

    // math
    V3  operator*(const V3& v) const;   // (3x3) * (3x1) -> (3x1)
    M33 operator*(const M33& m) const;  // (3x3) * (3x3) -> (3x3)

    // scalar scale (matrix * scalar)
    M33 operator*(float s) const { return M33(cols[0] * s, cols[1] * s, cols[2] * s); }
    M33& operator*=(float s) { cols[0] *= s; cols[1] *= s; cols[2] *= s; return *this; }

    // transforms
    M33  Transposed() const;
    float Det() const;
    M33  Inverted() const;
};

// scalar * matrix (needed for: (1.0f/d) * adj.Transposed())
inline M33 operator*(float s, const M33& M) { return M * s; }

