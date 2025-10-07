#pragma once
#include <cmath>
#include <cassert>
#include <iostream>

class V3 {
public:
    float xyz[3];

    V3() : xyz{ 0,0,0 } {}
    V3(float x, float y, float z) : xyz{ x,y,z } {}

    inline float& operator[](int i) { assert(0 <= i && i < 3); return xyz[i]; }
    inline float  operator[](int i) const { assert(0 <= i && i < 3); return xyz[i]; }

    // arithmetic
    V3 operator+(const V3& b) const { return V3(xyz[0] + b[0], xyz[1] + b[1], xyz[2] + b[2]); }
    V3 operator-(const V3& b) const { return V3(xyz[0] - b[0], xyz[1] - b[1], xyz[2] - b[2]); }
    V3 operator*(float s)     const { return V3(xyz[0] * s, xyz[1] * s, xyz[2] * s); }
    V3 operator/(float s)     const { return V3(xyz[0] / s, xyz[1] / s, xyz[2] / s); }

    V3& operator+=(const V3& b) { xyz[0] += b[0]; xyz[1] += b[1]; xyz[2] += b[2]; return *this; }
    V3& operator-=(const V3& b) { xyz[0] -= b[0]; xyz[1] -= b[1]; xyz[2] -= b[2]; return *this; }
    V3& operator*=(float s) { xyz[0] *= s; xyz[1] *= s; xyz[2] *= s; return *this; }

    // dot & cross
    float dot(const V3& b) const { return xyz[0] * b[0] + xyz[1] * b[1] + xyz[2] * b[2]; }
    V3    cross(const V3& b) const {
        return V3(
            xyz[1] * b[2] - xyz[2] * b[1],
            xyz[2] * b[0] - xyz[0] * b[2],
            xyz[0] * b[1] - xyz[1] * b[0]
        );
    }

    float Length() const { return std::sqrt(dot(*this)); }
    V3    Normalized() const { float l = Length(); return l > 0 ? (*this) / l : V3(); }
    void  Normalize() { float l = Length(); if (l > 0) { xyz[0] /= l; xyz[1] /= l; xyz[2] /= l; } }

    // Rotation of vector v around unit axis u by angle a (radians)
    static V3 RotateAroundAxis(const V3& v, const V3& u_unit, float a);
};

inline V3 operator*(float s, const V3& v) { return v * s; }
inline std::ostream& operator<<(std::ostream& os, const V3& v) {
    return os << "(" << v[0] << "," << v[1] << "," << v[2] << ")";
}
