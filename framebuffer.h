#pragma once
#include <cstdint>
#include <vector>

// Make sure FLTK GL window is known
#include <FL/Fl_Gl_Window.H>

#include "v3.h"

class Framebuffer : public Fl_Gl_Window {
public:
    int w = 0, h = 0;
    std::vector<uint32_t> color; // 0xAARRGGBB
    std::vector<float>    zbuf;

    Framebuffer(int W, int H)
        : Fl_Gl_Window(W, H), w(W), h(H),
        color(static_cast<size_t>(W)* H, 0xFF000000u),
        zbuf(static_cast<size_t>(W)* H, 1e30f) {
    }

    // FLTK draw hook
    void draw() override;

    // framebuffer ops
    void Clear(uint32_t rgba, float z);
    void Set(int u, int v, uint32_t rgba);
    void Set(int u, int v, const V3& rgb); // rgb in [0,1]
    void SetZ(int u, int v, float z);
    float GetZ(int u, int v) const;
    bool IsFarther(int u, int v, float z) const;

    // I/O (safe stubs so you can compile)
    void SaveAsTiff(const char* path) const;
    bool LoadTiff(const char* path);

    // GL blit
    void GLDrawPixels() const;
};
