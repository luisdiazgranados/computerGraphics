#pragma once
#include "v3.h"
#include "m33.h"

class PPC {
public:
    int   w = 0, h = 0;
    float f = 1.0f;
    V3    C, a, b, c;

    PPC() = default;
    PPC(int W, int H, float f_) : w(W), h(H), f(f_), C(0, 0, 0), a(1, 0, 0), b(0, 1, 0), c(0, 0, -1) {}
    PPC(int w, int h, float hfov_deg);

    bool Project(const V3& P, float& u, float& v, float& z) const;

    V3   GetVD() const;
    float GetF() const;
    void  Translate(const V3& d);
    void  Pan(float ang);
    void  Tilt(float ang);
    void  Roll(float ang);
};
