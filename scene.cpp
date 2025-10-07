#include "scene.h"
#include "framebuffer.h"
#include "ppc.h"
#include "v3.h"
#include "shading.h"
#include "raster.h"
Scene* g_scene = nullptr;


Scene::Scene() {
    // Create default buffers & cameras
    fb = new FrameBuffer(50, 50, 800, 600);
    shadowFB = new FrameBuffer(900, 50, 512, 512);
    cam = new PPC(800, 600, 60.0f);
    lightCam = new PPC(512, 512, 60.0f);
}

Scene::~Scene() {
    delete fb; delete shadowFB; delete cam; delete lightCam;
}

void Scene::DBG() {
    // Place any one-off debug draws or toggles here
}

void Scene::NewButton() {
    // Hook to do something when a UI button is pressed
}

V3 Scene::ShadeWithShadow(const V3& Pw, const V3& Nw, const V3& baseColor) {
    // project Pw into light camera to fetch shadow depth
    float lu, lv, lz;
    V3 L = lightPos - Pw;
    V3 Vv = (cam->C - Pw);

    V3 ka(0.05f, 0.05f, 0.05f), kd = baseColor, ks(0.2f, 0.2f, 0.2f);
    float shin = 32.0f;

    bool lit = true;
    if (lightCam->Project(Pw, lu, lv, lz)) {
        int iu = (int)std::floor(lu), iv = (int)std::floor(lv);
        if (shadowFB->InBounds(iu, iv)) {
            float stored = shadowFB->GetZ(iu, iv);
            lit = (lz <= stored + shadowBias);
        }
    }
    V3 lightTerm = lit ? Phong(Nw, L, Vv, ka, kd, ks, shin) : ka * 0.5f;
    return lightTerm;
}

void Scene::ShadowPass() {
    // Clear depth to +inf and color to black
    shadowFB->Clear(0xFF000000u, 1e30f);

    // TODO: draw scene geometry into shadowFB Z only (no color)
    // Example: Raster::DrawTri(...) with colors ignored or fixed
}

void Scene::CameraPass() {
    fb->Clear(0xFF101010u, 1e30f);

    // TODO: for each triangle:
    //  - compute Pw and Nw
    //  - shade color = ShadeWithShadow(Pw, Nw, baseColor)
    //  - rasterize with Raster::DrawTri(fb, cam, P0,C0,...)
}

void Scene::Render() {
    ShadowPass();
    CameraPass();
    fb->redraw();
    if (showShadowMap) shadowFB->redraw();
}
