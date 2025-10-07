#pragma once
#include "v3.h"

class FrameBuffer;
class PPC;

class Scene {
public:
    Scene();
    ~Scene();

    void Render();
    void DBG();        // debug UI hook
    void NewButton();  // another UI hook

private:
    FrameBuffer* fb = nullptr;
    FrameBuffer* shadowFB = nullptr;
    PPC* cam = nullptr;
    PPC* lightCam = nullptr;

    V3  lightPos = V3(100, 100, 100);
    bool showShadowMap = false;
    float shadowBias = 1e-3f;

    V3 ShadeWithShadow(const V3& Pw, const V3& Nw, const V3& baseColor);
    void ShadowPass();   // fills shadowFB z only
    void CameraPass();   // draws to fb using shadow test
};
