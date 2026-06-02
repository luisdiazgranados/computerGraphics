#pragma once

#include "v3.h"
#include <cfloat>

class AABB {
public:
	V3 c0, c1; // c0 = min corner, c1 = max corner

	AABB() : c0(FLT_MAX, FLT_MAX, FLT_MAX), c1(-FLT_MAX, -FLT_MAX, -FLT_MAX) {}

	AABB(V3 v) : c0(v), c1(v) {}

	AABB(V3 _c0, V3 _c1) : c0(_c0), c1(_c1) {}

	void AddPoint(V3 p) {
		for (int i = 0; i < 3; i++) {
			if (p[i] < c0[i]) c0[i] = p[i];
			if (p[i] > c1[i]) c1[i] = p[i];
		}
	}

	V3 GetPosition() { return c0; }
	V3 GetSize() { return c1 - c0; }

	bool operator==(AABB other) {
		return c0 == other.c0 && c1 == other.c1;
	}
};
