#pragma once
#include "v3.h"
#include "framebuffer.h"
#include "ppc.h"

class TM {
public:
	V3 *verts;
	int vertsN;
	V3 *colors; // vertex colors in V3 format
				// (one float in [0.0f, 1.0f] per R, G, and B channel)
	V3 *normals; // per vertex normals
	unsigned int *tris; // triples of vertex indices
	int trisN;
	TM() : verts(0), vertsN(0), colors(0), tris(0), trisN(0), normals(0) {};
	void LoadBin(char *fname); // load from file
	void DrawPoints(unsigned int color, int psize, PPC *ppc,
		FrameBuffer *fb);
	void DrawWireFrame(unsigned int color, PPC *ppc, FrameBuffer *fb);
	void RotateAboutArbitraryAxis(V3 aO, V3 ad, float theta);
	V3 GetCenter(); // return the average of all vertices
	void Position(V3 newCenter);
	void Translate(V3 tv);
	void VisualizeNormals(float nl, PPC *ppc, FrameBuffer *fb);
	void LightD(V3 ld, float ka);
	void LightP(V3 L, float ka);
};