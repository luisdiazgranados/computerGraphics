#pragma once
#include "gui.h"
#include "framebuffer.h"
#include "TM.h"
#include "CGInterface.h"
#include "GLSLShader.h"
#include <vector>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/glew.h>
#endif

class CubeMap;

class Scene {
public:

	GUI *gui;
	FrameBuffer *fb, *fb3, *hwfb;
	TM *tms;
	int tmsN;
	PPC *ppc, *ppc3;

	// Rendering
	void Render();
	void Render(PPC *usePPC, FrameBuffer *useFB);
	Scene();
	void DBG();
	void NewButton();
	void RenderHW();
	void InitHWRendering();

	// Shaders (Cg - stubbed on macOS)
	CGInterface *cgi;
	ShaderOneInterface *soi;
	float mFraction;

	// GLSL shaders
	GLSLShader *shadowShader;
	GLSLShader *projTexShader;
	int hwRenderMode; // 0=basic, 1=shadow mapping, 2=projective texture

	// Shadow map FBO
	GLuint shadowFBOHW;
	GLuint shadowDepthTexHW;
	int shadowMapSize;

	// Projective texture
	GLuint projTexHW;
	PPC *projectorPPC;

	// Light for HW shadow mapping
	PPC *hwLightPPC;
	V3 hwLightPos;

	// HW rendering methods
	void RenderHWShadowMap();
	void RenderHWProjTex();
	void SetupShadowFBO();
	void SetupProjTexture();
	void GetMVPMatrix(PPC *cam, float *mvp);

	// Camera path
	std::vector<PPC> cameraPath;
	void SaveCameraKeyframe();
	void ClearCameraPath();
	void PlayCameraPath();

	// Ray tracing
	void RayTrace();
	int IntersectWithRay(V3 O, V3 ray, V3& color, float &t, V3 &rO, V3 &rray);

	// Scene rendering techniques (software)
	void RenderMeshLighting();
	void RenderShadowMap();
	void RenderTextured();
	void RenderEnvMap();
	void RenderSoftShadow();

	// Scene resources
	CubeMap *envMap;
	FrameBuffer *shadowFB;
	PPC *lightPPC;
};

extern Scene *scene;