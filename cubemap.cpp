#include "cubemap.h"
#include "framebuffer.h"

#include <iostream>

CubeMap::CubeMap() : lookupFace(0) {
	for (int i = 0; i < N; i++)
		buffers[i] = 0;
}

CubeMap::CubeMap(const char* sides[6]) : lookupFace(0) {
	for (int i = 0; i < N; i++) {
		// Create offscreen framebuffers (hidden FLTK windows)
		buffers[i] = new FrameBuffer(0, 0, 1, 1);
		buffers[i]->hide();
		buffers[i]->LoadTiff((char*)sides[i]);
		cameras[i] = PPC(90.0f, buffers[i]->w, buffers[i]->h);
	}

	// camera 0 - forwards (default)
	// camera 1 - left
	cameras[1].RotateAroundDirection(V3(0.0f, 1.0f, 0.0f), 90.0f);
	// camera 2 - back
	cameras[2].RotateAroundDirection(V3(0.0f, 1.0f, 0.0f), 180.0f);
	// camera 3 - right
	cameras[3].RotateAroundDirection(V3(0.0f, 1.0f, 0.0f), 270.0f);
	// camera 4 - up
	cameras[4].RotateAroundDirection(V3(1.0f, 0.0f, 0.0f), 90.0f);
	// camera 5 - down
	cameras[5].RotateAroundDirection(V3(1.0f, 0.0f, 0.0f), -90.0f);
}

CubeMap::~CubeMap() {
	for (int i = 0; i < N; i++) {
		delete buffers[i];
		buffers[i] = 0;
	}
}

V3 CubeMap::Lookup(V3 direction) {
	V3 PP;

	for (int _i = 0; _i < N; _i++) {
		int i = (_i + lookupFace) % N;

		if (cameras[i].Project(cameras[0].C - direction, PP) &&
			PP[0] >= 0.0f && PP[1] >= 0.0f &&
			PP[0] < buffers[i]->w && PP[1] < buffers[i]->h) {
			lookupFace = i;
			return buffers[i]->GetColorBilinear(PP[0], PP[1]);
		}
	}

	return V3(0.5f, 0.0f, 1.0f); // fallback color
}

int CubeMap::LookupFaceIndex(V3 direction) {
	V3 PP;
	for (int _i = 0; _i < N; _i++) {
		int i = (_i + lookupFace) % N;
		if (cameras[i].Project(cameras[0].C - direction, PP) &&
			PP[0] >= 0.0f && PP[1] >= 0.0f &&
			PP[0] < buffers[i]->w && PP[1] < buffers[i]->h) {
			lookupFace = i;
			return i;
		}
	}
	return -1;
}
