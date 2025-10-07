#include "framebuffer.h"

// Use your local GL headers; include Windows/FL first if needed elsewhere
#include "GL.h"
#include "GLU.h"

#include <algorithm>
#include <cstring>

#ifndef GL_BGRA
#define GL_BGRA 0x80E1
#endif

static inline uint32_t packRGBA(const V3& c) {
    auto clamp01 = [](float x) { return x < 0.f ? 0.f : (x > 1.f ? 1.f : x); };
    uint8_t r = static_cast<uint8_t>(255.f * clamp01(c[0]) + 0.5f);
    uint8_t g = static_cast<uint8_t>(255.f * clamp01(c[1]) + 0.5f);
    uint8_t b = static_cast<uint8_t>(255.f * clamp01(c[2]) + 0.5f);
    return (0xFFu << 24) | (r << 16) | (g << 8) | b; // A R G B
}

void Framebuffer::Clear(uint32_t rgba, float z) {
    std::fill(color.begin(), color.end(), rgba);
    std::fill(zbuf.begin(), zbuf.end(), z);
}

void Framebuffer::Set(int u, int v, uint32_t rgba) {
    if (u < 0 || v < 0 || u >= w || v >= h) return;
    color[static_cast<size_t>(v) * w + u] = rgba;
}

void Framebuffer::Set(int u, int v, const V3& rgb) {
    Set(u, v, packRGBA(rgb));
}

void Framebuffer::SetZ(int u, int v, float z) {
    if (u < 0 || v < 0 || u >= w || v >= h) return;
    zbuf[static_cast<size_t>(v) * w + u] = z;
}

float Framebuffer::GetZ(int u, int v) const {
    if (u < 0 || v < 0 || u >= w || v >= h) return 1e30f;
    return zbuf[static_cast<size_t>(v) * w + u];
}

bool Framebuffer::IsFarther(int u, int v, float z) const {
    return z > GetZ(u, v);
}

void Framebuffer::GLDrawPixels() const {
    // Draw the CPU buffer to the current OpenGL context
    glPixelZoom(1.f, -1.f);          // flip Y so (0,0) is top-left in window space
    glRasterPos2i(-1, 1);            // start at upper-left of NDC
    glDrawPixels(w, h, GL_BGRA, GL_UNSIGNED_BYTE, color.data());
}

void Framebuffer::draw() {
    GLDrawPixels();
}

// Simple stubs so the linker stops complaining; replace with real I/O later.
void Framebuffer::SaveAsTiff(const char* /*path*/) const {}

bool Framebuffer::LoadTiff(const char* /*path*/) {return false}
