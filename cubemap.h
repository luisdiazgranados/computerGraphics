#pragma once

#include "v3.h"
#include "ppc.h"

class FrameBuffer;

class CubeMap {
public:
	static const int N = 6;
	PPC cameras[N];
	FrameBuffer *buffers[N];
	int lookupFace;

	CubeMap();
	CubeMap(const char* sides[6]);
	~CubeMap();

	V3 Lookup(V3 direction);
	int LookupFaceIndex(V3 direction);
};
