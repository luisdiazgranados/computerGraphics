#pragma once

#include <FL/Fl.H>
#include <FL/Fl_Gl_Window.H>
#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/glut.h>
#endif

#include "v3.h"
#include "ppc.h"

class AABB;

class FrameBuffer : public Fl_Gl_Window {
public:
	unsigned int *pix;
	int ishw, needInitHW;
	float *zb;
	int w, h;
	FrameBuffer(int u0, int v0, int _w, int _h);
	void draw();
	int handle(int guievent);
	void LoadTiff(char* fname);
	void SaveAsTiff(char* fname);
	void KeyboardHandle();

	void Set(unsigned int color);
	void SetZB(float z0);
	void SetSafe(int u, int v, unsigned int color);
	void SetSafeZB(int u, int v, float z);
	void Set(int u, int v, unsigned int color);
	void SetZB(int u, int v, float z);
	unsigned int Get(int u, int v);
	float GetZB(int u, int v);
	void SetChecker(int cw, unsigned int col0, unsigned int col1);
	void DrawPoint2D(V3 p, int psize, unsigned int color);
	void DrawPoint3D(V3 p, PPC *ppc, int psize, unsigned int color);
	void Draw3DSegment(unsigned int color, PPC *ppc, V3 V0, V3 V1);
	void Draw2DSegment(unsigned int color, V3 pV0, V3 pV1);
	void Draw3DSegment(V3 C0, V3 C1, PPC *ppc, V3 V0, V3 V1);
	void Draw2DSegment(V3 C0, V3 C1, V3 pV0, V3 pV1);
	void DrawFBPointCloud(FrameBuffer *pcfb, PPC *pcppc, PPC *ppc);
	int IsFarther(int u, int v, float z);

	// New drawing primitives
	void DrawRect(int u, int v, int width, int height, unsigned int color);
	void DrawCircle(int u, int v, int radius, unsigned int color);
	void DrawLine(int u0, int v0, int u1, int v1, unsigned int color);
	void DrawLine(int u0, int v0, int u1, int v1, V3 c0, V3 c1);
	void DrawTriangle2D(int u0, int v0, int u1, int v1, int u2, int v2, unsigned int color);

	// Font rendering
	void DrawChar(int u, int v, int scale, char ch, unsigned int color);
	void DrawString(int u, int v, int scale, const char* str, unsigned int color);

	// Color queries
	V3 GetColorV3(int u, int v);
	V3 GetColor(float x, float y, bool repeat = false, bool bilinear = false);
	V3 GetColorBilinear(float x, float y);

	// Z-buffer visualization
	void DrawZBuffer();
	void DrawZBuffer(FrameBuffer *other);

	// Copy
	void Copy(FrameBuffer *other);

	// Camera and AABB visualization
	void DrawCamera(PPC *viewPPC, PPC *drawnPPC);
	void DrawAABB(PPC *ppc, AABB aabb, unsigned int color);
};
