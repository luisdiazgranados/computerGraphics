#pragma once
#include "v3.h"
#include "M33.h"
#include "aabb.h"
#include "framebuffer.h"
#include "ppc.h"

class CubeMap;

class TM {
public:
	int on;
	V3 *verts, *projverts, *tcs;
	int vertsN;
	V3 *colors; // vertex colors in V3 format
				// (one float in [0.0f, 1.0f] per R, G, and B channel)
	V3 *normals; // per vertex normals
	unsigned int *tris; // triples of vertex indices
	int trisN;
	FrameBuffer *tex;
	V3 centerOfMass;

	TM() : verts(0), vertsN(0), colors(0), tris(0), trisN(0), normals(0),
		projverts(0), tcs(0), tex(0), on(1) {};
	void SetQuad(V3 *vs);
	void LoadBin(char *fname);

	// Drawing
	void DrawPoints(unsigned int color, int psize, PPC *ppc, FrameBuffer *fb);
	void DrawWireFrame(unsigned int color, PPC *ppc, FrameBuffer *fb);
	void VisualizeNormals(float nl, PPC *ppc, FrameBuffer *fb);
	void RenderFilled(PPC *ppc, FrameBuffer *fb);
	void SetEEQs(M33 pvs, M33 &eeqs);
	void RenderHW();

	// Transforms
	void RotateAboutArbitraryAxis(V3 aO, V3 ad, float theta);
	V3 GetCenter();
	void Position(V3 newCenter);
	void Translate(V3 tv);
	void TranslateTo(V3 position);
	void Scale(float factor);
	void UpdateCenterOfMass();

	// Lighting
	void LightD(V3 ld, float ka);
	void LightP(V3 L, float ka);

	// Geometry
	AABB GetAABB();
	void ProjectVertices(PPC *ppc);

	// Shape builders
	void LoadRectangle(V3 center, V3 dimensions, V3 color);
	void LoadRectangleWithNormals(V3 center, V3 dimensions, V3 color);
	void LoadPlane(V3 center, V3 dimensions, V3 color);
	void LoadAABB(AABB aabb, V3 color);
	void SetTriangle(int index, unsigned int v0, unsigned int v1, unsigned int v2);

	// Advanced rendering
	void DrawFilledPointLight(PPC *ppc, FrameBuffer *fb, V3 lightPos, float ka, float specularIntensity);
	void DrawFilledPointLightShadow(PPC *ppc, FrameBuffer *fb, V3 lightPos, float ka, float specularIntensity,
		PPC *lightPPC, FrameBuffer *shadowFB, float shadowEps = 0.3f);
	void DrawFilledSoftShadow(PPC *ppc, FrameBuffer *fb, V3 lightPos, float ka, float specularIntensity,
		PPC **lightPPCs, FrameBuffer **shadowFBs, int numSamples, float shadowEps = 0.3f);
	void DrawTextured(PPC *ppc, FrameBuffer *fb, FrameBuffer *texFB, int filterMode = 0, int tileMode = 0);
	void DrawFilledEnvMap(PPC *ppc, FrameBuffer *fb, CubeMap *map);

	// Ray tracing
	int IntersectWithRay(V3 O, V3 ray, V3 &color, float &t, V3 &rO, V3 &rray);
};
