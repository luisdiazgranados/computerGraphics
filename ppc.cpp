#include "ppc.h"
#include <cmath>

PPC::PPC(int W, int H, float hfov_deg) : w(W), h(H) {
    // Initialize canonical camera looking down -z with right-handed basis:
    // put image plane at distance f such that tan(hfov/2) = (w/2)/f -> f = (w/2)/tan(hfov/2)
    const float hfov = hfov_deg * (3.1415926535f / 180.0f);
    const float f = (w * 0.5f) / std::tan(hfov * 0.5f);

    C = V3(0, 0, 0);
    a = V3(1, 0, 0);
    b = V3(0, -1, 0);      // image space y down
    c = V3(-w * 0.5f, -h * 0.5f, -f); // top-left corner vector from C in camera basis
}

bool PPC::Project(const V3& P, float& u, float& v, float& z) const {
    // Solve for (u,v,1) such that C + c + a*u + b*v is collinear with (P-C)
    V3 pc = P - C;
    // Basis matrix [a b vd], where vd = a.cross(b) (camera forward)
    V3 vd = a.cross(b).Normalized();
    M33 M(a, b, vd);
    V3 q = M.Inverted() * pc;  // coordinates in camera basis -> (u, v, depth along vd)
    if (q[2] <= 0.0f) return false; // behind camera
    u = q[0] / q[2] + (-c[0]); // account for c = (-cx, -cy, -f)
    v = q[1] / q[2] + (-c[1]);
    z = q[2];
    return true;
}

V3 PPC::GetVD() const { return a.cross(b).Normalized(); }
float PPC::GetF() const { return -c[2]; }
void PPC::Translate(const V3& d) { C += d; }

void PPC::Pan(float ang) { M33 R = M33::RotY(ang); a = R * a; b = R * b; c = R * c; }
void PPC::Tilt(float ang) { M33 R = M33::RotX(ang); a = R * a; b = R * b; c = R * c; }
void PPC::Roll(float ang) { M33 R = M33::RotZ(ang); a = R * a; b = R * b; c = R * c; }
